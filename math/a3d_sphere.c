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

#include "a3d_sphere.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* public                                                   *
***********************************************************/

void a3d_sphere_load(a3d_sphere_t* self,
                     GLfloat cx, GLfloat cy, GLfloat cz,
                     GLfloat r)
{
	assert(self);
	LOGD("debug cx=%f, cy=%f, cz=%f, r=%f", cx, cy, cz, r);

	a3d_vec3f_load(&self->c, cx, cy, cz);
	self->r = r;
}

void a3d_sphere_copy(const a3d_sphere_t* self, a3d_sphere_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	a3d_vec3f_copy(&self->c, &copy->c);
	copy->r = self->r;
}
