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

#include "a3d_multimap.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private - multimapIter                                   *
***********************************************************/

static const void*
a3d_multimapIter_val(const a3d_multimapIter_t* self)
{
	assert(self);

	return a3d_list_peekitem(self->item);
}

static const char*
a3d_multimapIter_key(const a3d_multimapIter_t* self)
{
	assert(self);

	return self->key;
}

static a3d_multimapIter_t*
a3d_multimapIter_nextDown(a3d_multimapIter_t* self);

static a3d_multimapIter_t*
a3d_multimapIter_nextUp(a3d_multimapIter_t* self)
{
	assert(self);

	// end recursion
	a3d_multimapNode_t* parent = self->node->parent;
	if(parent == NULL)
	{
		return NULL;
	}

	// find node in parent
	int i;
	for(i = 0; i < A3D_MULTIMAP_KEY_COUNT; ++i)
	{
		a3d_multimapNode_t* node = parent->nodes[i];
		if(node == self->node)
		{
			break;
		}
	}

	// try to traverse down to the next node
	int j;
	for(j = i + 1; j < A3D_MULTIMAP_KEY_COUNT; ++j)
	{
		a3d_multimapNode_t* node = parent->nodes[j];
		if(node)
		{
			self->node = node;
			self->item = NULL;

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

			return a3d_multimapIter_nextDown(self);
		}
	}

	// try to traverse up again
	self->node = parent;
	self->item = NULL;
	self->key_len -= 1;
	self->key[self->key_len] = '\0';
	return a3d_multimapIter_nextUp(self);
}

static a3d_multimapIter_t*
a3d_multimapIter_nextDown(a3d_multimapIter_t* self)
{
	assert(self);

	// check if there are more items in list
	// or check if list exists but has not been traversed yet
	a3d_list_t* list = self->node->list;
	if(self->item)
	{
		self->item = a3d_list_next(self->item);
		if(self->item)
		{
			return self;
		}
	}
	else if(list)
	{
		self->item = a3d_list_head(list);
		assert(self->item);
		return self;
	}

	// try to traverse down
	int i;
	for(i = 0; i < A3D_MULTIMAP_KEY_COUNT; ++i)
	{
		a3d_multimapNode_t* node = self->node->nodes[i];
		if(node)
		{
			self->node = node;
			self->item = NULL;

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

			return a3d_multimapIter_nextDown(self);
		}
	}

	// try to traverse up
	return a3d_multimapIter_nextUp(self);
}

static a3d_multimapIter_t*
a3d_multimapIter_nextItem(a3d_multimapIter_t* self)
{
	assert(self);

	// check if there are more items in node
	if(self->item)
	{
		self->item = a3d_list_next(self->item);
		if(self->item)
		{
			return self;
		}
	}

	return NULL;
}

/***********************************************************
* private - multimapNode                                   *
***********************************************************/

static a3d_multimapNode_t*
a3d_multimapNode_new(a3d_multimapNode_t* parent)
{
	// parent may be NULL for head

	a3d_multimapNode_t* self = (a3d_multimapNode_t*)
	                           calloc(1, sizeof(a3d_multimapNode_t));
	if(self == NULL)
	{
		LOGE("calloc failed");
		return NULL;
	}

	self->parent = parent;

	return self;
}

static void a3d_multimapNode_delete(a3d_multimapNode_t** _self)
{
	assert(_self);

	a3d_multimapNode_t* self = *_self;
	if(self)
	{
		a3d_list_delete(&self->list);

		int i;
		for(i = 0; i < A3D_MULTIMAP_KEY_COUNT; ++i)
		{
			a3d_multimapNode_delete(&(self->nodes[i]));
		}

		free(self);
		*_self = NULL;
	}
}

static void a3d_multimapNode_discard(a3d_multimapNode_t** _self)
{
	assert(_self);

	a3d_multimapNode_t* self = *_self;
	if(self)
	{
		if(self->list)
		{
			a3d_list_discard(self->list);
		}
		a3d_list_delete(&self->list);

		int i;
		for(i = 0; i < A3D_MULTIMAP_KEY_COUNT; ++i)
		{
			a3d_multimapNode_discard(&(self->nodes[i]));
		}

		free(self);
		*_self = NULL;
	}
}

