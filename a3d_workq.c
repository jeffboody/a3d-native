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

static a3d_workqnode_t* a3d_workqnode_new(void* task, int purge_id,
                                          int priority)
{
	assert(task);
	assert((purge_id == 0) || (purge_id == 1));
	LOGD("debug task=%p, purge_id=%i, priority=%i",
	     task, purge_id, priority);

	a3d_workqnode_t* self = (a3d_workqnode_t*) malloc(sizeof(a3d_workqnode_t));
	if(!self)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->status   = A3D_WORKQ_PENDING;
	self->priority = priority;
	self->purge_id = purge_id;
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
	pthread_mutex_lock(&self->mutex);

	// checkout the next available thread id
	int tid  = self->next_tid++;
	while(1)
	{
		// pending for an event
		while((a3d_list_size(self->queue_pending) == 0) &&
		   (self->state == A3D_WORKQ_RUNNING))
		{
			pthread_cond_wait(&self->cond_pending, &self->mutex);
		}

		if(self->state == A3D_WORKQ_STOP)
		{
			// stop condition
			pthread_mutex_unlock(&self->mutex);
			return NULL;
		}

		// get the task
		a3d_listitem_t*  iter = a3d_list_head(self->queue_pending);
		a3d_workqnode_t* node = (a3d_workqnode_t*) a3d_list_peekitem(iter);
		a3d_list_swapn(self->queue_pending, self->queue_active, iter, NULL);

		// wake another thread
		// allows signal instead of broadcast for cond_pending
		if(a3d_list_size(self->queue_pending) > 0)
		{
			pthread_cond_signal(&self->cond_pending);
		}

		pthread_mutex_unlock(&self->mutex);

		// run the task
		int status = (*self->run_fn)(tid, self->owner, node->task);

		pthread_mutex_lock(&self->mutex);

		// put the task on the complete queue
		node->status = status;
		a3d_list_swapn(self->queue_active, self->queue_complete, iter, NULL);

		// signal anybody pending for the workq to become idle
		pthread_cond_signal(&self->cond_complete);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_workq_t* a3d_workq_new(void* owner, int thread_count,
                           a3d_workqrun_fn run_fn,
                           a3d_workqpurge_fn purge_fn)
{
	// owner may be NULL
	assert(run_fn);
	assert(purge_fn);
	LOGD("debug");

	a3d_workq_t* self = (a3d_workq_t*) malloc(sizeof(a3d_workq_t));
	if(!self)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->state        = A3D_WORKQ_RUNNING;
	self->owner        = owner;
	self->purge_id     = 0;
	self->thread_count = thread_count;
	self->next_tid     = 0;
	self->run_fn       = run_fn;
	self->purge_fn     = purge_fn;

	// PTHREAD_MUTEX_DEFAULT is not re-entrant
	if(pthread_mutex_init(&self->mutex, NULL) != 0)
	{
		LOGE("pthread_mutex_init failed");
		goto fail_mutex_init;
	}

	if(pthread_cond_init(&self->cond_pending, NULL) != 0)
	{
		LOGE("pthread_cond_init failed");
		goto fail_cond_pending;
	}

	if(pthread_cond_init(&self->cond_complete, NULL) != 0)
	{
		LOGE("pthread_cond_init failed");
		goto fail_cond_complete;
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

	self->queue_active = a3d_list_new();
	if(self->queue_active == NULL)
	{
		goto fail_queue_active;
	}

	// alloc threads
	int sz = thread_count*sizeof(pthread_t);
	self->threads = (pthread_t*) malloc(sz);
	if(self->threads == NULL)
	{
		LOGE("malloc failed");
		goto fail_threads;
	}

	// create threads
	pthread_mutex_lock(&self->mutex);
	int i;
	for(i = 0; i < thread_count; ++i)
	{
		if(pthread_create(&(self->threads[i]), NULL,
		                  a3d_workq_thread, (void*) self) != 0)
		{
			LOGE("pthread_create failed");
			goto fail_pthread_create;
		}
	}
	pthread_mutex_unlock(&self->mutex);

	// success
	return self;

	// fail
	fail_pthread_create:
		self->state = A3D_WORKQ_STOP;
		pthread_mutex_unlock(&self->mutex);

		int j;
		for(j = 0; j < i; ++j)
		{
			pthread_join(self->threads[j], NULL);
		}
		free(self->threads);
	fail_threads:
		a3d_list_delete(&self->queue_active);
	fail_queue_active:
		a3d_list_delete(&self->queue_complete);
	fail_queue_complete:
		a3d_list_delete(&self->queue_pending);
	fail_queue_pending:
		pthread_cond_destroy(&self->cond_complete);
	fail_cond_complete:
		pthread_cond_destroy(&self->cond_pending);
	fail_cond_pending:
		pthread_mutex_destroy(&self->mutex);
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

		pthread_mutex_lock(&self->mutex);

		// stop the workq thread
		self->state = A3D_WORKQ_STOP;
		pthread_cond_broadcast(&self->cond_pending);
		pthread_mutex_unlock(&self->mutex);
		int i;
		for(i = 0; i < self->thread_count; ++i)
		{
			pthread_join(self->threads[i], NULL);
		}
		free(self->threads);

		// destroy the queues
		// queue_active will be empty since the threads are stopped
		self->purge_id = A3D_WORKQ_PURGE;
		a3d_workq_purge(self);
		a3d_list_delete(&self->queue_active);
		a3d_list_delete(&self->queue_complete);
		a3d_list_delete(&self->queue_pending);

		// destroy the thread state
		pthread_cond_destroy(&self->cond_complete);
		pthread_cond_destroy(&self->cond_pending);
		pthread_mutex_destroy(&self->mutex);

		free(self);
		*_self = NULL;
	}
}

void a3d_workq_purge(a3d_workq_t* self)
{
	assert(self);
	LOGD("debug");

	pthread_mutex_lock(&self->mutex);

	// purge the pending queue
	a3d_listitem_t* iter = a3d_list_head(self->queue_pending);
	while(iter)
	{
		a3d_workqnode_t* node;
		node = (a3d_workqnode_t*) a3d_list_peekitem(iter);
		if(node->purge_id != self->purge_id)
		{
			a3d_list_remove(self->queue_pending, &iter);
			(*self->purge_fn)(self->owner, node->task, node->status);
			a3d_workqnode_delete(&node);
		}
		else
		{
			iter = a3d_list_next(iter);
		}
	}

	// purge the active queue (non-blocking)
	iter = a3d_list_head(self->queue_active);
	while(iter)
	{
		a3d_workqnode_t* node;
		node = (a3d_workqnode_t*) a3d_list_peekitem(iter);
		if(node->purge_id != self->purge_id)
		{
			node->purge_id = A3D_WORKQ_PURGE;
		}
		iter = a3d_list_next(iter);
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
			(*self->purge_fn)(self->owner, node->task, node->status);
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

	pthread_mutex_unlock(&self->mutex);
}

int a3d_workq_run(a3d_workq_t* self, void* task,
                  int priority)
{
	assert(self);
	assert(task);
	LOGD("debug task=%p, priority=%i", task, priority);

	pthread_mutex_lock(&self->mutex);

	// find the node containing the task or create a new one
	int status = A3D_WORKQ_ERROR;
	a3d_listitem_t*  iter = NULL;
	a3d_listitem_t*  pos  = NULL;
	a3d_workqnode_t* tmp  = NULL;
	a3d_workqnode_t* node = NULL;
	if((iter = a3d_list_find(self->queue_complete, task,
	                         a3d_taskcmp_fn)) != NULL)
	{
		// task completed
		node = (a3d_workqnode_t*) a3d_list_remove(self->queue_complete,
		                                          &iter);
		status = node->status;
		a3d_workqnode_delete(&node);
	}
	else if((iter = a3d_list_find(self->queue_active, task,
	                              a3d_taskcmp_fn)) != NULL)
	{
		node = (a3d_workqnode_t*) a3d_list_peekitem(iter);
		node->purge_id = self->purge_id;
		status = A3D_WORKQ_PENDING;
	}
	else if((iter = a3d_list_find(self->queue_pending, task,
	                              a3d_taskcmp_fn)) != NULL)
	{
		node = (a3d_workqnode_t*) a3d_list_peekitem(iter);
		node->purge_id = self->purge_id;
		if(priority > node->priority)
		{
			// move up
			pos = a3d_list_prev(iter);
			while(pos)
			{
				tmp = (a3d_workqnode_t*) a3d_list_peekitem(pos);
				if(tmp->priority >= node->priority)
				{
					break;
				}
				pos = a3d_list_prev(pos);
			}

			if(pos)
			{
				// move after pos
				a3d_list_moven(self->queue_pending, iter, pos);
			}
			else
			{
				// move to head of list
				a3d_list_move(self->queue_pending, iter, NULL);
			}
		}
		else if(priority < node->priority)
		{
			// move down
			pos = a3d_list_next(iter);
			while(pos)
			{
				tmp = (a3d_workqnode_t*) a3d_list_peekitem(pos);
				if(tmp->priority < node->priority)
				{
					break;
				}
				pos = a3d_list_next(pos);
			}

			if(pos)
			{
				// move before pos
				a3d_list_move(self->queue_pending, iter, pos);
			}
			else
			{
				// move to tail of list
				a3d_list_moven(self->queue_pending, iter, NULL);
			}
		}
		node->priority = priority;
		status = A3D_WORKQ_PENDING;
	}
	else
	{
		// create new node
		node = a3d_workqnode_new(task, self->purge_id, priority);
		if(node == NULL)
		{
			goto fail_node;
		}
		else
		{
			// find the insert position
			pos = a3d_list_tail(self->queue_pending);
			while(pos)
			{
				tmp = (a3d_workqnode_t*) a3d_list_peekitem(pos);
				if(tmp->priority >= node->priority)
				{
					break;
				}
				pos = a3d_list_prev(pos);
			}

			if(pos)
			{
				// append after pos
				if(a3d_list_append(self->queue_pending, pos,
				                   (const void*) node) == NULL)
				{
					goto fail_queue;
				}
			}
			else
			{
				// insert at head of queue
				// first item or highest priority
				if(a3d_list_insert(self->queue_pending, NULL,
				                   (const void*) node) == NULL)
				{
					goto fail_queue;
				}
			}

			status = A3D_WORKQ_PENDING;

			// wake up workq thread
			pthread_cond_signal(&self->cond_pending);
		}
	}

	pthread_mutex_unlock(&self->mutex);

	// success
	return status;

	// failure
	fail_queue:
		a3d_workqnode_delete(&node);
	fail_node:
		pthread_mutex_unlock(&self->mutex);
	return A3D_WORKQ_ERROR;
}

int a3d_workq_cancel(a3d_workq_t* self, void* task)
{
	assert(self);
	assert(task);
	LOGD("debug task=%p");

	int status = A3D_WORKQ_ERROR;
	pthread_mutex_lock(&self->mutex);

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
		while((iter = a3d_list_find(self->queue_active, task,
		                            a3d_taskcmp_fn)) != NULL)
		{
			// must wait for active task to complete
			pthread_cond_wait(&self->cond_complete, &self->mutex);
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

	pthread_mutex_unlock(&self->mutex);
	return status;
}

int a3d_workq_pending(a3d_workq_t* self)
{
	assert(self);
	LOGD("debug");

	int size;
	pthread_mutex_lock(&self->mutex);
	size = a3d_list_size(self->queue_pending);
	size += a3d_list_size(self->queue_active);
	pthread_mutex_unlock(&self->mutex);
	return size;
}
