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

#ifndef a3d_widget_H
#define a3d_widget_H

#include "../math/a3d_vec4f.h"
#include "../math/a3d_rect4f.h"

#define A3D_WIDGET_ANCHOR_TL 0
#define A3D_WIDGET_ANCHOR_TC 1
#define A3D_WIDGET_ANCHOR_TR 2
#define A3D_WIDGET_ANCHOR_CL 3
#define A3D_WIDGET_ANCHOR_CC 4
#define A3D_WIDGET_ANCHOR_CR 5
#define A3D_WIDGET_ANCHOR_BL 6
#define A3D_WIDGET_ANCHOR_BC 7
#define A3D_WIDGET_ANCHOR_BR 8

#define A3D_WIDGET_WRAP_SHRINK              0
#define A3D_WIDGET_WRAP_STRETCH_PARENT      1
#define A3D_WIDGET_WRAP_STRETCH_SCREEN      2
#define A3D_WIDGET_WRAP_STRETCH_TEXT_SMALL  3
#define A3D_WIDGET_WRAP_STRETCH_TEXT_MEDIUM 4
#define A3D_WIDGET_WRAP_STRETCH_TEXT_LARGE  5

#define A3D_WIDGET_STRETCH_NA     0
#define A3D_WIDGET_STRETCH_SQUARE 1
#define A3D_WIDGET_STRETCH_ASPECT 2

#define A3D_WIDGET_BORDER_NONE   0
#define A3D_WIDGET_BORDER_SMALL  1
#define A3D_WIDGET_BORDER_MEDIUM 2
#define A3D_WIDGET_BORDER_LARGE  3

#define A3D_WIDGET_LINE_NONE   0
#define A3D_WIDGET_LINE_SMALL  1
#define A3D_WIDGET_LINE_MEDIUM 2
#define A3D_WIDGET_LINE_LARGE  3

struct a3d_screen_s;

// derived class functions
struct a3d_widget_s;
typedef void (*a3d_widget_size_fn)(struct a3d_widget_s* widget,
                                   float* w, float* h);
typedef int  (*a3d_widget_click_fn)(struct a3d_widget_s* widget,
                                    float x, float y);
typedef void (*a3d_widget_layout_fn)(struct a3d_widget_s* widget);
typedef int  (*a3d_widget_drag_fn)(struct a3d_widget_s* widget,
                                   float x, float y,
                                   float dx, float dy,
                                   double dt);
typedef void (*a3d_widget_draw_fn)(struct a3d_widget_s* widget);
typedef void (*a3d_widget_refresh_fn)(struct a3d_widget_s* widget);

typedef struct a3d_widget_s
{
	struct a3d_screen_s* screen;

	// optional priv data
	void* priv;

	// anchor to the widget parent
	int anchor;

	// dragable rules (implicit widget property of drag event)
	// 1. wrapping must be shrink
	// 2. if widget is wider than clip then
	//    restrict right >= clipright and left <= clipleft
	// 3. if widget is taller than clip then
	//    restrict top <= cliptop and bottom >= clipbottom
	float drag_dx;
	float drag_dy;

	// defines rect after a layout update
	a3d_rect4f_t rect_draw;
	a3d_rect4f_t rect_clip;
	a3d_rect4f_t rect_border;

	// horizontal/vertical wrapping
	//    shrink:
	//       size of children
	//       outset border
	//       children must be shrink
	//    stretch:
	//       size of container
	//       inset border
	//       children may be stretch or shrink
	//       top level widget must be stretch
	int   wrapx;
	int   wrapy;
	int   stretch_mode;
	float stretch_factor;

	// decorations
	int         style_border;
	int         style_line;
	a3d_vec4f_t color_line;
	a3d_vec4f_t color_fill;

	// size_fn allows a derived widget to define
	// it's internal size (e.g. ignoring borders)
	// called internally by a3d_widget_layoutSize()
	a3d_widget_size_fn size_fn;

	// click_fn allows a derived widget to define it's click behavior
	// called internally by a3d_widget_click()
	a3d_widget_click_fn click_fn;

	// layout_fn allows a derived widget to layout it's children
	// called internally by a3d_widget_layoutXYClip
	a3d_widget_layout_fn layout_fn;

	// drag_fn allows a derived widget to drag it's children
	// called internally by a3d_widget_drag
	a3d_widget_drag_fn drag_fn;

	// draw_fn allows a derived widget to define
	// it's draw behavior
	// called internally by a3d_widget_draw
	a3d_widget_draw_fn draw_fn;

	// refresh_fn allows a widget to refresh it's external state
	// called internally by a3d_widget_refresh
	a3d_widget_refresh_fn refresh_fn;
} a3d_widget_t;

a3d_widget_t* a3d_widget_new(struct a3d_screen_s* screen,
                             int wsize,
                             int anchor,
                             int wrapx, int wrapy,
                             int stretch_mode,
                             float stretch_factor,
                             int style_border,
                             int style_line,
                             a3d_vec4f_t* color_line,
                             a3d_vec4f_t* color_fill,
                             a3d_widget_size_fn size_fn,
                             a3d_widget_click_fn click_fn,
                             a3d_widget_layout_fn layout_fn,
                             a3d_widget_drag_fn drag_fn,
                             a3d_widget_draw_fn draw_fn,
                             a3d_widget_refresh_fn refresh_fn);
void          a3d_widget_delete(a3d_widget_t** _self);
void          a3d_widget_priv(a3d_widget_t* self, void* priv);
void          a3d_widget_layoutXYClip(a3d_widget_t* self,
                                      float x, float y,
                                      a3d_rect4f_t* clip);
void          a3d_widget_layoutSize(a3d_widget_t* self,
                                    float* w, float* h);
int           a3d_widget_click(a3d_widget_t* self,
                               float x, float y);
int           a3d_widget_drag(a3d_widget_t* self,
                              float x, float y,
                              float dx, float dy,
                              double dt);
void          a3d_widget_draw(a3d_widget_t* self);
void          a3d_widget_anchorPt(a3d_rect4f_t* rect,
                                  int anchor,
                                  float* x, float * y);
void          a3d_widget_refresh(a3d_widget_t* self);

#endif
