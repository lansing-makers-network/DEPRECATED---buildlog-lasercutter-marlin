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

void timer3_init() {
	pinMode(LASER_FIRING_PIN, OUTPUT);
    analogWrite(LASER_FIRING_PIN, 1);  // let Arduino setup do it's thing to the PWM pin

    TCCR3B = 0x00;  // stop Timer4 clock for register updates
    TCCR3A = 0x82; // Clear OC3A on match, fast PWM mode, lower WGM3x=14
    ICR3 = labs(F_CPU / LASER_PWM); // clock cycles per PWM pulse
    OCR3A = labs(F_CPU / LASER_PWM) - 1; // ICR3 - 1 force immediate compare on next tick
    TCCR3B = 0x18 | 0x01; // upper WGM4x = 14, clock sel = prescaler, start running

    noInterrupts();
    TCCR3B &= 0xf8; // stop timer, OC3A may be active now
    TCNT3 = labs(F_CPU / LASER_PWM); // force immediate compare on next tick
    ICR3 = labs(F_CPU / LASER_PWM); // set new PWM period
    TCCR3B |= 0x01; // start the timer with proper prescaler value
    interrupts();
}

void timer4_init() {
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
}

void laser_init()
{
  #if LASER_CONTROL == 1
	timer3_init();
  #endif // LASER_CONTROL ==1
  #if LASER_CONTROL == 2
    timer3_init();
    timer4_init();
  #endif // LASER_CONTROL == 2

  // Engage the pullup resistor for TTL laser controllers which don't turn off entirely without it.
  pinMode(LASER_FIRING_PIN, OUTPUT);

  #ifdef LASER_PERIPHERALS
  digitalWrite(LASER_PERIPHERALS_PIN, HIGH);  // Laser peripherals are active LOW, so preset the pin
  pinMode(LASER_PERIPHERALS_PIN, OUTPUT);

  digitalWrite(LASER_PERIPHERALS_STATUS_PIN, HIGH);  // Set the peripherals status pin to pull-up.
  pinMode(LASER_PERIPHERALS_STATUS_PIN, INPUT);
  #endif // LASER_PERIPHERALS

  // initialize state to some sane defaults
  laser.intensity = 100.0;
  laser.ppm = 0.0;
  laser.duration = 0;
  laser.status = LASER_OFF;
  laser.firing = LASER_OFF;
  laser.mode = CONTINUOUS;
  laser.last_firing = 0;
  laser.diagnostics = false;
  laser.time = 0;
  #ifdef LASER_RASTER
    laser.raster_aspect_ratio = LASER_RASTER_ASPECT_RATIO;
    laser.raster_mm_per_pulse = LASER_RASTER_MM_PER_PULSE;
    laser.raster_direction = 1;
  #endif // LASER_RASTER
  #ifdef MUVE_Z_PEEL
    laser.peel_distance = 2.0;
    laser.peel_speed = 2.0;
    laser.peel_pause = 0.0;
  #endif // MUVE_Z_PEEL
}
void laser_fire(int intensity = 100.0){
	laser.firing = LASER_ON;
	laser.last_firing = micros(); // microseconds of last laser firing
	if (intensity > 100.0) intensity = 100.0; // restrict intensity between 0 and 100
	if (intensity < 0) intensity = 0;

    pinMode(LASER_FIRING_PIN, OUTPUT);
	#if LASER_CONTROL == 1
	  analogWrite(LASER_FIRING_PIN, labs((intensity / 100.0)*(F_CPU / LASER_PWM)));
    #endif
	#if LASER_CONTROL == 2
      analogWrite(LASER_INTENSITY_PIN, labs((intensity / 100.0)*(F_CPU / LASER_PWM)));
      digitalWrite(LASER_FIRING_PIN, HIGH);
    #endif

    if (laser.diagnostics) {
	  SERIAL_ECHOLN("Laser fired");
	}
}
void laser_extinguish(){
	if (laser.firing == LASER_ON) {
	  laser.firing = LASER_OFF;

	  // Engage the pullup resistor for TTL laser controllers which don't turn off entirely without it.
	  pinMode(LASER_FIRING_PIN, INPUT);
	  laser.time += millis() - (laser.last_firing / 1000);

	  if (laser.diagnostics) {
	    SERIAL_ECHOLN("Laser extinguished");
	  }
	}
}
#ifdef LASER_PERIPHERALS
bool laser_peripherals_ok(){
	return !digitalRead(LASER_PERIPHERALS_STATUS_PIN);
}
void laser_peripherals_on(){
	digitalWrite(LASER_PERIPHERALS_PIN, LOW);
	if (laser.diagnostics) {
	  SERIAL_ECHO_START;
	  SERIAL_ECHOLNPGM("Laser Peripherals Enabled");
    }
}
void laser_peripherals_off(){
	if (!digitalRead(LASER_PERIPHERALS_STATUS_PIN)) {
	  digitalWrite(LASER_PERIPHERALS_PIN, HIGH);
	  if (laser.diagnostics) {
	    SERIAL_ECHO_START;
	    SERIAL_ECHOLNPGM("Laser Peripherals Disabled");
      }
    }
}
void laser_wait_for_peripherals() {
	unsigned long timeout = millis() + LASER_PERIPHERALS_TIMEOUT;
	if (laser.diagnostics) {
	  SERIAL_ECHO_START;
	  SERIAL_ECHOLNPGM("Waiting for peripheral control board signal...");
	}
	while(!laser_peripherals_ok()) {
		if (millis() > timeout) {
			if (laser.diagnostics) {
			  SERIAL_ERROR_START;
			  SERIAL_ERRORLNPGM("Peripheral control board failed to respond");
			}
			Stop();
			break;
		}
	}
}

