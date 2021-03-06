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

#include "a3d_screen.h"
#include "a3d_hline.h"
#include "a3d_key.h"
#include "a3d_text.h"
#include "a3d_widget.h"
#include "../a3d_shader.h"
#include "../a3d_timestamp.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static const char A3D_SHIFTKEYS[128] =
{
	0x00,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x07,
	0x08,
	0x09,
	0x0A,
	0x0B,
	0x0C,
	0x0D,
	0x0E,
	0x0F,
	0x10,
	0x11,
	0x12,
	0x13,
	0x14,
	0x15,
	0x16,
	0x17,
	0x18,
	0x19,
	0x1A,
	0x1B,
	0x1C,
	0x1D,
	0x1E,
	0x1F,
	0x20,
	0x21,
	0x22,
	0x23,
	0x24,
	0x25,
	0x26,
	'\"',
	0x28,
	0x29,
	0x2A,
	0x2B,
	'<',
	'_',
	'>',
	'?',
	')',
	'!',
	'@',
	'#',
	'$',
	'%',
	'^',
	'&',
	'*',
	'(',
	0x3A,
	':',
	0x3C,
	'+',
	0x3E,
	0x3F,
	0x40,
	0x41,
	0x42,
	0x43,
	0x44,
	0x45,
	0x46,
	0x47,
	0x48,
	0x49,
	0x4A,
	0x4B,
	0x4C,
	0x4D,
	0x4E,
	0x4F,
	0x50,
	0x51,
	0x52,
	0x53,
	0x54,
	0x55,
	0x56,
	0x57,
	0x58,
	0x59,
	0x5A,
	'{',
	'|',
	'}',
	0x5E,
	0x5F,
	'~',
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	0x7B,
	0x7C,
	0x7D,
	0x7E,
	0x7F,
};

static int a3d_shiftkeycode(int keycode, int meta)
{
	if((keycode >= 0) && (keycode < 128))
	{
		int shiftchar = meta & (A3D_KEY_SHIFT | A3D_KEY_CAPS);
		int shiftsym  = meta & A3D_KEY_SHIFT;
		if(shiftchar && (keycode >= 'a') && (keycode <= 'z'))
		{
			return A3D_SHIFTKEYS[keycode];
		}
		else if(shiftsym)
		{
			return A3D_SHIFTKEYS[keycode];
		}
	}
	else if(keycode == A3D_KEY_DELETE)
	{
		return A3D_KEY_INSERT;
	}
	return keycode;
}

static int a3d_screen_compareTexFname(const void* a, const void* b)
{
	assert(a);
	assert(b);

	a3d_spriteTex_t* t = (a3d_spriteTex_t*) a;
	const char*      s = (const char*) b;

	return strncmp(t->fname, s, 256);
}

