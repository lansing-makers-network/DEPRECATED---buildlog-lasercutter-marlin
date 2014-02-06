/*
  laser.h - Laser cutter control library for Arduino using 16 bit timers- Version 1
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

#ifndef LASER_H
#define LASER_H

#include <inttypes.h>
#include "Configuration.h"

typedef struct {
  static float pwm; // PWM frequency used to control laser intensity in HZ
  static float intensity; // 0.0 - 100.0
  static float ppm; // pulses per millimeter, for pulsed firing mode
  static uint16_t duration; // laser firing duration in microseconds
  static bool status; // LASER_ON / LASER_OFF
  static uint8_t mode; // LASER_CONTINUOUS, LASER_PULSED, LASER_RASTER
  static uint16_t last_firing; // microseconds since last laser firing
  static bool diagnostics; // Verbose debugging output over serial
  #ifdef LASER_RASTER
    static char raster_data[LASER_MAX_RASTER_LINE];
    static float raster_aspect_ratio;
    static float raster_mm_per_dot;
    static float raster_increment;
    static int raster_raw_length;
    static int raster_num_pixels;
  #endif // LASER_RASTER
} laser_t;

laser_t laser;

extern laser_t laser;

void laser_setup();
void laser_fire(int intensity);
void laser_extinguish();
#ifdef LASER_PERIPHERALS
  bool laser_peripherals_ok();
  void laser_peripherals_on();
  void laser_peripherals_off();
  void laser_wait_for_peripherals();
#endif // LASER_PERIPHERALS

#endif // LASER_H
