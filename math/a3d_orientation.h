/*
 * Copyright (c) 2013 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef a3d_orientation_H
#define a3d_orientation_H

#include "a3d_mat4f.h"

#define A3D_ORIENTATION_COUNT 64

typedef struct
{
	// accelerometer
	double a_utime;
	float  a_ax;
	float  a_ay;
	float  a_az;
	int    a_rotation;

	// magnetometer
	double m_utime;
	float  m_mx;
	float  m_my;
	float  m_mz;

	// gyroscope
	double g_utime;
	float  g_ax;
	float  g_ay;
	float  g_az;

	// matrix ring buffer
	int         ring_count;
	int         ring_index;
	a3d_mat4f_t ring[A3D_ORIENTATION_COUNT];
} a3d_orientation_t;

a3d_orientation_t* a3d_orientation_new(void);
void               a3d_orientation_delete(a3d_orientation_t** _self);
void               a3d_orientation_reset(a3d_orientation_t* self);
void               a3d_orientation_accelerometer(a3d_orientation_t* self,
                                                 double utime,
                                                 float ax,
                                                 float ay,
                                                 float az,
                                                 int rotation);
void               a3d_orientation_magnetometer(a3d_orientation_t* self,
                                                double utime,
                                                float mx,
                                                float my,
                                                float mz);
void               a3d_orientation_gyroscope(a3d_orientation_t* self,
                                             double utime,
                                             float ax,
                                             float ay,
                                             float az);
void               a3d_orientation_mat4f(a3d_orientation_t* self,
                                         a3d_mat4f_t* m);

#endif