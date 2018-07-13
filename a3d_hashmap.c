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

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private - hashmapIter                                    *
***********************************************************/

static const void*
a3d_hashmapIter_val(const a3d_hashmapIter_t* self)
{
	assert(self);

	return self->node->val;
}

static const char*
a3d_hashmapIter_key(const a3d_hashmapIter_t* self)
{
	assert(self);

	return self->key;
}

static a3d_hashmapIter_t*
a3d_hashmapIter_nextDown(a3d_hashmapIter_t* self, int skip);

static a3d_hashmapIter_t*
a3d_hashmapIter_nextUp(a3d_hashmapIter_t* self)
{
	assert(self);

	// end recursion
	a3d_hashmapNode_t* parent = self->node->parent;
	if(parent == NULL)
	{
		return NULL;
	}

	// find node in parent
	int i;
	for(i = 0; i < A3D_HASHMAP_KEY_COUNT; ++i)
	{
		a3d_hashmapNode_t* node = parent->nodes[i];
		if(node == self->node)
		{
			break;
		}
	}

	// try to traverse down to the next node
	int j;
	for(j = i + 1; j < A3D_HASHMAP_KEY_COUNT; ++j)
	{
		a3d_hashmapNode_t* node = parent->nodes[j];
		if(node)
		{
			self->node = node;

			// replace iter key
			if(j < 10)
			{
				// number
				self->key[self->key_len - 1] = '0' + (char) j;
			}
			else
			{
				// letter
				self->key[self->key_len - 1] = 'A' + (char) (j - 10);
			}

			return a3d_hashmapIter_nextDown(self, 0);
		}
	}

	// try to traverse up again
	self->node = parent;
	self->key_len -= 1;
	self->key[self->key_len] = '\0';
	return a3d_hashmapIter_nextUp(self);
}

static a3d_hashmapIter_t*
a3d_hashmapIter_nextDown(a3d_hashmapIter_t* self, int skip)
{
	assert(self);

	// check if val exists but has not been traversed yet
	if((skip == 0) && self->node->val)
	{
		return self;
	}

	// try to traverse down
	int i;
	for(i = 0; i < A3D_HASHMAP_KEY_COUNT; ++i)
	{
		a3d_hashmapNode_t* node = self->node->nodes[i];
		if(node)
		{
			self->node = node;

			// update iter key
			if(i < 10)
			{
				// number
				self->key[self->key_len] = '0' + (char) i;
				self->key_len += 1;
				self->key[self->key_len] = '\0';
			}
			else
			{
				// letter
				self->key[self->key_len] = 'A' + (char) (i - 10);
				self->key_len += 1;
				self->key[self->key_len] = '\0';
			}

			return a3d_hashmapIter_nextDown(self, 0);
		}
	}

	// try to traverse up
	return a3d_hashmapIter_nextUp(self);
}

/***********************************************************
* private - hashmapNode                                    *
***********************************************************/

static a3d_hashmapNode_t*
a3d_hashmapNode_new(a3d_hashmapNode_t* parent)
{
	// parent may be NULL for head

	a3d_hashmapNode_t* self = (a3d_hashmapNode_t*)
	                          calloc(1, sizeof(a3d_hashmapNode_t));
	if(self == NULL)
	{
		LOGE("calloc failed");
		return NULL;
	}

	self->parent = parent;

	return self;
}

static void a3d_hashmapNode_delete(a3d_hashmapNode_t** _self)
{
	assert(_self);

	a3d_hashmapNode_t* self = *_self;
	if(self)
	{
		int i;
		for(i = 0; i < A3D_HASHMAP_KEY_COUNT; ++i)
		{
			a3d_hashmapNode_delete(&(self->nodes[i]));
		}

		free(self);
		*_self = NULL;
	}
}

