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

#ifndef a3d_mat3f_H
#define a3d_mat3f_H

#include "../a3d_GL.h"

#include "a3d_vec3f.h"

// note: can cast a3d_mat3f_t* to GLfloat* (column-major)

typedef struct
{
	GLfloat m00;
	GLfloat m10;
	GLfloat m20;
	GLfloat m01;
	GLfloat m11;
	GLfloat m21;
	GLfloat m02;
	GLfloat m12;
	GLfloat m22;
} a3d_mat3f_t;

// standard matrix operations
void a3d_mat3f_identity(a3d_mat3f_t* self);
void a3d_mat3f_copy(const a3d_mat3f_t* self, a3d_mat3f_t* copy);
void a3d_mat3f_transpose(a3d_mat3f_t* self);
void a3d_mat3f_transpose_copy(const a3d_mat3f_t* self, a3d_mat3f_t* copy);
void a3d_mat3f_inverse(a3d_mat3f_t* self);
void a3d_mat3f_inverse_copy(const a3d_mat3f_t* self, a3d_mat3f_t* copy);
void a3d_mat3f_mulm(a3d_mat3f_t* self, const a3d_mat3f_t* m);
void a3d_mat3f_mulm_copy(const a3d_mat3f_t* self, const a3d_mat3f_t* m, a3d_mat3f_t* copy);
void a3d_mat3f_mulv(const a3d_mat3f_t* self, a3d_vec3f_t* v);
void a3d_mat3f_mulv_copy(const a3d_mat3f_t* self, const a3d_vec3f_t* v, a3d_vec3f_t* copy);
void a3d_mat3f_muls(a3d_mat3f_t* self, GLfloat s);
void a3d_mat3f_muls_copy(const a3d_mat3f_t* self, GLfloat s, a3d_mat3f_t* copy);

#endif
