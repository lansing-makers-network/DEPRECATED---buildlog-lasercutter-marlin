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

#define LASER_ON 1
#define LASER_OFF 0

extern uint8_t laserPower;
extern bool laserAccOn;
extern bool laserOn;
extern bool laserAok;

static bool waitForLaserAok();
static float laser_pwm = 0;
static float laser_intensity = 100;
static float laser_ppm = 0;
static uint8_t laser_duration = 0;
static bool laser_firing = LASER_OFF;

void setupLaser();
void fireLaser(float intensity);
void fireLaser(float intensity, uint16_t duration);
void offLaser();
void prepareLaser();
void shutdownLaser();

#endif
