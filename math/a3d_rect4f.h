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

#ifndef a3d_rect4f_H
#define a3d_rect4f_H

#include "../a3d_GL.h"

typedef struct
{
	GLfloat t;
	GLfloat l;
	GLfloat w;
	GLfloat h;
} a3d_rect4f_t;

void a3d_rect4f_init(a3d_rect4f_t* self,
                     float t, float l,
                     float w, float h);
void a3d_rect4f_copy(const a3d_rect4f_t* self,
                     a3d_rect4f_t* copy);
int  a3d_rect4f_contains(const a3d_rect4f_t* self,
                         float x, float y);
int a3d_rect4f_intersect(const a3d_rect4f_t* ra,
                         const a3d_rect4f_t* rb,
                         a3d_rect4f_t* rc);

#endif