static const a3d_list_t*
a3d_multimapNode_find(a3d_multimapNode_t* self,
                      a3d_multimapIter_t* iter,
                      int idx, const char* key)
{
	assert(self);
	assert(iter);
	assert(key);

	int len = strlen(key);
	if(len >= A3D_MULTIMAP_KEY_LEN)
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
			if(self->list)
			{
				iter->node = self;
				iter->item = a3d_list_head(self->list);
			}
			return self->list;
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
	a3d_multimapNode_t* node = self->nodes[(int) c];
	if(node)
	{
		return a3d_multimapNode_find(node, iter, idx, key);
	}

	// not found
	return NULL;
}

static int a3d_multimapNode_add(a3d_multimapNode_t* self,
                                a3d_multimapCmp_fn compare,
                                a3d_multimapIter_t* iter,
                                const void* val,
                                int idx, const char* key)
{
	// compare, val may be NULL
	assert(self);
	assert(iter);
	assert(key);

	// add at end or traverse to 0-9, A-Z
	char c = key[idx++];
	while(1)
	{
		if(c == '\0')
		{
			int created_list = 0;
			if(self->list == NULL)
			{
				self->list = a3d_list_new();
				if(self->list == NULL)
				{
					return 0;
				}

				created_list = 1;
			}

			// insert sorted
			a3d_listitem_t* item;
			if(compare)
			{
				a3d_listitem_t* li = a3d_list_head(self->list);
				while(li)
				{
					const void* v = a3d_list_peekitem(li);
					if((*compare)(val, v) < 0)
					{
						item = a3d_list_insert(self->list, li, val);
						if(item)
						{
							iter->node = self;
							iter->item = item;
							return 1;
						}
						else if(created_list)
						{
							a3d_list_delete(&self->list);
						}
						return 0;
					}

					li = a3d_list_next(li);
				}
			}

			// add to end
			item = a3d_list_append(self->list, NULL, val);
			if(item)
			{
				iter->node = self;
				iter->item = item;
				return 1;
			}
			else if(created_list)
			{
				a3d_list_delete(&self->list);
			}
			return 0;
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
	a3d_multimapNode_t* node = self->nodes[(int) c];
	if(node == NULL)
	{
		// create next node
		node = a3d_multimapNode_new(self);
		if(node == NULL)
		{
			return 0;
		}

		created = 1;
	}

	if(a3d_multimapNode_add(node, compare,
	                        iter, val, idx, key) == 0)
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
			a3d_multimapNode_delete(&node);
		}
	return 0;
}

static int a3d_multimapNode_empty(a3d_multimapNode_t* self)
{
	assert(self);

	// check if this multimap node is now empty
	int empty = 1;
	if(self->list)
	{
		empty = 0;
	}
	else
	{
		int i;
		for(i = 0; i < A3D_MULTIMAP_KEY_COUNT; ++i)
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
a3d_multimapNode_removeNode(a3d_multimapNode_t* self,
                            a3d_multimapNode_t* node,
                            a3d_multimap_t* multimap)
{
	// self may be null for the head node
	assert(node);
	assert(multimap);

	if(self == NULL)
	{
		multimap->head = NULL;
		return;
	}

	// remove the node
	int i;
	for(i = 0; i < A3D_MULTIMAP_KEY_COUNT; ++i)
	{
		if(self->nodes[i] == node)
		{
			a3d_multimapNode_delete(&(self->nodes[i]));
			break;
		}
	}

	// recursively remove empty nodes
	if(a3d_multimapNode_empty(self))
	{
		a3d_multimapNode_removeNode(self->parent,
		                            self, multimap);
	}
}

static const void*
a3d_multimapNode_removeItem(a3d_multimapNode_t* self,
                            a3d_listitem_t* item,
                            a3d_multimap_t* multimap)
{
	assert(self);
	assert(item);
	assert(multimap);

	const void* val = a3d_list_remove(self->list, &item);
	if(item)
	{
		return val;
	}

	// delete the list if empty
	if(a3d_list_empty(self->list))
	{
		a3d_list_delete(&self->list);
	}

	// recursively remove empty nodes
	if(a3d_multimapNode_empty(self))
	{
		a3d_multimapNode_removeNode(self->parent,
		                            self, multimap);
	}

	return val;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_multimap_t* a3d_multimap_new(a3d_multimapCmp_fn compare)
{
	// compare may be NULL

	a3d_multimap_t* self = (a3d_multimap_t*)
	                       malloc(sizeof(a3d_multimap_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->size    = 0;
	self->head    = NULL;
	self->compare = compare;

	return self;
}

void a3d_multimap_delete(a3d_multimap_t** _self)
{
	assert(_self);

	a3d_multimap_t* self = *_self;
	if(self)
	{
		if(self->size > 0)
		{
			LOGE("memory leak detected: size=%i", self->size);
		}

		a3d_multimapNode_delete(&self->head);
		free(self);
		*_self = NULL;
	}
}

void a3d_multimap_discard(a3d_multimap_t* self)
{
	assert(self);

	self->size = 0;
	a3d_multimapNode_discard(&self->head);
}

int a3d_multimap_size(const a3d_multimap_t* self)
{
	assert(self);

	return self->size;
}

int a3d_multimap_empty(const a3d_multimap_t* self)
{
	assert(self);

	return self->size ? 0 : 1;
}

a3d_multimapIter_t*
a3d_multimap_head(const a3d_multimap_t* self,
                  a3d_multimapIter_t* iter)
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
	iter->item    = NULL;

	return a3d_multimapIter_nextDown(iter);
}

a3d_multimapIter_t* a3d_multimap_next(a3d_multimapIter_t* iter)
{
	assert(iter);

	return a3d_multimapIter_nextDown(iter);
}

a3d_multimapIter_t* a3d_multimap_nextItem(a3d_multimapIter_t* iter)
{
	assert(iter);

	return a3d_multimapIter_nextItem(iter);
}

const void* a3d_multimap_val(const a3d_multimapIter_t* iter)
{
	assert(iter);

	return a3d_multimapIter_val(iter);
}

const char* a3d_multimap_key(const a3d_multimapIter_t* iter)
{
	assert(iter);

	return a3d_multimapIter_key(iter);
}

const a3d_list_t* a3d_multimap_find(const a3d_multimap_t* self,
                                    a3d_multimapIter_t* iter,
                                    const char* key)
{
	assert(self);
	assert(iter);
	assert(key);

	// initialize iter
	iter->key_len = 0;
	iter->key[0]  = '\0';
	iter->node    = NULL;
	iter->item    = NULL;

	if(self->head)
	{
		return a3d_multimapNode_find(self->head, iter, 0, key);
	}

	return NULL;
}

const a3d_list_t* a3d_multimap_findf(const a3d_multimap_t* self,
                                     a3d_multimapIter_t* iter,
                                     const char* fmt, ...)
{
	assert(self);
	assert(iter);
	assert(fmt);

	char key[A3D_MULTIMAP_KEY_LEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, A3D_MULTIMAP_KEY_LEN, fmt, argptr);
	va_end(argptr);

	return a3d_multimap_find(self, iter, key);
}

int a3d_multimap_add(a3d_multimap_t* self,
                     a3d_multimapIter_t* iter,
                     const void* val,
                     const char* key)
{
	// val may be NULL
	assert(self);
	assert(iter);
	assert(key);

	// initialize iter
	iter->key_len = 0;
	iter->key[0]  = '\0';
	iter->node    = NULL;
	iter->item    = NULL;

	int len = strlen(key);
	if(len >= A3D_MULTIMAP_KEY_LEN)
	{
		LOGE("invalid len=%i", len);
		return 0;
	}

	int created = 0;
	a3d_multimapNode_t* node = self->head;
	if(node == NULL)
	{
		node = a3d_multimapNode_new(NULL);
		if(node == NULL)
		{
			return 0;
		}

		created = 1;
	}

	if(a3d_multimapNode_add(node, self->compare,
	                        iter, val, 0, key) == 0)
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
			a3d_multimapNode_delete(&node);
		}
	return 0;
}

int a3d_multimap_addf(a3d_multimap_t* self,
                      a3d_multimapIter_t* iter,
                      const void* val,
                      const char* fmt, ...)
{
	// val may be NULL
	assert(self);
	assert(iter);
	assert(fmt);

	char key[A3D_MULTIMAP_KEY_LEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, A3D_MULTIMAP_KEY_LEN, fmt, argptr);
	va_end(argptr);

	return a3d_multimap_add(self, iter, val, key);
}

const void* a3d_multimap_remove(a3d_multimap_t* self,
                                a3d_multimapIter_t** _iter)
{
	assert(self);
	assert(_iter);

	// ignore null iter
	a3d_multimapIter_t* iter = *_iter;
	if(iter == NULL)
	{
		return NULL;
	}

	// update the iter
	a3d_multimapNode_t* node = iter->node;
	a3d_listitem_t*     item = iter->item;
	*_iter = a3d_multimapIter_nextDown(iter);

	// update size
	--self->size;

	// remove the item from the list
	// and recursively remove nodes if needed
	return a3d_multimapNode_removeItem(node, item, self);
}
