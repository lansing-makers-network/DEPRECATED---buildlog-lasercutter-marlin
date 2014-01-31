#ifndef LASER_H
#define LASER_H
/*
  laser.h - Laser cutter control library for Arduino using 16 bit timers- Version 1
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

#include <inttypes.h>

#define LASER_OFF 0
#define LASER_ON 1

#define LASER_CONTINUOUS 0
#define LASER_PPM 1
#define LASER_RASTER 2

static float laser_pwm = 0;
static float laser_intensity = 100;
static float laser_ppm = 0;
static uint16_t laser_duration = 0; // laser firing duration in microseconds
static bool laser_status = LASER_OFF;
static uint8_t laser_mode = LASER_CONTINUOUS;
static uint16_t laser_last_firing = 0; // microseconds since last laser firing
static bool laser_diagnostics = true;

#ifdef LASER_RASTER
  #define LASER_MAX_RASTER_LINE 60
  
  static char laser_raster_data[LASER_MAX_RASTER_LINE];
  static float laser_raster_aspect_ratio = 1.33;
  static float laser_raster_mm_per_dot = 0.2;
  static float laser_raster_increment = 0.2;
  static int laser_raster_raw_length;
  static int laser_raster_num_pixels;
#endif // LASER_RASTER

void laser_setup();
bool laser_peripherals_ok();
void laser_peripherals_on();
void laser_peripherals_off();
void laser_wait_for_peripherals();
void laser_fire(int intensity);
void laser_extinguish();

#endif
