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

#include "a3d_vec2f.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* public                                                   *
***********************************************************/

void a3d_vec2f_load(a3d_vec2f_t* self, GLfloat x, GLfloat y)
{
	assert(self);
	LOGD("debug x=%f, y=%f", x, y);

	self->x = x;
	self->y = y;
}

void a3d_vec2f_copy(const a3d_vec2f_t* self, a3d_vec2f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->x = self->x;
	copy->y = self->y;
}

GLfloat a3d_vec2f_mag(const a3d_vec2f_t* self)
{
	assert(self);
	LOGD("debug");

	return sqrtf(self->x*self->x +
	             self->y*self->y);
}

void a3d_vec2f_addv(a3d_vec2f_t* self, const a3d_vec2f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	self->x += v->x;
	self->y += v->y;
}

void a3d_vec2f_addv_copy(const a3d_vec2f_t* self, const a3d_vec2f_t* v, a3d_vec2f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->x + v->x;
	copy->y = self->y + v->y;
}

void a3d_vec2f_adds(a3d_vec2f_t* self, GLfloat s)
{
	assert(self);
	LOGD("debug");

	self->x += s;
	self->y += s;
}

void a3d_vec2f_adds_copy(const a3d_vec2f_t* self, GLfloat s, a3d_vec2f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->x = self->x + s;
	copy->y = self->y + s;
}

void a3d_vec2f_subv(a3d_vec2f_t* self, const a3d_vec2f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	self->x -= v->x;
	self->y -= v->y;
}

void a3d_vec2f_subv_copy(const a3d_vec2f_t* self, const a3d_vec2f_t* v, a3d_vec2f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->x - v->x;
	copy->y = self->y - v->y;
}

void a3d_vec2f_mulv(a3d_vec2f_t* self, const a3d_vec2f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	self->x *= v->x;
	self->y *= v->y;
}

void a3d_vec2f_mulv_copy(const a3d_vec2f_t* self, const a3d_vec2f_t* v, a3d_vec2f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->x * v->x;
	copy->y = self->y * v->y;
}

void a3d_vec2f_muls(a3d_vec2f_t* self, GLfloat s)
{
	assert(self);
	LOGD("debug s=%f", s);

	self->x *= s;
	self->y *= s;
}

void a3d_vec2f_muls_copy(const a3d_vec2f_t* self, GLfloat s, a3d_vec2f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug s=%f", s);

	copy->x = self->x * s;
	copy->y = self->y * s;
}

void a3d_vec2f_normalize(a3d_vec2f_t* self)
{
	assert(self);
	LOGD("debug");

	GLfloat mag = a3d_vec2f_mag(self);
	a3d_vec2f_muls(self, 1.0f / mag);
}

void a3d_vec2f_normalize_copy(const a3d_vec2f_t* self, a3d_vec2f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	GLfloat mag = a3d_vec2f_mag(self);
	a3d_vec2f_muls_copy(self, 1.0f / mag, copy);
}

GLfloat a3d_vec2f_dot(const a3d_vec2f_t* a, const a3d_vec2f_t* b)
{
	assert(a);
	assert(b);
	LOGD("debug");

	return a->x*b->x + a->y*b->y;
}

GLfloat a3d_vec2f_distance(const a3d_vec2f_t* a, const a3d_vec2f_t* b)
{
	assert(a);
	assert(b);
	LOGD("debug");

	a3d_vec2f_t v;
	a3d_vec2f_subv_copy(a, b, &v);
	return a3d_vec2f_mag(&v);
}

GLfloat a3d_vec2f_cross(const a3d_vec2f_t* a, const a3d_vec2f_t* b)
{
	assert(a);
	assert(b);
	LOGD("debug");

	return a->x*b->y - b->x*a->y;
}
