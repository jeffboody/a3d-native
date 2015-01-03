/*
 * Copyright (c) 2015 Jeff Boody
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

#include "a3d_rect4f.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* public                                                   *
***********************************************************/

void a3d_rect4f_init(a3d_rect4f_t* self,
                     float t, float l,
                     float w, float h)
{
	assert(self);
	LOGD("debug t=%f, l=%f, w=%f, h=%f",
	     t, l, w, h);

	self->t = t;
	self->l = l;
	self->w = w;
	self->h = h;
}

void a3d_rect4f_copy(const a3d_rect4f_t* self,
                     a3d_rect4f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->t = self->t;
	copy->l = self->l;
	copy->w = self->w;
	copy->h = self->h;
}

int a3d_rect4f_contains(const a3d_rect4f_t* self,
                        float x, float y)
{
	assert(self);
	LOGD("debug x=%f, y=%f", x, y);

	if((self->w <= 0.0f) || (self->h <= 0.0f) ||
	   (y < self->t) || (y >= (self->t + self->h)) ||
	   (x < self->l) || (x >= (self->l + self->w)))
	{
		return 0;
	}
	return 1;
}

int a3d_rect4f_intersect(const a3d_rect4f_t* ra,
                         const a3d_rect4f_t* rb,
                         a3d_rect4f_t* rc)
{
	assert(ra);
	assert(rb);
	assert(rc);
	LOGD("debug");

	// degenerate rectangles
	if((ra->w <= 0.0f) ||
	   (ra->h <= 0.0f) ||
	   (rb->w <= 0.0f) ||
	   (rb->h <= 0.0f))
	{
		return 0;
	}

	// check sides
	float ax0 = ra->l;
	float bx0 = rb->l;
	float ax1 = ra->l + ra->w - 1;
	float bx1 = rb->l + rb->w - 1;
	if((ax0 > bx1) || (bx0 > ax1))
	{
		return 0;
	}

	// check top/bottom
	float ay0 = ra->t;
	float by0 = rb->t;
	float ay1 = ra->t + ra->h - 1;
	float by1 = rb->t + rb->h - 1;
	if((ay0 > by1) || (by0 > ay1))
	{
		return 0;
	}

	// intersect
	float t = (ay0 > by0) ? ay0 : by0;
	float l = (ax0 > bx0) ? ax0 : bx0;
	float b = (ay1 < by1) ? ay1 : by1;
	float r = (ax1 < bx1) ? ax1 : bx1;

	rc->t = t;
	rc->l = l;
	rc->w = r - l + 1;
	rc->h = b - t + 1;
	return 1;
}
