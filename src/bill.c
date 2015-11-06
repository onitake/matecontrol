/**
 * @file bill.c
 * @brief Banknote scanner interface driver implementation
 * 
 * Description of scanner interface pins:
 * 
 * |Name |Short |Direction |Active |Function                    |
 * |-----|------|----------|-------|----------------------------|
 * |VALID|V     |Output    |Low    |Banknote valid              |
 * |STKF |S     |Output    |High   |Stacker full                |
 * |ABN  |A     |Output    |High   |Abnormal (error)            |
 * |BUSY |B     |Output    |High   |Busy                        |
 * |VEND |E     |Output    |Low    |Banknote type (bit pattern) |
 * |INH  |I     |Input     |High   |Inhibit                     |
 * |ACK  |K     |Input     |Low    |Acknowledge (escrow mode)   |
 * |REJ  |R     |Input     |Low    |Reject (escrow mode)        |
 * 
 * Bit pattern graph of initialisation sequence:
 * 
 * @dot
 * digraph SelfTest {
 *   rankdir=LR;
 *   node [shape=Mrecord fontsize=11 fontname="Helvetica"];
 *   S0 [label="{V|S|A|B|E}|{1|1|1|1|7}"];
 *   S1 [label="{V|S|A|B|E}|{1|0|0|1|3}"];
 *   S2 [label="{V|S|A|B|E}|{1|0|0|1|5}"];
 *   S3 [label="{V|S|A|B|E}|{1|0|0|1|6}"];
 *   S4 [label="{V|S|A|B|E}|{0|0|0|1|7}"];
 *   S5 [label="{V|S|A|B|E}|{1|0|1|1|7}"];
 *   S6 [label="{V|S|A|B|E}|{1|1|0|1|7}"];
 *   S7 [label="{V|S|A|B|E}|{1|0|0|1|7}"];
 *   S8 [label="{V|S|A|B|E}|{1|0|0|1|7}"];
 *   S9 [label="{V|S|A|B|E}|{1|0|0|0|7}"];
 *   S0->S1->S2->S3->S4->S5->S6->S7->S8->S9;
 * }
 * @enddot
 * 
 * After a banknote is accepted by the scanner, it will output the corresponding
 * bit pattern on VEND1:3 (and VALID will be low).
 * To return the scanner back into accept mode, pull INH low shortly and then
 * back high.
 * 
 * State machine diagram:
 * 
 * @dot
 * digraph State {
 *   node [shape=Mrecord fontsize=11 fontname="Helvetica"];
 *   edge [fontsize=11 fontname="Helvetica"];
 *   UNINITIALIZED [shape=point];
 *   SELFTEST [label="{SELFTEST | ¬INH ∧ ACK ∧ REJ}"];
 *   IDLE [label="{IDLE | INH}"];
 *   VALIDATION [label="{VALIDATION | ACK ∨ REJ}"];
 *   ACCEPT [label="{ACCEPT | REPORT}"];
 *   END [label="{END | ERROR ∧ ¬INH}"];
 *   UNINITIALIZED->SELFTEST [label=""];
 *   SELFTEST->IDLE [label="¬BUSY"];
 *   IDLE->VALIDATION [label="BUSY"];
 *   VALIDATION->END [label="ABN"];
 *   VALIDATION->ACCEPT [label="VALID"];
 *   ACCEPT->IDLE [label="¬BUSY"];
 *   ACCEPT->END [label="ABN"];
 *   END->IDLE [label="¬BUSY"];
 * }
 * @enddot
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
#include "memory.h"
#include "bill.h"

/**
 * Capture the input pin state of the scanner.
 * 
 * |Bit|07   |06   |05   |04  |03 |02   |01  |00|
 * |---|-----|-----|-----|----|---|-----|----|--|
 * |Pin|VEND1|VEND2|VEND3|BUSY|ABN|VALID|FULL|--|
 */
