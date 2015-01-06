/**
 * Matemat controller firmware
 * Copyright (C) 2014 Chaostreff Basel
 * 
 * dispatch.h
 * Dispatch queue interface
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DISPATCH_H
#define _DISPATCH_H

#include <stdint.h>
#include <stdbool.h>
#include <base/cirbuf/cirbuf.h>
#include "event.h"

/**
 * Storage size type for dispatch queues
 */
typedef uint16_t dispatch_size_t;

/**
 * Data structure holding dispatch queue metadata
 */
struct dispatch_t {
	dispatch_size_t length;
	dispatch_size_t used;
	struct cirbuf queue;
	// char queue[]
};
typedef struct dispatch_t dispatch_t;

/**
 * Calculates the minimum required memory pool size for a given number of queueable events
 */
#define DISPATCH_POOL_SIZE(events) (sizeof(dispatch_t) + sizeof(event_t) * events)

/**
 * Initialises a dispatch queue.
 * @param buffer a storage pool, will be used for both metadata and the data pool itself
 * @param size the size of the storage pool, in bytes
 * @return a pointer to a dispatch queue object, or NULL if the buffer was too small to hold metadata
 */
dispatch_t *dispatch_init(void *buffer, dispatch_size_t size);
/**
 * Destroys a dispatch queue.
 * External resources are not released by this function. For example, if you
 * used dynamic memory for the storage pool, you need to free it yourself.
 * @param dispatch the dispatch queue to destroy
 */
void dispatch_shutdown(dispatch_t *dispatch);

/**
 * Checks the number of queued events on a dispatch queue.
 * Note that concurrent modifications are possible after this method returns.
 * @param dispatch the dispatch queue to query
 * @return the number of events currently waiting
 */
dispatch_size_t dispatch_count(dispatch_t *dispatch);

/**
 * Atomically fetches an event from a dispatch queue and returns it.
 * Call dispatch_release after you are done processing the event. You may
 * enqueue additional events while the event object is still in use, but be
 * aware that it still occupies space in the data pool.
 * @param dispatch the dispatch queue to access
 * @param event an event buffer to copy the dequeued event into
 * @return true, if an event was dequeued, false if the queue was empty
 */
bool dispatch_dequeue(dispatch_t *dispatch, event_t *event);
/**
 * Atomically enqueues an event.
 * All data from event is copied into the pool.
 * @param dispatch the dispatch queue to access
 * @param event the event to put on the queue
 * @return true, if the event was queued successfully; false, if the queue is full
 */
bool dispatch_enqueue(dispatch_t *dispatch, const event_t *event);

#endif /*_DISPATCH_H*/
