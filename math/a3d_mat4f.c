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

#include "a3d_mat4f.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_mat4f_projuv(a3d_vec4f_t* u, a3d_vec4f_t* v, a3d_vec4f_t* projuv)
{
	assert(u);
	assert(v);
	assert(projuv);
	LOGD("debug");

	GLfloat dotvu = a3d_vec4f_dot(v, u);
	GLfloat dotuu = a3d_vec4f_dot(u, u);
	a3d_vec4f_muls_copy(u, dotvu/dotuu, projuv);
}

/***********************************************************
* public                                                   *
***********************************************************/

// TODO - optimize special cases instead of using a3d_mat4f_mulm

void a3d_mat4f_identity(a3d_mat4f_t* self)
{
	assert(self);
	LOGD("debug");

	self->m00 = 1.0f;
	self->m01 = 0.0f;
	self->m02 = 0.0f;
	self->m03 = 0.0f;
	self->m10 = 0.0f;
	self->m11 = 1.0f;
	self->m12 = 0.0f;
	self->m13 = 0.0f;
	self->m20 = 0.0f;
	self->m21 = 0.0f;
	self->m22 = 1.0f;
	self->m23 = 0.0f;
	self->m30 = 0.0f;
	self->m31 = 0.0f;
	self->m32 = 0.0f;
	self->m33 = 1.0f;
}

void a3d_mat4f_copy(const a3d_mat4f_t* self, a3d_mat4f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->m00 = self->m00;
	copy->m01 = self->m01;
	copy->m02 = self->m02;
	copy->m03 = self->m03;
	copy->m10 = self->m10;
	copy->m11 = self->m11;
	copy->m12 = self->m12;
	copy->m13 = self->m13;
	copy->m20 = self->m20;
	copy->m21 = self->m21;
	copy->m22 = self->m22;
	copy->m23 = self->m23;
	copy->m30 = self->m30;
	copy->m31 = self->m31;
	copy->m32 = self->m32;
	copy->m33 = self->m33;
}

void a3d_mat4f_transpose(a3d_mat4f_t* self)
{
	assert(self);
	LOGD("debug");

	a3d_mat4f_t copy;
	a3d_mat4f_transpose_copy(self, &copy);
	a3d_mat4f_copy(&copy, self);
}

void a3d_mat4f_transpose_copy(const a3d_mat4f_t* self, a3d_mat4f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	copy->m00 = self->m00;
	copy->m01 = self->m10;
	copy->m02 = self->m20;
	copy->m03 = self->m30;
	copy->m10 = self->m01;
	copy->m11 = self->m11;
	copy->m12 = self->m21;
	copy->m13 = self->m31;
	copy->m20 = self->m02;
	copy->m21 = self->m12;
	copy->m22 = self->m22;
	copy->m23 = self->m32;
	copy->m30 = self->m03;
	copy->m31 = self->m13;
	copy->m32 = self->m23;
	copy->m33 = self->m33;
}

void a3d_mat4f_inverse(a3d_mat4f_t* self)
{
	assert(self);
	LOGD("debug");

	a3d_mat4f_t copy;
	a3d_mat4f_inverse_copy(self, &copy);
	a3d_mat4f_copy(&copy, self);
}

