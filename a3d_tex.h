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

#ifndef a3d_tex_H
#define a3d_tex_H

#include "a3d_GL.h"

// texture state
extern const int A3D_TEX_ERROR;
extern const int A3D_TEX_LOADING;
extern const int A3D_TEX_LOADED;
extern const int A3D_TEX_CACHED;
extern const int A3D_TEX_UNLOADED;

typedef struct
{
	GLuint  id;
	GLenum  type;     // glTexImage2D type
	GLenum  format;   // glTexImage2D format
	GLsizei width;
	GLsizei height;
	GLsizei stride;
	GLsizei vstride;
	char    fname[256];
	unsigned char* pixels;
} a3d_tex_t;

a3d_tex_t* a3d_tex_new(const char* fname);
void       a3d_tex_delete(a3d_tex_t** _self);
int        a3d_tex_load(a3d_tex_t* self);
void       a3d_tex_reclaim(a3d_tex_t* self);
int        a3d_tex_size(const a3d_tex_t* self);

#endif
