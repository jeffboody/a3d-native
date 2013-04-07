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

#ifndef a3d_texstring_H
#define a3d_texstring_H

#include "a3d_texfont.h"
#include "math/a3d_mat4f.h"

extern const int A3D_TEXSTRING_TOP_LEFT;
extern const int A3D_TEXSTRING_TOP_CENTER;
extern const int A3D_TEXSTRING_TOP_RIGHT;
extern const int A3D_TEXSTRING_BOTTOM_LEFT;
extern const int A3D_TEXSTRING_BOTTOM_CENTER;
extern const int A3D_TEXSTRING_BOTTOM_RIGHT;

typedef struct
{
	a3d_texfont_t* font;

	// rendering properties
	GLfloat size;
	int justify;
	a3d_vec4f_t color;

	// string data
	// max_len includes null terminator
	int      max_len;
	char*    string;
	GLfloat* vertex;
	GLfloat* coords;
	GLuint   vertex_id;
	GLuint   coords_id;

	#if defined(A3D_GLESv2) || defined(A3D_GL2)
		a3d_mat4f_t pm;
		a3d_mat4f_t mvm;

		// shader state
		GLuint program;
		GLint  attribute_vertex;
		GLint  attribute_coords;
		GLint  uniform_color;
		GLint  uniform_mvp;
		GLint  uniform_sampler;
	#endif
} a3d_texstring_t;

a3d_texstring_t* a3d_texstring_new(a3d_texfont_t* font, int max_len,
                                   float size, int justify,
                                   float r, float g, float b, float a);
void             a3d_texstring_delete(a3d_texstring_t** _self);
void             a3d_texstring_printf(a3d_texstring_t* self, const char* fmt, ...);
void             a3d_texstring_draw(a3d_texstring_t* self,
                                    float x, float y,
                                    float screen_w, float screen_h);

#endif
