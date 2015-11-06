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
	LOGD("debug string=%s", string);

	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_text_t* text = a3d_text_new(widget->screen,
	                                0,
	                                self->anchor,
	                                self->style_border,
	                                self->style_line,
	                                self->style_text,
	                                &(self->color_fill),
	                                &(self->color_line),
	                                &(self->color_text),
	                                self->max_len,
	                                NULL, NULL);
	if(text == NULL)
	{
		return;
	}
	a3d_text_wrapx(text, self->text_wrapx);

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
	LOGD("debug w=%f, h=%f", w, h);

	a3d_textbox_t* self = (a3d_textbox_t*) widget;

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

	// determine maxi
	a3d_font_t* font   = a3d_screen_font(widget->screen);
	float       aspect = a3d_font_aspectRatio(font);
	float       size   = a3d_screen_layoutText(widget->screen,
	                                           self->style_text);
	int         maxi   = (int) (w/(aspect*size)) - 1;

	// maxi does not include null character
	// but max_len does
	// limit to max_len
	if((maxi >= self->max_len) ||
	   (maxi == 0))
	{
		maxi = self->max_len - 1;
	}

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
					a3d_textbox_printText(self, dst);
					a3d_textbox_printText(self, "");
				}
				else
				{
					a3d_textbox_printText(self, "");
				}
				dsti = 0;
				break;
			}

			if(dsti == 0)
			{
				strncpy(dst, tok, 256);
				dst[255] = '\0';
				dsti = toki;
			}
			else
			{
				if(dsti + toki + 1 <= maxi)
				{
					strcat(dst, " ");
					strcat(dst, tok);
					dst[255] = '\0';
					dsti += toki + 1;
				}
				else
				{
					a3d_textbox_printText(self, dst);

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
		a3d_textbox_printText(self, dst);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_textbox_t* a3d_textbox_new(a3d_screen_t* screen,
                               int wsize,
                               int orientation,
                               int anchor,
                               int wrapx, int wrapy,
                               int stretch_mode,
                               float stretch_factor,
                               int style_border,
                               int style_line,
                               a3d_vec4f_t* color_fill,
                               a3d_vec4f_t* color_line,
                               int text_anchor,
                               int text_wrapx,
                               int text_style_border,
                               int text_style_line,
                               int text_style_text,
                               a3d_vec4f_t* text_color_fill,
                               a3d_vec4f_t* text_color_line,
                               a3d_vec4f_t* text_color_text,
                               int text_max_len)
{
	assert(screen);
	assert(color_fill);
	assert(color_line);
	assert(text_color_fill);
	assert(text_color_line);
	assert(text_color_text);
	LOGD("debug wsize=%i, orientation=%i, anchor=%i, wrapx=%i, wrapy=%i",
	     wsize, orientation, anchor, wrapx, wrapy);
	LOGD("debug stretch_mode=%i, stretch_factor=%f, style_border=%i, style_line=%i",
	     stretch_mode, stretch_factor, style_border, style_line);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);
	LOGD("text_anchor=%i, text_style_border=%i, text_style_line=%i, text_style_text=%i",
	     text_anchor, text_style_border, text_style_line, text_style_text);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);
	LOGD("debug color_text: r=%f, g=%f, b=%f, a=%f",
	     color_text->r, color_text->g, color_text->b, color_text->a);
	LOGD("debug text_max_len=%i", text_max_len);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_textbox_t);
	}

	a3d_widget_reflow_fn reflow_fn = a3d_textbox_reflow;
	if(wrapx == A3D_WIDGET_WRAP_SHRINK)
	{
		reflow_fn = NULL;
	}

	a3d_textbox_t* self;
	self = (a3d_textbox_t*) a3d_listbox_new(screen,
	                                        wsize,
	                                        orientation,
	                                        anchor,
	                                        wrapx, wrapy,
	                                        stretch_mode,
	                                        stretch_factor,
	                                        style_border,
	                                        style_line,
	                                        color_fill,
	                                        color_line,
	                                        reflow_fn,
	                                        NULL);
	if(self == NULL)
	{
		return NULL;
	}

	self->strings = a3d_list_new();
	if(self->strings == NULL)
	{
		goto fail_strings;
	}

	self->dirty  = 1;
	self->last_w = 0.0f;
	self->last_h = 0.0f;

	self->anchor       = text_anchor;
	self->text_wrapx   = text_wrapx;
	self->style_border = text_style_border;
	self->style_line   = text_style_line;
	self->style_text   = text_style_text;
	self->max_len      = text_max_len;

	a3d_vec4f_copy(text_color_fill, &self->color_fill);
	a3d_vec4f_copy(text_color_line, &self->color_line);
	a3d_vec4f_copy(text_color_text, &self->color_text);

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
		LOGD("debug");

		a3d_textbox_clear(self);
		a3d_listbox_delete((a3d_listbox_t**) _self);
		*_self = NULL;
	}
}

void a3d_textbox_clear(a3d_textbox_t* self)
{
	assert(self);
	LOGD("debug");

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

	LOGD("debug %s", string);

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
