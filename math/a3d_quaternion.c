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

#include "a3d_quaternion.h"
#include "a3d_vec4f.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_quaternion_normalize(a3d_quaternion_t* self)
{
	assert(self);
	LOGD("debug");

	return a3d_vec4f_normalize((a3d_vec4f_t*) self);
}

static void a3d_quaternion_normalize_copy(const a3d_quaternion_t* self,
                                          a3d_quaternion_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	return a3d_vec4f_normalize_copy((const a3d_vec4f_t*) self,
	                                (a3d_vec4f_t*) copy);
}

/***********************************************************
* public                                                   *
***********************************************************/

void a3d_quaternion_load(a3d_quaternion_t* self,
                         GLfloat x, GLfloat y, GLfloat z, GLfloat s)
{
	assert(self);
	LOGD("debug x=%f, y=%f, z=%f, s=%f", x, y, z, s);

	a3d_vec3f_load(&self->v, x, y, z);
	self->s = s;
}

void a3d_quaternion_loadaxis(a3d_quaternion_t* self,
                             GLfloat a, GLfloat x, GLfloat y, GLfloat z)
{
	assert(self);
	LOGD("debug a=%f, x=%f, y=%f, z=%f", a, x, y, z);

	a3d_vec3f_t v;
	a3d_vec3f_load(&v, x, y, z);
	a3d_vec3f_normalize(&v);

	GLfloat a2     = (a/2.0f)*(M_PI/180.0f);
	GLfloat sin_a2 = sinf(a2);
	GLfloat cos_a2 = cosf(a2);

	// already has unit norm
	self->v.x = v.x*sin_a2;
	self->v.y = v.y*sin_a2;
	self->v.z = v.z*sin_a2;
	self->s   = cos_a2;
}

void a3d_quaternion_loadeuler(a3d_quaternion_t* self,
                              GLfloat rx, GLfloat ry, GLfloat rz)
{
	assert(self);
	LOGD("debug rx=%f, ry=%f, rz=%f", rx, ry, rz);

	#if 1
		GLfloat rx2 = (rx/2.0f)*(M_PI/180.0f);
		GLfloat ry2 = (ry/2.0f)*(M_PI/180.0f);
		GLfloat rz2 = (rz/2.0f)*(M_PI/180.0f);

		GLfloat sx = sinf(rx2);
		GLfloat sy = sinf(ry2);
		GLfloat sz = sinf(rz2);
		GLfloat cx = cosf(rx2);
		GLfloat cy = cosf(ry2);
		GLfloat cz = cosf(rz2);

		self->v.x = sx*cy*cz - cx*sy*sz;
		self->v.y = cx*sy*cz + sx*cy*sz;
		self->v.z = cx*cy*sz - sx*sy*cz;
		self->s   = cx*cy*cz + sx*sy*sz;

		a3d_quaternion_normalize(self);
	#else
		a3d_quaternion_t qx;
		a3d_quaternion_t qy;
		a3d_quaternion_t qz;
		a3d_quaternion_t qt;
		a3d_quaternion_loadaxis(&qx, rx, 1.0f, 0.0f, 0.0f);
		a3d_quaternion_loadaxis(&qy, ry, 0.0f, 1.0f, 0.0f);
		a3d_quaternion_loadaxis(&qz, rz, 0.0f, 0.0f, 1.0f);
		a3d_quaternion_rotateq_copy(&qx, &qy, &qt);
		a3d_quaternion_rotateq_copy(&qt, &qz, self);
		a3d_quaternion_normalize(self);
	#endif
}

void a3d_quaternion_rotateq(a3d_quaternion_t* self,
                            const a3d_quaternion_t* q)
{
	assert(self);
	assert(q);
	LOGD("debug");

	// reminder - quaternion multiplication is non-commutative
	const a3d_vec3f_t* av = &self->v;
	const a3d_vec3f_t* qv = &q->v;
	GLfloat            as = self->s;
	GLfloat            qs = q->s;

	#if 1
		a3d_quaternion_t   copy;
		copy.v.x = as*qv->x + av->x*qs    + av->y*qv->z - av->z*qv->y;
		copy.v.y = as*qv->y + av->y*qs    + av->z*qv->x - av->x*qv->z;
		copy.v.z = as*qv->z + av->z*qs    + av->x*qv->y - av->y*qv->x;
		copy.s   = as*qs    - av->x*qv->x - av->y*qv->y - av->z*qv->z;

		a3d_quaternion_normalize_copy(&copy, self);
	#else
		// website seems to have a bug for s
		// http://www.flipcode.com/documents/matrfaq.html
		a3d_vec3f_t a;
		a3d_vec3f_t b;
		a3d_vec3f_t c;
		GLfloat     s = (as*qs) - a3d_vec3f_dot(av, qv);
		a3d_vec3f_cross_copy(av, qv, &a);
		a3d_vec3f_muls_copy(av, qs, &b);
		a3d_vec3f_muls_copy(qv, as, &c);
		a3d_vec3f_addv(&a, &b);
		a3d_vec3f_addv(&a, &c);
		a3d_quaternion_load(self, a.x, a.y, a.z, s);
		a3d_quaternion_normalize(self);
	#endif
}

void a3d_quaternion_rotateq_copy(const a3d_quaternion_t* self,
                                 const a3d_quaternion_t* q,
                                 a3d_quaternion_t* copy)
{
	assert(self);
	assert(q);
	assert(copy);
	LOGD("debug");

	// reminder - quaternion multiplication is non-commutative
	const a3d_vec3f_t* av = &self->v;
	const a3d_vec3f_t* qv = &q->v;
	GLfloat            as = self->s;
	GLfloat            qs = q->s;

	#if 1
		copy->v.x = as*qv->x + av->x*qs    + av->y*qv->z - av->z*qv->y;
		copy->v.y = as*qv->y + av->y*qs    + av->z*qv->x - av->x*qv->z;
		copy->v.z = as*qv->z + av->z*qs    + av->x*qv->y - av->y*qv->x;
		copy->s   = as*qs    - av->x*qv->x - av->y*qv->y - av->z*qv->z;

		a3d_quaternion_normalize(copy);
	#else
		// website seems to have a bug for s
		// http://www.flipcode.com/documents/matrfaq.html
		a3d_vec3f_t a;
		a3d_vec3f_t b;
		a3d_vec3f_t c;
		GLfloat     s = (as*qs) - a3d_vec3f_dot(av, qv);
		a3d_vec3f_cross_copy(av, qv, &a);
		a3d_vec3f_muls_copy(av, qs, &b);
		a3d_vec3f_muls_copy(qv, as, &c);
		a3d_vec3f_addv(&a, &b);
		a3d_vec3f_addv(&a, &c);
		a3d_quaternion_load(copy, a.x, a.y, a.z, s);
		a3d_quaternion_normalize(copy);
	#endif
}
