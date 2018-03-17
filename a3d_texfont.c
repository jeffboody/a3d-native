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

#include "a3d_texstring.h"
#include "a3d_texfont.h"
#include "../texgz/texgz_tex.h"
#include "../texgz/texgz_png.h"
#include "../libpak/pak_file.h"
#include "../libexpat/expat/lib/expat.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// define cursor as ASCII unit separator
#define A3D_TEXFONT_CURSOR 31

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static texgz_tex_t* a3d_texfont_loadTex(a3d_texfont_t* self,
                                        const char* resource,
                                        const char* texname)
{
	assert(self);
	assert(resource);
	assert(texname);

	if(texname[0] != '$')
	{
		LOGE("invalid %s", texname);
		return NULL;
	}

	pak_file_t* pak = pak_file_open(resource, PAK_FLAG_READ);
	if(pak == NULL)
	{
		return NULL;
	}

	const char* key = &(texname[1]);
	int size = pak_file_seek(pak, key);
	if(size == 0)
	{
		goto fail_seek;
	}

	texgz_tex_t* tex = texgz_png_importf(pak->f);
	if(tex == NULL)
	{
		goto fail_tex;
	}

	pak_file_close(&pak);

	// success
	return tex;

	// failure
	fail_tex:
	fail_seek:
		pak_file_close(&pak);
	return NULL;
}

static void a3d_texfont_parseStart(void* _self,
                                   const XML_Char* name,
                                   const XML_Char** atts)
{
	assert(_self);
	assert(name);
	assert(atts);

	a3d_texfont_t* self = (a3d_texfont_t*) _self;

	if(strcmp(name, "font") == 0)
	{
		int idx = 0;
		while(atts[idx] && atts[idx + 1])
		{
			const XML_Char* key = atts[idx];
			const XML_Char* val = atts[idx + 1];
			if(strcmp(key, "size") == 0)
			{
				self->size = (int) strtol(val, NULL, 0);
			}
			else if(strcmp(key, "h") == 0)
			{
				self->h = (int) strtol(val, NULL, 0);
			}
			idx += 2;
		}
	}
	else if(strcmp(name, "coords") == 0)
	{
		int c   = 0;
		int x   = 0;
		int y   = 0;
		int w   = 0;
		int idx = 0;
		while(atts[idx] && atts[idx + 1])
		{
			const XML_Char* key = atts[idx];
			const XML_Char* val = atts[idx + 1];
			if(strcmp(key, "c") == 0)
			{
				c = (int) strtol(val, NULL, 0);
			}
			else if(strcmp(key, "x") == 0)
			{
				x = (int) strtol(val, NULL, 0);
			}
			else if(strcmp(key, "y") == 0)
			{
				y = (int) strtol(val, NULL, 0);
			}
			else if(strcmp(key, "w") == 0)
			{
				w = (int) strtol(val, NULL, 0);
			}
			idx += 2;
		}

		// check for a ascii/cursor character
		if((c >= 31) && (c <= 126))
		{
			self->coords[c].x = x;
			self->coords[c].y = y;
			self->coords[c].w = w;
		}
	}
}

static void a3d_texfont_parseEnd(void* _self,
                                 const XML_Char* name)
{
	assert(_self);
	assert(name);

	// ignore
}

static int a3d_texfont_parseXml(a3d_texfont_t* self,
                                const char* resource,
                                const char* key)
{
	assert(self);
	assert(resource);
	assert(key);

	pak_file_t* pak = pak_file_open(resource,
	                                PAK_FLAG_READ);
	if(pak == NULL)
	{
		return 0;
	}

	int size = pak_file_seek(pak, key);
	if(size == 0)
	{
		goto fail_seek;
	}

	XML_Parser xml = XML_ParserCreate("UTF-8");
	if(xml == NULL)
	{
		LOGE("XML_ParserCreate failed");
		goto fail_xml;
	}
	XML_SetUserData(xml, (void*) self);
	XML_SetElementHandler(xml,
	                      a3d_texfont_parseStart,
	                      a3d_texfont_parseEnd);

	int done = 0;
	while(done == 0)
	{
		void* buf = XML_GetBuffer(xml, 4096);
		if(buf == NULL)
		{
			LOGE("XML_GetBuffer buf=NULL");
			goto fail_parse;
		}

		int bytes = fread(buf, 1, size > 4096 ? 4096 : size, pak->f);
		if(bytes < 0)
		{
			LOGE("read failed");
			goto fail_parse;
		}

		size -= bytes;
		done = (size == 0) ? 1 : 0;
		if(XML_ParseBuffer(xml, bytes, done) == 0)
		{
			// make sure str is null terminated
			char* str = (char*) buf;
			str[(bytes > 0) ? (bytes - 1) : 0] = '\0';

			enum XML_Error e = XML_GetErrorCode(xml);
			LOGE("XML_ParseBuffer err=%s, bytes=%i, buf=%s",
			     XML_ErrorString(e), bytes, str);
			goto fail_parse;
		}
	}

	XML_ParserFree(xml);
	pak_file_close(&pak);

	// success
	return 1;

	// failure
	fail_parse:
		XML_ParserFree(xml);
	fail_xml:
	fail_seek:
		pak_file_close(&pak);
	return 0;
}

