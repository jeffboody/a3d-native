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
#include <unistd.h>
#include "a3d_workq.h"

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

// workq run state
const int A3D_WORKQ_RUNNING = 0;
const int A3D_WORKQ_STOP    = 1;

// force purge a task or workq
const int A3D_WORKQ_PURGE = -1;

static a3d_workqnode_t* a3d_workqnode_new(void* task, int purge_id)
{
	assert(task);
	assert((purge_id == 0) || (purge_id == 1));
	LOGD("debug task=%p, purge_id=%i", task, purge_id);

	a3d_workqnode_t* self = (a3d_workqnode_t*) malloc(sizeof(a3d_workqnode_t));
	if(!self)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->purge_id = purge_id;
	self->status   = A3D_WORKQ_PENDING;
	self->task     = task;

	return self;
}

static void a3d_workqnode_delete(a3d_workqnode_t** _self)
{
	assert(_self);

	a3d_workqnode_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		free(self);
		*_self = NULL;
	}
}

static int a3d_taskcmp_fn(const void *a, const void *b)
{
	assert(a);
	assert(b);
	LOGD("debug a=%p, b=%p", a, b);

	a3d_workqnode_t* node = (a3d_workqnode_t*) a;
	return (node->task == b) ? 0 : 1;
}

static void* a3d_workq_thread(void* arg)
{
	assert(arg);
	LOGD("debug");

	a3d_workq_t* self = (a3d_workq_t*) arg;
	while(1)
	{
		pthread_mutex_lock(&self->queue_mutex);

		// pending for an event
		while((a3d_list_size(self->queue_pending) == 0) &&
		   (self->state == A3D_WORKQ_RUNNING))
		{
			pthread_cond_wait(&self->queue_cond, &self->queue_mutex);
		}

		if(self->state == A3D_WORKQ_STOP)
		{
			// stop condition
			pthread_mutex_unlock(&self->queue_mutex);
			return NULL;
		}

		// get the task
		self->active_node = (a3d_workqnode_t*) a3d_list_dequeue(self->queue_pending);

		pthread_mutex_unlock(&self->queue_mutex);

		// run the task
		a3d_workqnode_t* node = self->active_node;
		int status = (*self->run_fn)(node->task);

		pthread_mutex_lock(&self->queue_mutex);

		// put the task on the complete queue
		node->status = status;
		a3d_list_enqueue(self->queue_complete, (const void*) node);
		self->active_node = NULL;

		// signal anybody pending for the workq to become idle
		pthread_cond_signal(&self->queue_cond);
		pthread_mutex_unlock(&self->queue_mutex);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_workq_t* a3d_workq_new(a3d_workqrun_fn   run_fn,
                           a3d_workqpurge_fn purge_fn)
{
	assert(run_fn);
	assert(purge_fn);
	LOGD("debug");

	a3d_workq_t* self = (a3d_workq_t*) malloc(sizeof(a3d_workq_t));
	if(!self)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->state       = A3D_WORKQ_RUNNING;
	self->purge_id    = 0;
	self->active_node = NULL;
	self->run_fn      = run_fn;
	self->purge_fn    = purge_fn;

	// PTHREAD_MUTEX_DEFAULT is not re-entrant
	if(pthread_mutex_init(&self->queue_mutex, NULL) != 0)
	{
		LOGE("pthread_mutex_init failed");
		goto fail_mutex_init;
	}

	if(pthread_cond_init(&self->queue_cond, NULL) != 0)
	{
		LOGE("pthread_cond_init failed");
		goto fail_cond_init;
	}

	self->queue_pending = a3d_list_new();
	if(self->queue_pending == NULL)
	{
		goto fail_queue_pending;
	}

	self->queue_complete = a3d_list_new();
	if(self->queue_complete == NULL)
	{
		goto fail_queue_complete;
	}

	if(pthread_create(&self->thread, NULL, a3d_workq_thread, (void*) self) != 0)
	{
		LOGE("pthread_create failed");
		goto fail_pthread_create;
	}

	// success
	return self;

	// fail
	fail_pthread_create:
		a3d_list_delete(&self->queue_complete);
	fail_queue_complete:
		a3d_list_delete(&self->queue_pending);
	fail_queue_pending:
		pthread_cond_destroy(&self->queue_cond);
	fail_cond_init:
		pthread_mutex_destroy(&self->queue_mutex);
	fail_mutex_init:
		free(self);
	return NULL;
}

void a3d_workq_delete(a3d_workq_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_workq_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		pthread_mutex_lock(&self->queue_mutex);

		// stop the workq thread
		self->state = A3D_WORKQ_STOP;
		if(self->active_node)
		{
			// pending for workq thread
			pthread_cond_wait(&self->queue_cond, &self->queue_mutex);
		}
		else
		{
			// wake up workq thread
			pthread_cond_signal(&self->queue_cond);
		}

		pthread_mutex_unlock(&self->queue_mutex);
		pthread_join(self->thread, NULL);

		// destroy the queues
		// active_node will be null since workq thread stopped
		self->purge_id = A3D_WORKQ_PURGE;
		a3d_workq_purge(self);
		a3d_list_delete(&self->queue_complete);
		a3d_list_delete(&self->queue_pending);

		// destroy the thread state
		pthread_cond_destroy(&self->queue_cond);
		pthread_mutex_destroy(&self->queue_mutex);

		free(self);
		*_self = NULL;
	}
}

void a3d_workq_purge(a3d_workq_t* self)
{
	assert(self);
	LOGD("debug");

	pthread_mutex_lock(&self->queue_mutex);

	// purge the pending queue
	a3d_listitem_t* iter = a3d_list_head(self->queue_pending);
	while(iter)
	{
		a3d_workqnode_t* node;
		node = (a3d_workqnode_t*) a3d_list_peekitem(iter);
		if(node->purge_id != self->purge_id)
		{
			a3d_list_remove(self->queue_pending, &iter);
			(*self->purge_fn)(node->task, node->status);
			a3d_workqnode_delete(&node);
		}
		else
		{
			iter = a3d_list_next(iter);
		}
	}

	// purge the active node (non-blocking)
	if(self->active_node &&
	   (self->active_node->purge_id != self->purge_id))
	{
		self->active_node->purge_id = A3D_WORKQ_PURGE;
	}

	// purge the complete queue
	iter = a3d_list_head(self->queue_complete);
	while(iter)
	{
		a3d_workqnode_t* node;
		node = (a3d_workqnode_t*) a3d_list_peekitem(iter);
		if((node->purge_id != self->purge_id) ||
		   (node->purge_id == A3D_WORKQ_PURGE))
		{
			a3d_list_remove(self->queue_complete, &iter);
			(*self->purge_fn)(node->task, node->status);
			a3d_workqnode_delete(&node);
		}
		else
		{
			iter = a3d_list_next(iter);
		}
	}

	// swap the purge id
	if(self->purge_id != A3D_WORKQ_PURGE)
	{
		self->purge_id = 1 - self->purge_id;
	}

	pthread_mutex_unlock(&self->queue_mutex);
}

int a3d_workq_run(a3d_workq_t* self, void* task)
{
	assert(self);
	assert(task);
	LOGD("debug task=%p", task);

	pthread_mutex_lock(&self->queue_mutex);

	// find the node containing the task or create a new one
	int status = A3D_WORKQ_ERROR;
	a3d_listitem_t* iter;
	if((iter = a3d_list_find(self->queue_complete, task,
	                         a3d_taskcmp_fn)) != NULL)
	{
		// task completed
		a3d_workqnode_t* node;
		node = (a3d_workqnode_t*) a3d_list_remove(self->queue_complete,
		                                          &iter);
		status = node->status;
		a3d_workqnode_delete(&node);
	}
	else if(self->active_node &&
	        (self->active_node->task == task))
	{
		a3d_workqnode_t* node = self->active_node;
		node->purge_id = self->purge_id;
		status = A3D_WORKQ_PENDING;
	}
	else if((iter = a3d_list_find(self->queue_pending, task,
	                              a3d_taskcmp_fn)) != NULL)
	{
		a3d_workqnode_t* node;
		node = (a3d_workqnode_t*) a3d_list_peekitem(iter);
		node->purge_id = self->purge_id;
		status = A3D_WORKQ_PENDING;
	}
	else
	{
		// create new node
		a3d_workqnode_t* node;
		node = a3d_workqnode_new(task, self->purge_id);
		if(node == NULL)
		{
			status = A3D_WORKQ_ERROR;
		}
		else
		{
			// put it on pending queue
			if(a3d_list_enqueue(self->queue_pending, (const void*) node) == 0)
			{
				goto fail_enqueue;
			}
			status = A3D_WORKQ_PENDING;

			// wake up workq thread
			pthread_cond_signal(&self->queue_cond);
		}
	}

	pthread_mutex_unlock(&self->queue_mutex);

	// success
	return status;

	// failure
	fail_enqueue:
		pthread_mutex_unlock(&self->queue_mutex);
	return A3D_WORKQ_ERROR;;
}

int a3d_workq_cancel(a3d_workq_t* self, void* task)
{
	assert(self);
	assert(task);
	LOGD("debug task=%p");

	int status = A3D_WORKQ_ERROR;
	pthread_mutex_lock(&self->queue_mutex);

	a3d_listitem_t* iter;
	if((iter = a3d_list_find(self->queue_pending, task,
	                         a3d_taskcmp_fn)) != NULL)
	{
		// cancel pending task
		a3d_workqnode_t* node;
		node = (a3d_workqnode_t*) a3d_list_remove(self->queue_pending,
		                                          &iter);
		status = node->status;
		a3d_workqnode_delete(&node);
	}
	else
	{
		if(self->active_node &&
		        (self->active_node->task == task))
		{
			// must wait for active task to finish
			pthread_cond_wait(&self->queue_cond, &self->queue_mutex);
		}

		if((iter = a3d_list_find(self->queue_complete, task,
		                         a3d_taskcmp_fn)) != NULL)
		{
			// cancel completed task
			a3d_workqnode_t* node;
			node = (a3d_workqnode_t*) a3d_list_remove(self->queue_complete,
			                                          &iter);
			status = node->status;
			a3d_workqnode_delete(&node);
		}
	}

	pthread_mutex_unlock(&self->queue_mutex);
	return status;
}

int a3d_workq_pending(a3d_workq_t* self)
{
	assert(self);
	LOGD("debug");

	int size;
	pthread_mutex_lock(&self->queue_mutex);
	size = a3d_list_size(self->queue_pending);
	if(self->active_node)
	{
		++size;
	}
	pthread_mutex_unlock(&self->queue_mutex);
	return size;
}
