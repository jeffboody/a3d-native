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

#ifndef a3d_vec3f_H
#define a3d_vec3f_H

#include "../a3d_GL.h"

// note: can cast a3d_vec3f_t* to GLfloat*

typedef struct
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
} a3d_vec3f_t;

// dynamic constructor/destructor
a3d_vec3f_t* a3d_vec3f_new(GLfloat x, GLfloat y, GLfloat z);
void         a3d_vec3f_delete(a3d_vec3f_t** _self);

// standard vector operations
void    a3d_vec3f_load(a3d_vec3f_t* self, GLfloat x, GLfloat y, GLfloat z);
void    a3d_vec3f_copy(const a3d_vec3f_t* self, a3d_vec3f_t* copy);
GLfloat a3d_vec3f_mag(const a3d_vec3f_t* self);
void    a3d_vec3f_addv(a3d_vec3f_t* self, const a3d_vec3f_t* v);
void    a3d_vec3f_addv_copy(const a3d_vec3f_t* self, const a3d_vec3f_t* v, a3d_vec3f_t* copy);
void    a3d_vec3f_adds(a3d_vec3f_t* self, GLfloat s);   // don't need subs
void    a3d_vec3f_adds_copy(const a3d_vec3f_t* self, GLfloat s, a3d_vec3f_t* copy);
void    a3d_vec3f_subv(a3d_vec3f_t* self, const a3d_vec3f_t* v);
void    a3d_vec3f_subv_copy(const a3d_vec3f_t* self, const a3d_vec3f_t* v, a3d_vec3f_t* copy);
void    a3d_vec3f_mulv(a3d_vec3f_t* self, const a3d_vec3f_t* v);
void    a3d_vec3f_mulv_copy(const a3d_vec3f_t* self, const a3d_vec3f_t* v, a3d_vec3f_t* copy);
void    a3d_vec3f_muls(a3d_vec3f_t* self, GLfloat s);
void    a3d_vec3f_muls_copy(const a3d_vec3f_t* self, GLfloat s, a3d_vec3f_t* copy);
void    a3d_vec3f_normalize(a3d_vec3f_t* self);
void    a3d_vec3f_normalize_copy(const a3d_vec3f_t* self, a3d_vec3f_t* copy);
GLfloat a3d_vec3f_dot(const a3d_vec3f_t* a, const a3d_vec3f_t* b);
GLfloat a3d_vec3f_distance(const a3d_vec3f_t* a, const a3d_vec3f_t* b);
void    a3d_vec3f_cross(a3d_vec3f_t* self, const a3d_vec3f_t* v);
void    a3d_vec3f_cross_copy(const a3d_vec3f_t* self, const a3d_vec3f_t* v, a3d_vec3f_t* copy);

// TODO - homogeneous operations

#endif