void a3d_mat4f_inverse_copy(const a3d_mat4f_t* self, a3d_mat4f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	// augmented matrix [a|I]
	// copy is initialized to I but will contain a^-1
	a3d_mat4f_t a;
	a3d_mat4f_copy(self, &a);
	a3d_mat4f_identity(copy);

	// make shortcuts to access a and inverse-v (aka copy)
	GLfloat* aref = (GLfloat*) &a;
	GLfloat* vref = (GLfloat*) copy;
	#define A(row, col) aref[(row) + 4*(col)]
	#define V(row, col) vref[(row) + 4*(col)]

	// perform gauss-jordan elimination to determine inverse of a
	int i;   // rows
	int j;   // pivotal entry
	int l;   // largest magnitude pivotal entry
	int k;   // columns
	GLfloat x[4];
	GLfloat s;
	for(j = 0; j < 4; ++j)
	{
		// find largest magnitude element in column-j where i >= j
		l = j;
		for(i = j + 1; i < 4; ++i)
		{
			if(fabs(A(i,j)) > fabs(A(l,j)))
				l = i;
		}

		// interchange row-l and row-j of a and v
		if(l != j)
		{
			for(k = 0; k < 4; ++k)
			{
				x[k] = A(j,k);
				A(j,k) = A(l,k);
				A(l,k) = x[k];
			}
			for(k = 0; k < 4; ++k)
			{
				x[k] = V(j,k);
				V(j,k) = V(l,k);
				V(l,k) = x[k];
			}
		}

		// use the row sum operation to ensure zeros appear
		// below the pivotal entry
		// skip j=3
		for(i = j + 1; i < 4; ++i)
		{
			s = A(i,j) / A(j,j);
			for(k = j + 1; k < 4; ++k)
				A(i,k) -= s*A(j,k);
			for(k = 0; k < 4; ++k)
				V(i,k) -= s*V(j,k);
			A(i,j) = 0.0f;
		}

		// force the pivotal entry to be one
		s = 1.0f / A(j,j);
		for(k = j + 1; k < 4; ++k)
			A(j,k) *= s;
		for(k = 0; k < 4; ++k)
			V(j,k) *= s;
		A(j,j) = 1.0f;
	}

	// force zeros above all leading coefficients
	// skip j=0
	for(j = 3; j > 0; --j)
	{
		for(i = j - 1; i >= 0; --i)
		{
			s = A(i,j);   // A(j,j) is 1.0f in this case
			for(k = j; k < 4; ++k)
				A(i,k) -= s*A(j,k);
			for(k = 0; k < 4; ++k)
				V(i,k) -= s*V(j,k);
		}
	}
}

void a3d_mat4f_mulm(a3d_mat4f_t* self, const a3d_mat4f_t* m)
{
	assert(self);
	assert(m);
	LOGD("debug");

	a3d_mat4f_t copy;
	a3d_mat4f_mulm_copy(self, m, &copy);
	a3d_mat4f_copy(&copy, self);
}

void a3d_mat4f_mulm_copy(const a3d_mat4f_t* self, const a3d_mat4f_t* m, a3d_mat4f_t* copy)
{
	assert(self);
	assert(m);
	assert(copy);
	LOGD("debug");

	copy->m00 = self->m00*m->m00 + self->m01*m->m10 + self->m02*m->m20 + self->m03*m->m30;
	copy->m01 = self->m00*m->m01 + self->m01*m->m11 + self->m02*m->m21 + self->m03*m->m31;
	copy->m02 = self->m00*m->m02 + self->m01*m->m12 + self->m02*m->m22 + self->m03*m->m32;
	copy->m03 = self->m00*m->m03 + self->m01*m->m13 + self->m02*m->m23 + self->m03*m->m33;
	copy->m10 = self->m10*m->m00 + self->m11*m->m10 + self->m12*m->m20 + self->m13*m->m30;
	copy->m11 = self->m10*m->m01 + self->m11*m->m11 + self->m12*m->m21 + self->m13*m->m31;
	copy->m12 = self->m10*m->m02 + self->m11*m->m12 + self->m12*m->m22 + self->m13*m->m32;
	copy->m13 = self->m10*m->m03 + self->m11*m->m13 + self->m12*m->m23 + self->m13*m->m33;
	copy->m20 = self->m20*m->m00 + self->m21*m->m10 + self->m22*m->m20 + self->m23*m->m30;
	copy->m21 = self->m20*m->m01 + self->m21*m->m11 + self->m22*m->m21 + self->m23*m->m31;
	copy->m22 = self->m20*m->m02 + self->m21*m->m12 + self->m22*m->m22 + self->m23*m->m32;
	copy->m23 = self->m20*m->m03 + self->m21*m->m13 + self->m22*m->m23 + self->m23*m->m33;
	copy->m30 = self->m30*m->m00 + self->m31*m->m10 + self->m32*m->m20 + self->m33*m->m30;
	copy->m31 = self->m30*m->m01 + self->m31*m->m11 + self->m32*m->m21 + self->m33*m->m31;
	copy->m32 = self->m30*m->m02 + self->m31*m->m12 + self->m32*m->m22 + self->m33*m->m32;
	copy->m33 = self->m30*m->m03 + self->m31*m->m13 + self->m32*m->m23 + self->m33*m->m33;
}

