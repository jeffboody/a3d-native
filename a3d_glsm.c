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

#include "a3d_glsm.h"
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_glsm_draincache(a3d_glsm_t* self)
{
	assert(self);
	LOGD("debug");

	// empty the vb list
	a3d_vec3f_t* v = (a3d_vec3f_t*) a3d_list_dequeue(self->cache_vb);
	while(v)
	{
		free(v);
		v = (a3d_vec3f_t*) a3d_list_dequeue(self->cache_vb);
	}

	// empty the nb list
	v = (a3d_vec3f_t*) a3d_list_dequeue(self->cache_nb);
	while(v)
	{
		free(v);
		v = (a3d_vec3f_t*) a3d_list_dequeue(self->cache_nb);
	}
}

static void a3d_glsm_freebuffers(a3d_glsm_t* self)
{
	assert(self);
	LOGD("debug");

	free(self->vb);
	free(self->nb);
	self->vb = NULL;
	self->nb = NULL;
}

/***********************************************************
* public                                                   *
***********************************************************/

const int A3D_GLSM_COMPLETE   = 0;
const int A3D_GLSM_INCOMPLETE = 1;
const int A3D_GLSM_ERROR      = 2;

a3d_glsm_t* a3d_glsm_new(void)
{
	LOGD("debug");

	a3d_glsm_t* self = (a3d_glsm_t*) malloc(sizeof(a3d_glsm_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->normal.x = 0.0f;
	self->normal.y = 0.0f;
	self->normal.z = 1.0f;
	self->ec       = 0;
	self->vb       = NULL;
	self->nb       = NULL;
	self->status   = A3D_GLSM_INCOMPLETE;

	self->cache_vb = a3d_list_new();
	if(self->cache_vb == NULL)
		goto fail_cache_vb;

	self->cache_nb = a3d_list_new();
	if(self->cache_nb == NULL)
		goto fail_cache_nb;

	// success
	return self;

	// failure
	fail_cache_nb:
		a3d_list_delete(&self->cache_vb);
	fail_cache_vb:
		free(self);
	return NULL;
}

void a3d_glsm_delete(a3d_glsm_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_glsm_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		a3d_glsm_draincache(self);
		a3d_list_delete(&self->cache_vb);
		a3d_list_delete(&self->cache_nb);
		a3d_glsm_freebuffers(self);
		free(self);
		*_self = NULL;
	}
}

void a3d_glsm_begin(a3d_glsm_t* self)
{
	assert(self);
	LOGD("debug");

	self->normal.x = 0.0f;
	self->normal.y = 0.0f;
	self->normal.z = 1.0f;
	self->ec       = 0;
	self->status   = A3D_GLSM_INCOMPLETE;
	a3d_glsm_draincache(self);
	a3d_glsm_freebuffers(self);
}

void a3d_glsm_normal3f(a3d_glsm_t* self, float x, float y, float z)
{
	assert(self);
	LOGD("debug");

	if(self->status != A3D_GLSM_INCOMPLETE) return;

	self->normal.x = x;
	self->normal.y = y;
	self->normal.z = z;
}

void a3d_glsm_vertex3f(a3d_glsm_t* self, float x, float y, float z)
{
	assert(self);
	LOGD("debug");

	if(self->status != A3D_GLSM_INCOMPLETE) return;

	a3d_vec3f_t* v = (a3d_vec3f_t*) malloc(sizeof(a3d_vec3f_t*));
	if(v == NULL)
	{
		LOGE("malloc failed");
		goto fail_malloc_v;
	}
	v->x = x;
	v->y = y;
	v->z = z;

	if(a3d_list_enqueue(self->cache_vb, (const void*) v) == 0)
		goto fail_append_vb;

	a3d_vec3f_t* n = (a3d_vec3f_t*) malloc(sizeof(a3d_vec3f_t*));
	if(n == NULL)
	{
		LOGE("malloc failed");
		goto fail_malloc_n;
	}
	n->x = self->normal.x;
	n->y = self->normal.y;
	n->z = self->normal.z;

	if(a3d_list_enqueue(self->cache_nb, (const void*) n) == 0)
		goto fail_append_nb;

	// success
	return;

	// failure
	fail_append_nb:
		free(n);
	fail_malloc_n:
		// cache drained below
	fail_append_vb:
		free(v);
	fail_malloc_v:
		a3d_glsm_draincache(self);
		self->status = A3D_GLSM_ERROR;
}

void a3d_glsm_end(a3d_glsm_t* self)
{
	assert(self);
	LOGD("debug");

	if(self->status != A3D_GLSM_INCOMPLETE) return;

	// initialize buffers
	self->ec = a3d_list_size(self->cache_vb);   // vertex count
	self->vb = (GLfloat*) malloc(3 * self->ec * sizeof(GLfloat));
	self->nb = (GLfloat*) malloc(3 * self->ec * sizeof(GLfloat));
	if((self->vb == NULL) || (self->nb == NULL))
	{
		LOGE("malloc failed");
		goto fail_malloc;
	}

	// replay the cached vertices and normals
	GLsizei vi;
	a3d_vec3f_t* v;
	a3d_vec3f_t* n;
	for(vi = 0; vi < self->ec; ++vi)
	{
		v = (a3d_vec3f_t*) a3d_list_dequeue(self->cache_vb);
		if(v == NULL)
			goto fail_vert;

		n = (a3d_vec3f_t*) a3d_list_dequeue(self->cache_nb);
		if(n == NULL)
			goto fail_norm;
	
		self->vb[3 * vi    ] = v->x;
		self->vb[3 * vi + 1] = v->y;
		self->vb[3 * vi + 2] = v->z;
		self->nb[3 * vi    ] = n->x;
		self->nb[3 * vi + 1] = n->y;
		self->nb[3 * vi + 2] = n->z;

		// free the cache as we go
		free(v);
		free(n);
	}

	// success
	self->status = A3D_GLSM_COMPLETE;
	return;

	// failure
	fail_norm:
		free(v);
	fail_vert:
		a3d_glsm_freebuffers(self);
	fail_malloc:
		a3d_glsm_draincache(self);
	self->status = A3D_GLSM_ERROR;
}

int a3d_glsm_status(a3d_glsm_t* self)
{
	assert(self);
	LOGD("debug status=%i", self->status);
	return self->status;
}
