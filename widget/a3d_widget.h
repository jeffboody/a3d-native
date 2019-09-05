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

#define A3D_WIDGET_ASPECT_DEFAULT 0
#define A3D_WIDGET_ASPECT_SQUARE  1

#define A3D_WIDGET_BORDER_NONE    0x0000
#define A3D_WIDGET_BORDER_HSMALL  0x0001
#define A3D_WIDGET_BORDER_HMEDIUM 0x0002
#define A3D_WIDGET_BORDER_HLARGE  0x0004
#define A3D_WIDGET_BORDER_VSMALL  0x0010
#define A3D_WIDGET_BORDER_VMEDIUM 0x0020
#define A3D_WIDGET_BORDER_VLARGE  0x0040
#define A3D_WIDGET_BORDER_SMALL   A3D_WIDGET_BORDER_HSMALL  | A3D_WIDGET_BORDER_VSMALL
#define A3D_WIDGET_BORDER_MEDIUM  A3D_WIDGET_BORDER_HMEDIUM | A3D_WIDGET_BORDER_VMEDIUM
#define A3D_WIDGET_BORDER_LARGE   A3D_WIDGET_BORDER_HLARGE  | A3D_WIDGET_BORDER_VLARGE

#define A3D_WIDGET_POINTER_UP   0
#define A3D_WIDGET_POINTER_DOWN 1
#define A3D_WIDGET_POINTER_MOVE 2

// vertices per corner
#define A3D_WIDGET_BEZEL 8

struct a3d_screen_s;

// derived class functions
struct a3d_widget_s;
typedef void (*a3d_widget_reflowFn)(struct a3d_widget_s* widget,
                                    float w, float h);
typedef void (*a3d_widget_sizeFn)(struct a3d_widget_s* widget,
                                  float* w, float* h);
typedef int  (*a3d_widget_clickFn)(struct a3d_widget_s* widget,
                                   void* priv, int state,
                                   float x, float y);
typedef int  (*a3d_widget_keyPressFn)(struct a3d_widget_s* widget,
                                      void* priv, int keycode, int meta);
typedef void (*a3d_widget_layoutFn)(struct a3d_widget_s* widget,
                                    int dragx, int dragy);
typedef void (*a3d_widget_dragFn)(struct a3d_widget_s* widget,
                                  float x, float y,
                                  float dx, float dy);
typedef void (*a3d_widget_scrollTopFn)(struct a3d_widget_s* widget);
typedef void (*a3d_widget_drawFn)(struct a3d_widget_s* widget);
typedef void (*a3d_widget_refreshFn)(struct a3d_widget_s* widget,
                                     void* priv);

typedef struct a3d_widgetLayout_s
{
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
	int   aspectx;
	int   aspecty;
	float stretchx;
	float stretchy;
} a3d_widgetLayout_t;

typedef struct a3d_widgetFn_s
{
	// functions and priv may be NULL

	void* priv;

	// reflow_fn allows a derived widget to reflow
	// it's content in a resize (e.g. textbox)
	// called internally by a3d_widget_layoutSize()
	a3d_widget_reflowFn reflow_fn;

	// size_fn allows a derived widget to define
	// it's internal size (e.g. ignoring borders)
	// called internally by a3d_widget_layoutSize()
	a3d_widget_sizeFn size_fn;

	// click_fn allows a derived widget to define it's click behavior
	// called internally by a3d_widget_click()
	a3d_widget_clickFn click_fn;

	// keyPress_fn allows a derived widget to define it's keyPress
	// behavior called internally by a3d_widget_keyPress()
	a3d_widget_keyPressFn keyPress_fn;

	// layout_fn allows a derived widget to layout it's children
	// called internally by a3d_widget_layoutXYClip
	a3d_widget_layoutFn layout_fn;

	// drag_fn allows a derived widget to drag it's children
	// called internally by a3d_widget_drag
	a3d_widget_dragFn drag_fn;

	// scrollTop_fn ensures widgets are at the top when
	// brought forward in a layer
	// called internally by a3d_widget_scrollTop
	a3d_widget_scrollTopFn scrollTop_fn;

	// draw_fn allows a derived widget to define
	// it's draw behavior
	// called internally by a3d_widget_draw
	a3d_widget_drawFn draw_fn;

	// refresh_fn allows a widget to refresh it's external state
	// called internally by a3d_widget_refresh
	a3d_widget_refreshFn refresh_fn;
} a3d_widgetFn_t;

typedef struct a3d_widget_s
{
	struct a3d_screen_s* screen;

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

	// anchor to the widget parent
	// anchor is not part of the layout since it is only
	// relevant for layers and defaults to TL otherwise
	int anchor;

	// widget layout
	a3d_widgetLayout_t layout;

	// style
	int         border;
	int         scroll_bar;
	a3d_vec4f_t color_header;
	a3d_vec4f_t color_body;
	a3d_vec4f_t color_scroll0;
	a3d_vec4f_t color_scroll1;

	// header
	float header_y;

	// sound fx for clicks
	int sound_fx;

	// widget callback functions
	a3d_widgetFn_t fn;

	// draw state
	GLuint id_xy_widget;
	GLuint id_xy_scroll;
	GLuint prog;
	GLint  attr_xy;
	GLint  unif_mvp;
	GLint  unif_color0;
	GLint  unif_color1;
	GLint  unif_ab;
} a3d_widget_t;

a3d_widget_t* a3d_widget_new(struct a3d_screen_s* screen,
                             int wsize,
                             a3d_widgetLayout_t* layout,
                             int border,
                             a3d_vec4f_t* color_header,
                             a3d_vec4f_t* color_body,
                             int scroll_bar,
                             a3d_vec4f_t* color_scroll0,
                             a3d_vec4f_t* color_scroll1,
                             a3d_widgetFn_t* fn);
void          a3d_widget_delete(a3d_widget_t** _self);
void          a3d_widget_layoutXYClip(a3d_widget_t* self,
                                      float x, float y,
                                      a3d_rect4f_t* clip,
                                      int dragx, int dragy);
void          a3d_widget_layoutSize(a3d_widget_t* self,
                                    float* w, float* h);
void          a3d_widget_layoutAnchor(a3d_widget_t* self,
                                      a3d_rect4f_t* rect,
                                      float* x, float * y);
int           a3d_widget_click(a3d_widget_t* self,
                               int state,
                               float x, float y);
int           a3d_widget_keyPress(a3d_widget_t* self,
                               int keycode, int meta);
void          a3d_widget_drag(a3d_widget_t* self,
                              float x, float y,
                              float dx, float dy);
void          a3d_widget_draw(a3d_widget_t* self);
void          a3d_widget_refresh(a3d_widget_t* self);
void          a3d_widget_anchor(a3d_widget_t* self,
                                int anchor);
void          a3d_widget_soundFx(a3d_widget_t* self,
                                 int sound_fx);
void          a3d_widget_headerY(a3d_widget_t* self, float y);
void          a3d_widget_scrollTop(a3d_widget_t* self);
int           a3d_widget_hasFocus(a3d_widget_t* self);

#endif
