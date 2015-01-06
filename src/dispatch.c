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

#include <string.h>
#include <aversive/irq_lock.h>
#include "dispatch.h"

dispatch_t *dispatch_init(void *buffer, dispatch_size_t size) {
	if (size >= sizeof(dispatch_t)) {
		dispatch_t *dispatch = (dispatch_t *) buffer;
		char *queue = ((char *) buffer) + sizeof(dispatch_t);
		dispatch->length = (size - sizeof(dispatch_t)) / sizeof(event_t);
		dispatch->used = 0;
		cirbuf_init(&(dispatch->queue), queue, 0, dispatch->length * sizeof(event_t));
	}
	return NULL;
}

void dispatch_shutdown(dispatch_t *dispatch) {
	// Do nothing
}

dispatch_size_t dispatch_count(dispatch_t *dispatch) {
	return dispatch->used;
}

bool dispatch_dequeue(dispatch_t *dispatch, event_t *event) {
	uint8_t flags;
	IRQ_LOCK(flags);
	bool ok = dispatch->used > 0;
	if (ok) {
		ok = cirbuf_get_buf_head(&(dispatch->queue), (char *) event, sizeof(event_t)) == sizeof(event_t);
		if (ok) {
			ok = cirbuf_del_buf_head(&(dispatch->queue), sizeof(event_t)) == 0;
			if (ok) {
				dispatch->used--;
			}
		}
	}
	IRQ_UNLOCK(flags);
	return ok;
}

bool dispatch_enqueue(dispatch_t *dispatch, const event_t *event) {
	uint8_t flags;
	IRQ_LOCK(flags);
	bool ok = dispatch->used < dispatch->length;
	if (ok) {
		ok = cirbuf_add_buf_tail(&(dispatch->queue), (const char *) event, sizeof(event_t)) == sizeof(event_t);
		if (ok) {
			dispatch->used++;
		}
	}
	IRQ_UNLOCK(flags);
	return ok;
}
