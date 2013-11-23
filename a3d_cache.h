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

#ifndef a3d_cache_H
#define a3d_cache_H

#include "a3d_workq.h"
#include "a3d_list.h"

#define A3D_CACHE_ERROR 0
#define A3D_CACHE_MISS  1
#define A3D_CACHE_HIT   2

// called by workq thread
typedef int (*a3d_cacheload_fn)(void* data);

// called by main thread for request
typedef int (*a3d_cachestore_fn)(void* data, int* size);

// called by main thread for delete, purge, unregister or request
// but never called by request for key
// automatically unregisters the item and invalidates the key
// may be called even if the item has not been loaded or
// stored in the cache
typedef void (*a3d_cacheevict_fn)(void* data);

typedef struct
{
	int          size;
	int          max_size;
	a3d_list_t*  lru;
	a3d_workq_t* loader;

	a3d_cacheload_fn  load_fn;
	a3d_cachestore_fn store_fn;
	a3d_cacheevict_fn evict_fn;
} a3d_cache_t;

a3d_cache_t*    a3d_cache_new(int max_size,
                              a3d_cacheload_fn  load_fn,
                              a3d_cachestore_fn store_fn,
                              a3d_cacheevict_fn evict_fn);
void            a3d_cache_delete(a3d_cache_t** _self);
void            a3d_cache_purge(a3d_cache_t* self);
a3d_listitem_t* a3d_cache_register(a3d_cache_t* self, void* data);
void            a3d_cache_unregister(a3d_cache_t* self,
                                     a3d_listitem_t* key);
int             a3d_cache_request(a3d_cache_t* self,
                                  a3d_listitem_t* key);

#endif
