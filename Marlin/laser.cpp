/*
  laser.cpp - Laser control library for Arduino using 16 bit timers- Version 1
  Copyright (c) 2013 Timothy Schmidt.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Configuration.h"
#include "pins.h"
#include <avr/interrupt.h>
#include <Arduino.h>

static void setupLaser()
{
  pinMode(LASER_FIRING_PIN, OUTPUT);
  pinMode(LASER_INTENSITY_PIN, OUTPUT);
  
  analogWrite(LASER_INTENSITY_PIN, 1);  // let Arduino setup do it's thing to the PWM pin
  
  TCCR4B = 0x00;  // stop Timer4 clock for register updates
  TCCR4A = 0x82; // Clear OC4A on match, fast PWM mode, lower WGM4x=14
  ICR4 = 800; // 800 clock cycles = 20,000hz
  OCR4A = 799; // ICR4 - 1 force immediate compare on next tick
  TCCR4B = 0x18 | 0x01; // upper WGM4x = 14, clock sel = prescaler, start running
}

static void fireLaser(float intensity)
{
  if (intensity < 0) intensity = 0;
  if (intensity > 1000) intensity = 1000;
  float laser_pwm = 16000000 / (LASER_PWM_MIN + ((LASER_PWM_MAX - LASER_PWM_MIN) * (intensity / 1000)));
  noInterrupts();
  TCCR4B &= 0xf8; // stop timer, OC4A may be active now
  TCNT4 = 799; // force immediate compare on next tick
  ICR4 = laser_pwm; // set new PWM period
  TCCR4B |= 0x01; // start the timer with proper prescaler value
  interrupts();
  digitalWrite(LASER_FIRING_PIN,HIGH);
  SERIAL_ECHO_START;
  SERIAL_ECHO("Laser firing intensity: ");
  SERIAL_ECHO(intensity);
  SERIAL_ECHO("Laser firing PWM: ");
  SERIAL_ECHOLN(laser_pwm);
}

static void offLaser()
{
  digitalWrite(LASER_FIRING_PIN,LOW);
}
