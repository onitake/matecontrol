/**
 * Matemat controller firmware
 * Copyright (C) 2014 Chaostreff Basel
 * 
 * dispatch.c
 * Dispatch queue implementation
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

#include <stdlib.h>
#include <string.h>
#include "dispatch.h"

typedef struct {
	bool initialized;
	dispatch_size_t size;
	dispatch_size_t in;
	dispatch_size_t out;
#if DYNMEM
	event_t **events;
#else
	event_t events[DISPATCH_SIZE];
#endif
} dispatch_queue_t;

static dispatch_queue_t global_dispatch;

void dispatch_init(dispatch_size_t size) {
	if (!global_dispatch.initialized) {
#if DYNMEM
		global_dispatch.size = size;
		global_dispatch.events = (event_t **) calloc(size, sizeof(event_t *));
#else
		global_dispatch.size = DISPATCH_SIZE;
		memset(global_dispatch.events, 0, global_dispatch.size);
#endif
		global_dispatch.in = 0;
		global_dispatch.out = 0;
		global_dispatch.initialized = true;
	}
}

void dispatch_shutdown() {
	if (global_dispatch.initialized) {
		// TODO Synchronise here
#if DYNMEM
		event_t *event;
		do {
			event = dispatch_dequeue();
			if (event) {
				// TODO use a proper event_delete here
				free(event);
			}
		} while (event);
		free(global_dispatch.events);
#endif
		global_dispatch.initialized = false;
	}
}

dispatch_size_t dispatch_events_count() {
	if (global_dispatch.in >= global_dispatch.out) {
		return global_dispatch.in - global_dispatch.out;
	} else {
		return global_dispatch.size - global_dispatch.out + global_dispatch.in;
	}
}

event_t *dispatch_dequeue() {
	if (global_dispatch.in == global_dispatch.out) {
		return NULL;
	} else {
#if DYNMEM
		event_t *event = global_dispatch.events[global_dispatch.out];
#else
		event_t *event = &global_dispatch.events[global_dispatch.out];
#endif
		global_dispatch.out = (global_dispatch.out == global_dispatch.size - 1) ? 0 : global_dispatch.out + 1;
		return event;
	}
}

bool dispatch_enqueue(event_t *event) {
	dispatch_size_t nextin = (global_dispatch.in == global_dispatch.size - 1) ? 0 : global_dispatch.in + 1;
	if (global_dispatch.out == nextin) {
		return false;
	} else {
#if DYNMEM
		global_dispatch.events[global_dispatch.in] = event;
#else
		memcpy(&global_dispatch.events[global_dispatch.in], &event, sizeof(event));
#endif
		global_dispatch.in = nextin;
		return true;
	}
}
