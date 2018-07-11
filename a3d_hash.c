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

#include "a3d_hash.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private - hashiter                                       *
***********************************************************/

static const void* a3d_hashiter_val(const a3d_hashiter_t* self)
{
	assert(self);

	return a3d_list_peekitem(self->item);
}

static const char* a3d_hashiter_key(const a3d_hashiter_t* self)
{
	assert(self);

	return self->key;
}

static a3d_hashiter_t*
a3d_hashiter_nextDown(a3d_hashiter_t* self);

static a3d_hashiter_t*
a3d_hashiter_nextUp(a3d_hashiter_t* self)
{
	assert(self);

	// end recursion
	a3d_hashnode_t* parent = self->node->parent;
	if(parent == NULL)
	{
		return NULL;
	}

	// find node in parent
	int i;
	for(i = 0; i < A3D_HASH_KEY_COUNT; ++i)
	{
		a3d_hashnode_t* node = parent->nodes[i];
		if(node == self->node)
		{
			break;
		}
	}

	// try to traverse down to the next node
	int j;
	for(j = i + 1; j < A3D_HASH_KEY_COUNT; ++j)
	{
		a3d_hashnode_t* node = parent->nodes[j];
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

			return a3d_hashiter_nextDown(self);
		}
	}

	// try to traverse up again
	self->node = parent;
	self->item = NULL;
	self->key_len -= 1;
	self->key[self->key_len] = '\0';
	return a3d_hashiter_nextUp(self);
}

static a3d_hashiter_t*
a3d_hashiter_nextDown(a3d_hashiter_t* self)
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
	for(i = 0; i < A3D_HASH_KEY_COUNT; ++i)
	{
		a3d_hashnode_t* node = self->node->nodes[i];
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

			return a3d_hashiter_nextDown(self);
		}
	}

	// try to traverse up
	return a3d_hashiter_nextUp(self);
}

/***********************************************************
* private - hashnode                                       *
***********************************************************/

static a3d_hashnode_t* a3d_hashnode_new(a3d_hashnode_t* parent)
{
	// parent may be NULL for head

	a3d_hashnode_t* self = (a3d_hashnode_t*)
	                       calloc(1, sizeof(a3d_hashnode_t));
	if(self == NULL)
	{
		LOGE("calloc failed");
		return NULL;
	}

	self->parent = parent;

	return self;
}

static void a3d_hashnode_delete(a3d_hashnode_t** _self)
{
	assert(_self);

	a3d_hashnode_t* self = *_self;
	if(self)
	{
		a3d_list_delete(&self->list);

		int i;
		for(i = 0; i < A3D_HASH_KEY_COUNT; ++i)
		{
			a3d_hashnode_delete(&(self->nodes[i]));
		}

		free(self);
		*_self = NULL;
	}
}

static void a3d_hashnode_discard(a3d_hashnode_t** _self)
{
	assert(_self);

	a3d_hashnode_t* self = *_self;
	if(self)
	{
		if(self->list)
		{
			a3d_list_discard(self->list);
		}
		a3d_list_delete(&self->list);

		int i;
		for(i = 0; i < A3D_HASH_KEY_COUNT; ++i)
		{
			a3d_hashnode_discard(&(self->nodes[i]));
		}

		free(self);
		*_self = NULL;
	}
}

static const a3d_list_t*
a3d_hashnode_find(a3d_hashnode_t* self, int idx, const char* key)
{
	assert(self);
	assert(key);

	int len = strlen(key);
	if(len >= A3D_HASH_KEY_LEN)
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
			return self->list;
		}
		else if((c >= 'a') &&
		        (c <= 'z'))
		{
			c = c - 'a' + 10;
			break;
		}
		else if((c >= 'A') &&
		        (c <= 'Z'))
		{
			c = c - 'A' + 10;
			break;
		}
		else if((c >= '0') &&
		        (c <= '9'))
		{
			c = c - '0';
			break;
		}

		c = key[idx++];
	}

	// find next node
	a3d_hashnode_t* node = self->nodes[(int) c];
	if(node)
	{
		return a3d_hashnode_find(node, idx, key);
	}

	// not found
	return NULL;
}

static int a3d_hashnode_add(a3d_hashnode_t* self,
                            a3d_hashcmp_fn compare,
                            const void* val,
                            int idx, const char* key)
{
	// val may be NULL
	// compare may be NULL
	assert(self);
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
			if(compare)
			{
				a3d_listitem_t* iter = a3d_list_head(self->list);
				while(iter)
				{
					const void* v = a3d_list_peekitem(iter);
					if((*compare)(val, v) < 0)
					{
						if(a3d_list_insert(self->list, iter, val))
						{
							return 1;
						}
						else if(created_list)
						{
							a3d_list_delete(&self->list);
						}
						return 0;
					}

					iter = a3d_list_next(iter);
				}
			}

			// add to end
			if(a3d_list_enqueue(self->list, val))
			{
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
			c = c - 'a' + 10;
			break;
		}
		else if((c >= 'A') &&
		        (c <= 'Z'))
		{
			c = c - 'A' + 10;
			break;
		}
		else if((c >= '0') &&
		        (c <= '9'))
		{
			c = c - '0';
			break;
		}

		c = key[idx++];
	}

	int created = 0;
	a3d_hashnode_t* node = self->nodes[(int) c];
	if(node == NULL)
	{
		// create next node
		node = a3d_hashnode_new(self);
		if(node == NULL)
		{
			return 0;
		}

		created = 1;
	}

	if(a3d_hashnode_add(node, compare, val, idx, key) == 0)
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
			a3d_hashnode_delete(&node);
		}
	return 0;
}

