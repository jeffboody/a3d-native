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
#include "a3d_line.h"

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* protected                                                *
***********************************************************/

extern void a3d_lineShader_blend(a3d_lineShader_t* self,
                                 int blend);

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_line_intersect(const a3d_vec2f_t* p1, const a3d_vec2f_t* p2,
                               const a3d_vec2f_t* p3, const a3d_vec2f_t* p4,
                               a3d_vec2f_t* p, float* _t)
{
	assert(p1);
	assert(p2);
	assert(p3);
	assert(p4);
	assert(p);
	assert(_t);

	// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

	// check for parallel lines
	float x1 = p1->x;
	float x2 = p2->x;
	float x3 = p3->x;
	float x4 = p4->x;
	float y1 = p1->y;
	float y2 = p2->y;
	float y3 = p3->y;
	float y4 = p4->y;
	float d = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4);
	if(fabs(d) < 0.001)
	{
		p->x = x2;
		p->y = y2;
		return;
	}

	// compute intersection point
	float n = (x1 - x3)*(y3 - y4) - (y1 - y3)*(x3 - x4);
	float t = n/d;
	p->x = x1 + t*(x2 - x1);
	p->y = y1 + t*(y2 - y1);
	*_t = t;
}

static void a3d_line_deleteVbo(a3d_line_t* self)
{
	assert(self);

	if(self->id_vtx)
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &self->id_vtx);
		glDeleteBuffers(1, &self->id_st);
		self->id_vtx    = 0;
		self->id_st     = 0;
		self->vtx_count = 0;
		self->gsize     = 0;
		self->dirty     = 0;
	}
}

