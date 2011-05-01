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

#include "a3d_vec4f.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* public                                                   *
***********************************************************/

void a3d_vec4f_load(a3d_vec4f_t* self, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	assert(self);
	LOGD("debug x=%f, y=%f, z=%f, w=%f", x, y, z, w);

	self->x = x;
	self->y = y;
	self->z = z;
	self->w = w;
}

void a3d_vec4f_copy(const a3d_vec4f_t* self, a3d_vec4f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->x = self->x;
	copy->y = self->y;
	copy->z = self->z;
	copy->w = self->w;
}

GLfloat a3d_vec4f_mag(const a3d_vec4f_t* self)
{
	assert(self);
	LOGD("debug");

	return sqrtf(self->x*self->x +
	             self->y*self->y +
	             self->z*self->z +
	             self->w*self->w);
}

void a3d_vec4f_addv(a3d_vec4f_t* self, const a3d_vec4f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	self->x += v->x;
	self->y += v->y;
	self->z += v->z;
	self->w += v->w;
}

void a3d_vec4f_addv_copy(const a3d_vec4f_t* self, const a3d_vec4f_t* v, a3d_vec4f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->x + v->x;
	copy->y = self->y + v->y;
	copy->z = self->z + v->z;
	copy->w = self->w + v->w;
}

void a3d_vec4f_adds(a3d_vec4f_t* self, GLfloat s)
{
	assert(self);
	assert(v);
	LOGD("debug");

	self->x += s;
	self->y += s;
	self->z += s;
	self->w += s;
}

void a3d_vec4f_adds_copy(const a3d_vec4f_t* self, GLfloat s, a3d_vec4f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->x = self->x + s;
	copy->y = self->y + s;
	copy->z = self->z + s;
	copy->w = self->w + s;
}

void a3d_vec4f_subv(a3d_vec4f_t* self, const a3d_vec4f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	self->x -= v->x;
	self->y -= v->y;
	self->z -= v->z;
	self->w -= v->w;
}

void a3d_vec4f_subv_copy(const a3d_vec4f_t* self, const a3d_vec4f_t* v, a3d_vec4f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->x - v->x;
	copy->y = self->y - v->y;
	copy->z = self->z - v->z;
	copy->w = self->w - v->w;
}

void a3d_vec4f_mulv(a3d_vec4f_t* self, const a3d_vec4f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	self->x *= v->x;
	self->y *= v->y;
	self->z *= v->z;
	self->w *= v->w;
}

void a3d_vec4f_mulv_copy(const a3d_vec4f_t* self, const a3d_vec4f_t* v, a3d_vec4f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->x * v->x;
	copy->y = self->y * v->y;
	copy->z = self->z * v->z;
	copy->w = self->w * v->w;
}

void a3d_vec4f_muls(a3d_vec4f_t* self, GLfloat s)
{
	assert(self);
	LOGD("debug s=%f", s);

	self->x *= s;
	self->y *= s;
	self->z *= s;
	self->w *= s;
}

void a3d_vec4f_muls_copy(const a3d_vec4f_t* self, GLfloat s, a3d_vec4f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug s=%f", s);

	copy->x = self->x * s;
	copy->y = self->y * s;
	copy->z = self->z * s;
	copy->w = self->w * s;
}

void a3d_vec4f_normalize(a3d_vec4f_t* self)
{
	assert(self);
	LOGD("debug");

	GLfloat mag = a3d_vec4f_mag(self);
	a3d_vec4f_muls(self, 1.0f / mag);
}

void a3d_vec4f_normalize_copy(const a3d_vec4f_t* self, a3d_vec4f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	GLfloat mag = a3d_vec4f_mag(self);
	a3d_vec4f_muls_copy(self, 1.0f / mag, copy);
}

GLfloat a3d_vec4f_dot(const a3d_vec4f_t* a, const a3d_vec4f_t* b)
{
	assert(a);
	assert(b);
	LOGD("debug");

	return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
}
