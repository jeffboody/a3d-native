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

#ifndef a3d_screen_H
#define a3d_screen_H

#include "a3d_widget.h"
#include "a3d_font.h"
#include "../math/a3d_vec4f.h"
#include "../math/a3d_mat4f.h"

#define A3D_SCREEN_SCALE_XSMALL 1
#define A3D_SCREEN_SCALE_SMALL  2
#define A3D_SCREEN_SCALE_MEDIUM 3
#define A3D_SCREEN_SCALE_LARGE  4
#define A3D_SCREEN_SCALE_XLARGE 5

#define A3D_SCREEN_POINTER_UP   0
#define A3D_SCREEN_POINTER_DOWN 1
#define A3D_SCREEN_POINTER_MOVE 2

typedef struct a3d_screen_s
{
	// screen size
	int w;
	int h;

	// UI scale factor
	int scale;

	// top level widget
	a3d_widget_t* top_widget;

	// layout dirty flag
	int dirty;

	// pointer generates click and drag events
	int    pointer_state;
	float  pointer_x0;
	float  pointer_y0;
	double pointer_t0;

	// font - shared resource
	a3d_font_t* font;

	// fill/line state
	GLuint id_coords;
	GLuint prog;
	GLint  attr_coords;
	GLint  unif_mvp;
	GLint  unif_rect;
	GLint  unif_color;
} a3d_screen_t;

a3d_screen_t* a3d_screen_new(a3d_font_t* font);
void          a3d_screen_delete(a3d_screen_t** _self);
void          a3d_screen_top(a3d_screen_t* self, a3d_widget_t* top);
a3d_font_t*   a3d_screen_font(a3d_screen_t* self);
void          a3d_screen_resize(a3d_screen_t* self, int w, int h);
void          a3d_screen_sizei(a3d_screen_t* self, int* w, int* h);
void          a3d_screen_sizef(a3d_screen_t* self, float* w, float* h);
void          a3d_screen_rescale(a3d_screen_t* self, int scale);
int           a3d_screen_scalei(a3d_screen_t* self);
float         a3d_screen_scalef(a3d_screen_t* self);
void          a3d_screen_dirty(a3d_screen_t* self);
float         a3d_screen_layoutBorder(a3d_screen_t* self, int style);
float         a3d_screen_layoutLine(a3d_screen_t* self, int style);
float         a3d_screen_layoutText(a3d_screen_t* self, int style);
void          a3d_screen_pointerDown(a3d_screen_t* self, float x, float y, double t0);
void          a3d_screen_pointerUp(a3d_screen_t* self, float x, float y, double t0);
void          a3d_screen_pointerMove(a3d_screen_t* self, float x, float y, double t0);
void          a3d_screen_scissor(a3d_screen_t* self, a3d_rect4f_t* rect);
void          a3d_screen_draw(a3d_screen_t* self);

#endif
