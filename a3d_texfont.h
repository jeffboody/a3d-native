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

#ifndef a3d_texfont_H
#define a3d_texfont_H

#include "math/a3d_regionf.h"

typedef struct
{
	int x;
	int y;
	int w;
} a3d_texfontcoords_t;

typedef struct
{
	// tex attributes
	int tex_width;
	int tex_height;

	// font attributes
	int   size;
	int   h;
	float aspect_ratio_avg;
	a3d_texfontcoords_t coords[128];

	GLuint id;
} a3d_texfont_t;

a3d_texfont_t* a3d_texfont_new(const char* resource,
                               const char* texname,
                               const char* xmlname);
void           a3d_texfont_delete(a3d_texfont_t** _self);
void           a3d_texfont_request(a3d_texfont_t* self,
                                   int mode, char c,
                                   a3d_regionf_t* tc,
                                   a3d_regionf_t* vc);
int            a3d_texfont_width(a3d_texfont_t* self, char c);
int            a3d_texfont_height(a3d_texfont_t* self);

#endif
