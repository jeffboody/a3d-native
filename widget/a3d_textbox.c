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
#include "a3d_text.h"
#include "a3d_textbox.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_textbox_printText(a3d_textbox_t* self,
                                  const char* string)
{
	assert(self);
	assert(string);

	a3d_vec4f_t clear;
	a3d_vec4f_load(&clear, 0.0f, 0.0f, 0.0f, 0.0f);

	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_text_t* text = a3d_text_new(widget->screen,
	                                0,
	                                self->border,
	                                self->text_size,
	                                &clear,
	                                &(self->color_text),
	                                NULL, NULL,
	                                NULL, NULL);
	if(text == NULL)
	{
		return;
	}
	a3d_text_font(text, self->font_type);

	a3d_listbox_t* listbox = (a3d_listbox_t*) self;
	if(a3d_list_enqueue(listbox->list, (const void*) text) == 0)
	{
		goto fail_enqueue;
	}

	a3d_text_printf(text, "%s", string);

	// success
	return;

	// failure
	fail_enqueue:
		a3d_text_delete(&text);
}

#define A3D_TOKEN_END   0
#define A3D_TOKEN_TEXT  1
#define A3D_TOKEN_BREAK 2

static int getToken(const char* src, char* tok,
                    int* _srci, int* _toki)
{
	assert(src);
	assert(tok);
	assert(_srci);
	assert(_toki);

	int srci = *_srci;
	int toki = 0;

	// skip whitespace
	while((src[srci] == '\n') ||
	      (src[srci] == '\r') ||
	      (src[srci] == '\t') ||
	      (src[srci] == ' '))
	{
		++srci;
	}

	// check for the end
	if(src[srci] == '\0')
	{
		*_srci = srci;
		*_toki = toki;
		return (srci == 0) ? A3D_TOKEN_BREAK : A3D_TOKEN_END;
	}

	// read text
	while((src[srci] != '\0') &&
	      (src[srci] != '\n') &&
	      (src[srci] != '\r') &&
	      (src[srci] != '\t') &&
	      (src[srci] != ' '))
	{
		tok[toki++] = src[srci++];
		tok[toki]   = '\0';
	}

	*_srci = srci;
	*_toki = toki;
	return A3D_TOKEN_TEXT;
}