static int a3d_hashnode_empty(a3d_hashnode_t* self)
{
	assert(self);

	// check if this hash node is now empty
	int empty = 1;
	if(self->list)
	{
		empty = 0;
	}
	else
	{
		int i;
		for(i = 0; i < A3D_HASH_KEY_COUNT; ++i)
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

static void a3d_hashnode_removeNode(a3d_hashnode_t* self,
                                    a3d_hashnode_t* node,
                                    a3d_hash_t* hash)
{
	// self may be null for the head node
	assert(node);
	assert(hash);

	if(self == NULL)
	{
		hash->head = NULL;
		return;
	}

	// remove the node
	int i;
	for(i = 0; i < A3D_HASH_KEY_COUNT; ++i)
	{
		if(self->nodes[i] == node)
		{
			a3d_hashnode_delete(&(self->nodes[i]));
			break;
		}
	}

	// recursively remove empty nodes
	if(a3d_hashnode_empty(self))
	{
		a3d_hashnode_removeNode(self->parent, self, hash);
	}
}

static const void* a3d_hashnode_removeItem(a3d_hashnode_t* self,
                                           a3d_listitem_t* item,
                                           a3d_hash_t* hash)
{
	assert(self);
	assert(item);
	assert(hash);

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
	if(a3d_hashnode_empty(self))
	{
		a3d_hashnode_removeNode(self->parent, self, hash);
	}

	return val;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_hash_t* a3d_hash_new(a3d_hashcmp_fn compare)
{
	// compare may be NULL

	a3d_hash_t* self = (a3d_hash_t*)
	                   malloc(sizeof(a3d_hash_t));
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

void a3d_hash_delete(a3d_hash_t** _self)
{
	assert(_self);

	a3d_hash_t* self = *_self;
	if(self)
	{
		if(self->size > 0)
		{
			LOGE("memory leak detected: size=%i", self->size);
		}

		a3d_hashnode_delete(&self->head);
		free(self);
		*_self = NULL;
	}
}

void a3d_hash_discard(a3d_hash_t* self)
{
	assert(self);

	self->size = 0;
	a3d_hashnode_discard(&self->head);
}

int a3d_hash_size(const a3d_hash_t* self)
{
	assert(self);

	return self->size;
}

int a3d_hash_empty(const a3d_hash_t* self)
{
	assert(self);

	return self->size ? 0 : 1;
}

a3d_hashiter_t* a3d_hash_head(const a3d_hash_t* self,
                              a3d_hashiter_t* iter)
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

	return a3d_hashiter_nextDown(iter);
}

a3d_hashiter_t* a3d_hash_next(a3d_hashiter_t* iter)
{
	assert(iter);

	return a3d_hashiter_nextDown(iter);
}

const void* a3d_hash_val(const a3d_hashiter_t* iter)
{
	assert(iter);

	return a3d_hashiter_val(iter);
}

const char* a3d_hash_key(const a3d_hashiter_t* iter)
{
	assert(iter);

	return a3d_hashiter_key(iter);
}

const a3d_list_t* a3d_hash_find(const a3d_hash_t* self,
                                const char* key)
{
	assert(self);
	assert(key);

	if(self->head)
	{
		return a3d_hashnode_find(self->head, 0, key);
	}

	return NULL;
}

const a3d_list_t* a3d_hash_findf(const a3d_hash_t* self,
                                 const char* fmt, ...)
{
	assert(self);
	assert(fmt);

	char key[A3D_HASH_KEY_LEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, A3D_HASH_KEY_LEN, fmt, argptr);
	va_end(argptr);

	return a3d_hash_find(self, key);
}

int a3d_hash_add(a3d_hash_t* self,
                 const void* val,
                 const char* key)
{
	// val may be NULL
	assert(self);
	assert(key);

	int len = strlen(key);
	if(len >= A3D_HASH_KEY_LEN)
	{
		LOGE("invalid len=%i", len);
		return 0;
	}

	int created = 0;
	a3d_hashnode_t* node = self->head;
	if(node == NULL)
	{
		node = a3d_hashnode_new(NULL);
		if(node == NULL)
		{
			return 0;
		}

		created = 1;
	}

	if(a3d_hashnode_add(node, self->compare,
	                    val, 0, key) == 0)
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
			a3d_hashnode_delete(&node);
		}
	return 0;
}

int a3d_hash_addf(a3d_hash_t* self,
                  const void* val,
                  const char* fmt, ...)
{
	// val may be NULL
	assert(self);
	assert(fmt);

	char key[A3D_HASH_KEY_LEN];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, A3D_HASH_KEY_LEN, fmt, argptr);
	va_end(argptr);

	return a3d_hash_add(self, val, key);
}

const void* a3d_hash_remove(a3d_hash_t* self,
                            a3d_hashiter_t** _iter)
{
	assert(self);
	assert(_iter);

	// ignore null iter
	a3d_hashiter_t* iter = *_iter;
	if(iter == NULL)
	{
		return NULL;
	}

	// update the iter
	a3d_hashnode_t* node = iter->node;
	a3d_listitem_t* item = iter->item;
	*_iter = a3d_hashiter_nextDown(iter);

	// update size
	--self->size;

	// remove the item from the list
	// and recursively remove nodes if needed
	return a3d_hashnode_removeItem(node, item, self);
}
