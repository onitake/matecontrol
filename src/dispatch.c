/**
 * @file dispatch.c
 * @brief Multi-priority dispatch queue
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

#include <string.h>
#include <avr/interrupt.h>
#include <aversive/irq_lock.h>
#include "dispatch.h"

dispatch_t *dispatch_init(void *buffer, size_t size, dispatch_priority_t queues, uint8_t *lengths) {
	if (buffer) {
		dispatch_t *dispatch = (dispatch_t *) buffer;
		buffer = (uint8_t *) buffer + sizeof(dispatch_t);
		size -= sizeof(dispatch_t);
		dispatch->queues = queues;
		dispatch->priority = 0;
		dispatch->time = 0;
		if (size >= sizeof(dispatch_head_t) * queues) {
			dispatch->heads = (dispatch_head_t *) buffer;
			buffer = (uint8_t *) buffer + sizeof(dispatch_head_t) * queues;
			size -= sizeof(dispatch_head_t) * queues;
			dispatch_priority_t priority;
			for (priority = 0; priority < queues; priority++) {
				dispatch_head_t *queue = &dispatch->heads[priority];
				queue->length = lengths[priority];
				queue->queued = 0;
				queue->priority = priority;
				dispatch_event_t *first = (dispatch_event_t *) buffer;
				queue->in = first;
				queue->out = first;
				dispatch_event_t *prev = first;
				uint8_t length;
				for (length = 0; length < queue->length; length++) {
					if (size < sizeof(dispatch_event_t)) {
						return NULL;
					}
					buffer = (uint8_t *) buffer + sizeof(dispatch_event_t);
					size -= sizeof(dispatch_event_t);
					prev->priority = priority;
					if (length == queue->length - 1) {
						prev->next = first;
					} else {
						dispatch_event_t *next = (dispatch_event_t *) buffer;
						prev->next = next;
						prev = next;
					}
				}
			}
			return dispatch;
		}
	}
	return NULL;
}

void dispatch_shutdown(dispatch_t *dispatch) {
	if (dispatch) {
	}
}

size_t dispatch_count(dispatch_t *dispatch, dispatch_priority_t priority) {
	if (dispatch && priority < dispatch->queues) {
		uint8_t flags;
		IRQ_LOCK(flags);
		size_t count = dispatch->heads[priority].queued;
		IRQ_UNLOCK(flags);
		return count;
	}
	return 0;
}

void dispatch_tick(dispatch_t *dispatch) {
	if (dispatch) {
		dispatch->time++;
		bool handled = false;
		dispatch_priority_t priority;
		for (priority = dispatch->queues - 1; !handled && priority > dispatch->priority; priority--) {
			if (dispatch->heads[priority].queued > 0) {
				dispatch_priority_t old = dispatch->priority;
				dispatch->priority = priority;
				
				dispatch->priority = old;
				handled = true;
			}
		}
		if (!handled) {
		}
		// TODO check if any queue of higher priority than dispatch->priority has events waiting
		// set dispatch->priority to the priority of that queue
		// unqueue the first event
		// reenable interrupts
		// call the event handler
		// release the event object memory
	}
}

bool dispatch_schedule(dispatch_t *dispatch, dispatch_handler_t *handler, void *argument, dispatch_priority_t priority) {
	return dispatch_schedule_in(dispatch, handler, argument, priority, 0);
}

bool dispatch_schedule_in(dispatch_t *dispatch, dispatch_handler_t *handler, void *argument, dispatch_priority_t priority, dispatch_time_t deadline) {
	return dispatch_schedule_at(dispatch, handler, argument, priority, dispatch->time + deadline);
}

bool dispatch_schedule_at(dispatch_t *dispatch, dispatch_handler_t *handler, void *argument, dispatch_priority_t priority, dispatch_time_t wallclock) {
	if (dispatch && priority < dispatch->queues) {
		uint8_t flags;
		IRQ_LOCK(flags);
		dispatch_head_t *queue = &dispatch->heads[priority];
		if (queue->queued < queue->length) {
			dispatch_event_t *event = queue->in;
			queue->in = queue->in->next;
			event->handler = handler;
			event->argument = argument;
			event->deadline = wallclock;
		}
		IRQ_UNLOCK(flags);
		return true;
	}
	return false;
}
