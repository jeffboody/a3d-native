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

#include "a3d_ray.h"
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* public                                                   *
***********************************************************/

void a3d_ray_load(a3d_ray_t* self,
                  GLfloat px, GLfloat py, GLfloat pz,
                  GLfloat vx, GLfloat vy, GLfloat vz)
{
	assert(self);
	LOGD("px=%f, py=%f, pz=%f, vx=%f, vy=%f, vz=%f",
	     px, py, pz, vx, vy, vz);

	a3d_vec3f_load(&self->p, px, py, pz);
	a3d_vec3f_load(&self->v, vx, vy, vz);
}

int a3d_ray_hitsphere(const a3d_ray_t* self, const a3d_sphere_t* s)
{
	assert(self);
	assert(s);
	LOGD("debug");

	a3d_vec3f_t v;
	a3d_vec3f_t p;
	a3d_vec3f_subv_copy(&s->c, &self->p, &v);
	if(a3d_vec3f_mag(&v) <= s->r)
	{
		// ray origin inside sphere
		return 1;
	}

	GLfloat t = a3d_vec3f_dot(&self->v, &v);
	if(t < 0.0f)
	{
		// ray pointing in wrong direction
		return 0;
	}

	// distance to the nearest point on the ray
	a3d_vec3f_muls_copy(&self->v, t, &v);
	a3d_vec3f_addv_copy(&v, &self->p, &p);
	a3d_vec3f_subv_copy(&s->c, &p, &v);
	float d = a3d_vec3f_mag(&v);

	// does ray pass through sphere
	return (d < s->r) ? 1 : 0;
}
