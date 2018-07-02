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
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_orientation_update(a3d_orientation_t* self, int slerp)
{
	assert(self);
	LOGD("debug");

	if((self->a_ts == 0.0) || (self->m_ts == 0.0))
	{
		// wait for sensors
		return;
	}

	// TODO - handle rotation and flat/vertical flags
	// compute the current rotation matrix
	a3d_vec3f_t at;
	a3d_vec3f_t up;
	a3d_vec3f_t x;
	a3d_vec3f_t y;
	a3d_vec3f_t z;
	a3d_vec3f_load(&up, self->a_ax, self->a_ay, self->a_az);
	a3d_vec3f_load(&at, self->m_mx, self->m_my, self->m_mz);
	a3d_vec3f_normalize_copy(&up, &z);
	a3d_vec3f_cross_copy(&at, &z, &x);
	a3d_vec3f_normalize(&x);
	a3d_vec3f_cross_copy(&z, &x, &y);
	a3d_vec3f_normalize(&y);

	a3d_mat4f_t m;
	if(self->m_north == A3D_ORIENTATION_TRUE)
	{
		// load the rotation matrix
		a3d_mat4f_t r;
		r.m00 = x.x;
		r.m10 = y.x;
		r.m20 = z.x;
		r.m30 = 0.0f;
		r.m01 = x.y;
		r.m11 = y.y;
		r.m21 = z.y;
		r.m31 = 0.0f;
		r.m02 = x.z;
		r.m12 = y.z;
		r.m22 = z.z;
		r.m32 = 0.0f;
		r.m03 = 0.0f;
		r.m13 = 0.0f;
		r.m23 = 0.0f;
		r.m33 = 1.0f;

		// compute the true north rotation matrix
		a3d_vec3f_load(&z, 0.0f, 0.0f, 1.0f);
		a3d_vec3f_load(&at, self->m_gfx, self->m_gfy, self->m_gfz);
		a3d_vec3f_cross_copy(&at, &z, &x);
		a3d_vec3f_normalize(&x);
		a3d_vec3f_cross_copy(&z, &x, &y);
		a3d_vec3f_normalize(&y);

		// load the true north rotation matrix
		a3d_mat4f_t n;
		n.m00 = x.x;
		n.m10 = x.y;
		n.m20 = x.z;
		n.m30 = 0.0f;
		n.m01 = y.x;
		n.m11 = y.y;
		n.m21 = y.z;
		n.m31 = 0.0f;
		n.m02 = z.x;
		n.m12 = z.y;
		n.m22 = z.z;
		n.m32 = 0.0f;
		n.m03 = 0.0f;
		n.m13 = 0.0f;
		n.m23 = 0.0f;
		n.m33 = 1.0f;

		// correct for true north
		a3d_mat4f_mulm_copy(&n, &r, &m);
	}
	else
	{
		m.m00 = x.x;
		m.m10 = y.x;
		m.m20 = z.x;
		m.m30 = 0.0f;
		m.m01 = x.y;
		m.m11 = y.y;
		m.m21 = z.y;
		m.m31 = 0.0f;
		m.m02 = x.z;
		m.m12 = y.z;
		m.m22 = z.z;
		m.m32 = 0.0f;
		m.m03 = 0.0f;
		m.m13 = 0.0f;
		m.m23 = 0.0f;
		m.m33 = 1.0f;
	}

	// convert matrix to quaternion
	a3d_quaternion_t q;
	a3d_quaternion_t out;
	a3d_mat4f_quaternion(&m, &q);

	if(slerp)
	{
		// derive slerp from err
		float err = a3d_quaternion_compare(&q, &self->Q);
		float t   = 0.001f + err*err;

		// clamp the slerp
		if(t > 0.004f)
		{
			t = 0.004f;
		}

		// update the filtered rotation quaternion
		a3d_quaternion_slerp(&self->Q, &q, t, &out);
		a3d_quaternion_copy(&out, &self->Q);
	}
	else
	{
		a3d_quaternion_copy(&q, &self->Q);
	}
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

	self->m_north = A3D_ORIENTATION_TRUE;
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

	self->a_ts       = 0.0;
	self->a_ax       = 0.0f;
	self->a_ay       = 0.0f;
	self->a_az       = 9.8f;
	self->a_rotation = 0;
	self->m_ts       = 0.0;
	self->m_mx       = 0.0f;
	self->m_my       = 1.0f;
	self->m_mz       = 0.0f;
	self->m_gfx      = 0.0f;
	self->m_gfy      = 1.0f;
	self->m_gfz      = 0.0f;
	self->g_ts       = 0.0;
	self->g_ax       = 0.0f;
	self->g_ay       = 0.0f;
	self->g_az       = 0.0f;

	a3d_quaternion_identity(&self->Q);
}

void a3d_orientation_accelerometer(a3d_orientation_t* self,
                                   double ts,
                                   float ax,
                                   float ay,
                                   float az,
                                   int rotation)
{
	assert(self);
	LOGD("debug ts=%lf, ax=%f, ay=%f, az=%f, rotation=%i",
	     ts, ax, ay, az, rotation);

	// reset orientation when rotation changes
	// because this often times happens when
	// performing figure 8 motion to calibrate
	// the compass
	if(rotation != self->a_rotation)
	{
		a3d_orientation_reset(self);
	}

	// don't slerp for the first rotation
	int slerp = 1;
	if((self->a_ts == 0.0) && (self->m_ts > 0.0))
	{
		slerp = 0;
	}

	self->a_ts       = ts;
	self->a_ax       = ax;
	self->a_ay       = ay;
	self->a_az       = az;
	self->a_rotation = rotation;

	a3d_orientation_update(self, slerp);
}