void a3d_mat4f_mulv(const a3d_mat4f_t* self, a3d_vec4f_t* v)
{
	assert(self);
	assert(v);
	LOGD("debug");

	a3d_vec4f_t copy;
	a3d_mat4f_mulv_copy(self, v, &copy);
	a3d_vec4f_copy(&copy, v);
}

void a3d_mat4f_mulv_copy(const a3d_mat4f_t* self, const a3d_vec4f_t* v, a3d_vec4f_t* copy)
{
	assert(self);
	assert(v);
	assert(copy);
	LOGD("debug");

	copy->x = self->m00*v->x + self->m01*v->y + self->m02*v->z + self->m03*v->w;
	copy->y = self->m10*v->x + self->m11*v->y + self->m12*v->z + self->m13*v->w;
	copy->z = self->m20*v->x + self->m21*v->y + self->m22*v->z + self->m23*v->w;
	copy->w = self->m30*v->x + self->m31*v->y + self->m32*v->z + self->m33*v->w;
}

void a3d_mat4f_muls(a3d_mat4f_t* self, GLfloat s)
{
	assert(self);
	LOGD("debug s=%f", s);

	self->m00 *= s;
	self->m01 *= s;
	self->m02 *= s;
	self->m03 *= s;
	self->m10 *= s;
	self->m11 *= s;
	self->m12 *= s;
	self->m13 *= s;
	self->m20 *= s;
	self->m21 *= s;
	self->m22 *= s;
	self->m23 *= s;
	self->m30 *= s;
	self->m31 *= s;
	self->m32 *= s;
	self->m33 *= s;
}

void a3d_mat4f_muls_copy(const a3d_mat4f_t* self, GLfloat s, a3d_mat4f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug s=%f", s);

	copy->m00 = self->m00 * s;
	copy->m01 = self->m01 * s;
	copy->m02 = self->m02 * s;
	copy->m03 = self->m03 * s;
	copy->m10 = self->m10 * s;
	copy->m11 = self->m11 * s;
	copy->m12 = self->m12 * s;
	copy->m13 = self->m13 * s;
	copy->m20 = self->m20 * s;
	copy->m21 = self->m21 * s;
	copy->m22 = self->m22 * s;
	copy->m23 = self->m23 * s;
	copy->m30 = self->m30 * s;
	copy->m31 = self->m31 * s;
	copy->m32 = self->m32 * s;
	copy->m33 = self->m33 * s;
}

void a3d_mat4f_addm(a3d_mat4f_t* self, const a3d_mat4f_t* m)
{
	assert(self);
	assert(m);
	LOGD("debug");

	self->m00 += m->m00;
	self->m01 += m->m01;
	self->m02 += m->m02;
	self->m03 += m->m03;
	self->m10 += m->m10;
	self->m11 += m->m11;
	self->m12 += m->m12;
	self->m13 += m->m13;
	self->m20 += m->m20;
	self->m21 += m->m21;
	self->m22 += m->m22;
	self->m23 += m->m23;
	self->m30 += m->m30;
	self->m31 += m->m31;
	self->m32 += m->m32;
	self->m33 += m->m33;
}

