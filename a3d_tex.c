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

#include "a3d_tex.h"
#include <stdlib.h>
#include <assert.h>
#include <zlib.h>
#include <string.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int a3d_tex_readint(const unsigned char* buffer, int offset)
{
	assert(buffer);

	int b0 = (int) buffer[offset + 0];
	int b1 = (int) buffer[offset + 1];
	int b2 = (int) buffer[offset + 2];
	int b3 = (int) buffer[offset + 3];
	int o = (b3 << 24) & 0xFF000000;
	o = o | ((b2 << 16) & 0x00FF0000);
	o = o | ((b1 << 8) & 0x0000FF00);
	o = o | (b0 & 0x000000FF);
	return o;
}

static int a3d_tex_swapendian(int i)
{
	int o = (i << 24) & 0xFF000000;
	o = o | ((i << 8) & 0x00FF0000);
	o = o | ((i >> 8) & 0x0000FF00);
	o = o | ((i >> 24) & 0x000000FF);
	return o;
}

/***********************************************************
* public                                                   *
***********************************************************/

// texture state
const int A3D_TEX_ERROR    = 0;
const int A3D_TEX_LOADING  = 1;
const int A3D_TEX_LOADED   = 2;
const int A3D_TEX_CACHED   = 3;
const int A3D_TEX_UNLOADED = 4;

a3d_tex_t* a3d_tex_new(const char* fname)
{
	assert(fname);
	LOGD("debug fname=%s", fname);

	// allocate the tex
	a3d_tex_t* self = (a3d_tex_t*) malloc(sizeof(a3d_tex_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	// initialize member variables
	strncpy(self->fname, fname, 256);
	self->fname[255] = '\0';
	self->type    = 0;
	self->format  = 0;
	self->width   = 0;
	self->height  = 0;
	self->stride  = 0;
	self->vstride = 0;
	self->pixels  = NULL;
	self->id      = 0;

	return self;
}

void a3d_tex_delete(a3d_tex_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_tex_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		// should have been freed externally
		if(self->id) LOGW("leaking texture id");

		a3d_tex_reclaim(self);
		free(self);
		*_self = NULL;
	}
}

int a3d_tex_load(a3d_tex_t* self)
{
	assert(self);
	LOGD("debug fname=%s", self->fname);

	// check to see if the texture is already loaded
	if(self->pixels) return A3D_TEX_LOADED;

	// open texture
	gzFile f = gzopen(self->fname, "rb");
	if(!f)
	{
		LOGE("gzopen %s failed", self->fname);
		return A3D_TEX_ERROR;
	}

	// read header (28 bytes)
	unsigned char buffer[4096];   // 4KB
	int bytes_read = gzread(f, buffer, 28);
	if(bytes_read != 28)
	{
		LOGE("gzread %s failed - could not read header", self->fname);
		goto fail;
	}
	int magic = a3d_tex_readint(buffer, 0);
	if(magic == 0x000B00D9)
	{
		self->type    = a3d_tex_readint(buffer, 4);
		self->format  = a3d_tex_readint(buffer, 8);
		self->width   = a3d_tex_readint(buffer, 12);
		self->height  = a3d_tex_readint(buffer, 16);
		self->stride  = a3d_tex_readint(buffer, 20);
		self->vstride = a3d_tex_readint(buffer, 24);
	}
	else if(a3d_tex_swapendian(magic) == 0x000B00D9)
	{
		self->type    = a3d_tex_swapendian(a3d_tex_readint(buffer, 4));
		self->format  = a3d_tex_swapendian(a3d_tex_readint(buffer, 8));
		self->width   = a3d_tex_swapendian(a3d_tex_readint(buffer, 12));
		self->height  = a3d_tex_swapendian(a3d_tex_readint(buffer, 16));
		self->stride  = a3d_tex_swapendian(a3d_tex_readint(buffer, 20));
		self->vstride = a3d_tex_swapendian(a3d_tex_readint(buffer, 24));
	}
	else
	{
		LOGE("bad magic=0x%.8X", magic);
		goto fail;
	}

	// get texture size and check for supported formats
	int bytes = a3d_tex_size(self);
	if(bytes == 0)
	{
		LOGE("unsupported width=%i, height=%i, stride=%i, vstride=%i, format=0x%X, type=0x%X", self->width, self->height, self->stride, self->vstride, self->format, self->type);
		goto fail;
	}

	// allocate pixels
	self->pixels = (unsigned char*) malloc(sizeof(unsigned char) * bytes);
	if(self->pixels == NULL)
	{
		LOGE("malloc failed");
		goto fail;
	}

	// read texture
	unsigned char* pixels = self->pixels;
	while((bytes > 0) && ((bytes_read = gzread(f, pixels, bytes)) > 0))
	{
		pixels += bytes_read;
		bytes -= bytes_read;
	}

	// make sure we read the right size
	if(bytes != 0)
	{
		LOGE("invalid file size");
		free(self->pixels);
		self->pixels = NULL;
		goto fail;
	}

	// success
	gzclose(f);
	return A3D_TEX_LOADED;

	// failure
	fail:
	{
		// don't touch fname
		// id is already 0
		// pixels are already null
		self->type    = 0;
		self->format  = 0;
		self->width   = 0;
		self->height  = 0;
		self->stride  = 0;
		self->vstride = 0;
		gzclose(f);
		return A3D_TEX_ERROR;
	}
}

void a3d_tex_reclaim(a3d_tex_t* self)
{
	assert(self);

	if(self->pixels)
	{
		LOGD("debug");
		free(self->pixels);
		self->pixels = NULL;
	}
}

int a3d_tex_size(const a3d_tex_t* self)
{
	assert(self);

	int bpp = 0;   // bytes-per-pixel
	if     ((self->type == GL_UNSIGNED_BYTE)          && (self->format == GL_RGB))  bpp = 3;
	else if((self->type == GL_UNSIGNED_BYTE)          && (self->format == GL_RGBA)) bpp = 4;
	else if((self->type == GL_UNSIGNED_SHORT_5_6_5)   && (self->format == GL_RGB))  bpp = 2;
	else if((self->type == GL_UNSIGNED_SHORT_4_4_4_4) && (self->format == GL_RGBA)) bpp = 2;
	else if((self->type == GL_UNSIGNED_SHORT_5_5_5_1) && (self->format == GL_RGBA)) bpp = 2;

	return bpp * self->stride * self->vstride;
}
