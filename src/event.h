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

#ifndef _EVENT_H
#define _EVENT_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Maximum size of a private event data structure.
 * You should verify that all your private structures do not exceed this size.
 * If your compiler supports C11 static assertions, you may use:
 * struct private_struct {
 *     // members
 * };
 * EVENT_SIZE_CHECK(struct private_struct);
 */
#define EVENT_SIZE_MAX 16

/**
 * Loads and casts the private member from an event structure
 */
#define EVENT_PRIVATE(event, type) ((type *) &((event)->argument))

#if (__STDC_VERSION__ >= 201112L || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#define EVENT_SIZE_CHECK(type) _Static_assert(sizeof(type) <= EVENT_SIZE_MAX, #type " is too large to fit into the dispatch queue")
#else
#define EVENT_SIZE_CHECK(type)
#warning Compiler doesnt support C11 static asserts. Cannot check type size at compile time.
#endif

/* Forward declaration */
struct dispatch_t;
typedef struct dispatch_t dispatch_t;

/**
 * Event source/destination identifiers.
 */
enum event_target_e {
	EVENT_TARGET_NONE,
	EVENT_TARGET_MAIN,
	EVENT_TARGET_LED,
};
typedef enum event_target_e event_target_e;

/**
 * Event structure
 */
struct event_t {
	dispatch_t *dispatch;
	event_target_e source;
	event_target_e destination;
	uint8_t argument[EVENT_SIZE_MAX];
};
typedef struct event_t event_t;

#endif /*_EVENT_H*/