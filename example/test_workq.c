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
#include "test_workq.h"
#include "a3d/a3d_workq.h"

#define LOG_TAG "test_workq"
#include "a3d/a3d_log.h"

typedef struct
{
	int  cancel;
	char name;
} test_task_t;

static test_task_t* test_task_new(char name, int cancel)
{
	LOGD("debug name=%c", name);

	test_task_t* self = (test_task_t*) malloc(sizeof(test_task_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->name   = name;
	self->cancel = cancel;
	return self;
}

static void test_task_delete(test_task_t** _self)
{
	assert(_self);

	test_task_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		free(self);
		*_self = NULL;
	}
}

static int test_run_fn(int tid, void* owner, void* _task)
{
	// ignore owner
	test_task_t* task = (test_task_t*) _task;
	LOGI("[%i] START: task=%p, name=%c", tid, task, task->name);
	if(task->cancel)
	{
		LOGE("failed to cancel task=%p, name=%c", task, task->name);
	}
	usleep(100000);
	LOGI("[%i] STOP:  task=%p, name=%c", tid, task, task->name);
	return 1;
}

static void test_purge_fn(void* owner, void* _task, int status)
{
	// ignore owner
	test_task_t* task = (test_task_t*) _task;
	LOGI("PURGE: task=%p, name=%c, status=%i", task, task->name, status);
}

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

void test_workq(void)
{
	// test abx
	{
		LOGI("ABX");

		// note: tests were designed for a single thread
		a3d_workq_t* workq = a3d_workq_new(NULL, 1,
		                                   test_run_fn,
		                                   test_purge_fn);
		if(workq == NULL)
		{
			return;
		}

		test_task_t* a = test_task_new('a', 0);
		test_task_t* b = test_task_new('b', 0);
		test_task_t* c = test_task_new('c', 1);
		test_task_t* x = test_task_new('x', 0);
		test_task_t* y = test_task_new('y', 1);

		testeq(a3d_workq_run(workq, (void*) a), A3D_WORKQ_PENDING);
		testeq(a3d_workq_run(workq, (void*) b), A3D_WORKQ_PENDING);
		testeq(a3d_workq_run(workq, (void*) c), A3D_WORKQ_PENDING);
		testeq(a3d_workq_run(workq, (void*) x), A3D_WORKQ_PENDING);
		testeq(a3d_workq_run(workq, (void*) y), A3D_WORKQ_PENDING);
		testeq(a3d_workq_pending(workq), 5);

		// cancel c
		a3d_workq_cancel(workq, (void*) c);

		// wait for a
		usleep(150000);
		a3d_workq_purge(workq);

		testeq(a3d_workq_run(workq, (void*) a), A3D_WORKQ_COMPLETE);
		testeq(a3d_workq_run(workq, (void*) b), A3D_WORKQ_PENDING);
		testeq(a3d_workq_run(workq, (void*) x), A3D_WORKQ_PENDING);
		testeq(a3d_workq_pending(workq), 3);

		// purge y
		a3d_workq_purge(workq);
		testeq(a3d_workq_pending(workq), 2);

		// wait for b, x
		usleep(200000);

		testeq(a3d_workq_run(workq, (void*) b), A3D_WORKQ_COMPLETE);
		testeq(a3d_workq_run(workq, (void*) x), A3D_WORKQ_COMPLETE);
		testeq(a3d_workq_pending(workq), 0);

		a3d_workq_delete(&workq);

		test_task_delete(&a);
		test_task_delete(&b);
		test_task_delete(&c);
		test_task_delete(&x);
		test_task_delete(&y);
	}
}