static void a3d_textbox_reflow(a3d_widget_t* widget,
                               float w, float h)
{
	assert(widget);

	a3d_textbox_t* self = (a3d_textbox_t*) widget;

	// subtract the text_border which is added when
	// printing text lines
	float h_bo = 0.0f;
	float v_bo = 0.0f;
	a3d_screen_layoutBorder(widget->screen,
	                        self->border,
	                        &h_bo, &v_bo);
	w = w - 2.0f*h_bo;

	// reflow text when changes occur
	if((self->dirty  == 0) &&
	   (self->last_w == w) &&
	   (self->last_h == h))
	{
		return;
	}
	self->dirty  = 0;
	self->last_w = w;
	self->last_h = h;

	// reflow text for variable width font
	a3d_font_t* font   = a3d_screen_font(widget->screen,
	                                     self->font_type);
	float       size   = a3d_screen_layoutText(widget->screen,
	                                           self->text_size);
	float       height = (float) a3d_font_height(font);

	// clear the text
	a3d_listbox_t*  listbox = (a3d_listbox_t*) self;
	a3d_listitem_t* iter    = a3d_list_head(listbox->list);
	while(iter)
	{
		a3d_text_t* text;
		text = (a3d_text_t*) a3d_list_remove(listbox->list, &iter);
		a3d_text_delete(&text);
	}

	// initialize parser
	char tok[256];
	char dst[256];
	char tmp[256];
	int  srci = 0;
	int  toki = 0;
	int  dsti = 0;
	int  type = A3D_TOKEN_END;

	// reflow the string(s)
	iter = a3d_list_head(self->strings);
	while(iter)
	{
		const char* src = (const char*) a3d_list_peekitem(iter);

		srci = 0;
		type = getToken(src, tok, &srci, &toki);
		while(type != A3D_TOKEN_END)
		{
			if(type == A3D_TOKEN_BREAK)
			{
				if(dsti > 0)
				{
					// print current line and line break
					a3d_textbox_printText(self, dst);
					a3d_textbox_printText(self, "");
				}
				else
				{
					// print line break
					a3d_textbox_printText(self, "");
				}
				dsti = 0;
				break;
			}

			if(dsti == 0)
			{
				// start a new line
				strncpy(dst, tok, 256);
				dst[255] = '\0';
				dsti = toki;
			}
			else
			{
				// measure width of "dst tok"
				snprintf(tmp, 256, "%s %s", dst, tok);
				tmp[255] = '\0';
				float width = (float) a3d_font_measure(font, tmp);
				int   len   = strlen(tmp);

				if((size*(width/height) <= w) &&
				   (len < self->max_len))
				{
					// append to current line
					snprintf(dst, 256, "%s", tmp);
					dst[255] = '\0';
					dsti += toki + 1;
				}
				else
				{
					// print the current line
					a3d_textbox_printText(self, dst);

					// start a new line
					strncpy(dst, tok, 256);
					dst[255] = '\0';
					dsti = toki;
				}
			}

			type = getToken(src, tok, &srci, &toki);
		}

		iter = a3d_list_next(iter);
	}

	if(dsti > 0)
	{
		// print the last line
		a3d_textbox_printText(self, dst);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_textbox_t* a3d_textbox_new(a3d_screen_t* screen,
                               int wsize,
                               int orientation,
                               a3d_widgetLayout_t* layout,
                               int border,
                               a3d_vec4f_t* color_fill,
                               int text_wrapx,
                               int text_border,
                               int text_size,
                               a3d_vec4f_t* color_text,
                               int max_len,
                               int scroll_bar,
                               a3d_vec4f_t* color_scroll0,
                               a3d_vec4f_t* color_scroll1,
                               void* priv,
                               a3d_widget_clickFn click_fn)
{
	// priv, click_fn may be NULL
	assert(screen);
	assert(color_fill);
	assert(color_text);
	assert(color_scroll0);
	assert(color_scroll1);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_textbox_t);
	}

	a3d_widget_reflowFn reflow_fn = a3d_textbox_reflow;
	if(layout->wrapx == A3D_WIDGET_WRAP_SHRINK)
	{
		reflow_fn = NULL;
	}

	a3d_textbox_t* self;
	self = (a3d_textbox_t*)
	a3d_listbox_new(screen, wsize, orientation, layout, border,
	                color_fill, scroll_bar, color_scroll0,
	                color_scroll1, priv, click_fn, reflow_fn,
	                NULL);
	if(self == NULL)
	{
		return NULL;
	}

	// enable sound effects since textbox derives from listbox
	a3d_widget_soundFx((a3d_widget_t*) self,
	                   click_fn ? 1 : 0);

	self->strings = a3d_list_new();
	if(self->strings == NULL)
	{
		goto fail_strings;
	}

	self->dirty  = 1;
	self->last_w = 0.0f;
	self->last_h = 0.0f;

	self->text_wrapx = text_wrapx;
	self->border     = text_border;
	self->text_size  = text_size;
	self->font_type  = A3D_SCREEN_FONT_REGULAR;
	self->max_len    = max_len;

	a3d_vec4f_copy(color_text, &self->color_text);

	// success
	return self;

	// failure
	fail_strings:
		a3d_listbox_delete((a3d_listbox_t**) &self);
	return NULL;
}

void a3d_textbox_delete(a3d_textbox_t** _self)
{
	assert(_self);

	a3d_textbox_t* self = *_self;
	if(self)
	{
		a3d_textbox_clear(self);
		a3d_listbox_delete((a3d_listbox_t**) _self);
		*_self = NULL;
	}
}

void a3d_textbox_clear(a3d_textbox_t* self)
{
	assert(self);

	a3d_listbox_t*  listbox = (a3d_listbox_t*) self;
	a3d_listitem_t* iter    = a3d_list_head(listbox->list);
	while(iter)
	{
		a3d_text_t* text;
		text = (a3d_text_t*) a3d_list_remove(listbox->list, &iter);
		a3d_text_delete(&text);
	}

	iter = a3d_list_head(self->strings);
	while(iter)
	{
		void* string = (void*) a3d_list_remove(self->strings, &iter);
		free(string);
	}

	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_screen_dirty(widget->screen);
}

void a3d_textbox_printf(a3d_textbox_t* self,
                        const char* fmt, ...)
{
	assert(self);
	assert(fmt);

	char* string = (char*) calloc(self->max_len, sizeof(char));
	if(string == NULL)
	{
		LOGE("calloc failed");
		return;
	}

	// decode string
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(string, self->max_len, fmt, argptr);
	va_end(argptr);

	self->dirty = 1;

	if(a3d_list_enqueue(self->strings, (const void*) string) == 0)
	{
		goto fail_enqueue;
	}

	a3d_textbox_printText(self, string);

	// success
	return;

	// failure
	fail_enqueue:
		free(string);
}

void a3d_textbox_font(a3d_textbox_t* self,
                      int font_type)
{
	assert(self);

	self->font_type = font_type;
}