void a3d_mat4f_addm_copy(const a3d_mat4f_t* self, const a3d_mat4f_t* m, a3d_mat4f_t* copy)
{
	assert(self);
	assert(m);
	assert(copy);
	LOGD("debug");

	copy->m00 = self->m00 + m->m00;
	copy->m01 = self->m01 + m->m01;
	copy->m02 = self->m02 + m->m02;
	copy->m03 = self->m03 + m->m03;
	copy->m10 = self->m10 + m->m10;
	copy->m11 = self->m11 + m->m11;
	copy->m12 = self->m12 + m->m12;
	copy->m13 = self->m13 + m->m13;
	copy->m20 = self->m20 + m->m20;
	copy->m21 = self->m21 + m->m21;
	copy->m22 = self->m22 + m->m22;
	copy->m23 = self->m23 + m->m23;
	copy->m30 = self->m30 + m->m30;
	copy->m31 = self->m31 + m->m31;
	copy->m32 = self->m32 + m->m32;
	copy->m33 = self->m33 + m->m33;
}

void a3d_mat4f_orthonormal(a3d_mat4f_t* self)
{
	assert(self);
	LOGD("debug");

	a3d_mat4f_t copy;
	a3d_mat4f_orthonormal_copy(self, &copy);
	*self = copy;
}

void a3d_mat4f_orthonormal_copy(const a3d_mat4f_t* self, a3d_mat4f_t* copy)
{
	assert(self);
	assert(copy);
	LOGD("debug");

	/*
	 * perform modified Gram-Schmitt ortho-normalization
	 */

	a3d_vec4f_t v0 = { .x = self->m00, .y = self->m01, .z = self->m02, .w = self->m03 };
	a3d_vec4f_t v1 = { .x = self->m10, .y = self->m11, .z = self->m12, .w = self->m13 };
	a3d_vec4f_t v2 = { .x = self->m20, .y = self->m21, .z = self->m22, .w = self->m23 };
	a3d_vec4f_t v3 = { .x = self->m30, .y = self->m31, .z = self->m32, .w = self->m33 };

	// normalize u0
	a3d_vec4f_t u0;
	a3d_vec4f_normalize_copy(&v0, &u0);

	// subtract the component of v1 in the direction of u0
	// normalize u1
	a3d_vec4f_t u1;
	a3d_vec4f_t projuv;
	a3d_mat4f_projuv(&u0, &v1, &projuv);
	a3d_vec4f_subv_copy(&v1, &projuv, &u1);
	a3d_vec4f_normalize(&u1);

	// subtract the component of v2 in the direction of u1
	// subtract the component of u2 in the direction of u0
	// normalize u2
	a3d_vec4f_t u2;
	a3d_mat4f_projuv(&u1, &v2, &projuv);
	a3d_vec4f_subv_copy(&v2, &projuv, &u2);
	a3d_mat4f_projuv(&u0, &u2, &projuv);
	a3d_vec4f_subv(&u2, &projuv);
	a3d_vec4f_normalize(&u2);

	// subtract the component of v3 in the direction of u2
	// subtract the component of u3 in the direction of u1
	// subtract the component of u3 in the direction of u0
	// normalize u3
	a3d_vec4f_t u3;
	a3d_mat4f_projuv(&u2, &v3, &projuv);
	a3d_vec4f_subv_copy(&v3, &projuv, &u3);
	a3d_mat4f_projuv(&u1, &u3, &projuv);
	a3d_vec4f_subv(&u3, &projuv);
	a3d_mat4f_projuv(&u0, &u3, &projuv);
	a3d_vec4f_subv(&u3, &projuv);
	a3d_vec4f_normalize(&u3);

	// copy the orthonormal vectors
	copy->m00 = u0.x;
	copy->m01 = u0.y;
	copy->m02 = u0.z;
	copy->m03 = u0.w;
	copy->m10 = u1.x;
	copy->m11 = u1.y;
	copy->m12 = u1.z;
	copy->m13 = u1.w;
	copy->m20 = u2.x;
	copy->m21 = u2.y;
	copy->m22 = u2.z;
	copy->m23 = u2.w;
	copy->m30 = u3.x;
	copy->m31 = u3.y;
	copy->m32 = u3.z;
	copy->m33 = u3.w;
}