#define BILL_PINS() (((PINB & (_BV(PB6) | _BV(PB7))) >> 5) | (PINC & (_BV(PB3) | _BV(PB4) | _BV(PB5) | _BV(PB6) | _BV(PB7))))
/**
 * Mask the VEND bits of an input state (but do not shift)
 */
#define BILL_PINS_VEND(pins) (pins & (_BV(5) | _BV(6) | _BV(7)))
/**
 * Generate a VEND bit pattern to match with BILL_PINS()
 * @param vend1 the bit value of vend1 (0 = L, 1 = H)
 * @param vend2 the bit value of vend2 (0 = L, 1 = H)
 * @param vend3 the bit value of vend3 (0 = L, 1 = H)
 */
#define BILL_BITS_VEND(vend1, vend2, vend3) ((vend1 << 7) | (vend2 << 6) | (vend3 << 5))
/**
 * Mask the BUSY bit of an input state (but do not shift)
 */
#define BILL_PINS_BUSY(pins) (pins & _BV(4))
/**
 * Mask the ABN bit of an input state (but do not shift)
 */
#define BILL_PINS_ABN(pins) (pins & _BV(3))
/**
 * Mask the VALID bit of an input state (but do not shift)
 */
#define BILL_PINS_VALID(pins) (pins & _BV(2))
/**
 * Mask the STKF bit of an input state (but do not shift)
 */
#define BILL_PINS_STKF(pins) (pins & _BV(1))
/**
 * Set the state of the INH output port.
 * @param value 0 = L, 1 = H, 2 = toggle
 */
#define BILL_PORT_INH(value) if ((value) == 0) { PORTC &= ~_BV(2); } else if ((value) == 1) { PORTC |= _BV(2); } else if ((value) == 2) { PORTC ^= _BV(2); }
/**
 * Set the state of the ACK output port.
 * @param value 0 = L, 1 = H, 2 = toggle
 */
#define BILL_PORT_ACK(value) if ((value) == 0) { PORTC &= ~_BV(1); } else if ((value) == 1) { PORTC |= _BV(1); } else if ((value) == 2) { PORTC ^= _BV(1); }
/**
 * Set the state of the REJ output port.
 * @param value 0 = L, 1 = H, 2 = toggle
 */
#define BILL_PORT_REJ(value) if ((value) == 0) { PORTC &= ~_BV(0); } else if ((value) == 1) { PORTC |= _BV(0); } else if ((value) == 2) { PORTC ^= _BV(0); }
/**
 * Initialize the banknote scanner input and output GPIO ports,
 * enable pull-up resistors, and set the default output port states.
 */
#define BILL_INIT() do { DDRB &= ~(_BV(PB6) | _BV(PB7)); PORTB |= _BV(PB6) | _BV(PB7); DDRC = _BV(PC0) | _BV(PC1) | _BV(PC2); PORTC = _BV(PC0) | _BV(PC1) | _BV(PC3) | _BV(PC4) | _BV(PC5) | _BV(PC6) | _BV(PC7); } while(0);

#ifndef BILL_POLL_TIME
/** Polling period (~100ms) */
#define BILL_POLL_TIME 1600
#endif

/**
 * Descriptor for a single banknote denomination
 */
typedef struct {
	/**
	 * VEND value bit map (pre-shifted, so may directly mask BILL_PINS())
	 * 
	 * Use BILL_BITS_VEND() to generate suitable bit patterns.
	 */
	uint8_t vend;
	/** Banknote value */
	uint16_t denomination;
} bill_denomination_t;

static const bill_denomination_t BILL_DENOMINATIONS[] PROGMEM = {
	{ BILL_BITS_VEND(0, 1, 1), 10 },
	{ BILL_BITS_VEND(1, 0, 1), 20 },
	{ BILL_BITS_VEND(0, 0, 1), 50 },
	{ BILL_BITS_VEND(1, 1, 0), 100 },
	{ BILL_BITS_VEND(0, 1, 0), 200 },
};

/**
 * Event type
 */
typedef enum {
	/** Port polling event (periodic) */
	BILL_EVENT_POLL,
} bill_event_type_t;

