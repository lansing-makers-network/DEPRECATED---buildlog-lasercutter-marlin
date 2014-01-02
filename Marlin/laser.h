
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
#ifndef LASER_H
#define LASER_H

#define RASTER_DURATION 1  // Raster pulse duration in MS
#define RASTER_FEEDRATE 18000 // Raster feedrate.  Can be fairly high as the moves will be exceedingly small
#define MAX_RASTER_LINE 60
#define laserRasterLine() { \
	static char pixels[MAX_RASTER_LINE]; \
	int numLength; \
	int numPixels; \
	if (code_seen('L')) { \
		numLength = int(code_value()); \
		if (code_seen('D')) { \
			numPixels = base64_decode(pixels, &cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1], numLength); \
			for (int i = 0; i<numPixels; i++) { \
			  if (pixels[i] > 15) { \
				  SERIAL_ECHO("Pixel: "); \
				  SERIAL_ECHOLN(itostr3(pixels[i])); \
				fireLaser((float(pixels[i])/255.0)*100.0, RASTER_DURATION); \
				} \
			  destination[X_AXIS] += laser_raster_step; \
			  prepare_move(); \
			  st_synchronize(); \
			} \
		} \
	}\
}

#define laserRasterNewLine(DIR) { \
	laser_raster_step = laser_raster_mm_per_dot * DIR;  \
	next_feedrate = RASTER_FEEDRATE;  \
	destination[Y_AXIS] = current_position[Y_AXIS] + (laser_raster_mm_per_dot *1.33); \
	prepare_move(); \
	st_synchronize(); \
}





/*
  A laser is activated by c
  
  
 */
#include <inttypes.h>

/*
 * Defines for 16 bit timers used with  Servo library
 *
 * If _useTimerX is defined then TimerX is a 16 bit timer on the curent board
 * timer16_Sequence_t enumerates the sequence that the timers should be allocated
 * _Nbr_16timers indicates how many 16 bit timers are available.
 *
 */

//typedef enum { _timer5, _timer3, _timer4, _Nbr_16timers } timer16_Sequence_t ;
extern uint8_t laserPower;
extern bool laserAccOn;
extern bool laserOn;
extern bool laserAok;
extern float laser_raster_step;
extern float laser_raster_mm_per_dot;

static bool waitForLaserAok();
void setupLaser();
void fireLaser(float intensity);
void fireLaser(float intensity, uint16_t duration);
void offLaser();
void prepareLaser();
void shutdownLaser();
#endif