// from http://forums.reprap.org/read.php?147,93577
void parse_bezier() {
	float p[4][2] = {{0.0,0.0},{0.0,0.0},{0.0,0.0},{0.0,0.0}};
	int steps = 10;
	float stepsPerUnit = 1;
	float f[2]={0,0};
	float fd[2]={0,0};
	float fdd[2]={0,0};
	float fddd[2]={0,0};
	float fdd_per_2[2]={0,0};
	float fddd_per_2[2]={0,0};
	float fddd_per_6[2]={0,0};
	float t = (1.0);
	float temp;
	// get coordinates
	//---------------------------------------
	// start point
	p[0][0] = current_position[0];
	p[0][1] = current_position[1];
	// control point 1
	if(code_seen('I')) p[1][0] = (float)code_value() + (axis_relative_modes[0] || relative_mode)*current_position[0];
	if(code_seen('J')) p[1][1] = (float)code_value() + (axis_relative_modes[1] || relative_mode)*current_position[1];
 
	// control point 2
	if(code_seen('K')) p[2][0] = (float)code_value() + (axis_relative_modes[0] || relative_mode)*current_position[0];
	if(code_seen('L')) p[2][1] = (float)code_value() + (axis_relative_modes[1] || relative_mode)*current_position[1];
	// end point
	if(code_seen(axis_codes[0])) p[3][0] = (float)code_value() + (axis_relative_modes[0] || relative_mode)*current_position[0];
	if(code_seen(axis_codes[1])) p[3][1] = (float)code_value() + (axis_relative_modes[1] || relative_mode)*current_position[1];
 
	#ifdef DEBUG
		log_float("CX", p[0][0]);
		log_float("CY", p[0][1]);
		log_float("I", p[1][0]);
		log_float("J", p[1][1]);
		log_float("K", p[2][0]);
		log_float("L", p[2][1]);
		log_float("X", p[3][0]);
		log_float("Y", p[3][1]);
	#endif
	// calc num steps
	float maxD = 0, sqrD = 0;
	for (int i=1; i<4; i++) {
		sqrD = (p[i][0] - p[i-1][0])*(p[i][0] - p[i-1][0]) + (p[i][1] - p[i-1][1])*(p[i][1] - p[i-1][1]);
		if (sqrD > maxD) {maxD = sqrD; };
	}
	maxD = sqrt(maxD);
	if (maxD > 0) {
		steps = round((3 * maxD * stepsPerUnit));
	}
	if (steps < 1) steps = 1;
	if (steps > 200) steps = 200;
	#ifdef DEBUG
		log_float("maxD",maxD);
		log_int("steps", steps);
	#endif
	// init Forward Differencing algo
	//---------------------------------------
	t = 1.0 / steps;
	temp = t*t;
	for (int i=0; i<2; i++) {
		f[i] = p[0][i];
		fd[i] = 3 * (p[1][i] - p[0][i]) * t;
		fdd_per_2[i] = 3 * (p[0][i] - 2 * p[1][i] + p[2][i]) * temp;
		fddd_per_2[i] = 3 * (3 * (p[1][i] - p[2][i]) + p[3][i] - p[0][i]) * temp * t;
 
		fddd[i] = fddd_per_2[i] + fddd_per_2[i];
		fdd[i] = fdd_per_2[i] + fdd_per_2[i];
		fddd_per_6[i] = (fddd_per_2[i] * (1.0 / 3));
	}
	// prep destination
	for(int i=0; i < NUM_AXIS; i++) {
		destination[i] = current_position[i];
	}
	// iterate through curve
	//---------------------------------------
	for (int loop=0; loop < steps; loop++) {
		destination[0] = f[0];
		destination[1] = f[1];
		#ifdef DEBUG
			log_float("X",f[0]);
			log_float("Y",f[1]);
		#endif
		prepare_move();
		previous_millis_cmd = millis();
 
		// update f
		for (int i=0; i<2; i++) {
			f[i] = f[i] + fd[i] + fdd_per_2[i] + fddd_per_6[i];
			fd[i] = fd[i] + fdd[i] + fddd_per_2[i];
			fdd[i] = fdd[i] + fddd[i];
			fdd_per_2[i] = fdd_per_2[i] + fddd_per_2[i];
		}
	}
	// Move to final position
	destination[0] = p[3][0];
	destination[1] = p[3][1];
	prepare_move();
	previous_millis_cmd = millis();
}

#endif // LASER_PERIPHERALS
