/**
 * Matemat controller firmware
 * Copyright (C) 2014 Chaostreff Basel
 * 
 * event.h
 * Event type definitions
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

#warning Don't include this file
#if 0

#ifndef _EVENT_H
#define _EVENT_H

#include <stdint.h>
#include <stdbool.h>
#include "util.h"

/**
 * Maximum size of a private event data structure.
 * You should verify that all your private structures do not exceed this size.
 * If your compiler supports C11 static assertions, the following code will
 * do this for you:
 * struct handler_t {
 *     // members
 * };
 * EVENT_SIZE_CHECK(struct handler_t);
 */
#define EVENT_SIZE_MAX 16

/**
 * Loads and casts the private member from an event structure
 */
#define EVENT_PRIVATE(event, type) ((type *) &((event)->priv))

/**
 * Compile-time type size check for private event data structures.
 * On compilers without compile time assertions, does nothing.
 */
#define EVENT_SIZE_CHECK(type) static_assert(sizeof(type) <= EVENT_SIZE_MAX, #type " is too large to fit into the dispatch queue")

/* Forward declarations */
struct dispatch_s;
typedef struct dispatch_s dispatch_t;
struct event_s;
typedef struct event_s event_t;

/**
 * Priority value
 */
typedef uint8_t event_priority_t;

/**
 * Wall clock time type
 */
typedef uint16_t event_time_t;

/**
 * Event handler callback type.
 * Example callback function declaration:
 * bool handler(event_t *event) {
 *     struct handler_t *priv = EVENT_PRIVATE(event, struct handler_t);
 *     if (priv) {
 *         // handle event
 *     }
 * }
 */
typedef void (event_handler_t)(void *event);

/**
 * Event object
 */
struct event_s {
	dispatch_t *dispatch;
	event_t *next;
	event_handler_t *handler;
	event_priority_t priority;
	event_time_t deadline;
	uint8_t priv[EVENT_SIZE_MAX];
};

/**
 * Schedules an event for execution.
 * The handler will be executed as soon as all events with a higher priority
 * have been processed and no previous events of the same priority are waiting.
 * @param dispatch the dispatch queue to schedule on
 * @param handler the handler callback for the event
 * @param priority the priority level of this thread
 * @return true, if the event was scheduled successfully
 */
bool event_schedule(dispatch_t *dispatch, event_handler_t *handler, event_priority_t priority);
/**
 * Schedules an event for execution, with a delay.
 * The handler will be executed as soon as all events with a higher priority
 * have been processed, no previous events of the same priority are waiting and
 * the deadline has elapsed.
 * @param dispatch the dispatch queue to schedule on
 * @param event the event to schedule for execution (needs to be initialised)
 * @param deadline the minimum number of timer ticks to wait before executing the event
 * @return true, if the event was scheduled successfully
 */
bool event_schedule_in(dispatch_t *dispatch, event_handler_t *handler, event_priority_t priority, event_time_t deadline);
/**
 * Schedules an event for execution, with a delay.
 * The handler will be executed as soon as all events with a higher priority
 * have been processed, no previous events of the same priority are waiting and
 * the deadline has elapsed.
 * @param dispatch the dispatch queue to schedule on
 * @param event the event to schedule for execution (needs to be initialised)
 * @param time the exact wall clock time when to execute the event (may be delayed due to prioritised events)
 * @return true, if the event was scheduled successfully
 */
bool event_schedule_at(dispatch_t *dispatch, event_handler_t *handler, event_priority_t priority, event_time_t wallclock);

#endif /*_EVENT_H*/

#endif