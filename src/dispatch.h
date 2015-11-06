/**
 * @file dispatch.h
 * @brief Multi-priority dispatch queue
 * 
 * Each priority level has its own queue for a fixed number of events;
 * the queue length is configurable.
 * 
 * The queue relies on a storage pool that needs to be specified on creation.
 * 
 * Memory pool layout:
 * | offset | usage           |
 * |--------|-----------------|
 * | 0      | dispatch        |
 * | 6      | queue head 0    |
 * | 15     | queue head 1    |
 * | 24     | ...             |
 * | N      | queue 0 event 0 |
 * | N+9    | queue 0 event 1 |
 * | N+18   | queue 0 event 2 |
 * | N+27   | ...             |
 * | M      | queue 1 event 0 |
 * | M+9    | queue 1 event 1 |
 * | M+18   | ...             |
 * 
 * @copyright Matemat controller firmware
 * Copyright © 2014 Chaostreff Basel
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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"

/**
 * Priority level
 */
typedef uint8_t dispatch_priority_t;

/**
 * Wall clock time type
 */
typedef uint16_t dispatch_time_t;

/**
 * Event handler callback type.
 * 
 * Example callback function declaration:
 * ~~~{.c}
 * void handler(void *event) {
 *     if (event) {
 *         type_t *priv = (type_t *) event;
 *         ...
 *     }
 * }
 * ~~~
 */
typedef void (dispatch_handler_t)(void *event);

/**
 * Single event
 * 
 * All events are chained in a circular fashion. The last event points to
 * the first.
 */
typedef struct dispatch_event_t {
	/** Pointer to next event */
	struct dispatch_event_t *next;
	/** Event handler */
	dispatch_handler_t *handler;
	/** Event handler argument */
	void *argument;
	/** Priority level to be set when the handler is called */
	dispatch_priority_t priority;
	/** When this event must be scheduled */
	dispatch_time_t deadline;
} dispatch_event_t;

/**
 * Queue header
 */
typedef struct {
	/** Length of this queue */
	size_t length;
	/** Number of queued events */
	size_t queued;
	/** Priority level of this queue */
	dispatch_priority_t priority;
	/** Pointer to first free event */
	dispatch_event_t *in;
	/** Pointer to first allocated event */
	dispatch_event_t *out;
} dispatch_head_t;

/**
 * Multi-priority dispatch queue/scheduler.
 */
typedef struct {
	/** Number of queues/priority levels */
	dispatch_priority_t queues;
	/** Current wallclock time */
	dispatch_time_t time;
	/** Current priority level */
	dispatch_priority_t priority;
	/** Pointer to array of queue headers */
	dispatch_head_t *heads;
} dispatch_t;

/**
 * Calculates the required memory pool size for a maximum priority level
 * and a given number of queueable events, distributed over all priorities.
 * @param priorities the number of priority levels
 * @param events the total number of queueable events
 * @return the number of bytes required for the dispatch queue storage pool
 */
#define DISPATCH_POOL_SIZE(priorities, events) (sizeof(dispatch_t) + sizeof(dispatch_head_t) * priorities + sizeof(dispatch_event_t) * events)

/**
 * Creates and initialises a dispatch queue.
 * All storage will be drawn from the pool, even the dispatch object itself.
 * Make sure the buffer is large enough. You can use
 * DISPATCH_POOL_SIZE to allocate a suitable buffer.
 * After creating the dispatch queue, call dispatch_schedule() in regular
 * intervals (preferably from a timer interrupt).
 * @param buffer a storage pool, used for the event queues and private event data
 * @param size the size of the storage pool, in bytes
 * @param queues the number of priority levels
 * @param lengths an array containing the queue depth for each priority level
 * @return a pointer to a newly initialized dispatch queue
 */
dispatch_t *dispatch_init(void *buffer, size_t size, dispatch_priority_t queues, uint8_t *lengths);
/**
 * Destroys a dispatch queue.
 * External resources are not released by this function. For example, if you
 * used dynamic memory for the storage pool, you need to free it yourself.
 * The system timer will be stopped if it is running.
 * The interrupt flag is not modified.
 * @param dispatch the dispatch queue to destroy
 */
void dispatch_shutdown(dispatch_t *dispatch);

/**
 * Periodic scheduler callback.
 * Typically, call this from a system timer interrupt.
 * When the function is entered, interrupts should be disabled
 * (which is the case if called from an ISR).
 * Events are scheduled strictly by priority, there is no preemption if no
 * events of higher priority are waiting. Interrupts are reenabled before the
 * event handler is called.
 * After the handler returns, the event object is released automatically.
 * @param dispatch the dispatch queue being updated
 */
void dispatch_tick(dispatch_t *dispatch);

/**
 * Checks the number of queued events on a dispatch queue.
 * 
 * Please take concurrent modifications of the queue into account when
 * calling this method.
 * 
 * @param dispatch the dispatch queue to query
 * @param priority the priority level to count
 * @return the number of events currently waiting
 */
size_t dispatch_count(dispatch_t *dispatch, dispatch_priority_t priority);

/**
 * Gets the current wall clock time of the timer used for scheduling events.
 * @return the current time in ticks
 */
dispatch_time_t dispatch_time(dispatch_t *dispatch);

/**
 * Schedules an event for execution.
 * 
 * The handler will be executed as soon as all events with a higher priority
 * have been processed and no previous events of the same priority are waiting.
 * 
 * @param dispatch the dispatch queue to schedule on
 * @param handler the handler callback for the event
 * @param event a private event argument, or NULL
 * @param priority the priority level of this thread
 * @return true, if the event was scheduled successfully
 */
bool dispatch_schedule(dispatch_t *dispatch, dispatch_handler_t *handler, void *event, dispatch_priority_t priority);
/**
 * Schedules an event for execution, with a delay.
 * The handler will be executed as soon as all events with a higher priority
 * have been processed, no previous events of the same priority are waiting and
 * the deadline has elapsed.
 * @param dispatch the dispatch queue to schedule on
 * @param handler the handler callback for the event
 * @param event a private event argument, or NULL
 * @param priority the priority level of this thread
 * @param deadline the minimum number of timer ticks to wait before executing the event
 * @return true, if the event was scheduled successfully
 */
bool dispatch_schedule_in(dispatch_t *dispatch, dispatch_handler_t *handler, void *event, dispatch_priority_t priority, dispatch_time_t deadline);
/**
 * Schedules an event for execution, with a delay.
 * The handler will be executed as soon as all events with a higher priority
 * have been processed, no previous events of the same priority are waiting and
 * the deadline has elapsed.
 * @param dispatch the dispatch queue to schedule on
 * @param handler the handler callback for the event
 * @param event a private event argument, or NULL
 * @param priority the priority level of this thread
 * @param wallclock the exact wall clock time when to execute the event (may be delayed due to prioritised events)
 * @return true, if the event was scheduled successfully
 */
bool dispatch_schedule_at(dispatch_t *dispatch, dispatch_handler_t *handler, void *event, dispatch_priority_t priority, dispatch_time_t wallclock);

#endif /*_DISPATCH_H*/