/**
 * Event argument structure
 */
typedef struct {
	/** Event type */
	bill_event_type_t type;
	/** Callout event data */
	struct callout co;
} bill_event_t;

/**
 * Driver state structure
 */
typedef struct {
	/** Event queue */
	struct callout_mgr *manager;
	/** Scan success callback */
	bill_report_cb *report;
	/** Scan error callback */
	bill_error_cb *error;
	/** State machine state */
	bill_state_t state;
	/** Input pin state bit map */
	uint8_t input;
	/** Banknote reception inhibit (on/off) */
	bool inhibit;
	/** Escrow mode (i.e. verify bill before stacking it) */
	bool escrow;
	/** Value of the vend register */
	uint8_t vend;
	/** Periodic polling event */
	bill_event_t poll;
	/** Memory manager for the event queue */
	memory_t *memory;
	/** Managed memory pool */
	uint8_t pool[MEMORY_POOL_SIZE(BILL_QUEUE_SIZE, sizeof(bill_event_t))];
} bill_t;

/**
 * Global driver state
 */
static bill_t bill_global __attribute__((section(".noinit")));

/**
 * Event callback
 */
static void bill_callback(struct callout_mgr *cm, struct callout *tim, void *arg);

bool bill_init(struct callout_mgr *manager, bill_report_cb *report, bill_error_cb *error) {
	bill_global.memory = memory_init(bill_global.pool, sizeof(bill_global.pool), sizeof(bill_event_t));

	if (bill_global.memory) {
		bill_global.manager = manager;
		bill_global.report = report;
		bill_global.error = error;
		bill_global.inhibit = false;
		bill_global.escrow = false;
		bill_global.vend = 0;
		
		// Signal the poll handler to capture state first
		bill_global.state = BILL_STATE_UNINITIALIZED;
		
		BILL_INIT();
		BILL_PORT_ACK(1);
		BILL_PORT_REJ(1);
		BILL_PORT_INH(0);
		//bill_global.input = BILL_PINS();
		
		// ATmega128 doesn't support PCINT interrupts - use polling instead
		bill_global.poll.type = BILL_EVENT_POLL;
		callout_init(&bill_global.poll.co, bill_callback, &bill_global.poll, BILL_PRIORITY);
		callout_schedule(bill_global.manager, &bill_global.poll.co, BILL_POLL_TIME);
		
		return true;
	}
	
	return false;
}

void bill_shutdown(void) {
	callout_stop(bill_global.manager, &bill_global.poll.co);
}