static int a3d_texfont_loadCoords(a3d_texfont_t* self,
                                  const char* resource,
                                  const char* xmlname)
{
	assert(self);
	assert(resource);
	assert(xmlname);

	if(xmlname[0] != '$')
	{
		LOGE("invalid %s", xmlname);
		return 0;
	}

	const char* key = &(xmlname[1]);
	if(a3d_texfont_parseXml(self, resource, key) == 0)
	{
		return 0;
	}

	// validate xml
	// check height
	if(self->h > self->tex_height)
	{
		LOGE("invalid h=%i, tex_height=%i",
		     self->h, self->tex_height);
		return 0;
	}

	// check font size
	if(self->size > self->h)
	{
		LOGE("invalid size=%i, h=%i",
		     self->size, self->h);
		return 0;
	}

	// check ascii/cursor character(s)
	int c;
	for(c = 31; c <= 126; ++c)
	{
		int W = self->tex_width;
		int H = self->tex_height;
		int w = self->coords[c].w;
		int h = self->h;
		int t = self->coords[c].y;
		int l = self->coords[c].x;
		int b = self->coords[c].y + h - 1;
		int r = self->coords[c].x + w - 1;
		if((t <  0) || (t >= H) ||
		   (l <  0) || (l >= W) ||
		   (b <  0) || (b >= H) ||
		   (r <  0) || (r >= W) ||
		   (w <= 0) || (w > W)  ||
		   (h <= 0) || (h > H))
		{
			LOGE("invalid c=0x%X=%c, t=%i, l=%i, b=%i, r=%i, w=%i, h=%i, W=%i, H=%i",
			     c, (char) c, t, l, b, r, w, h, W, H);
			return 0;
		}
	}

	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_texfont_t* a3d_texfont_new(const char* resource,
                               const char* texname,
                               const char* xmlname)
{
	assert(resource);
	assert(texname);
	assert(xmlname);
	LOGD("debug");

	a3d_texfont_t* self = (a3d_texfont_t*)
	                      malloc(sizeof(a3d_texfont_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	texgz_tex_t* tex = a3d_texfont_loadTex(self, resource, texname);
	if(tex == NULL)
	{
		goto fail_tex;
	}
	self->tex_width  = tex->width;
	self->tex_height = tex->height;

	if(a3d_texfont_loadCoords(self, resource, xmlname) == 0)
	{
		goto fail_coords;
	}

	// measure printable ascii characters
	int c;
	int w = 0;
	int h = 0;
	for(c = 32; c <= 126; ++c)
	{
		w += a3d_texfont_width(self, c);
		h += a3d_texfont_height(self);
	}
	self->aspect_ratio_avg = (float) w/(float) h;

	glGenTextures(1, &self->id);
	glBindTexture(GL_TEXTURE_2D, self->id);
	#if defined(A3D_GLESv1_CM)
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	#else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	#endif
	glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->stride, tex->vstride,
	             0, tex->format, tex->type, tex->pixels);
	texgz_tex_delete(&tex);

	// success
	return self;

	// failure
	fail_coords:
		texgz_tex_delete(&tex);
	fail_tex:
		free(self);
	return NULL;
}

void a3d_texfont_delete(a3d_texfont_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_texfont_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &self->id);
		free(self);
		*_self = NULL;
	}
}

void a3d_texfont_request(a3d_texfont_t* self,
                         int mode, char c,
                         a3d_regionf_t* tc,
                         a3d_regionf_t* vc)
{
	assert(self);
	assert(tc);
	assert(vc);
	LOGD("debug");

	// check for a ascii/cursor character
	if((c < 31) && (c > 126))
	{
		c = A3D_TEXFONT_CURSOR;
	}

	float w = (float) self->coords[(int) c].w;
	float h = (float) self->h;
	float W = (float) (self->tex_width  - 1);
	float H = (float) (self->tex_height - 1);

	// fill in tex coords
	tc->t = (float) self->coords[(int) c].y;
	tc->l = (float) self->coords[(int) c].x;
	tc->b = tc->t + h - 1.0f;
	tc->r = tc->l + w - 1.0f;
	tc->t /= H;
	tc->l /= W;
	tc->b /= H;
	tc->r /= W;

	// fill in vertex coords
	if(mode == A3D_TEXSTRING_2D)
	{
	        vc->t = 0.0f;
	        vc->b = 1.0f;
	}
	else
	{
	        vc->t = 1.0f;
	        vc->b = 0.0f;
	}
	vc->l = 0.0f;
	vc->r = w/h;
}

int a3d_texfont_width(a3d_texfont_t* self, char c)
{
	assert(self);

	// check for a ascii/cursor character
	if((c >= 31) && (c <= 126))
	{
		return self->coords[(int) c].w;
	}
	return 0;
}

int a3d_texfont_height(a3d_texfont_t* self)
{
	assert(self);

	return self->h;
}