static void a3d_line_buildContour(a3d_line_t* self,
                                  int first, int last,
                                  a3d_list_t* contour,
                                  GLfloat* vtx,
                                  GLfloat* st,
                                  float* _sab, float* _sbc,
                                  int* _idx)
{
	assert(self);
	assert(contour);
	assert(vtx);
	assert(st);
	assert(_sab);
	assert(_sbc);
	assert(_idx);

	float sab = *_sab;
	float sbc = *_sbc;
	int   idx = *_idx;

	// skip the degenerate vertex
	int first_idx = idx;
	if(first == 0)
	{
		idx += 2;
	}

	/*
	 * Corners Diagram
	 *
	 *    q----g----r----------f    i    c    k
	 *    |         |               |         |
	 *    |         |               |         |  RIGHT TURN
	 *    j    b    h          a    |         |  tp > 1
	 *    |         |               |         |
	 *    |         |               |         |
	 *    s----e----p----------d    r----g----q----------f
	 *    |         |               |         |
	 *    |         |               |         |
	 *    |         |               h    b    j          a
	 *    |         |  LEFT TURN    |         |
	 *    |         |  tp < 1       |         |
	 *    k    c    i               p----e----s----------d
	 *
	 * Triangle Strips:
	 * LEFT:  df-pr-pq-ps-ik
	 * RIGHT: df-sq-pq-rq-ik
	 *
	 */

	// build line
	a3d_vec2f_t p;
	a3d_vec2f_t q;
	a3d_vec2f_t r;
	a3d_vec2f_t s;
	a3d_listitem_t* iter = a3d_list_head(contour);
	while(iter)
	{
		a3d_listitem_t* prev = a3d_list_prev(iter);
		a3d_listitem_t* next = a3d_list_next(iter);

		a3d_vec2f_t* a = prev ? (a3d_vec2f_t*) a3d_list_peekitem(prev) : NULL;
		a3d_vec2f_t* b = (a3d_vec2f_t*) a3d_list_peekitem(iter);
		a3d_vec2f_t* c = next ? (a3d_vec2f_t*) a3d_list_peekitem(next) : NULL;

		// connect the loop
		if(self->loop)
		{
			if(a == NULL)
			{
				a = (a3d_vec2f_t*) a3d_list_peektail(contour);
			}
			else if(c == NULL)
			{
				c = (a3d_vec2f_t*) a3d_list_peekhead(contour);
			}
		}

		// handle endpoints
		if((a == NULL) || (c == NULL))
		{
			a3d_vec2f_t e;
			float se; // direction to vtx
			if(a == NULL)
			{
				se = -1.0f;
				a3d_vec2f_subv_copy(c, b, &e);
			}
			else
			{
				se = 1.0f;
				a3d_vec2f_subv_copy(b, a, &e);
				sab += a3d_vec2f_mag(&e);
			}
			a3d_vec2f_normalize(&e);

			a3d_vec3f_t t  = { e.x, e.y, 0.0f  };
			a3d_vec3f_t up = { 0.0f, 0.0f, 1.0f };
			a3d_vec3f_t v;
			a3d_vec3f_cross_copy(&t, &up, &v);
			a3d_vec3f_muls(&v, self->width/2.0f);
			a3d_vec2f_load(&p, b->x, b->y);
			a3d_vec2f_load(&q, b->x, b->y);
			a3d_vec2f_subv(&p, (const a3d_vec2f_t*) &v);
			a3d_vec2f_addv(&q, (const a3d_vec2f_t*) &v);
			if(self->rounded)
			{
				// add a rounded cap
				a3d_vec2f_muls(&e, se*self->width/2.0f);
				a3d_vec2f_addv(&p, (const a3d_vec2f_t*) &e);
				a3d_vec2f_addv(&q, (const a3d_vec2f_t*) &e);
			}
			st[idx]    = sab;
			vtx[idx++] = p.x;
			st[idx]    = -1.0f;
			vtx[idx++] = p.y;
			st[idx]    = sab;
			vtx[idx++] = q.x;
			st[idx]    = 1.0f;
			vtx[idx++] = q.y;
			iter = a3d_list_next(iter);
			continue;
		}

		// compute the vectors for ab and bc
		a3d_vec2f_t ab;
		a3d_vec2f_t bc;
		a3d_vec2f_t uab;
		a3d_vec2f_t ubc;
		a3d_vec2f_subv_copy(b, a, &ab);
		a3d_vec2f_subv_copy(c, b, &bc);
		sab += a3d_vec2f_mag(&ab);
		sbc  = sab + a3d_vec2f_mag(&bc);
		a3d_vec2f_normalize_copy(&ab, &uab);
		a3d_vec2f_normalize_copy(&bc, &ubc);

		// compute points on parallel lines to ab and bc
		a3d_vec2f_t d    = { a->x, a->y };
		a3d_vec2f_t f    = { a->x, a->y };
		a3d_vec2f_t e    = { b->x, b->y };
		a3d_vec2f_t g    = { b->x, b->y };
		a3d_vec2f_t h    = { b->x, b->y };
		a3d_vec2f_t j    = { b->x, b->y };
		a3d_vec2f_t i    = { c->x, c->y };
		a3d_vec2f_t k    = { c->x, c->y };
		a3d_vec3f_t uab3 = { uab.x, uab.y, 0.0f };
		a3d_vec3f_t ubc3 = { ubc.x, ubc.y, 0.0f };
		a3d_vec3f_t up   = { 0.0f,  0.0f,  1.0f };
		a3d_vec3f_t vab;
		a3d_vec3f_t vbc;
		a3d_vec3f_cross_copy(&uab3, &up, &vab);
		a3d_vec3f_cross_copy(&ubc3, &up, &vbc);
		a3d_vec3f_muls(&vab, self->width/2.0f);
		a3d_vec3f_muls(&vbc, self->width/2.0f);
		a3d_vec2f_subv(&d, (const a3d_vec2f_t*) &vab);
		a3d_vec2f_addv(&f, (const a3d_vec2f_t*) &vab);
		a3d_vec2f_subv(&e, (const a3d_vec2f_t*) &vab);
		a3d_vec2f_addv(&g, (const a3d_vec2f_t*) &vab);
		a3d_vec2f_subv(&h, (const a3d_vec2f_t*) &vbc);
		a3d_vec2f_addv(&j, (const a3d_vec2f_t*) &vbc);
		a3d_vec2f_subv(&i, (const a3d_vec2f_t*) &vbc);
		a3d_vec2f_addv(&k, (const a3d_vec2f_t*) &vbc);

		// compute intersection points of the parallel lines
		float tp = 1.0f;
		float tq = 1.0f;
		a3d_line_intersect(&d, &e, &h, &i, &p, &tp);
		a3d_line_intersect(&f, &g, &j, &k, &q, &tq);
		a3d_vec3f_muls(&vab, 2.0f);
		a3d_vec3f_muls(&vbc, 2.0f);

		if(tp < 1.0f)
		{
			a3d_vec2f_addv_copy(&p, (const a3d_vec2f_t*) &vab, &r);
			a3d_vec2f_addv_copy(&p, (const a3d_vec2f_t*) &vbc, &s);

			st[idx]    = sab;
			vtx[idx++] = p.x;
			st[idx]    = -1.0f;
			vtx[idx++] = p.y;
			st[idx]    = sab;
			vtx[idx++] = r.x;
			st[idx]    = 1.0f;
			vtx[idx++] = r.y;

			st[idx]    = sab;
			vtx[idx++] = p.x;
			st[idx]    = -1.0f;
			vtx[idx++] = p.y;
			st[idx]    = sab;
			vtx[idx++] = q.x;
			st[idx]    = 1.0f;
			vtx[idx++] = q.y;

			st[idx]    = sab;
			vtx[idx++] = p.x;
			st[idx]    = -1.0f;
			vtx[idx++] = p.y;
			st[idx]    = sab;
			vtx[idx++] = s.x;
			st[idx]    = 1.0f;
			vtx[idx++] = s.y;
		}
		else
		{
			a3d_vec2f_subv_copy(&q, (const a3d_vec2f_t*) &vab, &s);
			a3d_vec2f_subv_copy(&q, (const a3d_vec2f_t*) &vbc, &r);

			st[idx]    = sab;
			vtx[idx++] = s.x;
			st[idx]    = -1.0f;
			vtx[idx++] = s.y;
			st[idx]    = sab;
			vtx[idx++] = q.x;
			st[idx]    = 1.0f;
			vtx[idx++] = q.y;

			st[idx]    = sab;
			vtx[idx++] = p.x;
			st[idx]    = -1.0f;
			vtx[idx++] = p.y;
			st[idx]    = sab;
			vtx[idx++] = q.x;
			st[idx]    = 1.0f;
			vtx[idx++] = q.y;

			st[idx]    = sab;
			vtx[idx++] = r.x;
			st[idx]    = -1.0f;
			vtx[idx++] = r.y;
			st[idx]    = sab;
			vtx[idx++] = q.x;
			st[idx]    = 1.0f;
			vtx[idx++] = q.y;
		}

		iter = a3d_list_next(iter);
	}

	// close loop
	if(self->loop)
	{
		st[idx]      = sbc;
		vtx[idx++]   = vtx[0];
		st[idx]      = -1.0f;
		vtx[idx++]   = vtx[1];
		st[idx]      = sbc;
		vtx[idx++]   = vtx[2];
		st[idx]      = 1.0f;
		vtx[idx++]   = vtx[3];
		self->length = sbc;
	}
	else
	{
		self->length = sab;
	}

	// add degenerate vertices
	// e.g. repeat the first vertex and last vertex on a
	// triangle strip
	if(first == 0)
	{
		st[first_idx]      = st[first_idx + 2];
		vtx[first_idx]     = vtx[first_idx + 2];
		st[first_idx + 1]  = st[first_idx + 3];
		vtx[first_idx + 1] = vtx[first_idx + 3];
	}

	if(last == 0)
	{
		int last_idx = idx - 2;
		st[idx]    = st[last_idx];
		vtx[idx++] = vtx[last_idx];
		st[idx]    = st[last_idx + 1];
		vtx[idx++] = vtx[last_idx + 1];
	}

	// update build state
	*_sab = sab;
	*_sbc = sbc;
	*_idx = idx;
}

