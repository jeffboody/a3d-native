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

#include "a3d_mat3f.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* public                                                   *
***********************************************************/

void a3d_mat3f_identity(a3d_mat3f_t* self)
{
	assert(self);
	LOGD("debug");

	self->m00 = 1.0f;
	self->m01 = 0.0f;
	self->m02 = 0.0f;
	self->m10 = 0.0f;
	self->m11 = 1.0f;
	self->m12 = 0.0f;
	self->m20 = 0.0f;
	self->m21 = 0.0f;
	self->m22 = 1.0f;
}

void a3d_mat3f_copy(const a3d_mat3f_t* self, a3d_mat3f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->m00 = self->m00;
	copy->m01 = self->m01;
	copy->m02 = self->m02;
	copy->m10 = self->m10;
	copy->m11 = self->m11;
	copy->m12 = self->m12;
	copy->m20 = self->m20;
	copy->m21 = self->m21;
	copy->m22 = self->m22;
}

void a3d_mat3f_transpose(a3d_mat3f_t* self)
{
	assert(self);
	LOGD("debug");

	a3d_mat3f_t copy;
	a3d_mat3f_transpose_copy(self, &copy);
	a3d_mat3f_copy(&copy, self);
}

void a3d_mat3f_transpose_copy(const a3d_mat3f_t* self, a3d_mat3f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->m00 = self->m00;
	copy->m01 = self->m10;
	copy->m02 = self->m20;
	copy->m10 = self->m01;
	copy->m11 = self->m11;
	copy->m12 = self->m21;
	copy->m20 = self->m02;
	copy->m21 = self->m12;
	copy->m22 = self->m22;
}

void a3d_mat3f_inverse(a3d_mat3f_t* self)
{
	assert(self);
	LOGD("debug");

	a3d_mat3f_t copy;
	a3d_mat3f_inverse_copy(self, &copy);
	a3d_mat3f_copy(&copy, self);
}

void a3d_mat3f_inverse_copy(const a3d_mat3f_t* self, a3d_mat3f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	// augmented matrix [a|I]
	// copy is initialized to I but will contain a^-1
	a3d_mat3f_t a;
	a3d_mat3f_copy(self, &a);
	a3d_mat3f_identity(copy);

	// make shortcuts to access a and inverse-v (aka copy)
	GLfloat* aref = (GLfloat*) &a;
	GLfloat* vref = (GLfloat*) copy;
	#define A(row, col) aref[(row) + 3*(col)]
	#define V(row, col) vref[(row) + 3*(col)]

	// perform gauss-jordan elimination to determine inverse of a
	int i;   // rows
	int j;   // pivotal entry
	int l;   // largest magnitude pivotal entry
	int k;   // columns
	GLfloat x[3];
	GLfloat s;
	for(j = 0; j < 3; ++j)
	{
		// find largest magnitude element in column-j where i >= j
		l = j;
		for(i = j + 1; i < 3; ++i)
		{
			if(fabs(A(i,j)) > fabs(A(l,j)))
				l = i;
		}

		// interchange row-l and row-j of a and v
		if(l != j)
		{
			for(k = 0; k < 3; ++k)
			{
				x[k] = A(j,k);
				A(j,k) = A(l,k);
				A(l,k) = x[k];
			}
			for(k = 0; k < 3; ++k)
			{
				x[k] = V(j,k);
				V(j,k) = V(l,k);
				V(l,k) = x[k];
			}
		}

		// use the row sum operation to ensure zeros appear
		// below the pivotal entry
		// skip j=2
		for(i = j + 1; i < 3; ++i)
		{
			s = A(i,j) / A(j,j);
			for(k = j + 1; k < 3; ++k)
				A(i,k) -= s*A(j,k);
			for(k = 0; k < 3; ++k)
				V(i,k) -= s*V(j,k);
			A(i,j) = 0.0f;
		}

		// force the pivotal entry to be one
		s = 1.0f / A(j,j);
		for(k = j + 1; k < 3; ++k)
			A(j,k) *= s;
		for(k = 0; k < 3; ++k)
			V(j,k) *= s;
		A(j,j) = 1.0f;
	}

	// force zeros above all leading coefficients
	// skip j=0
	for(j = 2; j > 0; --j)
	{
		for(i = j - 1; i >= 0; --i)
		{
			s = A(i,j);   // A(j,j) is 1.0f in this case
			for(k = j; k < 3; ++k)
				A(i,k) -= s*A(j,k);
			for(k = 0; k < 3; ++k)
				V(i,k) -= s*V(j,k);
		}
	}
}

void a3d_mat3f_mulm(a3d_mat3f_t* self, const a3d_mat3f_t* m)
{
	assert(self);
	assert(m);
	LOGD("debug");

	a3d_mat3f_t copy;
	a3d_mat3f_mulm_copy(self, m, &copy);
	a3d_mat3f_copy(&copy, self);
}

void a3d_mat3f_mulm_copy(const a3d_mat3f_t* self, const a3d_mat3f_t* m, a3d_mat3f_t* copy)
{
	assert(self);
	assert(m);
	assert(copy);
	LOGD("debug");

	copy->m00 = self->m00*m->m00 + self->m01*m->m10 + self->m02*m->m20;
	copy->m01 = self->m00*m->m01 + self->m01*m->m11 + self->m02*m->m21;
	copy->m02 = self->m00*m->m02 + self->m01*m->m12 + self->m02*m->m22;
	copy->m10 = self->m10*m->m00 + self->m11*m->m10 + self->m12*m->m20;
	copy->m11 = self->m10*m->m01 + self->m11*m->m11 + self->m12*m->m21;
	copy->m12 = self->m10*m->m02 + self->m11*m->m12 + self->m12*m->m22;
	copy->m20 = self->m20*m->m00 + self->m21*m->m10 + self->m22*m->m20;
	copy->m21 = self->m20*m->m01 + self->m21*m->m11 + self->m22*m->m21;
	copy->m22 = self->m20*m->m02 + self->m21*m->m12 + self->m22*m->m22;
}

void a3d_mat3f_mulv(const a3d_mat3f_t* self, a3d_vec3f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	a3d_vec3f_t copy;
	a3d_mat3f_mulv_copy(self, v, &copy);
	a3d_vec3f_copy(&copy, v);
}

void a3d_mat3f_mulv_copy(const a3d_mat3f_t* self, const a3d_vec3f_t* v, a3d_vec3f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->m00*v->x + self->m01*v->y + self->m02*v->z;
	copy->y = self->m10*v->x + self->m11*v->y + self->m12*v->z;
	copy->z = self->m20*v->x + self->m21*v->y + self->m22*v->z;
}

void a3d_mat3f_muls(a3d_mat3f_t* self, GLfloat s)
{
	assert(self);
	LOGD("debug s=%f", s);

	self->m00 *= s;
	self->m01 *= s;
	self->m02 *= s;
	self->m10 *= s;
	self->m11 *= s;
	self->m12 *= s;
	self->m20 *= s;
	self->m21 *= s;
	self->m22 *= s;
}

void a3d_mat3f_muls_copy(const a3d_mat3f_t* self, GLfloat s, a3d_mat3f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug s=%f", s);

	copy->m00 = self->m00 * s;
	copy->m01 = self->m01 * s;
	copy->m02 = self->m02 * s;
	copy->m10 = self->m10 * s;
	copy->m11 = self->m11 * s;
	copy->m12 = self->m12 * s;
	copy->m20 = self->m20 * s;
	copy->m21 = self->m21 * s;
	copy->m22 = self->m22 * s;
}
