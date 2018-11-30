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

#include "a3d_hashmap.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private - hashmapIter                                    *
***********************************************************/

static void
a3d_hashmapIter_init(a3d_hashmapIter_t* self,
                     a3d_hashmapNode_t* node)
{
	assert(self);
	assert(node);

	self->depth   = 0;
	self->key[0]  = node->k;
	self->key[1]  = '\0';
	self->node[0] = node;
}

static void
a3d_hashmapIter_update(a3d_hashmapIter_t* self,
                       int d, a3d_hashmapNode_t* node)
{
	assert(self);
	assert(d >= 0);
	assert(d < (A3D_HASHMAP_KEY_LEN - 1));
	assert(node);

	self->depth      = d;
	self->key[d]     = node->k;
	self->key[d + 1] = '\0';
	self->node[d]    = node;
}

/***********************************************************
* private - hashmapNode                                    *
***********************************************************/

static a3d_hashmapNode_t*
a3d_hashmapNode_new(a3d_hashmapNode_t* prev,
                    a3d_hashmap_t* hash, char k)
{
	assert(hash);

	// prev may be NULL for head

	a3d_hashmapNode_t* self = (a3d_hashmapNode_t*)
	                          malloc(sizeof(a3d_hashmapNode_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->prev = prev;
	self->next = NULL;
	self->down = NULL;
	self->val  = NULL;
	self->k    = k;

	++hash->nodes;

	return self;
}

static void
a3d_hashmapNode_delete(a3d_hashmapNode_t** _self,
                       a3d_hashmap_t* hash)
{
	assert(_self);
	assert(hash);

	a3d_hashmapNode_t* self = *_self;
	if(self)
	{
		// prev is a reference
		--hash->nodes;
		a3d_hashmapNode_delete(&self->next, hash);
		a3d_hashmapNode_delete(&self->down, hash);
		free(self);
		*_self = NULL;
	}
}

/***********************************************************
* private hashmap                                          *
***********************************************************/

static void
a3d_hashmap_clean(a3d_hashmap_t* self,
                  a3d_hashmapNode_t* node)
{
	assert(self);
	assert(node);

	// check if the node is an endpoint or traversal node
	if(node->val || node->down)
	{
		return;
	}

	// detach empty nodes
	a3d_hashmapNode_t* prev = node->prev;
	a3d_hashmapNode_t* next = node->next;
	if(prev == NULL)
	{
		self->head = next;
	}
	else if(prev->down == node)
	{
		prev->down = next;
		a3d_hashmap_clean(self, prev);
	}
	else
	{
		prev->next = next;
	}

	if(next)
	{
		next->prev = prev;
	}

	node->prev = NULL;
	node->next = NULL;
	a3d_hashmapNode_delete(&node, self);
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_hashmap_t* a3d_hashmap_new(void)
{
	a3d_hashmap_t* self = (a3d_hashmap_t*)
	                      malloc(sizeof(a3d_hashmap_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->size  = 0;
	self->nodes = 0;
	self->head  = NULL;

	return self;
}

void a3d_hashmap_delete(a3d_hashmap_t** _self)
{
	assert(_self);

	a3d_hashmap_t* self = *_self;
	if(self)
	{
		if(self->size > 0)
		{
			LOGE("memory leak detected: size=%i", self->size);
		}

		a3d_hashmapNode_delete(&self->head, self);
		free(self);
		*_self = NULL;
	}
}

void a3d_hashmap_discard(a3d_hashmap_t* self)
{
	assert(self);

	self->size = 0;
	a3d_hashmapNode_delete(&self->head, self);
}

int a3d_hashmap_size(const a3d_hashmap_t* self)
{
	assert(self);

	return self->size;
}

int a3d_hashmap_hashmapNodes(const a3d_hashmap_t* self)
{
	assert(self);

	return self->nodes;
}

int a3d_hashmap_hashmapSize(const a3d_hashmap_t* self)
{
	assert(self);

	return (int) self->nodes*sizeof(a3d_hashmapNode_t);
}

int a3d_hashmap_empty(const a3d_hashmap_t* self)
{
	assert(self);

	return self->size ? 0 : 1;
}

a3d_hashmapIter_t*
a3d_hashmap_head(const a3d_hashmap_t* self,
                 a3d_hashmapIter_t* iter)
{
	assert(self);
	assert(iter);

	if(self->head == NULL)
	{
		return NULL;
	}

	a3d_hashmapIter_init(iter, self->head);

	// find an endpoint
	if(self->head->val)
	{
		return iter;
	}
	return a3d_hashmap_next(iter);
}

a3d_hashmapIter_t* a3d_hashmap_next(a3d_hashmapIter_t* iter)
{
	assert(iter);

	int d = iter->depth;
	a3d_hashmapNode_t* node = iter->node[d];
	if(node->down)
	{
		// down
		++d;
		node = node->down;
		a3d_hashmapIter_update(iter, d, node);
	}
	else if(node->next)
	{
		// sideways
		node = node->next;
		a3d_hashmapIter_update(iter, d, node);
	}
	else
	{
		// up
		while(1)
		{
			--d;
			if(d < 0)
			{
				return NULL;
			}

			node = iter->node[d];
			if(node->next)
			{
				node = node->next;
				a3d_hashmapIter_update(iter, d, node);
				break;
			}
		}
	}

	// find an endpoint
	if(node->val)
	{
		return iter;
	}
	return a3d_hashmap_next(iter);
}

const void* a3d_hashmap_val(const a3d_hashmapIter_t* iter)
{
	assert(iter);

	int d = iter->depth;
	a3d_hashmapNode_t* node = iter->node[d];
	return node->val;
}

const char* a3d_hashmap_key(const a3d_hashmapIter_t* iter)
{
	assert(iter);

	return iter->key;
}

const void* a3d_hashmap_find(const a3d_hashmap_t* self,
                             a3d_hashmapIter_t* iter,
                             const char* key)
{
	assert(self);
	assert(iter);
	assert(key);

	int len = strlen(key);
	if((len >= A3D_HASHMAP_KEY_LEN) || (len == 0))
	{
		LOGE("invalid len=%i", len);
		return NULL;
	}

	// check for empty hashmap
	if(self->head == NULL)
	{
		return NULL;
	}

	a3d_hashmapIter_init(iter, self->head);

	// traverse the hashmap
	int d = 0;
	a3d_hashmapNode_t* node = self->head;
	while(d < len)
	{
		if(key[d] == iter->key[d])
		{
			if(d == (len - 1))
			{
				// success
				return node->val;
			}
			else if(node->down)
			{
				// down
				++d;
				node = node->down;
				a3d_hashmapIter_update(iter, d, node);
				continue;
			}

			return NULL;
		}
		else if((key[d] < iter->key[d]) ||
		        (node->next == NULL))
		{
			// not found
			return NULL;
		}

		// traverse to the next decision point
		while(node->next && (key[d] > iter->key[d]))
		{
			// sideways
			node = node->next;
			a3d_hashmapIter_update(iter, d, node);
		}
	}

	return NULL;
}

const void* a3d_hashmap_findf(const a3d_hashmap_t* self,
                              a3d_hashmapIter_t* iter,
                              const char* fmt, ...)
{
	assert(self);
	assert(iter);
	assert(fmt);

	char key[A3D_HASHMAP_KEY_LEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, A3D_HASHMAP_KEY_LEN, fmt, argptr);
	va_end(argptr);

	return a3d_hashmap_find(self, iter, key);
}

int a3d_hashmap_add(a3d_hashmap_t* self,
                    const void* val,
                    const char* key)
{
	assert(self);
	assert(val);
	assert(key);

	int len = strlen(key);
	if((len >= A3D_HASHMAP_KEY_LEN) || (len == 0))
	{
		LOGE("invalid key=%s, len=%i", key, len);
		return 0;
	}

	int created = 0;
	if(self->head == NULL)
	{
		self->head = a3d_hashmapNode_new(NULL, self, key[0]);
		if(self->head == NULL)
		{
			return 0;
		}

		created = 1;
	}

	a3d_hashmapIter_t  iterator;
	a3d_hashmapIter_t* iter = &iterator;
	a3d_hashmapIter_init(&iterator, self->head);

	// traverse the hashmap
	int d = 0;
	a3d_hashmapNode_t* node = self->head;
	while(d < len)
	{
		if(key[d] == iter->key[d])
		{
			if(d == (len - 1))
			{
				if(node->val)
				{
					// hash already contains key
					return 0;
				}

				// success
				++self->size;
				node->val = val;
				return 1;
			}
			else if(node->down == NULL)
			{
				// insert a new down node
				a3d_hashmapNode_t* down;
				down = a3d_hashmapNode_new(node, self, key[d + 1]);
				if(down == NULL)
				{
					a3d_hashmap_clean(self, node);
					goto fail_add;
				}
				node->down = down;
			}

			// down
			++d;
			node = node->down;
			a3d_hashmapIter_update(iter, d, node);
			continue;
		}
		else if(key[d] < iter->key[d])
		{
			// insert a new prev node
			a3d_hashmapNode_t* prev;
			prev = a3d_hashmapNode_new(node->prev, self, key[d]);
			if(prev == NULL)
			{
				a3d_hashmap_clean(self, node);
				goto fail_add;
			}
			prev->next = node;

			if(node->prev)
			{
				if(node->prev->next == node)
				{
					node->prev->next = prev;
				}
				else
				{
					node->prev->down = prev;
				}
			}
			else
			{
				self->head = prev;
			}
			node->prev = prev;

			// sideways
			node = prev;
			a3d_hashmapIter_update(iter, d, node);
			continue;
		}
		else if(node->next == NULL)
		{
			// append a new next node
			a3d_hashmapNode_t* next;
			next = a3d_hashmapNode_new(node, self, key[d]);
			if(next == NULL)
			{
				a3d_hashmap_clean(self, node);
				goto fail_add;
			}
			node->next = next;

			node = next;
			a3d_hashmapIter_update(iter, d, node);
			continue;
		}

		// traverse to the next decision point
		while(node->next && (key[d] > iter->key[d]))
		{
			// sideways
			node = node->next;
			a3d_hashmapIter_update(iter, d, node);
		}
	}

	// the key/val can always be added before d equals len
	assert(0);

	// failure
	fail_add:
		if(created)
		{
			a3d_hashmapNode_delete(&self->head, self);
		}
	return 0;
}

int a3d_hashmap_addf(a3d_hashmap_t* self,
                     const void* val,
                     const char* fmt, ...)
{
	assert(self);
	assert(val);
	assert(fmt);

	char key[A3D_HASHMAP_KEY_LEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, A3D_HASHMAP_KEY_LEN, fmt, argptr);
	va_end(argptr);

	return a3d_hashmap_add(self, val, key);
}

const void* a3d_hashmap_replace(a3d_hashmapIter_t* iter,
                                const void*  val)
{
	assert(iter);
	assert(val);

	int d = iter->depth;
	a3d_hashmapNode_t* node = iter->node[d];

	const void* old = node->val;
	node->val = val;
	return old;
}

const void* a3d_hashmap_remove(a3d_hashmap_t* self,
                               a3d_hashmapIter_t** _iter)
{
	assert(self);
	assert(_iter);
	assert(*_iter);

	a3d_hashmapIter_t* iter = *_iter;

	// save node and update iter
	int d = iter->depth;
	a3d_hashmapNode_t* node = iter->node[d];
	*_iter = a3d_hashmap_next(iter);

	// clear value and clean traversal nodes
	const void* val = node->val;
	node->val = NULL;
	a3d_hashmap_clean(self, node);

	--self->size;

	return val;
}
