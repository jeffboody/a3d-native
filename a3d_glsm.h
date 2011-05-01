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

#ifndef a3d_glsm_H
#define a3d_glsm_H

#include "a3d_list.h"
#include "math/a3d_vec3f.h"
#include "a3d_GL.h"

extern const int A3D_GLSM_COMPLETE;
extern const int A3D_GLSM_INCOMPLETE;
extern const int A3D_GLSM_ERROR;

// "glsm" - gl state machine
typedef struct
{
	// state
	int status;
	a3d_vec3f_t normal;
	a3d_list_t* cache_vb;   // vertex(s)
	a3d_list_t* cache_nb;   // normal(s)

	// "completed" arrays
	GLsizei  ec;   // element count
	GLfloat* vb;   // vertex(s)
	GLfloat* nb;   // normal(s)
} a3d_glsm_t;

a3d_glsm_t* a3d_glsm_new(void);
void        a3d_glsm_delete(a3d_glsm_t** _self);
void        a3d_glsm_begin(a3d_glsm_t* self);
void        a3d_glsm_normal3f(a3d_glsm_t* self, GLfloat x, GLfloat y, GLfloat z);
void        a3d_glsm_vertex3f(a3d_glsm_t* self, GLfloat x, GLfloat y, GLfloat z);
void        a3d_glsm_end(a3d_glsm_t* self);
int         a3d_glsm_status(a3d_glsm_t* self);

#endif
