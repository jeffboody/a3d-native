/*
 * Copyright (c) 2018 Jeff Boody
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

#ifndef a3d_lineShader_H
#define a3d_lineShader_H

#include "a3d_GL.h"

typedef struct
{
	GLuint prog;
	GLint  attr_vtx;
	GLint  attr_st;
	GLint  unif_width;
	GLint  unif_dist;
	GLint  unif_depth;
	GLint  unif_rounded;
	GLint  unif_brush1;
	GLint  unif_brush2;
	GLint  unif_stripe;
	GLint  unif_color1;
	GLint  unif_color2;
	GLint  unif_mvp;

	// GL state
	int blend;
} a3d_lineShader_t;

a3d_lineShader_t* a3d_lineShader_new(void);
void              a3d_lineShader_delete(a3d_lineShader_t** _self);
void              a3d_lineShader_begin(a3d_lineShader_t* self);
void              a3d_lineShader_end(a3d_lineShader_t* self);

#endif