static const void*
a3d_hashmapNode_find(a3d_hashmapNode_t* self,
                     a3d_hashmapIter_t* iter,
                     int idx, const char* key)
{
	assert(self);
	assert(iter);
	assert(key);

	int len = strlen(key);
	if(len >= A3D_HASHMAP_KEY_LEN)
	{
		LOGE("invalid len=%i", len);
		return NULL;
	}

	// find end or 0-9, A-Z
	char c = key[idx++];
	while(1)
	{
		if(c == '\0')
		{
			if(self->val)
			{
				iter->node = self;
			}
			return self->val;
		}
		else if((c >= 'a') &&
		        (c <= 'z'))
		{
			iter->key[iter->key_len++] = c - 'a' + 'A';
			iter->key[iter->key_len]   = '\0';
			c = c - 'a' + 10;
			break;
		}
		else if((c >= 'A') &&
		        (c <= 'Z'))
		{
			iter->key[iter->key_len++] = c;
			iter->key[iter->key_len]   = '\0';
			c = c - 'A' + 10;
			break;
		}
		else if((c >= '0') &&
		        (c <= '9'))
		{
			iter->key[iter->key_len++] = c;
			iter->key[iter->key_len]   = '\0';
			c = c - '0';
			break;
		}

		c = key[idx++];
	}

	// find next node
	a3d_hashmapNode_t* node = self->nodes[(int) c];
	if(node)
	{
		return a3d_hashmapNode_find(node, iter, idx, key);
	}

	// not found
	return NULL;
}

static int a3d_hashmapNode_add(a3d_hashmapNode_t* self,
                               a3d_hashmapIter_t* iter,
                               const void* val,
                               int idx, const char* key)
{
	assert(self);
	assert(iter);
	assert(val);
	assert(key);

	// add at end or traverse to 0-9, A-Z
	char c = key[idx++];
	while(1)
	{
		if(c == '\0')
		{
			if(self->val)
			{
				// hashmap already has val for this key
				// use replace instead
				return 0;
			}

			iter->node = self;
			self->val  = val;
			return 1;
		}
		else if((c >= 'a') &&
		        (c <= 'z'))
		{
			iter->key[iter->key_len++] = c - 'a' + 'A';
			iter->key[iter->key_len]   = '\0';
			c = c - 'a' + 10;
			break;
		}
		else if((c >= 'A') &&
		        (c <= 'Z'))
		{
			iter->key[iter->key_len++] = c;
			iter->key[iter->key_len]   = '\0';
			c = c - 'A' + 10;
			break;
		}
		else if((c >= '0') &&
		        (c <= '9'))
		{
			iter->key[iter->key_len++] = c;
			iter->key[iter->key_len]   = '\0';
			c = c - '0';
			break;
		}

		c = key[idx++];
	}

	int created = 0;
	a3d_hashmapNode_t* node = self->nodes[(int) c];
	if(node == NULL)
	{
		// create next node
		node = a3d_hashmapNode_new(self);
		if(node == NULL)
		{
			return 0;
		}

		created = 1;
	}

	if(a3d_hashmapNode_add(node, iter, val, idx, key) == 0)
	{
		goto fail_add;
	}
	else if(created)
	{
		self->nodes[(int) c] = node;
	}

	// success
	return 1;

	// failure
	fail_add:
		if(created)
		{
			a3d_hashmapNode_delete(&node);
		}
	return 0;
}

static int a3d_hashmapNode_empty(a3d_hashmapNode_t* self)
{
	assert(self);

	// check if this hashmap node is now empty
	int empty = 1;
	if(self->val)
	{
		empty = 0;
	}
	else
	{
		int i;
		for(i = 0; i < A3D_HASHMAP_KEY_COUNT; ++i)
		{
			if(self->nodes[i])
			{
				empty = 0;
				break;
			}
		}
	}

	return empty;
}

static void
a3d_hashmapNode_removeNode(a3d_hashmapNode_t* self,
                           a3d_hashmapNode_t* node,
                           a3d_hashmap_t* hashmap)
{
	// self may be null for the head node
	assert(node);
	assert(hashmap);

	if(self == NULL)
	{
		a3d_hashmapNode_delete(&hashmap->head);
		return;
	}

	// remove the node
	int i;
	for(i = 0; i < A3D_HASHMAP_KEY_COUNT; ++i)
	{
		if(self->nodes[i] == node)
		{
			a3d_hashmapNode_delete(&(self->nodes[i]));
			break;
		}
	}

	// recursively remove empty nodes
	if(a3d_hashmapNode_empty(self))
	{
		a3d_hashmapNode_removeNode(self->parent,
		                           self, hashmap);
	}
}