/*
 * quaternion operations
 */

void a3d_mat4f_rotateq(a3d_mat4f_t* self, int load,
                       const a3d_quaternion_t* q)
{
	assert(self);
	assert(q);
	LOGD("debug load=%i", load);

	float x2 = q->v.x*q->v.x;
	float y2 = q->v.y*q->v.y;
	float z2 = q->v.z*q->v.z;
	float xy = q->v.x*q->v.y;
	float xz = q->v.x*q->v.z;
	float yz = q->v.y*q->v.z;
	float xw = q->v.x*q->s;
	float yw = q->v.y*q->s;
	float zw = q->v.z*q->s;

	// requires normalized quaternions
	a3d_mat4f_t m =
	{
		1.0f - 2.0f*(y2 + z2), 2.0f*(xy - zw), 2.0f*(xz + yw), 0.0f,
		2.0f*(xy + zw), 1.0f - 2.0f*(x2 + z2), 2.0f*(yz - xw), 0.0f,
		2.0f*(xz - yw), 2.0f*(yz + xw), 1.0f - 2.0f*(x2 + y2), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	if(load)
		a3d_mat4f_copy(&m, self);
	else
		a3d_mat4f_mulm(self, &m);
}

/*
 * GL matrix operations
 */

void a3d_mat4f_lookat(a3d_mat4f_t* self, int load,
                      GLfloat eyex, GLfloat eyey, GLfloat eyez,
                      GLfloat centerx, GLfloat centery, GLfloat centerz,
                      GLfloat upx, GLfloat upy, GLfloat upz)
{
	assert(self);
	LOGD("debug load=%i, eye=(%f,%f,%f), center=(%f,%f,%f), up=(%f,%f,%f)",
	     load, eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);

	a3d_vec3f_t eye =
	{
		eyex, eyey, eyez
	};

	a3d_vec3f_t center =
	{
		centerx, centery, centerz
	};

	a3d_vec3f_t up =
	{
		upx, upy, upz
	};

	a3d_vec3f_t n;
	a3d_vec3f_subv_copy(&center, &eye, &n);
	a3d_vec3f_normalize(&n);
	a3d_vec3f_normalize(&up);

	a3d_vec3f_t u;
	a3d_vec3f_t v;
	a3d_vec3f_cross_copy(&n, &up, &u);
	a3d_vec3f_cross_copy(&u, &n, &v);
	a3d_vec3f_normalize(&u);
	a3d_vec3f_normalize(&v);

	a3d_mat4f_t m =
	{
		 u.x,  v.x, -n.x, 0.0f,
		 u.y,  v.y, -n.y, 0.0f,
		 u.z,  v.z, -n.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	a3d_mat4f_translate(&m, 0, -eye.x, -eye.y, -eye.z);

	if(load)
		a3d_mat4f_copy(&m, self);
	else
		a3d_mat4f_mulm(self, &m);
}

void a3d_mat4f_perspective(a3d_mat4f_t* self, int load,
                           GLfloat fovy, GLfloat aspect,
                           GLfloat znear, GLfloat zfar)
{
	assert(self);
	LOGD("debug load=%i, fovy=%f, aspect=%f, znear=%f, zfar=%f",
	     load, fovy, aspect, znear, zfar);

	GLfloat f   = 1.0f/tanf(fovy*(M_PI/180.0f)/2.0f);
	GLfloat m00 = f/aspect;
	GLfloat m11 = f;
	GLfloat m22 = (zfar + znear)/(znear - zfar);
	GLfloat m23 = (2.0f*zfar*znear)/(znear - zfar);

	a3d_mat4f_t m =
	{
		 m00, 0.0f, 0.0f,  0.0f,
		0.0f,  m11, 0.0f,  0.0f,
		0.0f, 0.0f,  m22, -1.0f,
		0.0f, 0.0f,  m23,  0.0f,
	};

	if(load)
		a3d_mat4f_copy(&m, self);
	else
		a3d_mat4f_mulm(self, &m);
}

void a3d_mat4f_perspectiveStereo(a3d_mat4f_t* pmL,
                                 a3d_mat4f_t* pmR,
                                 int load,
                                 GLfloat fovy, GLfloat aspect,
                                 GLfloat znear, GLfloat zfar,
                                 GLfloat convergence,
                                 GLfloat eye_separation)
{
	assert(pmL);
	assert(pmR);

	// http://www.animesh.me/2011/05/rendering-3d-anaglyph-in-opengl.html

	GLfloat tan_fovy2 = tanf(fovy*(M_PI/180.0f)/2.0f);
	GLfloat es2       = eye_separation/2.0f;
	GLfloat top       = znear*tan_fovy2;
	GLfloat bottom    = -top;
	GLfloat a         = aspect*tan_fovy2*convergence;
	GLfloat b         = a - es2;
	GLfloat c         = a + es2;
	GLfloat d         = znear/convergence;
	GLfloat left      = -b*d;
	GLfloat right     =  c*d;

	// left perspective matrix
	a3d_mat4f_frustum(pmL, load,
	                  left, right,
	                  bottom, top,
	                  znear, zfar);

	// right perspective matrix
	left  = -c*d;
	right =  b*d;
	a3d_mat4f_frustum(pmR, load,
	                  left, right,
	                  bottom, top,
	                  znear, zfar);
}

void a3d_mat4f_rotate(a3d_mat4f_t* self, int load,
                      GLfloat a,
                      GLfloat x, GLfloat y, GLfloat z)
{
	assert(self);
	LOGD("debug a=%f, x=%f, y=%f, z=%f", a, x, y, z);

	// normalize x, y, z
	GLfloat n = x*x + y*y + z*z;
	if(n != 1.0f)
	{
		n = 1.0f / sqrtf(n);
		x *= n;
		y *= n;
		z *= n;
	}

	// from http://www.manpagez.com/man/3/glRotatef/
	GLfloat c = cosf(M_PI * a / 180.0f);
	GLfloat s = sinf(M_PI * a / 180.0f);
	GLfloat p = 1.0f - c;
	GLfloat xxp = x*x*p;
	GLfloat xyp = x*y*p;
	GLfloat xzp = x*z*p;
	GLfloat yyp = y*y*p;
	GLfloat yzp = y*z*p;
	GLfloat zzp = z*z*p;
	GLfloat xs  = x * s;
	GLfloat ys  = y * s;
	GLfloat zs  = z * s;

	a3d_mat4f_t m =
	{
		 xxp + c, xyp + zs, xzp - ys, 0.0f,
		xyp - zs,  yyp + c, yzp + xs, 0.0f,
		xzp + ys, yzp - xs,  zzp + c, 0.0f,
		    0.0f,     0.0f,     0.0f, 1.0f,
	};

	if(load)
		a3d_mat4f_copy(&m, self);
	else
		a3d_mat4f_mulm(self, &m);
}

void a3d_mat4f_translate(a3d_mat4f_t* self, int load,
                         GLfloat x, GLfloat y, GLfloat z)
{
	assert(self);
	LOGD("debug x=%f, y=%f, z=%f", x, y, z);

	a3d_mat4f_t m =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		   x,    y,    z, 1.0f,
	};

	if(load)
		a3d_mat4f_copy(&m, self);
	else
		a3d_mat4f_mulm(self, &m);
}

void a3d_mat4f_scale(a3d_mat4f_t* self, int load,
                     GLfloat x, GLfloat y, GLfloat z)
{
	assert(self);
	LOGD("debug x=%f, y=%f, z=%f", x, y, z);

	a3d_mat4f_t m =
	{
		   x, 0.0f, 0.0f, 0.0f,
		0.0f,    y, 0.0f, 0.0f,
		0.0f, 0.0f,    z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	if(load)
		a3d_mat4f_copy(&m, self);
	else
		a3d_mat4f_mulm(self, &m);
}

void a3d_mat4f_frustum(a3d_mat4f_t* self, int load,
                       GLfloat l, GLfloat r,
                       GLfloat b, GLfloat t,
                       GLfloat n, GLfloat f)
{
	assert(self);
	LOGD("debug l=%f, r=%f, t=%f, b=%f, n=%f, f=%f", l, r, t, b, n, f);

	if((n <= 0.0f) || (f <= 0.0f) ||
	   (l == r) || (t == b) || (n == f))
	{
		LOGE("invalid l=%f, r=%f, t=%f, b=%f, n=%f, f=%f", l, r, t, b, n, f);
		return;
	}

	GLfloat n2 = 2.0f * n;
	GLfloat rml = r - l;
	GLfloat rpl = r + l;
	GLfloat tmb = t - b;
	GLfloat tpb = t + b;
	GLfloat fmn = f - n;
	GLfloat fpn = f + n;
	a3d_mat4f_t m =
	{
		 n2/rml,    0.0f,      0.0f,  0.0f,
		   0.0f,  n2/tmb,      0.0f,  0.0f,
		rpl/rml, tpb/tmb,  -fpn/fmn, -1.0f,
		   0.0f,    0.0f, -n2*f/fmn,  0.0f,
	};

	if(load)
		a3d_mat4f_copy(&m, self);
	else
		a3d_mat4f_mulm(self, &m);
}

void a3d_mat4f_ortho(a3d_mat4f_t* self, int load,
                     GLfloat l, GLfloat r,
                     GLfloat b, GLfloat t,
                     GLfloat n, GLfloat f)
{
	assert(self);
	LOGD("debug l=%f, r=%f, t=%f, b=%f, n=%f, f=%f", l, r, t, b, n, f);

	if((l == r) || (t == b) || (n == f))
	{
		LOGE("invalid l=%f, r=%f, t=%f, b=%f, n=%f, f=%f", l, r, t, b, n, f);
		return;
	}

	GLfloat rml = r - l;
	GLfloat rpl = r + l;
	GLfloat tmb = t - b;
	GLfloat tpb = t + b;
	GLfloat fmn = f - n;
	GLfloat fpn = f + n;
	a3d_mat4f_t m =
	{
		2.0f/rml,     0.0f,      0.0f, 0.0f,
		    0.0f, 2.0f/tmb,      0.0f, 0.0f,
		    0.0f,     0.0f, -2.0f/fmn, 0.0f,
		-rpl/rml, -tpb/tmb,  -fpn/fmn, 1.0f,
	};

	if(load)
		a3d_mat4f_copy(&m, self);
	else
		a3d_mat4f_mulm(self, &m);
}

void a3d_mat4f_normalmatrix(const a3d_mat4f_t* self, a3d_mat3f_t* nm)
{
	assert(self);
	assert(nm);
	LOGD("debug");

	// see link for the derivation of normal matrix
	// http://www.lighthouse3d.com/opengl/glsl/index.php?normalmatrix

	// use top-left 3x3 sub-region of the matrix
	nm->m00 = self->m00;
	nm->m01 = self->m01;
	nm->m02 = self->m02;
	nm->m10 = self->m10;
	nm->m11 = self->m11;
	nm->m12 = self->m12;
	nm->m20 = self->m20;
	nm->m21 = self->m21;
	nm->m22 = self->m22;

	// solve nm = (m^-1)^T
	a3d_mat3f_inverse(nm);
	a3d_mat3f_transpose(nm);
}
