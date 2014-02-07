/*
  laser.cpp - Laser control library for Arduino using 16 bit timers- Version 1
  Copyright (c) 2013 Timothy Schmidt.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "laser.h"
#include "Configuration.h"
#include "ConfigurationStore.h"
#include "pins.h"
#include <avr/interrupt.h>
#include <Arduino.h>
#include "Marlin.h"

laser_t laser;

void laser_setup()
{
  pinMode(LASER_FIRING_PIN, OUTPUT);

  #ifdef LASER_INTENSITY_PIN
    pinMode(LASER_INTENSITY_PIN, OUTPUT);
    analogWrite(LASER_INTENSITY_PIN, 1);  // let Arduino setup do it's thing to the PWM pin

    TCCR4B = 0x00;  // stop Timer4 clock for register updates
    TCCR4A = 0x82; // Clear OC4A on match, fast PWM mode, lower WGM4x=14
    ICR4 = labs(F_CPU / LASER_PWM); // clock cycles per PWM pulse
    OCR4A = labs(F_CPU / LASER_PWM) - 1; // ICR4 - 1 force immediate compare on next tick
    TCCR4B = 0x18 | 0x01; // upper WGM4x = 14, clock sel = prescaler, start running

    noInterrupts();
    TCCR4B &= 0xf8; // stop timer, OC4A may be active now
    TCNT4 = labs(F_CPU / LASER_PWM); // force immediate compare on next tick
    ICR4 = labs(F_CPU / LASER_PWM); // set new PWM period
    TCCR4B |= 0x01; // start the timer with proper prescaler value
    interrupts();
  #endif // LASER_INTENSITY_PIN

  #ifdef LASER_PERIPHERALS
  digitalWrite(LASER_ACC_PIN, HIGH);  // Laser accessories are active LOW, so preset the pin
  pinMode(LASER_ACC_PIN, OUTPUT);

  digitalWrite(LASER_AOK_PIN, HIGH);  // Set the AOK pin to pull-up.
  pinMode(LASER_AOK_PIN, INPUT_PULLUP);
  #endif // LASER_PERIPHERALS

  // initialize state to some sane defaults
  laser.intensity = 100.0;
  laser.ppm = 0.0;
  laser.duration = 0;
  laser.status = LASER_OFF;
  laser.mode = LASER_CONTINUOUS;
  laser.last_firing = 0;
  laser.diagnostics = true;
  laser.time = 0;
  #ifdef LASER_RASTER
    laser.raster_aspect_ratio = LASER_RASTER_ASPECT_RATIO;
    laser.raster_mm_per_dot = LASER_RASTER_MM_PER_DOT;
    laser.raster_direction = 1;
  #endif // LASER_RASTER
  #ifdef MUVE_Z_PEEL
    laser.peel_distance = 2.0;
    laser.peel_speed = 2.0;
    laser.peel_pause = 0.0;
  #endif // MUVE_Z_PEEL
}
void laser_fire(int intensity = 100.0){
	laser.last_firing = micros(); // microseconds of last laser firing

	#ifdef LASER_INTENSITY_PIN
      analogWrite(LASER_INTENSITY_PIN, labs((intensity / 100.0)*(F_CPU / LASER_PWM)));
      digitalWrite(LASER_FIRING_PIN, HIGH);
    #else
      analogWrite(LASER_FIRING_PIN, (intensity / 100.0)*255);
    #endif // LASER_INTENSITY_PIN
}
void laser_extinguish(){
	if (digitalRead(LASER_FIRING_PIN) == HIGH) {
	  digitalWrite(LASER_FIRING_PIN, LOW);
	  laser.time += micros() - laser.last_firing;
	  if (laser.time > 4200000000) laser_update_lifetime(); // 70 minutes, nearing overflow of unsigned long
	}
}

void laser_update_lifetime(){
	laser.lifetime += laser.time / 60000000; // convert to minutes
	laser.time = 0;
    Config_StoreSettings();
}

#ifdef LASER_PERIPHERALS
bool laser_peripherals_ok(){
	return !digitalRead(LASER_AOK_PIN);
}
void laser_peripherals_on(){
	digitalWrite(LASER_ACC_PIN, LOW);
	if (laser.diagnostics == true) {
	  SERIAL_ECHO_START;
	  SERIAL_ECHOLNPGM("POWER: Laser Peripherals Enabled");
    }
}
void laser_peripherals_off(){
	digitalWrite(LASER_ACC_PIN, HIGH);
	if (laser.diagnostics == true) {
	  SERIAL_ECHO_START;
	  SERIAL_ECHOLNPGM("POWER: Laser Peripherals Disabled");
    }
}
void laser_wait_for_peripherals() {
	unsigned long timeout = millis() + LASER_AOK_TIMEOUT;
	if (laser.diagnostics == true) {
	  SERIAL_ECHO_START;
	  SERIAL_ECHOLNPGM("POWER: Waiting for relay board AOK...");
	}
	while(!laser_peripherals_ok()) {
		if (millis() > timeout) {
			if (laser.diagnostics == true) {
			  SERIAL_ERROR_START;
			  SERIAL_ERRORLNPGM("Power supply failed to indicate AOK");
			}
			Stop();
			break;
		}
	}
}
#endif // LASER_PERIPHERALS