static int a3d_line_build(a3d_line_t* self)
{
	assert(self);

	if(self->id_vtx)
	{
		if(self->dirty)
		{
			a3d_line_deleteVbo(self);
		}
		else
		{
			return 1;
		}
	}

	// determine vtx_count
	int vtx_count = 0;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		a3d_list_t* contour = (a3d_list_t*)
		                      a3d_list_peekitem(iter);

		// at least 2 points required
		if(a3d_list_size(contour) < 2)
		{
			iter = a3d_list_next(iter);
			continue;
		}

		if(self->loop)
		{
			// 6 for interior points
			// 2 to connect loop
			vtx_count += 6*(a3d_list_size(contour)) + 2;
		}
		else
		{
			// 6 for interior points
			// 2 for end points
			vtx_count += 6*(a3d_list_size(contour) - 2) + 4;
		}

		// add degenerate triangle vertices
		int first = a3d_list_prev(iter) ? 0 : 1;
		int last  = a3d_list_next(iter) ? 0 : 1;
		if((first == 0) && (last == 0))
		{
			vtx_count += 2;
		}
		else if((first == 0) || (last == 0))
		{
			vtx_count += 1;
		}

		iter = a3d_list_next(iter);
	}

	GLfloat* vtx = (GLfloat*) malloc(2*vtx_count*sizeof(GLfloat));
	if(vtx == NULL)
	{
		LOGE("malloc failed");
		return 0;
	}

	GLfloat* st = (GLfloat*) malloc(2*vtx_count*sizeof(GLfloat));
	if(st == NULL)
	{
		LOGE("malloc failed");
		goto fail_st;
	}

	// build contours
	float sab = 0.0f;
	float sbc = 0.0f;
	int   idx = 0;
	iter = a3d_list_head(self->list);
	while(iter)
	{
		a3d_list_t* contour = (a3d_list_t*)
		                      a3d_list_peekitem(iter);

		// at least 2 points required
		if(a3d_list_size(contour) < 2)
		{
			iter = a3d_list_next(iter);
			continue;
		}

		int first = a3d_list_prev(iter) ? 0 : 1;
		int last  = a3d_list_next(iter) ? 0 : 1;
		a3d_line_buildContour(self, first, last,
		                      contour, vtx, st,
		                      &sab, &sbc, &idx);

		iter = a3d_list_next(iter);
	}

	// buffer data
	glGenBuffers(1, &self->id_vtx);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vtx);
	glBufferData(GL_ARRAY_BUFFER,
	             2*vtx_count*sizeof(GLfloat),
	             vtx, GL_STATIC_DRAW);
	glGenBuffers(1, &self->id_st);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_st);
	glBufferData(GL_ARRAY_BUFFER,
	             2*vtx_count*sizeof(GLfloat),
	             st, GL_STATIC_DRAW);
	self->vtx_count = vtx_count;
	self->gsize     = 4*vtx_count*4;
	self->dirty     = 0;

	free(vtx);
	free(st);

	// success
	return 1;

	// failure
	fail_st:
		free(vtx);
	return 0;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_line_t* a3d_line_new(int loop)
{
	a3d_line_t* self = (a3d_line_t*)
	                   malloc(sizeof(a3d_line_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	// attributes
	self->dirty   = 0;
	self->loop    = 0;
	self->width   = 1.0f;
	self->length  = 0.0f;
	self->rounded = 0;
	self->brush1  = 1.0f;
	self->brush2  = 0.0f;
	self->stripe  = 1.0f;
	self->blend   = 0;
	a3d_vec4f_load(&self->color1, 1.0f, 1.0f, 1.0f, 1.0f);
	a3d_vec4f_load(&self->color2, 1.0f, 1.0f, 1.0f, 1.0f);

	self->list = a3d_list_new();
	if(self->list == NULL)
	{
		goto fail_list;
	}

	self->id_vtx = 0;
	self->id_st  = 0;
	self->gsize  = 0;

	// success
	return self;

	// failure
	fail_list:
		free(self);
	return NULL;
}

void a3d_line_delete(a3d_line_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_line_t* self = *_self;
	if(self)
	{
		a3d_line_deleteVbo(self);

		a3d_listitem_t* iter1 = a3d_list_head(self->list);
		while(iter1)
		{
			a3d_list_t* contour = (a3d_list_t*)
			                      a3d_list_remove(self->list, &iter1);

			a3d_listitem_t* iter2 = a3d_list_head(contour);
			while(iter2)
			{
				a3d_vec2f_t* p = (a3d_vec2f_t*)
				                 a3d_list_remove(contour, &iter2);
				a3d_vec2f_delete(&p);
			}

			a3d_list_delete(&contour);
		}

		a3d_list_delete(&self->list);
		free(self);
		*_self = NULL;
	}
}

void a3d_line_point(a3d_line_t* self, int first, float x, float y)
{
	assert(self);

	a3d_list_t*     contour;
	a3d_listitem_t* contour_iter = NULL;
	if(first)
	{
		// create a new contour
		contour = a3d_list_new();
		if(contour == NULL)
		{
			return;
		}

		contour_iter = a3d_list_append(self->list, NULL,
		                               (const void*) contour);
		if(contour_iter == NULL)
		{
			goto fail_append_contour;
		}
	}
	else
	{
		// get the last contour
		contour = (a3d_list_t*)
		          a3d_list_peektail(self->list);
		if(contour == NULL)
		{
			LOGE("invalid contour");
			return;
		}
	}

	// eliminate duplicate points
	a3d_vec2f_t* tail = (a3d_vec2f_t*)
	                    a3d_list_peektail(contour);
	if(tail && (tail->x == x) && (tail->y == y))
	{
		goto fail_dup;
	}

	a3d_vec2f_t* p = a3d_vec2f_new(x, y);
	if(p == NULL)
	{
		goto fail_new_pt;
	}

	if(a3d_list_append(contour, NULL, (const void*) p) == NULL)
	{
		goto fail_append_pt;
	}

	self->dirty = 1;

	// success
	return;

	// failure
	fail_append_pt:
		a3d_vec2f_delete(&p);
	fail_new_pt:
	fail_dup:
	{
		if(contour_iter)
		{
			a3d_list_remove(self->list, &contour_iter);
		}
	}
	fail_append_contour:
	{
		if(first)
		{
			a3d_list_delete(&contour);
		}
	}
}

int a3d_line_gsize(a3d_line_t* self)
{
	assert(self);

	return self->gsize;
}

void a3d_line_blend(a3d_line_t* self, int blend)
{
	assert(self);

	self->blend = blend;
}

void a3d_line_rounded(a3d_line_t* self, int rounded)
{
	assert(self);

	if(self->rounded == rounded)
	{
		return;
	}

	// set the dirty flag since the rounded flag causes
	// line geometry to include a cap
	self->rounded = rounded;
	self->dirty   = 1;
}

void a3d_line_width(a3d_line_t* self, float width)
{
	assert(self);

	if(self->width == width)
	{
		return;
	}

	// set the dirty flag since the width causes
	// line geometry to change
	self->width = width;
	self->dirty = 1;
}

void a3d_line_brush(a3d_line_t* self,
                    float brush1, float brush2)
{
	assert(self);

	self->brush1 = brush1;
	self->brush2 = brush2;
}

void a3d_line_stripe(a3d_line_t* self, float stripe)
{
	assert(self);

	self->stripe = stripe;
}

void a3d_line_color(a3d_line_t* self,
                    a3d_vec4f_t* color1,
                    a3d_vec4f_t* color2)
{
	assert(self);
	assert(color1);
	assert(color2);

	a3d_vec4f_copy(color1, &self->color1);
	a3d_vec4f_copy(color2, &self->color2);
}

void a3d_line_draw(a3d_line_t* self,
                   a3d_lineShader_t* shader,
                   a3d_mat4f_t* mvp)
{
	assert(self);
	assert(shader);
	assert(mvp);

	// build lines
	if(a3d_line_build(self) == 0)
	{
		return;
	}

	// optionally enable blending
	if(self->blend &&
	   ((self->color1.a < 1.0f) || (self->color2.a < 1.0f)))
	{
		a3d_lineShader_blend(shader, 1);
	}
	else
	{
		a3d_lineShader_blend(shader, 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, self->id_vtx);
	glVertexAttribPointer(shader->attr_vtx, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_st);
	glVertexAttribPointer(shader->attr_st, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glUniformMatrix4fv(shader->unif_mvp, 1, GL_FALSE, (GLfloat*) mvp);
	glUniform1f(shader->unif_width, self->width);
	glUniform1f(shader->unif_length, self->length);
	glUniform1i(shader->unif_rounded, self->rounded);
	glUniform1f(shader->unif_brush1,  self->brush1);
	glUniform1f(shader->unif_brush2,  self->brush2);
	glUniform1f(shader->unif_stripe, self->stripe);
	glUniform4fv(shader->unif_color1, 1, (GLfloat*) &self->color1);
	glUniform4fv(shader->unif_color2, 1, (GLfloat*) &self->color2);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, self->vtx_count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
