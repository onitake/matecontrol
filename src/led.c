/**
 * @file led.c
 * @brief LED port driver implementation
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <aversive/irq_lock.h>
#include "led.h"
#include "memory.h"

/* TODO These should go into a configuration header */

/** LED A bit index */
#define LED_P_A PG0
/** LED A port register */
#define LED_PORT_A PORTG
/** LED A pin register */
#define LED_PIN_A PING
/** LED A direction register */
#define LED_DDR_A DDRG
/** LED B bit index */
#define LED_P_B PG1
/** LED B port register */
#define LED_PORT_B PORTG
/** LED B pin register */
#define LED_PIN_B PING
/** LED B direction register */
#define LED_DDR_B DDRG
/** LED C bit index */
#define LED_P_C PG2
/** LED C port register */
#define LED_PORT_C PORTG
/** LED C pin register */
#define LED_PIN_C PING
/** LED C direction register */
#define LED_DDR_C DDRG

/**
 * LED driver event structure
 */
typedef struct {
	/** Event data (self-referenced) */
	struct callout co;
	/** Event type */
	led_event_type_e type;
	/** Affected LED */
	led_name_e name;
	/** On time */
	uint16_t on;
	/** Off time */
	uint16_t off;
	/** Blink? */
	bool blink;
	/** Periodic? */
	bool periodic;
} led_event_t;

/**
 * LED driver object
 */
typedef struct {
	/** Event queue */
	struct callout_mgr *manager;
	/** Preallocated events for blink requests */
	led_event_t blink[3];
	/** Memory manager for the event queue */
	memory_t *memory;
	/** Managed memory pool */
	uint8_t pool[MEMORY_POOL_SIZE(LED_QUEUE_SIZE, sizeof(led_event_t))];
} led_t;

/**
 * Global driver object
 */
static led_t led_global __attribute__((section(".noinit")));

/**
 * Handle a LED event.
 * @param cm the event queue manager
 * @param tim the event information
 * @param arg a private argument
 */
static void led_callback(struct callout_mgr *cm, struct callout *tim, void *arg);

bool led_init(struct callout_mgr *manager) {
	led_global.memory = memory_init(led_global.pool, sizeof(led_global.pool), sizeof(led_event_t));

	led_name_e led;
	for (led = 0; led < LED_MAX; led++) {
		led_event_t *event = &led_global.blink[led];
		event->name = led;
		event->blink = true;
		callout_init(&event->co, led_callback, event, LED_PRIORITY);
	}
	
	if (led_global.memory) {
		led_global.manager = manager;
		
		LED_PORT_A &= ~_BV(LED_P_A);
		LED_DDR_A |= _BV(LED_P_A);
		LED_PORT_B &= ~_BV(LED_P_B);
		LED_DDR_B |= _BV(LED_P_B);
		LED_PORT_C &= ~_BV(LED_P_C);
		LED_DDR_C |= _BV(LED_P_C);
		
		return true;
	}

	return false;
}

void led_shutdown(bool off) {
	led_name_e led;
	for (led = 0; led < LED_MAX; led++) {
		callout_stop(led_global.manager, &led_global.blink[led].co);
	}
	
	if (off) {
		LED_PORT_A &= ~_BV(LED_P_A);
		LED_DDR_A &= ~_BV(LED_P_A);
		LED_PORT_B &= ~_BV(LED_P_B);
		LED_DDR_B &= ~_BV(LED_P_B);
		LED_PORT_C &= ~_BV(LED_P_C);
		LED_DDR_C &= ~_BV(LED_P_C);
	}
}

bool led_action(led_name_e led, led_event_type_e action) {
	if (led < LED_MAX) {
		uint8_t flags;
		IRQ_LOCK(flags);
		led_event_t *event = (led_event_t *) memory_allocate(led_global.memory);
		IRQ_UNLOCK(flags);
		
		if (event) {
			event->type = action;
			event->name = led;
			event->blink = false;
			
			callout_init(&event->co, led_callback, event, LED_PRIORITY);
			
			return callout_schedule(led_global.manager, &event->co, 0) == 0;
		}
	}
	return false;
}

bool led_blink(led_name_e led, uint16_t ontime, uint16_t offtime, bool repeat) {
	if (led < LED_MAX) {
		led_event_t *event = &led_global.blink[led];
		
		callout_stop(led_global.manager, &event->co);
	
		event->type = LED_EVENT_TYPE_ON;
		event->on = ontime;
		event->off = offtime;
		event->periodic = repeat;

		// TODO use callout.now instead of 0
		return callout_schedule(led_global.manager, &event->co, 0) == 0;
	}
	return false;
}

static void led_callback(struct callout_mgr *cm, struct callout *tim, void *arg) {
	uint8_t flags;
	if (arg) {
		led_event_t *priv = (led_event_t *) arg;
		
		switch (priv->type) {
			case LED_EVENT_TYPE_ON:
				switch (priv->name) {
					case 0:
						LED_PORT_A |= _BV(LED_P_A);
						break;
					case 1:
						LED_PORT_B |= _BV(LED_P_B);
						break;
					case 2:
						LED_PORT_C |= _BV(LED_P_C);
						break;
					default:
						break;
				}
				break;
			case LED_EVENT_TYPE_OFF:
				switch (priv->name) {
					case 0:
						LED_PORT_A &= ~_BV(LED_P_A);
						break;
					case 1:
						LED_PORT_B &= ~_BV(LED_P_B);
						break;
					case 2:
						LED_PORT_C &= ~_BV(LED_P_C);
						break;
					default:
						break;
				}
				break;
			case LED_EVENT_TYPE_TOGGLE:
				switch (priv->name) {
					case 0:
						LED_PORT_A ^= _BV(LED_P_A);
						break;
					case 1:
						LED_PORT_B ^= _BV(LED_P_B);
						break;
					case 2:
						LED_PORT_C ^= _BV(LED_P_C);
						break;
					default:
						break;
				}
				break;
		}
		
		if (priv->blink) {
			if (priv->type == LED_EVENT_TYPE_ON) {
				priv->type = LED_EVENT_TYPE_OFF;
				callout_init(&priv->co, led_callback, priv, LED_PRIORITY);
				// FIXME callout_reschedule does not seem to do what we expect it to do, so we use callout_schedule instead.
				if (callout_schedule(led_global.manager, &priv->co, priv->on) == 0) {
					return;
				}
			} else {
				if (priv->periodic) {
					priv->type = LED_EVENT_TYPE_ON;
					callout_init(&priv->co, led_callback, priv, LED_PRIORITY);
					// FIXME callout_reschedule does not seem to do what we expect it to do, so we use callout_schedule instead.
					if (callout_schedule(led_global.manager, &priv->co, priv->off) == 0) {
						return;
					}
				}
			}
		} else {
			callout_stop(led_global.manager, &led_global.blink[priv->name].co);
		}
		
		IRQ_LOCK(flags);
		memory_release(arg);
		IRQ_UNLOCK(flags);
	}
}