static int a3d_screen_compareTexId(const void* a, const void* b)
{
	assert(a);
	assert(b);

	a3d_spriteTex_t* t  = (a3d_spriteTex_t*) a;
	GLuint*          id = (GLuint*) b;

	return ((*id) == t->id_tex) ? 0 : 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_screen_t* a3d_screen_new(const char* resource,
                             a3d_font_t* font_regular,
                             a3d_font_t* font_bold,
                             void* sound_fx,
                             a3d_screen_playClickFn playClick)
{
	assert(resource);
	assert(font_regular);
	assert(font_bold);
	assert(sound_fx);
	assert(playClick);

	a3d_screen_t* self = (a3d_screen_t*) malloc(sizeof(a3d_screen_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->sprite_shader_alpha = a3d_spriteShader_new(A3D_SPRITESHADER_ALPHA);
	if(self->sprite_shader_alpha == NULL)
	{
		goto fail_sprite_shader_alpha;
	}

	self->sprite_shader_color = a3d_spriteShader_new(A3D_SPRITESHADER_COLOR);
	if(self->sprite_shader_color == NULL)
	{
		goto fail_sprite_shader_color;
	}

	self->sprite_list = a3d_list_new();
	if(self->sprite_list == NULL)
	{
		goto fail_sprite_list;
	}

	self->w             = 0;
	self->h             = 0;
	self->density       = 1.0f;
	self->scale         = A3D_SCREEN_SCALE_MEDIUM;
	self->top_widget    = NULL;
	self->focus_widget  = NULL;
	self->dirty         = 1;
	self->pointer_state = A3D_WIDGET_POINTER_UP;
	self->pointer_x0    = 0.0f;
	self->pointer_y0    = 0.0f;
	self->pointer_t0    = 0.0;
	self->pointer_vx    = 0.0f;
	self->pointer_vy    = 0.0f;
	self->font[0]       = font_regular;
	self->font[1]       = font_bold;
	self->clicked       = 0;
	self->sound_fx      = sound_fx;
	self->playClick     = playClick;

	strncpy(self->resource, resource, 256);
	self->resource[255] = '\0';

	// success
	return self;

	// failure
	fail_sprite_list:
		a3d_spriteShader_delete(&self->sprite_shader_color);
	fail_sprite_shader_color:
		a3d_spriteShader_delete(&self->sprite_shader_alpha);
	fail_sprite_shader_alpha:
		free(self);
	return NULL;
}

void a3d_screen_delete(a3d_screen_t** _self)
{
	assert(_self);

	a3d_screen_t* self = *_self;
	if(self)
	{
		// all sprites should have been unmapped already
		a3d_list_delete(&self->sprite_list);

		a3d_spriteShader_delete(&self->sprite_shader_color);
		a3d_spriteShader_delete(&self->sprite_shader_alpha);
		free(self);
		*_self = NULL;
	}
}

void a3d_screen_top(a3d_screen_t* self, a3d_widget_t* top)
{
	assert(self);

	if(self->top_widget == top)
	{
		return;
	}

	self->top_widget = top;
	self->dirty      = 1;
}

void a3d_screen_focus(a3d_screen_t* self, a3d_widget_t* focus)
{
	// focus may be NULL
	assert(self);

	self->focus_widget = focus;
}

a3d_font_t* a3d_screen_font(a3d_screen_t* self, int type)
{
	assert(self);

	return self->font[type];
}

a3d_spriteShader_t* a3d_screen_spriteShader(a3d_screen_t* self, int format)
{
	assert(self);

	if(format == A3D_SPRITESHADER_ALPHA)
	{
		return self->sprite_shader_alpha;
	}
	else
	{
		return self->sprite_shader_color;
	}
}

GLuint a3d_screen_spriteTexMap(a3d_screen_t* self, const char* fname,
                               int* format)
{
	assert(self);
	assert(fname);
	assert(format);

	a3d_spriteTex_t* t    = NULL;
	a3d_listitem_t*  iter = a3d_list_find(self->sprite_list,
	                                      fname,
	                                      a3d_screen_compareTexFname);
	if(iter)
	{
		t = (a3d_spriteTex_t*) a3d_list_peekitem(iter);
	}
	else
	{
		t = a3d_spriteTex_new(fname, self->resource);
		if(t == NULL)
		{
			return 0;
		}

		if(a3d_list_enqueue(self->sprite_list, (const void*) t) == 0)
		{
			goto fail_enqueue;
		}
	}

	// success
	a3d_spriteTex_incRef(t);
	*format = t->format;
	return t->id_tex;

	// failure
	fail_enqueue:
		a3d_spriteTex_delete(&t);
	return 0;
}

void a3d_screen_spriteTexUnmap(a3d_screen_t* self, GLuint* _id)
{
	assert(self);
	assert(_id);

	// ignore zero
	if(*_id == 0)
	{
		return;
	}

	a3d_listitem_t* iter = a3d_list_find(self->sprite_list,
	                                     (const void*) _id,
	                                     a3d_screen_compareTexId);
	if(iter == NULL)
	{
		LOGW("id=%u not found", *_id);
		return;
	}

	// the texture may be mapped by multiple sprites
	a3d_spriteTex_t* t = (a3d_spriteTex_t*) a3d_list_peekitem(iter);
	if(a3d_spriteTex_decRef(t) == 0)
	{
		a3d_list_remove(self->sprite_list, &iter);
		a3d_spriteTex_delete(&t);
	}

	// always clear the id
	*_id = 0;
}

void a3d_screen_resize(a3d_screen_t* self, int w, int h)
{
	assert(self);

	if((self->w == w) && (self->h == h))
	{
		return;
	}

	self->w     = w;
	self->h     = h;
	self->dirty = 1;
}

void a3d_screen_density(a3d_screen_t* self, float density)
{
	assert(self);

	if(self->density == density)
	{
		return;
	}

	self->density = density;
	self->dirty   = 1;
}

void a3d_screen_sizei(a3d_screen_t* self, int* w, int* h)
{
	assert(self);

	*w = self->w;
	*h = self->h;
}

void a3d_screen_sizef(a3d_screen_t* self, float* w, float* h)
{
	assert(self);

	*w = (float) self->w;
	*h = (float) self->h;
}

void a3d_screen_rescale(a3d_screen_t* self, int scale)
{
	assert(self);

	if(self->scale == scale)
	{
		return;
	}

	if((scale >= A3D_SCREEN_SCALE_XSMALL) &&
	   (scale <= A3D_SCREEN_SCALE_XLARGE))
	{
		self->scale = scale;
		self->dirty = 1;
	}
}

int a3d_screen_scalei(a3d_screen_t* self)
{
	assert(self);

	return self->scale;
}

float a3d_screen_scalef(a3d_screen_t* self)
{
	assert(self);

	if(self->scale == A3D_SCREEN_SCALE_XSMALL)
	{
		return 0.79f;
	}
	else if(self->scale == A3D_SCREEN_SCALE_SMALL)
	{
		return 0.89f;
	}
	else if(self->scale == A3D_SCREEN_SCALE_LARGE)
	{
		return 1.13f;
	}
	else if(self->scale == A3D_SCREEN_SCALE_XLARGE)
	{
		return 1.27f;
	}
	return 1.0f;
}

void a3d_screen_dirty(a3d_screen_t* self)
{
	assert(self);

	self->dirty = 1;
}

void a3d_screen_layoutBorder(a3d_screen_t* self, int border,
                             float* hborder, float* vborder)
{
	assert(self);
	assert(hborder);
	assert(vborder);

	*hborder = 0.0f;
	*vborder = 0.0f;

	float size = a3d_screen_layoutText(self, A3D_TEXT_SIZE_MEDIUM);

	// horizontal
	if(border & A3D_WIDGET_BORDER_HSMALL)
	{
		*hborder = 0.125f*size;
	}
	else if(border & A3D_WIDGET_BORDER_HMEDIUM)
	{
		*hborder = 0.25f*size;
	}
	else if(border & A3D_WIDGET_BORDER_HLARGE)
	{
		*hborder = 0.5f*size;
	}

	// vertical
	if(border & A3D_WIDGET_BORDER_VSMALL)
	{
		*vborder = 0.125f*size;
	}
	else if(border & A3D_WIDGET_BORDER_VMEDIUM)
	{
		*vborder = 0.25f*size;
	}
	else if(border & A3D_WIDGET_BORDER_VLARGE)
	{
		*vborder = 0.5f*size;
	}
}

float a3d_screen_layoutHLine(a3d_screen_t* self, int size)
{
	assert(self);

	if(size == A3D_HLINE_SIZE_SMALL)
	{
		return 3.0f;
	}
	else if(size == A3D_HLINE_SIZE_LARGE)
	{
		return 9.0f;
	}
	return 6.0f;
}

float a3d_screen_layoutText(a3d_screen_t* self, int size)
{
	assert(self);

	// default size is 24 px at density 1.0
	float sizef = 24.0f*self->density*a3d_screen_scalef(self);
	if(size == A3D_TEXT_SIZE_SMALL)
	{
		return 0.66f*sizef;
	}
	else if(size == A3D_TEXT_SIZE_LARGE)
	{
		return 1.5f*sizef;
	}
	return sizef;
}

int a3d_screen_pointerDown(a3d_screen_t* self,
                           float x, float y, double t0)
{
	assert(self);

	if((self->top_widget == NULL) ||
	   (self->pointer_state != A3D_WIDGET_POINTER_UP))
	{
		// ignore
		return 0;
	}

	if(a3d_widget_click(self->top_widget,
	                    A3D_WIDGET_POINTER_DOWN, x, y))
	{
		self->pointer_state = A3D_WIDGET_POINTER_DOWN;
		self->pointer_x0    = x;
		self->pointer_y0    = y;
		self->pointer_t0    = t0;
		self->pointer_vx    = 0.0f;
		self->pointer_vy    = 0.0f;
		return 1;
	}

	return 0;
}

int a3d_screen_pointerUp(a3d_screen_t* self,
                         float x, float y, double t0)
{
	assert(self);

	int touch = self->pointer_state != A3D_WIDGET_POINTER_UP;
	if(self->top_widget &&
	   (self->pointer_state == A3D_WIDGET_POINTER_DOWN))
	{
		a3d_widget_click(self->top_widget,
		                 A3D_WIDGET_POINTER_UP, x, y);
	}
	self->pointer_state = A3D_WIDGET_POINTER_UP;

	return touch;
}

int a3d_screen_pointerMove(a3d_screen_t* self,
                           float x, float y, double t0)
{
	assert(self);

	if((self->top_widget == NULL) ||
	   (self->pointer_state == A3D_WIDGET_POINTER_UP))
	{
		// ignore
		return 0;
	}

	float dx = x - self->pointer_x0;
	float dy = y - self->pointer_y0;
	if(self->pointer_state == A3D_WIDGET_POINTER_DOWN)
	{
		// reject small motions
		float d = sqrtf(dx*dx + dy*dy);
		float s = 0.2f*a3d_screen_layoutText(self, A3D_TEXT_SIZE_MEDIUM);
		if(d < s)
		{
			// ignore
			return 1;
		}

		// initialize move state
		self->pointer_state = A3D_WIDGET_POINTER_MOVE;
		self->pointer_x0    = x;
		self->pointer_y0    = y;
		self->pointer_t0    = t0;
		self->pointer_vx    = 0.0f;
		self->pointer_vy    = 0.0f;
		return 1;
	}

	// ignore events with less than 8ms time delta
	float dt = (float) (t0 - self->pointer_t0);
	if(dt >= 0.008f)
	{
		// update the move state
		self->pointer_x0 = x;
		self->pointer_y0 = y;
		self->pointer_t0 = t0;
		self->pointer_vx = dx/dt;
		self->pointer_vy = dy/dt;
		self->dirty      = 1;
	}

	return 1;
}

int a3d_screen_keyPress(a3d_screen_t* self,
                        int keycode, int meta)
{
	assert(self);

	if(self->focus_widget == NULL)
	{
		return 0;
	}

	keycode = a3d_shiftkeycode(keycode, meta);
	return a3d_widget_keyPress(self->focus_widget,
	                           keycode, meta);
}

void a3d_screen_scissor(a3d_screen_t* self, a3d_rect4f_t* rect)
{
	assert(self);
	assert(rect);

	glScissor((GLint) (rect->l + 0.5f),
	          self->h - (GLint) (rect->t + rect->h + 0.5f),
	          (GLsizei) (rect->w + 0.5f),
	          (GLsizei) (rect->h + 0.5f));
}

void a3d_screen_draw(a3d_screen_t* self, float dt)
{
	assert(self);

	a3d_widget_t* top = self->top_widget;
	if(top == NULL)
	{
		// ignore
		return;
	}

	a3d_widget_refresh(top);

	// dragging
	float w = (float) self->w;
	float h = (float) self->h;
	if((self->pointer_vx != 0.0f) ||
	   (self->pointer_vy != 0.0f))
	{
		float x  = self->pointer_x0;
		float y  = self->pointer_y0;
		float vx = self->pointer_vx;
		float vy = self->pointer_vy;

		// clamp the speed to be proportional to the range
		float range  = 4.0f*sqrtf(w*w + h*h);
		float speed1 = sqrtf(vx*vx + vy*vy);
		float drag   = 1.0f*range*dt;
		if(speed1 > range)
		{
			vx *= range/speed1;
			vy *= range/speed1;
			speed1 = range;
		}

		// TODO - change drag to return status for
		// bump animation and to minimize dirty updates
		a3d_widget_drag(self->top_widget,
		                x, y, vx*dt, vy*dt);

		// update the speed
		if((speed1 > drag) && (speed1 > 0.1f))
		{
			float speed2 = speed1 - drag;
			float coef   = speed2/speed1;
			self->pointer_vx *= coef;
			self->pointer_vy *= coef;
		}
		else
		{
			self->pointer_vx = 0.0f;
			self->pointer_vy = 0.0f;
		}

		self->dirty = 1;
	}

	if(self->dirty)
	{
		a3d_rect4f_t clip = { .t = 0.0f, .l = 0.0f, .w = w, .h = h };

		a3d_widget_layoutSize(top, &w, &h);
		a3d_widget_layoutXYClip(top, 0.0f, 0.0f, &clip, 1, 1);
		self->dirty = 0;
	}

	glEnable(GL_SCISSOR_TEST);
	a3d_widget_draw(self->top_widget);
	glScissor((GLint) 0,
	          (GLint) 0,
	          (GLsizei) self->w,
	          (GLsizei) self->h);
	glDisable(GL_SCISSOR_TEST);

	// play sound fx
	if(self->clicked)
	{
		a3d_screen_playClickFn playClick = self->playClick;
		(*playClick)(self->sound_fx);
		self->clicked = 0;
	}
}

void a3d_screen_playClick(a3d_screen_t* self)
{
	assert(self);

	self->clicked = 1;
}
