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

#ifndef a3d_polygon_H
#define a3d_polygon_H

#include "a3d_list.h"
#include "a3d_GL.h"
#include "math/a3d_vec4f.h"
#include "math/a3d_mat4f.h"
#include "a3d_polygonShader.h"

typedef struct
{
	// attributes
	int         dirty;
	a3d_vec4f_t color;

	// contours
	a3d_list_t* list;

	// geometry
	GLuint      id_vtx;
	a3d_list_t* list_idx;
} a3d_polygon_t;

a3d_polygon_t* a3d_polygon_new(void);
void           a3d_polygon_delete(a3d_polygon_t** _self);
void           a3d_polygon_point(a3d_polygon_t* self,
                                 int first,
                                 float x, float y);
void           a3d_polygon_color(a3d_polygon_t* self,
                                 a3d_vec4f_t* color);
void           a3d_polygon_draw(a3d_polygon_t* self,
                                a3d_polygonShader_t* shader,
                                a3d_mat4f_t* mvp);

#endif