void bill_callback(struct callout_mgr *cm, struct callout *tim, void *arg) {
	if (arg) {
		bill_event_t *priv = (bill_event_t *) arg;
		if (priv->type == BILL_EVENT_POLL) {
			// Capture pin state
			uint8_t pins = BILL_PINS();
			
			// Check if pin state has changed (or is unknown)
			if (pins != bill_global.input || bill_global.state == BILL_STATE_UNINITIALIZED) {
				// Calculate the difference in state (0 = same, 1 = changed)
				// -> not possible if previous state is unknown
				//uint8_t diff = pins ^ bill_global.input;
				
				// Update input pin cache
				bill_global.input = pins;
				
				// FIXME REMOVETHIS Print out pin state
				char ports[17];
				size_t i;
				for (i = 0; i < 8; i++) {
					ports[i] = (PINC & (0x80 >> i)) ? '1' : '0';
				}
				for (i = 0; i < 8; i++) {
					ports[i + 8] = (PINB & (0x80 >> i)) ? '1' : '0';
				}
				ports[16] = '\0';
				printf_P(PSTR("PINC:PINB=%s\r\n"), ports);
				printf_P(PSTR("bill valid=%c stkf=%c abn=%c busy=%c vend=%u\r\n"), BILL_PINS_VALID(pins) ? 'H' : 'L', BILL_PINS_STKF(pins) ? 'H' : 'L', BILL_PINS_ABN(pins) ? 'H' : 'L', BILL_PINS_BUSY(pins) ? 'H' : 'L', BILL_PINS_VEND(pins) >> 5);

				// Evaluate state
				switch (bill_global.state) {
					case BILL_STATE_UNINITIALIZED:
					case BILL_STATE_SELFTEST:
						if (BILL_PINS_VALID(pins) && !BILL_PINS_STKF(pins) && !BILL_PINS_ABN(pins) && !BILL_PINS_BUSY(pins) && BILL_PINS_VEND(pins) == BILL_BITS_VEND(1, 1, 1)) {
							// Self-test complete
							bill_global.state = BILL_STATE_IDLE;
						} else if (bill_global.state == BILL_STATE_UNINITIALIZED) {
							BILL_PORT_INH(1);
							bill_global.state = BILL_STATE_SELFTEST;
						}
						break;
						
					case BILL_STATE_IDLE:
						if (BILL_PINS_BUSY(pins) && !BILL_PINS_ABN(pins) && !bill_global.inhibit) {
							// Scanning started
							bill_global.state = BILL_STATE_VALIDATION;
						}
						break;
						
					case BILL_STATE_VALIDATION:
						if (BILL_PINS_ABN(pins)) {
							// Abort, jam
							if (bill_global.error) {
								bill_global.error(BILL_ERROR_SCAN, 0);
							}
							bill_global.state = BILL_STATE_END;
						} else if (!BILL_PINS_VALID(pins)) {
							// Escrow mode?
							if (bill_global.escrow) {
								// Accept bill (active L)
								BILL_PORT_ACK(0);
							}
							// Store banknote value
							bill_global.vend = BILL_PINS_VEND(pins);
							bill_global.state = BILL_STATE_ACCEPT;
						}
						break;
						
					case BILL_STATE_ACCEPT:
						if (BILL_PINS_ABN(pins)) {
							if (bill_global.escrow) {
								// Disable accept/reject state
								BILL_PORT_ACK(1);
								BILL_PORT_REJ(1);
							}
							
							// Abort, jam
							if (bill_global.error) {
								bill_global.error(BILL_ERROR_SCAN, 0);
							}
							
							bill_global.state = BILL_STATE_END;
						} else if (BILL_PINS_STKF(pins) || !BILL_PINS_BUSY(pins)) {
							if (bill_global.escrow) {
								// Disable accept/reject state
								BILL_PORT_ACK(0);
								BILL_PORT_REJ(0);
							}
							
							// Report accepted bill first
							if (bill_global.report) {
								size_t i;
								for (i = 0; i < sizeof(BILL_DENOMINATIONS) / sizeof(BILL_DENOMINATIONS[0]); i++) {
									if (pgm_read_byte(&BILL_DENOMINATIONS[i].vend) == BILL_PINS_VEND(pins)) {
										bill_global.report(pgm_read_word(&BILL_DENOMINATIONS[i].denomination));
										break;
									}
								}
							}
							
							if (BILL_PINS_STKF(pins)) {
								// Then report the full stack
								if (bill_global.error) {
									bill_global.error(BILL_ERROR_FULL, 0);
								}
							}
							
							// Signal completion
							BILL_PORT_INH(0);
							
							bill_global.state = BILL_STATE_END;
						}
						break;
						
					case BILL_STATE_END:
						if (!BILL_PINS_BUSY(pins)) {
							// Return to accept state
							BILL_PORT_INH(0);
							
							bill_global.state = BILL_STATE_IDLE;
						}
						
						// Reset stored value
						bill_global.vend = 0;
						break;
				}
			}
			
			// Reschedule next poll event
			callout_schedule(bill_global.manager, tim, BILL_POLL_TIME);
		} else {
			memory_release(arg);
		}
	}
}

void bill_inhibit(bool inhibit) {
	bill_global.inhibit = inhibit;
	BILL_PORT_INH(inhibit ? 1 : 0);
}

void bill_escrow(bool escrow) {
	//bill_global.escrow = escrow;
}

bill_state_t bill_state(void) {
	return bill_global.state;
}