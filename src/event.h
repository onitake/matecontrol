#ifndef _EVENT_H
#define _EVENT_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Event source and destination identifier type.
 * @see event_target_e for the valid identifiers.
 */
typedef uint8_t event_target_t;
/**
 * Event type.
 * The type codes are module specific, this is just the data type used to hold the code.
 */
typedef uint8_t event_type_t;

/**
 * Event source/destination identifiers.
 */
enum event_target_e {
	EVENT_TARGET_NONE,
	EVENT_TARGET_MAIN,
};

/**
 * Event structure
 */
typedef struct {
	event_target_t source;
	event_target_t destination;
	event_type_t type;
} event_t;

#endif /*_EVENT_H*/