static const void*
a3d_hashmapNode_removeVal(a3d_hashmapNode_t* self,
                          a3d_hashmap_t* hashmap)
{
	assert(self);
	assert(hashmap);

	// remove the val
	const void* val = self->val;
	self->val = NULL;

	// recursively remove empty nodes
	if(a3d_hashmapNode_empty(self))
	{
		a3d_hashmapNode_removeNode(self->parent,
		                           self, hashmap);
	}

	return val;
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

	self->size = 0;
	self->head = NULL;

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

		a3d_hashmapNode_delete(&self->head);
		free(self);
		*_self = NULL;
	}
}

void a3d_hashmap_discard(a3d_hashmap_t* self)
{
	assert(self);

	self->size = 0;
	a3d_hashmapNode_delete(&self->head);
}

int a3d_hashmap_size(const a3d_hashmap_t* self)
{
	assert(self);

	return self->size;
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

	// at least one node must exist since head is not NULL
	iter->key_len = 0;
	iter->key[0]  = '\0';
	iter->node    = self->head;

	return a3d_hashmapIter_nextDown(iter, 0);
}

a3d_hashmapIter_t* a3d_hashmap_next(a3d_hashmapIter_t* iter)
{
	assert(iter);

	return a3d_hashmapIter_nextDown(iter, 1);
}

const void* a3d_hashmap_val(const a3d_hashmapIter_t* iter)
{
	assert(iter);

	return a3d_hashmapIter_val(iter);
}

const char* a3d_hashmap_key(const a3d_hashmapIter_t* iter)
{
	assert(iter);

	return a3d_hashmapIter_key(iter);
}

const void* a3d_hashmap_find(const a3d_hashmap_t* self,
                             a3d_hashmapIter_t* iter,
                             const char* key)
{
	assert(self);
	assert(iter);
	assert(key);

	// initialize iter
	iter->key_len = 0;
	iter->key[0]  = '\0';
	iter->node    = NULL;

	if(self->head)
	{
		return a3d_hashmapNode_find(self->head, iter, 0, key);
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
                    a3d_hashmapIter_t* iter,
                    const void* val,
                    const char* key)
{
	assert(self);
	assert(iter);
	assert(val);
	assert(key);

	// initialize iter
	iter->key_len = 0;
	iter->key[0]  = '\0';
	iter->node    = NULL;

	int len = strlen(key);
	if(len >= A3D_HASHMAP_KEY_LEN)
	{
		LOGE("invalid len=%i", len);
		return 0;
	}

	int created = 0;
	a3d_hashmapNode_t* node = self->head;
	if(node == NULL)
	{
		node = a3d_hashmapNode_new(NULL);
		if(node == NULL)
		{
			return 0;
		}

		created = 1;
	}

	if(a3d_hashmapNode_add(node, iter, val, 0, key) == 0)
	{
		goto fail_add;
	}
	else if(created)
	{
		self->head = node;
	}

	++self->size;

	// success
	return 1;

	// failure
	fail_add:
		if(created)
		{
			a3d_hashmapNode_delete(&node);
		}
	return 0;
}

int a3d_hashmap_addf(a3d_hashmap_t* self,
                     a3d_hashmapIter_t* iter,
                     const void* val,
                     const char* fmt, ...)
{
	assert(self);
	assert(iter);
	assert(val);
	assert(fmt);

	char key[A3D_HASHMAP_KEY_LEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, A3D_HASHMAP_KEY_LEN, fmt, argptr);
	va_end(argptr);

	return a3d_hashmap_add(self, iter, val, key);
}

const void* a3d_hashmap_replace(a3d_hashmapIter_t* iter,
                                const void*  val)
{
	assert(iter);
	assert(val);

	const void* old = iter->node->val;
	iter->node->val = val;
	return old;
}

const void* a3d_hashmap_remove(a3d_hashmap_t* self,
                               a3d_hashmapIter_t** _iter)
{
	assert(self);
	assert(_iter);
	assert(*_iter);

	// update the iter
	a3d_hashmapIter_t* iter = *_iter;
	a3d_hashmapNode_t* node = iter->node;
	*_iter = a3d_hashmapIter_nextDown(iter, 1);

	// update size
	--self->size;

	// remove the val from the node
	// and recursively remove nodes if needed
	return a3d_hashmapNode_removeVal(node, self);
}