void a3d_orientation_magnetometer(a3d_orientation_t* self,
                                  double ts,
                                  float mx,
                                  float my,
                                  float mz,
                                  float gfx,
                                  float gfy,
                                  float gfz)
{
	assert(self);
	LOGD("debug ts=%lf, mx=%f, my=%f, mz=%f",
	     ts, mx, my, mz);

	// don't slerp for the first rotation
	int slerp = 1;
	if((self->m_ts == 0.0) && (self->a_ts > 0.0))
	{
		slerp = 0;
	}

	self->m_ts  = ts;
	self->m_mx  = mx;
	self->m_my  = my;
	self->m_mz  = mz;
	self->m_gfx = gfx;
	self->m_gfy = gfy;
	self->m_gfz = gfz;

	a3d_orientation_update(self, slerp);
}

void a3d_orientation_gyroscope(a3d_orientation_t* self,
                               double ts,
                               float ax,
                               float ay,
                               float az)
{
	assert(self);
	LOGD("debug ts=%lf, ax=%f, ay=%f, az=%f",
	     ts, ax, ay, az);

	// https://developer.android.com/guide/topics/sensors/sensors_motion.html
	// http://www.flipcode.com/documents/matrfaq.html#Q56

	// update the filtered estimate
	if((self->a_ts > 0.0) &&
	   (self->m_ts > 0.0) &&
	   (self->g_ts > 0.0))
	{
		float dt    = (float) (ts - self->g_ts);
		float mag   = sqrtf(ax*ax + ay*ay + az*az);
		float angle = (180.0f/M_PI)*mag*dt;


		a3d_quaternion_t q;
		a3d_quaternion_loadaxisangle(&q, ax, ay, az, angle);
		a3d_quaternion_rotateq(&self->Q, &q);
	}

	self->g_ts = ts;
	self->g_ax = ax;
	self->g_ay = ay;
	self->g_az = az;
}

void a3d_orientation_mat4f(a3d_orientation_t* self,
                           a3d_mat4f_t* m)
{
	assert(self);
	assert(m);
	LOGD("debug");

	a3d_mat4f_rotateq(m, 1, &self->Q);
}

void a3d_orientation_vpn(a3d_orientation_t* self,
                         float* vx,
                         float* vy,
                         float* vz)
{
	assert(self);
	assert(vx);
	assert(vy);
	assert(vz);

	// vpn is the negative z-axis of the
	// transpose/inverse rotation matrix
	a3d_mat4f_t m;
	a3d_orientation_mat4f(self, &m);
	*vx = -m.m02;
	*vy = -m.m12;
	*vz = -m.m22;
}

void a3d_orientation_spherical(a3d_orientation_t* self,
                               float* theta,
                               float* phi)
{
	assert(self);
	assert(theta);
	assert(phi);

	a3d_mat4f_t m;
	a3d_orientation_mat4f(self, &m);

	// load the transpose/inverse vectors
	a3d_vec3f_t x;
	a3d_vec3f_t y;
	a3d_vec3f_t z;
	a3d_vec3f_load(&x, m.m00, m.m10, m.m20);
	a3d_vec3f_load(&y, m.m01, m.m11, m.m21);
	a3d_vec3f_load(&z, m.m02, m.m12, m.m22);

	*theta = (180.0f/M_PI)*atan2f(z.y, z.x);
	while(*theta < 0.0f)
	{
		*theta += 360.0f;
	}
	while(*theta >= 360.0f)
	{
		*theta -= 360.0f;
	}

	*phi = (180.0f/M_PI)*asinf(z.z);
}

void a3d_orientation_euler(a3d_orientation_t* self,
                           float* yaw,
                           float* pitch,
                           float* roll)
{
	assert(self);
	assert(yaw);
	assert(pitch);
	assert(roll);

	a3d_mat4f_t m;
	a3d_orientation_mat4f(self, &m);

	a3d_vec3f_t x;
	a3d_vec3f_t y;
	a3d_vec3f_t z;
	a3d_vec3f_load(&x, m.m00, m.m01, m.m02);
	a3d_vec3f_load(&y, m.m10, m.m11, m.m12);
	a3d_vec3f_load(&z, m.m20, m.m21, m.m22);

	// TODO - fix euler angles per
	// https://en.wikipedia.org/wiki/Euler_angles
	// TODO - euler corner cases
	// convert to euler angles
	*yaw   = (180.0f/M_PI)*(atan2f(-y.x, y.y));
	*pitch = (180.0f/M_PI)*(acosf(y.z));
	*roll  = (180.0f/M_PI)*(-atan2f(x.z, z.z));

	// TODO - handle rotation
	// workaround screen rotations for yaw
	*yaw += (float) self->a_rotation;
	while(*yaw < 0.0f)
	{
		*yaw += 360.0f;
	}
	while(*yaw > 360.0f)
	{
		*yaw -= 360.0f;
	}
}
