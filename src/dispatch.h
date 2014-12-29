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
#include "event.h"

typedef uint8_t dispatch_size_t;

/**
 * Initialises the global dispatch queue.
 * The size parameter is ignored if dynamic memory is not in use.
 * Set the queue size with the DISPATCH_SIZE instead.
 * @param size The queue length in events.
 */
void dispatch_init(dispatch_size_t size);
/**
 * Shuts the global dispatch queue down.
 */
void dispatch_shutdown(void);

/**
 * Checks the number of queued events on the global dispatch queue.
 * @return The number of events currently waiting.
 */
dispatch_size_t dispatch_events_count(void);

/**
 * Atomically fetches an event from the global dispatch queue and returns it.
 * The event must be free'd by the caller if dynamic allocation for events is in use.
 * @return The next event from the queue, or NULL if no event is waiting.
 */
event_t *dispatch_dequeue(void);
/**
 * Atomically enqueues an event.
 * @parm event The event to put on the queue.
 * @return true, if the event was queued successfully. false, if the queue is full.
 */
bool dispatch_enqueue(event_t *event);

#endif /*_DISPATCH_H*/
