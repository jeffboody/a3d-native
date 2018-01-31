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

#include "../texgz/texgz_tex.h"
#include "math/a3d_regionf.h"

typedef struct
{
	int char_w;
	int char_h;

	texgz_tex_t* tex;

	GLuint id;
} a3d_texfont_t;

a3d_texfont_t* a3d_texfont_new(const char* fname,
                               const char* resource);
void           a3d_texfont_delete(a3d_texfont_t** _self);
void           a3d_texfont_request(a3d_texfont_t* self,
                                   int mode, char c,
                                   a3d_regionf_t* tc, a3d_regionf_t* vc);
float          a3d_texfont_aspect_ratio(a3d_texfont_t* self);

#endif
