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

#ifndef a3d_line_H
#define a3d_line_H

#include "a3d_list.h"
#include "a3d_GL.h"
#include "math/a3d_vec4f.h"
#include "math/a3d_mat4f.h"
#include "a3d_lineShader.h"

typedef struct
{
	// attributes
	int         dirty;
	int         loop;
	float       width;
	float       brush1;
	float       brush2;
	float       stripe1;
	float       stripe2;
	a3d_vec4f_t color1;
	a3d_vec4f_t color2;
	int         blend;

	// points
	a3d_list_t* list;

	// GL state
	GLuint id_vtx;
	GLuint id_st;
} a3d_line_t;

a3d_line_t* a3d_line_new(int loop, float width, int blend);
void        a3d_line_delete(a3d_line_t** _self);
void        a3d_line_point(a3d_line_t* self,
                           float x, float y);
void        a3d_line_brush(a3d_line_t* self,
                           float brush1, float brush2);
void        a3d_line_stripe(a3d_line_t* self,
                            float stripe1, float stripe2);
void        a3d_line_color(a3d_line_t* self,
                           a3d_vec4f_t* color1,
                           a3d_vec4f_t* color2);
void        a3d_line_draw(a3d_line_t* self,
                          a3d_lineShader_t* shader,
                          a3d_mat4f_t* mvp);

#endif
