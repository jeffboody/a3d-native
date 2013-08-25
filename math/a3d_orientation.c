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

#include "a3d_orientation.h"
#include "a3d_mat4f.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_orientation_update(a3d_orientation_t* self)
{
	assert(self);
	LOGD("debug");

	if((self->a_utime == 0.0) || (self->m_utime == 0.0))
	{
		// wait for sensors
		return;
	}

	// TODO - handle rotation and flat/vertical flags
	// compute the current rotation matrix
	a3d_vec3f_t at;
	a3d_vec3f_t up;
	a3d_vec3f_t n;
	a3d_vec3f_t u;
	a3d_vec3f_t v;
	a3d_vec3f_load(&up, self->a_ax, self->a_ay, self->a_az);
	a3d_vec3f_load(&at, self->m_mx, self->m_my, self->m_mz);
	a3d_vec3f_normalize_copy(&up, &v);
	a3d_vec3f_cross_copy(&at, &v, &u);
	a3d_vec3f_normalize(&u);
	a3d_vec3f_cross_copy(&v, &u, &n);

	// load the matrix
	a3d_mat4f_t* m = &self->ring[self->ring_index];
	m->m00 = u.x;
	m->m10 = v.x;
	m->m20 = n.x;
	m->m30 = 0.0f;
	m->m01 = u.y;
	m->m11 = v.y;
	m->m21 = n.y;
	m->m31 = 0.0f;
	m->m02 = u.z;
	m->m12 = v.z;
	m->m22 = n.z;
	m->m32 = 0.0f;
	m->m03 = 0.0f;
	m->m13 = 0.0f;
	m->m23 = 0.0f;
	m->m33 = 1.0f;

	// increment the matrix ring buffer
	if(self->ring_count < A3D_ORIENTATION_COUNT)
	{
		++self->ring_count;
	}
	self->ring_index = (self->ring_index + 1) % A3D_ORIENTATION_COUNT;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_orientation_t* a3d_orientation_new(void)
{
	LOGD("debug");

	a3d_orientation_t* self = (a3d_orientation_t*) malloc(sizeof(a3d_orientation_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	a3d_orientation_reset(self);

	return self;
}

void a3d_orientation_delete(a3d_orientation_t** _self)
{
	assert(_self);

	a3d_orientation_t* self = *_self;
	if(self)
	{
		LOGD("delete");

		free(self);
		*_self = NULL;
	}
}

void a3d_orientation_reset(a3d_orientation_t* self)
{
	assert(self);
	LOGD("debug");

	self->a_utime    = 0.0;
	self->a_ax       = 0.0f;
	self->a_ay       = 0.0f;
	self->a_az       = 9.8f;
	self->a_rotation = 0;
	self->m_utime    = 0.0;
	self->m_mx       = 0.0f;
	self->m_my       = 1.0f;
	self->m_mz       = 0.0f;
	self->g_utime    = 0.0;
	self->g_ax       = 0.0f;
	self->g_ay       = 0.0f;
	self->g_az       = 0.0f;

	self->ring_count = 0;
	self->ring_index = 0;
	memset(self->ring, 0, sizeof(self->ring));
}

void a3d_orientation_accelerometer(a3d_orientation_t* self,
                                   double utime,
                                   float ax,
                                   float ay,
                                   float az,
                                   int rotation)
{
	assert(self);
	LOGD("debug utime=%lf, ax=%f, ay=%f, az=%f, rotation=%i",
	     utime, ax, ay, az, rotation);

	self->a_utime    = utime;
	self->a_ax       = ax;
	self->a_ay       = ay;
	self->a_az       = az;
	self->a_rotation = rotation;

	a3d_orientation_update(self);
}

void a3d_orientation_magnetometer(a3d_orientation_t* self,
                                  double utime,
                                  float mx,
                                  float my,
                                  float mz)
{
	assert(self);
	LOGD("debug utime=%lf, mx=%f, my=%f, mz=%f",
	     utime, mx, my, mz);

	self->m_utime = utime;
	self->m_mx    = mx;
	self->m_my    = my;
	self->m_mz    = mz;

	a3d_orientation_update(self);
}

void a3d_orientation_gyroscope(a3d_orientation_t* self,
                               double utime,
                               float ax,
                               float ay,
                               float az)
{
	assert(self);
	LOGD("debug utime=%lf, ax=%f, ay=%f, az=%f",
	     utime, ax, ay, az);

	self->g_utime = utime;
	self->g_ax    = ax;
	self->g_ay    = ay;
	self->g_az    = az;

	// TODO - update gyro
}

void a3d_orientation_mat4f(a3d_orientation_t* self,
                           a3d_mat4f_t* m)
{
	assert(self);
	assert(m);
	LOGD("debug");

	// verify that orientation exists
	if(self->ring_count == 0)
	{
		a3d_mat4f_identity(m);
		return;
	}

	// See "On Averaging Rotations"
	// http://www.soest.hawaii.edu/wessel/courses/gg711/pdf/Gramkow_2001_JMIV.pdf

	// compute average rotation matrix
	// TODO - compute running average
	int i;
	a3d_mat4f_copy(&self->ring[0], m);
	for(i = 1; i < self->ring_count; ++i)
	{
		a3d_mat4f_addm(m, &self->ring[i]);
	}
	a3d_mat4f_muls(m, 1.0f/(GLfloat) self->ring_count);

	// enforce orthonormal constraint for rotation matrix
	a3d_mat4f_orthonormal(m);
}
