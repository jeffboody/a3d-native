/*
 * Copyright (c) 2018 Jeff Boody
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

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "math/a3d_vec2f.h"
#include "math/a3d_vec3f.h"
#include "a3d_polygon.h"
#include "../libtess2/Include/tesselator.h"

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* protected                                                *
***********************************************************/

extern void a3d_polygonShader_blend(a3d_polygonShader_t* self,
                                    int blend);

/***********************************************************
* private - polygon idx                                    *
***********************************************************/

typedef struct
{
	int    count;
	GLuint id;
} a3d_polygonIdx_t;

static a3d_polygonIdx_t* a3d_polygonIdx_new(int count)
{
	a3d_polygonIdx_t* self = (a3d_polygonIdx_t*)
	                         malloc(sizeof(a3d_polygonIdx_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->count = count;
	self->id    = 0;

	return self;
}

static void a3d_polygonIdx_delete(a3d_polygonIdx_t** _self)
{
	assert(_self);

	a3d_polygonIdx_t* self = *_self;
	if(self)
	{
		free(self);
		*_self = NULL;
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_polygon_t* a3d_polygon_new(int blend)
{
	a3d_polygon_t* self = (a3d_polygon_t*)
	                      malloc(sizeof(a3d_polygon_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	// attributes
	self->dirty = 0;
	self->blend = blend;
	self->layer = 0;
	a3d_vec4f_load(&self->color, 1.0f, 1.0f, 1.0f, 1.0f);

	self->list = a3d_list_new();
	if(self->list == NULL)
	{
		goto fail_list;
	}

	self->list_idx = a3d_list_new();
	if(self->list_idx == NULL)
	{
		goto fail_list_idx;
	}
	self->id_vtx = 0;
	self->gsize  = 0;

	// success
	return self;

	// failure
	fail_list_idx:
		a3d_list_delete(&self->list);
	fail_list:
		free(self);
	return NULL;
}

void a3d_polygon_delete(a3d_polygon_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_polygon_t* self = *_self;
	if(self)
	{
		a3d_polygon_evict(self);
		a3d_list_delete(&self->list_idx);

		a3d_listitem_t* iter = a3d_list_head(self->list);
		while(iter)
		{
			a3d_list_t* points = (a3d_list_t*)
			                     a3d_list_remove(self->list,
			                                     &iter);

			a3d_listitem_t* pter = a3d_list_head(points);
			while(pter)
			{
				a3d_vec2f_t* p = (a3d_vec2f_t*)
				                 a3d_list_remove(points, &pter);
				a3d_vec2f_delete(&p);
			}

			a3d_list_delete(&points);
		}
		a3d_list_delete(&self->list);

		free(self);
		*_self = NULL;
	}
}

void a3d_polygon_point(a3d_polygon_t* self, int first,
                       float x, float y)
{
	assert(self);

	a3d_list_t* points;
	if(first)
	{
		// add the point to a new list
		points = a3d_list_new();
		if(points == NULL)
		{
			return;
		}

		if(a3d_list_append(self->list, NULL, (const void*) points) == NULL)
		{
			a3d_list_delete(&points);
			return;
		}
	}
	else
	{
		// add the point to the last list
		a3d_listitem_t* iter = a3d_list_tail(self->list);
		if(iter == NULL)
		{
			return;
		}

		points = (a3d_list_t*)
		         a3d_list_peekitem(iter);
		if(points == NULL)
		{
			return;
		}

		// eliminate duplicate points
		a3d_vec2f_t* last = (a3d_vec2f_t*)
		                    a3d_list_peektail(points);
		if((last == NULL) ||
		   ((last->x == x) && (last->y == y)))
		{
			return;
		}
	}

	a3d_vec2f_t* p = a3d_vec2f_new(x, y);
	if(p == NULL)
	{
		return;
	}

	if(a3d_list_append(points, NULL, (const void*) p) == NULL)
	{
		a3d_vec2f_delete(&p);
		return;
	}

	self->dirty = 1;

	// success
	return;
}

int a3d_polygon_gsize(a3d_polygon_t* self)
{
	assert(self);

	return self->gsize;
}

void a3d_polygon_layer(a3d_polygon_t* self,
                       int layer)
{
	assert(self);

	self->layer = layer;
}

void a3d_polygon_color(a3d_polygon_t* self,
                       a3d_vec4f_t* color)
{
	assert(self);
	assert(color);

	a3d_vec4f_copy(color, &self->color);
}

int a3d_polygon_build(a3d_polygon_t* self)
{
	assert(self);

	if(self->id_vtx)
	{
		if(self->dirty)
		{
			a3d_polygon_evict(self);
		}
		else
		{
			return 1;
		}
	}

	if(a3d_list_size(self->list) == 0)
	{
		// at least one contour required
		return 0;
	}

	TESStesselator* tess = tessNewTess(NULL);
	if(tess == NULL)
	{
		LOGE("tessNewTess failed");
		return 0;
	}

	// generate contours
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		a3d_list_t* points = (a3d_list_t*)
		                     a3d_list_peekitem(iter);

		// at least 3 points required
		if(a3d_list_size(points) < 3)
		{
			iter = a3d_list_next(iter);
			continue;
		}

		// alloc temp points
		int    tmp_cnt = a3d_list_size(points);
		float* tmp_pts = (float*)
		                  malloc(2*tmp_cnt*sizeof(float));
		if(tmp_pts == NULL)
		{
			LOGE("malloc failed");
			goto fail_pts;
		}

		// copy temp points
		int idx = 0;
		a3d_listitem_t* pter = a3d_list_head(points);
		while(pter)
		{
			a3d_vec2f_t* v = (a3d_vec2f_t*)
			                 a3d_list_peekitem(pter);
			tmp_pts[idx++] = v->x;
			tmp_pts[idx++] = v->y;
			pter = a3d_list_next(pter);
		}

		tessAddContour(tess, 2, (const void*) tmp_pts,
		               2*sizeof(float), tmp_cnt);
		free(tmp_pts);

		iter = a3d_list_next(iter);
	}

	// build polygon(s)
	int polySize = 32;
	if(tessTesselate(tess, TESS_WINDING_ODD,
	                 TESS_POLYGONS, polySize, 2, NULL) == 0)
	{
		LOGE("tessTesselate failed");
		goto fail_tesselate;
	}

	// buffer vertices
	const GLfloat* vtx = tessGetVertices(tess);
	int vtx_count = tessGetVertexCount(tess);
	int gsize     = 0;
	glGenBuffers(1, &self->id_vtx);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vtx);
	glBufferData(GL_ARRAY_BUFFER,
	             2*vtx_count*sizeof(GLfloat),
	             vtx, GL_STATIC_DRAW);
	gsize += 2*vtx_count*4;

	// buffer indices
	int i;
	int ele_count = tessGetElementCount(tess);
	const TESSindex* elems = tessGetElements(tess);
	for(i = 0; i < ele_count; ++i)
	{
		// count the number of elements (j) in polyi
		const TESSindex* poly = &elems[i*polySize];
		int j;
		for(j = 0; j < polySize; ++j)
		{
			if(poly[j] == TESS_UNDEF)
			{
				break;
			}
		}

		a3d_polygonIdx_t* pi = a3d_polygonIdx_new(j);
		if(pi == NULL)
		{
			goto fail_polygon_idx;
		}

		if(a3d_list_enqueue(self->list_idx, (const void*) pi) == 0)
		{
			a3d_polygonIdx_delete(&pi);
			goto fail_polygon_idx;
		}

		// buffer data
		glGenBuffers(1, &pi->id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pi->id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		             pi->count*sizeof(GLushort),
		             poly, GL_STATIC_DRAW);
		gsize += pi->count*4;
	}
	self->gsize = gsize;
	self->dirty = 0;

	tessDeleteTess(tess);

	// success
	return 1;

	// failure
	fail_polygon_idx:
		self->dirty = 1;
	fail_tesselate:
	fail_pts:
		tessDeleteTess(tess);
	return 0;
}

void a3d_polygon_draw(a3d_polygon_t* self,
                      a3d_polygonShader_t* shader,
                      a3d_mat4f_t* mvp)
{
	assert(self);
	assert(shader);
	assert(mvp);

	// build polygons
	if(a3d_polygon_build(self) == 0)
	{
		return;
	}

	// optionally enable blending
	if(self->blend && (self->color.a < 1.0f))
	{
		a3d_polygonShader_blend(shader, 1);
	}
	else
	{
		a3d_polygonShader_blend(shader, 0);
	}

	// draw polygons
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vtx);
	glVertexAttribPointer(shader->attr_vtx, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glUniformMatrix4fv(shader->unif_mvp, 1, GL_FALSE, (GLfloat*) mvp);
	glUniform1i(shader->unif_layer, self->layer);
	glUniform1i(shader->unif_layers, shader->layers);
	glUniform4fv(shader->unif_color, 1, (GLfloat*) &self->color);
	a3d_listitem_t* iter = a3d_list_head(self->list_idx);
	while(iter)
	{
		a3d_polygonIdx_t* pi;
		pi = (a3d_polygonIdx_t*)
		     a3d_list_peekitem(iter);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pi->id);
		glDrawElements(GL_TRIANGLE_FAN, pi->count,
		               GL_UNSIGNED_SHORT, 0);
		iter = a3d_list_next(iter);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void a3d_polygon_evict(a3d_polygon_t* self)
{
	assert(self);

	if(self->id_vtx)
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &self->id_vtx);
		self->id_vtx = 0;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		a3d_listitem_t* iter = a3d_list_head(self->list_idx);
		while(iter)
		{
			a3d_polygonIdx_t* pi;
			pi = (a3d_polygonIdx_t*)
			      a3d_list_remove(self->list_idx, &iter);
			glDeleteBuffers(1, &pi->id);
			a3d_polygonIdx_delete(&pi);
		}
		self->gsize = 0;
		self->dirty = 0;
	}
}
