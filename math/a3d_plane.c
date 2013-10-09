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

#include "a3d_plane.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* public                                                   *
***********************************************************/

void a3d_plane_load(a3d_plane_t* self,
                    GLfloat nx, GLfloat ny, GLfloat nz,
                    GLfloat d)
{
	assert(self);
	LOGD("debug nx=%f, ny=%f, nz=%f, d=%f", nx, ny, nz, d);

	a3d_vec3f_load(&self->n, nx, ny, nz);
	self->d = d;
}

void a3d_plane_copy(const a3d_plane_t* self, a3d_plane_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	a3d_vec3f_copy(&self->n, &copy->n);
	copy->d = self->d;
}

GLfloat a3d_plane_distance(const a3d_plane_t* self, const a3d_vec3f_t* p)
{
	assert(self);
	assert(p);
	LOGD("debug");

	a3d_vec3f_t v;
	a3d_vec3f_t q;
	a3d_vec3f_muls_copy(&self->n, self->d, &q);
	a3d_vec3f_subv_copy(p, &q, &v);
	return a3d_vec3f_dot(&self->n, &v);
}

int a3d_plane_clipsphere(const a3d_plane_t* self, const a3d_sphere_t* s)
{
	assert(self);
	assert(s);
	LOGD("debug");

	GLfloat d = a3d_plane_distance(self, &s->c);
	return ((s->r + d) < 0.0f) ? 1 : 0;
}
