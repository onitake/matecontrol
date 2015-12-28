/**
 * @file coin.c
 * @brief Coin acceptor interface driver implementation
 * 
 * @copyright Matemat controller firmware
 * Copyright © 2015 Chaostreff Basel
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

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "coin.h"
#include "memory.h"

/**
 * Capture the input pin state of the acceptor.
 * 
 * |Bit|07|06|05|04|03|02|01|00|
 * |---|--|--|--|--|--|--|--|--|
 * |Pin|--|--|--|E |D |C |B |A |
 */
#define COIN_PINS() (((PINA & (_BV(PA4) | _BV(PA5) | _BV(PA6) | _BV(PA7))) >> 4) | ((PINB & _BV(PB5)) >> 1))

#ifndef COIN_POLL_TIME
/** Polling period (~100ms) */
#define COIN_POLL_TIME 1600
#endif

/**
 * Event type
 */
typedef enum {
	/** Port polling event (periodic) */
	COIN_EVENT_POLL,
} coin_event_type_t;

/**
 * Event argument structure
 */
typedef struct {
	/** Event type */
	coin_event_type_t type;
	/** Callout event data */
	struct callout co;
} coin_event_t;

/**
 * Driver state structure
 */
typedef struct {
	/** Event queue */
	struct callout_mgr *manager;
	/** Accept callback */
	coin_report_cb *report;
	/** Periodic polling event */
	coin_event_t poll;
	/** Port state */
	uint8_t pins;
	/** Memory manager for the event queue */
	memory_t *memory;
	/** Managed memory pool */
	uint8_t pool[MEMORY_POOL_SIZE(COIN_QUEUE_SIZE, sizeof(coin_event_t))];
} coin_t;

/**
 * Global driver state
 */
static coin_t coin_global __attribute__((section(".noinit")));

/**
 * State debugging
 */
static void coin_debug(uint8_t pins);
/**
 * Event callback
 */
static void coin_callback(struct callout_mgr *cm, struct callout *tim, void *arg);

bool coin_init(struct callout_mgr *manager, coin_report_cb *report) {
	coin_global.memory = memory_init(coin_global.pool, sizeof(coin_global.pool), sizeof(coin_event_t));
	
	if (coin_global.memory) {
		coin_global.manager = manager;
		coin_global.report = report;

		coin_global.pins = COIN_PINS();
		
		// ATmega128 doesn't support PCINT interrupts - use polling instead
		coin_global.poll.type = COIN_EVENT_POLL;
		callout_init(&coin_global.poll.co, coin_callback, &coin_global.poll, COIN_PRIORITY);
		callout_schedule(coin_global.manager, &coin_global.poll.co, COIN_POLL_TIME);

		return true;
	}
	
	return false;
}

void coin_shutdown(void) {
	// Nothing
}

void coin_debug(uint8_t pins) {
	// Calculate the difference in state (0 = same, 1 = changed)
	uint8_t diff = pins ^ coin_global.pins;
	printf_P(PSTR("pins=0x%x diff=0x%x\r\n"), pins, diff);
}

void coin_callback(struct callout_mgr *cm, struct callout *tim, void *arg) {
	if (arg) {
		coin_event_t *priv = (coin_event_t *) arg;
		if (priv->type == COIN_EVENT_POLL) {
			// Capture pin state
			uint8_t pins = COIN_PINS();
			
			// Check if any pins have changed
			if (pins != coin_global.pins) {
				coin_debug(pins);
			}
			
			// Update cached pin state
			coin_global.pins = pins;
		}
	}
}