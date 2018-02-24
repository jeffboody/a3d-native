/*
 * Copyright (c) 2015 Jeff Boody
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

#ifndef a3d_font_H
#define a3d_font_H

#include "../a3d_GL.h"
#include "../math/a3d_regionf.h"

// define cursor as ASCII unit separator
#define A3D_FONT_CURSOR 31

typedef struct
{
	int x;
	int y;
	int w;
} a3d_fontcoords_t;

typedef struct
{
	// tex attributes
	int tex_width;
	int tex_height;

	// font attributes
	int   size;
	int   h;
	float aspect_ratio_avg;
	a3d_fontcoords_t coords[128];

	// GL state
	GLuint prog;
	GLint  attr_vertex;
	GLint  attr_coords;
	GLint  unif_color;
	GLint  unif_mvp;
	GLint  unif_sampler;
	GLuint id_tex;
} a3d_font_t;

a3d_font_t* a3d_font_new(const char* resource,
                         const char* texname,
                         const char* xmlname);
void        a3d_font_delete(a3d_font_t** _self);
void        a3d_font_request(a3d_font_t* self,
                             char c,
                             a3d_regionf_t* tc,
                             a3d_regionf_t* vc);
float       a3d_font_aspectRatioAvg(a3d_font_t* self);
int         a3d_font_width(a3d_font_t* self, char c);
int         a3d_font_height(a3d_font_t* self);

#endif
