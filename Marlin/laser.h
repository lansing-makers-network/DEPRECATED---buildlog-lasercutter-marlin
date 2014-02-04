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
#include "Configuration.h"

#define LASER_OFF 0
#define LASER_ON 1

#define LASER_CONTINUOUS 0
#define LASER_PPM 1
#define LASER_RASTER 2

typedef struct {
  static float pwm;
  static float intensity;
  static float ppm;
  static uint16_t duration; // laser firing duration in microseconds
  static bool status;
  static uint8_t mode;
  static uint16_t last_firing; // microseconds since last laser firing
  static bool diagnostics;
  #ifdef LASER_RASTER
    static char raster_data[LASER_MAX_RASTER_LINE];
    static float raster_aspect_ratio;
    static float raster_mm_per_dot;
    static float raster_increment;
    static int raster_raw_length;
    static int raster_num_pixels;
  #endif // LASER_RASTER
} laser_t;

laser_t *laser;

void laser_setup();
bool laser_peripherals_ok();
void laser_peripherals_on();
void laser_peripherals_off();
void laser_wait_for_peripherals();
void laser_fire(int intensity);
void laser_extinguish();

#endif
