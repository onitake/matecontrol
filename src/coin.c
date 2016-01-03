/**
 * @file coin.c
 * @brief Coin acceptor interface driver implementation
 * 
 * Due to limited documentation, some mistakes where made during the design
 * of the interface circuit. To allow monitoring of the COIN_OUT_F pin,
 * the COIN_OUT_A pin will be disconnected and BCO mode chosen by
 * DIP switch on the coin acceptor. As the COIN_OUT_A is only used to signal
 * BCO mode activation in this case, the other output pins can be mapped
 * directly. On top of that, the inhibit line will not be available and
 * coin input is enabled at all times (unless a hardware condition signals
 * otherwise).
 * 
 * In BCO mode, a bit pattern signals when a coin was inserted or an error has
 * occured. This pattern will be sent for a duration of 80-120ms, so a
 * suitable polling interval needs to be chosen.
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
 * |Pin|--|--|F |E |D |C |B |A*|
 * 
 * * The state of pin A is not captured.
 */
#define COIN_PINS() (((PINA & (_BV(PA4) | _BV(PA5) | _BV(PA6) | _BV(PA7))) >> 3) | (PINB & _BV(PB5)))
/**
 * Generate a coin bit pattern to match with COIN_PINS_PATTERN()
 */
#define COIN_BITS_PATTERN(f, e, d, b) ((f << 5) | (e << 4) | (d << 3) | (b << 1))
/**
 * Mask the ALARM bit (C) of an input state (but do not shift)
 */
#define COIN_PINS_ALARM(pins) (pins & _BV(2))
/**
 * Mask the coin pattern bits (B, D, E, F) of an input state (but do not shift)
 */
#define COIN_PINS_PATTERN(pins) (pins & (_BV(1) | _BV(3) | _BV(4) | _BV(5)))

#ifndef COIN_POLL_TIME
/** Polling period (~12ms) */
#define COIN_POLL_TIME 200
#endif

/**
 * Descriptor for a single coin pattern
 */
typedef struct {
	/**
	 * PATTERN value bit map (pre-shifted, so may directly mask COIN_PINS_PATTERN())
	 * 
	 * Use COIN_BITS_PATTERN() to generate suitable bit patterns.
	 */
	uint8_t pattern;
	/** Coin value */
	currency_t denomination;
} coin_denomination_t;

static const coin_denomination_t COIN_DENOMINATIONS[] PROGMEM = {
	{ COIN_BITS_PATTERN(0, 0, 0, 0), { 0, 5 } },
	{ COIN_BITS_PATTERN(0, 0, 1, 1), { 0, 10 } },
	{ COIN_BITS_PATTERN(1, 1, 0, 0), { 0, 20 } },
	{ COIN_BITS_PATTERN(1, 0, 0, 1), { 0, 50 } },
	{ COIN_BITS_PATTERN(0, 1, 0, 1), { 1, 0 } },
	{ COIN_BITS_PATTERN(1, 1, 1, 1), { 2, 0 } },
	{ COIN_BITS_PATTERN(1, 0, 1, 0), { 5, 0 } },
};

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
	/** Acceptor error callback */
	coin_error_cb *error;
	/** Periodic polling event */
	coin_event_t poll;
	/** Port state */
	uint8_t pins;
	/** Error state */
	bool alarm;
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

bool coin_init(struct callout_mgr *manager, coin_report_cb *report, coin_error_cb *error) {
	coin_global.memory = memory_init(coin_global.pool, sizeof(coin_global.pool), sizeof(coin_event_t));
	
	if (coin_global.memory) {
		coin_global.manager = manager;
		coin_global.report = report;
		coin_global.error = error;
		coin_global.alarm = false;

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
				
				if (COIN_PINS_ALARM(pins)) {
					if (!coin_global.alarm) {
						coin_global.alarm = true;
						if (coin_global.error) {
							coin_global.error(COIN_ERROR_ALARM);
						}
					}
				} else {
					coin_global.alarm = false;
				}
				
				if (!coin_global.alarm && coin_global.report) {
					size_t i;
					for (i = 0; i < sizeof(COIN_DENOMINATIONS) / sizeof(COIN_DENOMINATIONS[0]); i++) {
						if (pgm_read_byte(&COIN_DENOMINATIONS[i].pattern) == COIN_PINS_PATTERN(pins)) {
							currency_t denomination;
							denomination.base = pgm_read_word(&COIN_DENOMINATIONS[i].denomination.base);
							denomination.cents = pgm_read_byte(&COIN_DENOMINATIONS[i].denomination.cents);
							coin_global.report(denomination);
							break;
						}
					}
				}
			}
			
			// Update cached pin state
			coin_global.pins = pins;
		}
	}
}