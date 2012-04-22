/*
 * Copyright (c) 2009-2010 Jeff Boody
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

#include "a3d_vec3f.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* public                                                   *
***********************************************************/

void a3d_vec3f_load(a3d_vec3f_t* self, GLfloat x, GLfloat y, GLfloat z)
{
	assert(self);
	LOGD("debug x=%f, y=%f, z=%f", x, y, z);

	self->x = x;
	self->y = y;
	self->z = z;
}

void a3d_vec3f_copy(const a3d_vec3f_t* self, a3d_vec3f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->x = self->x;
	copy->y = self->y;
	copy->z = self->z;
}

GLfloat a3d_vec3f_mag(const a3d_vec3f_t* self)
{
	assert(self);
	LOGD("debug");

	return sqrtf(self->x*self->x +
	             self->y*self->y +
	             self->z*self->z);
}

void a3d_vec3f_addv(a3d_vec3f_t* self, const a3d_vec3f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	self->x += v->x;
	self->y += v->y;
	self->z += v->z;
}

void a3d_vec3f_addv_copy(const a3d_vec3f_t* self, const a3d_vec3f_t* v, a3d_vec3f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->x + v->x;
	copy->y = self->y + v->y;
	copy->z = self->z + v->z;
}

void a3d_vec3f_adds(a3d_vec3f_t* self, GLfloat s)
{
	assert(self);
	LOGD("debug");

	self->x += s;
	self->y += s;
	self->z += s;
}

void a3d_vec3f_adds_copy(const a3d_vec3f_t* self, GLfloat s, a3d_vec3f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->x = self->x + s;
	copy->y = self->y + s;
	copy->z = self->z + s;
}

void a3d_vec3f_subv(a3d_vec3f_t* self, const a3d_vec3f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	self->x -= v->x;
	self->y -= v->y;
	self->z -= v->z;
}

void a3d_vec3f_subv_copy(const a3d_vec3f_t* self, const a3d_vec3f_t* v, a3d_vec3f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->x - v->x;
	copy->y = self->y - v->y;
	copy->z = self->z - v->z;
}

void a3d_vec3f_mulv(a3d_vec3f_t* self, const a3d_vec3f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	self->x *= v->x;
	self->y *= v->y;
	self->z *= v->z;
}

void a3d_vec3f_mulv_copy(const a3d_vec3f_t* self, const a3d_vec3f_t* v, a3d_vec3f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->x * v->x;
	copy->y = self->y * v->y;
	copy->z = self->z * v->z;
}

void a3d_vec3f_muls(a3d_vec3f_t* self, GLfloat s)
{
	assert(self);
	LOGD("debug s=%f", s);

	self->x *= s;
	self->y *= s;
	self->z *= s;
}

void a3d_vec3f_muls_copy(const a3d_vec3f_t* self, GLfloat s, a3d_vec3f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug s=%f", s);

	copy->x = self->x * s;
	copy->y = self->y * s;
	copy->z = self->z * s;
}

void a3d_vec3f_normalize(a3d_vec3f_t* self)
{
	assert(self);
	LOGD("debug");

	GLfloat mag = a3d_vec3f_mag(self);
	a3d_vec3f_muls(self, 1.0f / mag);
}

void a3d_vec3f_normalize_copy(const a3d_vec3f_t* self, a3d_vec3f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	GLfloat mag = a3d_vec3f_mag(self);
	a3d_vec3f_muls_copy(self, 1.0f / mag, copy);
}

GLfloat a3d_vec3f_dot(const a3d_vec3f_t* a, const a3d_vec3f_t* b)
{
	assert(a);
	assert(b);
	LOGD("debug");

	return a->x*b->x + a->y*b->y + a->z*b->z;
}

void a3d_vec3f_cross(a3d_vec3f_t* self, const a3d_vec3f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	a3d_vec3f_t copy;
	a3d_vec3f_cross_copy(self, v, &copy);
	a3d_vec3f_copy(&copy, self);
}

void a3d_vec3f_cross_copy(const a3d_vec3f_t* self, const a3d_vec3f_t* v, a3d_vec3f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	const a3d_vec3f_t* a = self;
	const a3d_vec3f_t* b = v;
	copy->x =  (a->y*b->z) - (b->y*a->z);
	copy->y = -(a->x*b->z) + (b->x*a->z);
	copy->z =  (a->x*b->y) - (b->x*a->y);
}
