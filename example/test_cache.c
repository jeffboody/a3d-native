/*
 * Copyright (c) 2013 Jeff Boody
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
#include <unistd.h>
#include "test_cache.h"
#include "a3d/a3d_cache.h"

#define LOG_TAG "test_cache"
#include "a3d/a3d_log.h"

static void testeq(int a, int b)
{
	if(a == b)
	{
		LOGI("[pass] %i %i", a, b);
	}
	else
	{
		LOGI("[fail] %i %i", a, b);
	}
}

#define TEST_REGISTERED        1   // status
#define TEST_LOADED            2   // status
#define TEST_STORED            3   // status
#define TEST_EVICTED           4   // status
#define TEST_LOAD_ERROR        5   // status/test
#define TEST_STORE_ERROR       6   // status/test
#define TEST_EVICT_BEFORE_LOAD 7   // test
#define TEST_EVICT_AFTER_STORE 8   // test

typedef struct
{
	a3d_listitem_t* key;
	char            name;
	int             status;
	int             test;
} test_item_t;

static test_item_t* test_item_new(char name, int test)
{
	LOGD("debug name=%c", name);

	test_item_t* self = (test_item_t*) malloc(sizeof(test_item_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->key    = NULL;
	self->name   = name;
	self->status = TEST_REGISTERED;
	self->test   = test;
	return self;
}

static void test_item_delete(test_item_t** _self)
{
	assert(_self);

	test_item_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		testeq(self->status, TEST_EVICTED);
		free(self);
		*_self = NULL;
	}
}

static int test_load_fn(void* _item)
{
	test_item_t* item = (test_item_t*) _item;
	LOGI("START: item=%p, name=%c", item, item->name);
	usleep(100000);
	LOGI("STOP:  item=%p, name=%c", item, item->name);

	if(item->test == TEST_LOAD_ERROR)
	{
		item->status = TEST_LOAD_ERROR;
		return 0;
	}
	item->status = TEST_LOADED;
	return 1;
}

static int test_store_fn(void* _item, int* size)
{
	test_item_t* item = (test_item_t*) _item;
	if(item->test == TEST_STORE_ERROR)
	{
		item->status = TEST_STORE_ERROR;
		return 0;
	}
	item->status = TEST_STORED;
	*size = 1;
	return 1;
}

static void test_evict_fn(void* _item)
{
	test_item_t* item = (test_item_t*) _item;
	if(item->test == TEST_LOAD_ERROR)
	{
		testeq(item->status, TEST_LOAD_ERROR);
	}
	else if(item->test == TEST_STORE_ERROR)
	{
		testeq(item->status, TEST_STORE_ERROR);
	}
	else if(item->test == TEST_EVICT_BEFORE_LOAD)
	{
		testeq(item->status, TEST_REGISTERED);
	}
	else if(item->test == TEST_EVICT_AFTER_STORE)
	{
		testeq(item->status, TEST_STORED);
	}
	else
	{
		LOGI("[fail] unknown test=%i", item->test);
	}

	LOGI("EVICT:  item=%p, name=%c", item, item->name);
	item->key    = NULL;
	item->status = TEST_EVICTED;
}

void test_cache(void)
{
	// test abcdefg
	{
		LOGI("abcdefg");

		a3d_cache_t* cache = a3d_cache_new(3,
		                                   test_load_fn,
		                                   test_store_fn,
		                                   test_evict_fn);
		if(cache == NULL)
		{
			return;
		}

		test_item_t* a = test_item_new('a', TEST_EVICT_AFTER_STORE);
		test_item_t* b = test_item_new('b', TEST_LOAD_ERROR);
		test_item_t* c = test_item_new('c', TEST_STORE_ERROR);
		test_item_t* d = test_item_new('d', TEST_EVICT_AFTER_STORE);
		test_item_t* e = test_item_new('e', TEST_EVICT_AFTER_STORE);
		test_item_t* f = test_item_new('f', TEST_EVICT_AFTER_STORE);
		test_item_t* g = test_item_new('g', TEST_EVICT_BEFORE_LOAD);

		a->key = a3d_cache_register(cache, a);
		b->key = a3d_cache_register(cache, b);
		c->key = a3d_cache_register(cache, c);

		testeq(a3d_cache_request(cache, a->key), A3D_CACHE_MISS);
		testeq(a3d_cache_request(cache, b->key), A3D_CACHE_MISS);
		testeq(a3d_cache_request(cache, c->key), A3D_CACHE_MISS);

		// wait for loads to complete
		usleep(400000);
		a3d_cache_purge(cache);

		testeq(a3d_cache_request(cache, a->key), A3D_CACHE_HIT);
		testeq(a3d_cache_request(cache, b->key), A3D_CACHE_ERROR);
		testeq(a3d_cache_request(cache, c->key), A3D_CACHE_ERROR);

		// evict b, c
		a3d_cache_unregister(cache, &b->key);
		a3d_cache_unregister(cache, &c->key);

		// load d and e
		d->key = a3d_cache_register(cache, d);
		e->key = a3d_cache_register(cache, e);
		testeq(a3d_cache_request(cache, a->key), A3D_CACHE_HIT);
		testeq(a3d_cache_request(cache, d->key), A3D_CACHE_MISS);
		testeq(a3d_cache_request(cache, e->key), A3D_CACHE_MISS);

		// wait for loads to complete
		usleep(300000);
		a3d_cache_purge(cache);

		testeq(a3d_cache_request(cache, a->key), A3D_CACHE_HIT);
		testeq(a3d_cache_request(cache, d->key), A3D_CACHE_HIT);
		testeq(a3d_cache_request(cache, e->key), A3D_CACHE_HIT);

		// the cache is full
		// try to load a, f and g but purge g

		f->key = a3d_cache_register(cache, f);
		g->key = a3d_cache_register(cache, g);
		testeq(a3d_cache_request(cache, a->key), A3D_CACHE_HIT);
		testeq(a3d_cache_request(cache, f->key), A3D_CACHE_MISS);
		testeq(a3d_cache_request(cache, g->key), A3D_CACHE_MISS);

		usleep(50000);
		a3d_cache_purge(cache);

		testeq(a3d_cache_request(cache, a->key), A3D_CACHE_HIT);
		testeq(a3d_cache_request(cache, f->key), A3D_CACHE_MISS);

		// purge g
		a3d_cache_purge(cache);
		usleep(200000);

		// evict g, d
		testeq(a3d_cache_request(cache, a->key), A3D_CACHE_HIT);
		testeq(a3d_cache_request(cache, f->key), A3D_CACHE_HIT);

		// evict e, a, f
		a3d_cache_delete(&cache);

		test_item_delete(&a);
		test_item_delete(&b);
		test_item_delete(&c);
		test_item_delete(&d);
		test_item_delete(&e);
		test_item_delete(&f);
		test_item_delete(&g);
	}
}
