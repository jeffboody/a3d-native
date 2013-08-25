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

#ifndef a3d_mat4f_H
#define a3d_mat4f_H

#include "../a3d_GL.h"

#include "a3d_vec4f.h"
#include "a3d_mat3f.h"
#include "a3d_quaternion.h"

// note: can cast a3d_mat4f_t* to GLfloat* (column-major)

typedef struct
{
	GLfloat m00;
	GLfloat m10;
	GLfloat m20;
	GLfloat m30;
	GLfloat m01;
	GLfloat m11;
	GLfloat m21;
	GLfloat m31;
	GLfloat m02;
	GLfloat m12;
	GLfloat m22;
	GLfloat m32;
	GLfloat m03;
	GLfloat m13;
	GLfloat m23;
	GLfloat m33;
} a3d_mat4f_t;

// standard matrix operations
void a3d_mat4f_identity(a3d_mat4f_t* self);
void a3d_mat4f_copy(const a3d_mat4f_t* self, a3d_mat4f_t* copy);
void a3d_mat4f_transpose(a3d_mat4f_t* self);
void a3d_mat4f_transpose_copy(const a3d_mat4f_t* self, a3d_mat4f_t* copy);
void a3d_mat4f_inverse(a3d_mat4f_t* self);
void a3d_mat4f_inverse_copy(const a3d_mat4f_t* self, a3d_mat4f_t* copy);
void a3d_mat4f_mulm(a3d_mat4f_t* self, const a3d_mat4f_t* m);
void a3d_mat4f_mulm_copy(const a3d_mat4f_t* self, const a3d_mat4f_t* m, a3d_mat4f_t* copy);
void a3d_mat4f_mulv(const a3d_mat4f_t* self, a3d_vec4f_t* v);
void a3d_mat4f_mulv_copy(const a3d_mat4f_t* self, const a3d_vec4f_t* v, a3d_vec4f_t* copy);
void a3d_mat4f_muls(a3d_mat4f_t* self, GLfloat s);
void a3d_mat4f_muls_copy(const a3d_mat4f_t* self, GLfloat s, a3d_mat4f_t* copy);
void a3d_mat4f_addm(a3d_mat4f_t* self, const a3d_mat4f_t* m);
void a3d_mat4f_addm_copy(const a3d_mat4f_t* self, const a3d_mat4f_t* m, a3d_mat4f_t* copy);
void a3d_mat4f_orthonormal(a3d_mat4f_t* self);
void a3d_mat4f_orthonormal_copy(const a3d_mat4f_t* self, a3d_mat4f_t* copy);

// quaternion operations
void a3d_mat4f_rotateq(a3d_mat4f_t* self, int load,
                       const a3d_quaternion_t* q);

// GL matrix operations
void a3d_mat4f_lookat(a3d_mat4f_t* self, int load,
                      GLfloat eyex, GLfloat eyey, GLfloat eyez,
                      GLfloat centerx, GLfloat centery, GLfloat centerz,
                      GLfloat upx, GLfloat upy, GLfloat upz);
void a3d_mat4f_perspective(a3d_mat4f_t* self, int load,
                           GLfloat fovy, GLfloat aspect,
                           GLfloat znear, GLfloat zfar);
void a3d_mat4f_rotate(a3d_mat4f_t* self, int load,
                      GLfloat a,
                      GLfloat x, GLfloat y, GLfloat z);
void a3d_mat4f_translate(a3d_mat4f_t* self, int load,
                         GLfloat x, GLfloat y, GLfloat z);
void a3d_mat4f_scale(a3d_mat4f_t* self, int load,
                     GLfloat x, GLfloat y, GLfloat z);
void a3d_mat4f_frustum(a3d_mat4f_t* self, int load,
                       GLfloat l, GLfloat r,
                       GLfloat b, GLfloat t,
                       GLfloat n, GLfloat f);
void a3d_mat4f_ortho(a3d_mat4f_t* self, int load,
                     GLfloat l, GLfloat r,
                     GLfloat b, GLfloat t,
                     GLfloat n, GLfloat f);
void a3d_mat4f_normalmatrix(const a3d_mat4f_t* self, a3d_mat3f_t* nm);

#endif
