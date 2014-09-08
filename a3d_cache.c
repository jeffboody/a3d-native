/*
 * Copyright (c) 2010 Jeff Boody
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
#include "a3d_cache.h"

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

typedef struct
{
	int          status;
	int          size;
	void*        data;
	a3d_cache_t* cache;
} a3d_cachenode_t;

static a3d_cachenode_t* a3d_cachenode_new(void* data, a3d_cache_t* cache)
{
	assert(data);
	assert(cache);
	LOGD("debug");

	a3d_cachenode_t* self = (a3d_cachenode_t*) malloc(sizeof(a3d_cachenode_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->status = A3D_CACHE_MISS;
	self->size   = 0;
	self->data   = data;
	self->cache  = cache;

	return self;
}

static void a3d_cachenode_delete(a3d_cachenode_t** _self)
{
	assert(_self);

	a3d_cachenode_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		// don't free cache and data references
		self->cache->size -= self->size;
		free(self);
		*_self = NULL;
	}
}

static int a3d_cache_runfn(void* task)
{
	assert(task);
	LOGD("debug");

	a3d_listitem_t*  key   = (a3d_listitem_t*)  task;
	a3d_cachenode_t* n     = (a3d_cachenode_t*) a3d_list_peekitem(key);
	a3d_cache_t*     cache = n->cache;

	return (*cache->load_fn)(n->data);
}

static void a3d_cache_purgefn(void* task, int status)
{
	assert(task);
	LOGD("debug");

	a3d_listitem_t*  key   = (a3d_listitem_t*)  task;
	a3d_cachenode_t* n     = (a3d_cachenode_t*) a3d_list_peekitem(key);
	a3d_cache_t*     cache = n->cache;

	// items which have been purged from the workq are not likely
	// to be needed again so move them to the beginning of the
	// cache so they are the first to be evicted
	a3d_list_move(cache->lru, key, a3d_list_head(cache->lru));
}

static void a3d_cache_trim(a3d_cache_t* self, a3d_listitem_t* key)
{
	// key may be NULL
	assert(self);
	LOGD("debug");

	a3d_listitem_t* iter = a3d_list_head(self->lru);
	while(iter && (self->size > self->max_size))
	{
		if(iter == key)
		{
			// don't evict the key we just added no matter how big
			// key is already at end of the list
			return;
		}

		a3d_cachenode_t* n;
		a3d_workq_cancel(self->loader, (void*) iter);
		n = (a3d_cachenode_t*) a3d_list_remove(self->lru, &iter);
		(*self->evict_fn)(n->data);
		a3d_cachenode_delete(&n);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_cache_t* a3d_cache_new(int max_size,
                           a3d_cacheload_fn  load_fn,
                           a3d_cachestore_fn store_fn,
                           a3d_cacheevict_fn evict_fn)
{
	assert(load_fn);
	assert(store_fn);
	assert(evict_fn);
	LOGD("debug max_size=%i", max_size);

	a3d_cache_t* self = (a3d_cache_t*) malloc(sizeof(a3d_cache_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	// order lru by last access from oldest (head) to newest (tail)
	self->lru = a3d_list_new();
	if(self->lru == NULL)
	{
		goto fail_lru;
	}

	self->loader = a3d_workq_new(a3d_cache_runfn, a3d_cache_purgefn);
	if(self->loader == NULL)
	{
		goto fail_loader;
	}

	self->size     = 0;
	self->max_size = max_size;
	self->load_fn  = load_fn;
	self->store_fn = store_fn;
	self->evict_fn = evict_fn;

	// success
	return self;

	// failure
	fail_loader:
		a3d_list_delete(&self->lru);
	fail_lru:
		free(self);
	return NULL;
}

void a3d_cache_delete(a3d_cache_t** _self)
{
	assert(_self);

	a3d_cache_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		// purge tasks and stop the loader
		a3d_workq_delete(&self->loader);

		// evict any items remaining in the cache
		a3d_listitem_t* item = a3d_list_head(self->lru);
		while(item)
		{
			a3d_cachenode_t* n;
			n = (a3d_cachenode_t*) a3d_list_remove(self->lru, &item);
			(*self->evict_fn)(n->data);
			a3d_cachenode_delete(&n);
		}

		a3d_list_delete(&self->lru);
		free(self);
		*_self = NULL;
	}
}

void a3d_cache_purge(a3d_cache_t* self)
{
	assert(self);
	LOGD("debug");

	a3d_workq_purge(self->loader);
}

void a3d_cache_resize(a3d_cache_t* self, int max_size)
{
	assert(self);
	LOGD("debug max_size=%i");

	self->max_size = max_size;
	a3d_cache_trim(self, NULL);
}

a3d_listitem_t* a3d_cache_register(a3d_cache_t* self, void* data)
{
	assert(self);
	assert(data);
	LOGD("debug");

	a3d_cachenode_t* node = a3d_cachenode_new(data, self);
	if(node == NULL)
	{
		return NULL;
	}

	a3d_listitem_t* key = a3d_list_append(self->lru, NULL, (const void*) node);
	if(key == NULL)
	{
		goto fail_key;
	}

	// success
	return key;

	// failure
	fail_key:
		a3d_cachenode_delete(&node);
	return NULL;
}

void a3d_cache_unregister(a3d_cache_t* self,
                          a3d_listitem_t* key)
{
	assert(self);
	assert(key);
	LOGD("debug");

	a3d_workq_cancel(self->loader, (void*) key);
	a3d_cachenode_t* n = (a3d_cachenode_t*) a3d_list_remove(self->lru, &key);
	(*self->evict_fn)(n->data);
	a3d_cachenode_delete(&n);
}

int a3d_cache_request(a3d_cache_t* self,
                      a3d_listitem_t* key)
{
	assert(self);
	assert(key);
	LOGD("debug");

	a3d_cachenode_t* n = (a3d_cachenode_t*) a3d_list_peekitem(key);
	if(n->status == A3D_CACHE_HIT)
	{
		a3d_list_moven(self->lru, key, a3d_list_tail(self->lru));
	}
	else if(n->status == A3D_CACHE_MISS)
	{
		a3d_list_moven(self->lru, key, a3d_list_tail(self->lru));

		int r = a3d_workq_run(self->loader, (void*) key);
		if(r == A3D_WORKQ_COMPLETE)
		{
			int s = (*self->store_fn)(n->data, &n->size);
			if(s)
			{
				n->status = A3D_CACHE_HIT;
				self->size += n->size;
				a3d_cache_trim(self, key);
			}
			else
			{
				n->size   = 0;
				n->status = A3D_CACHE_ERROR;
			}
		}
		else if(r == A3D_WORKQ_ERROR)
		{
			n->status = A3D_CACHE_ERROR;
		}
	}

	return n->status;
}
