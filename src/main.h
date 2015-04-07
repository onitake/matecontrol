/**
 * Matemat controller firmware
 * Copyright (C) 2014 Chaostreff Basel
 * 
 * main.h
 * Main module interface
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

#ifndef _MAIN_H
#define _MAIN_H

#include "event.h"

/**
 * Main process event types
 */
enum main_event_type_e {
	MAIN_EVENT_TYPE_SHUTDOWN,
};
typedef enum main_event_type_e main_event_type_e;

/**
 * Main process event structure
 */
struct main_event_t {
	main_event_type_e type;
};
typedef struct main_event_t main_event_t;
EVENT_SIZE_CHECK(struct main_event_t);

/**
 * Global dispatch queue
 */
extern dispatch_t *main_dispatch;

#endif /*_MAIN_H*/