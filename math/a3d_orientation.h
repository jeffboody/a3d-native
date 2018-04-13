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
#include "a3d_quaternion.h"

#define A3D_ORIENTATION_TRUE     0
#define A3D_ORIENTATION_MAGNETIC 1

typedef struct
{
	// accelerometer
	double a_ts;
	float  a_ax;
	float  a_ay;
	float  a_az;
	int    a_rotation;

	// magnetometer
	int    m_north;
	double m_ts;
	float  m_mx;
	float  m_my;
	float  m_mz;
	float  m_gfx;
	float  m_gfy;
	float  m_gfz;

	// gyroscope
	double g_ts;
	float  g_ax;
	float  g_ay;
	float  g_az;

	// filtered rotation quaternion
	a3d_quaternion_t Q;
} a3d_orientation_t;

a3d_orientation_t* a3d_orientation_new(void);
void               a3d_orientation_delete(a3d_orientation_t** _self);
void               a3d_orientation_reset(a3d_orientation_t* self);
void               a3d_orientation_accelerometer(a3d_orientation_t* self,
                                                 double ts,
                                                 float ax,
                                                 float ay,
                                                 float az,
                                                 int rotation);
void               a3d_orientation_magnetometer(a3d_orientation_t* self,
                                                double ts,
                                                float mx,
                                                float my,
                                                float mz,
                                                float gfx,
                                                float gfy,
                                                float gfz);
void               a3d_orientation_gyroscope(a3d_orientation_t* self,
                                             double ts,
                                             float ax,
                                             float ay,
                                             float az);
void               a3d_orientation_mat4f(a3d_orientation_t* self,
                                         a3d_mat4f_t* m);
void               a3d_orientation_vpn(a3d_orientation_t* self,
                                       float* vx,
                                       float* vy,
                                       float* vz);
void               a3d_orientation_spherical(a3d_orientation_t* self,
                                             float* theta,
                                             float* phi);
void               a3d_orientation_euler(a3d_orientation_t* self,
                                         float* yaw,
                                         float* pitch,
                                         float* roll);

#endif
