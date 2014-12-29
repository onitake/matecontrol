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
 * Event source and destination identifier type.
 * @see event_target_e for valid identifiers.
 */
typedef uint8_t event_target_t;
/**
 * Event type.
 * The type codes are module specific, this is just the data type used to hold the code.
 */
typedef uint8_t event_type_t;
/**
 * Event argument type.
 * The meaning of this type is module specific. This is just the maximum data size.
 */
typedef uint16_t event_argument_t;

/**
 * Event source/destination identifiers.
 */
enum event_target_e {
	EVENT_TARGET_NONE,
	EVENT_TARGET_MAIN,
	EVENT_TARGET_LED,
};

/**
 * Event structure
 */
typedef struct {
	event_target_t source;
	event_target_t destination;
	event_type_t type;
	event_argument_t argument;
} event_t;

#endif /*_EVENT_H*/