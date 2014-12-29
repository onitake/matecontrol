/**
 * Matemat controller firmware
 * Copyright (C) 2014 Chaostreff Basel
 * 
 * led.c
 * LED port driver implementation
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
#if DYNMEM
#include <stdlib.h>
#endif
#include "led.h"
#include "dispatch.h"

#define LED_P_A PG0
#define LED_PORT_A PORTG
#define LED_PIN_A PING
#define LED_DDR_A DDRG
#define LED_P_B PG1
#define LED_PORT_B PORTG
#define LED_PIN_B PING
#define LED_DDR_B DDRG
#define LED_P_C PG2
#define LED_PORT_C PORTG
#define LED_PIN_C PING
#define LED_DDR_C DDRG

void led_init() {
	LED_PORT_A &= ~_BV(LED_P_A);
	LED_DDR_A |= _BV(LED_P_A);
	LED_PORT_B &= ~_BV(LED_P_B);
	LED_DDR_B |= _BV(LED_P_B);
	LED_PORT_C &= ~_BV(LED_P_C);
	LED_DDR_C |= _BV(LED_P_C);
}

void led_shutdown(bool off) {
	if (off) {
		LED_PORT_A &= ~_BV(LED_P_A);
		LED_DDR_A &= ~_BV(LED_P_A);
		LED_PORT_B &= ~_BV(LED_P_B);
		LED_DDR_B &= ~_BV(LED_P_B);
		LED_PORT_C &= ~_BV(LED_P_C);
		LED_DDR_C &= ~_BV(LED_P_C);
	}
}

bool led_send(event_target_t source, uint8_t led, uint8_t action) {
#if DYNMEM
	event_t *event = (event_t *) calloc(1, sizeof(event_t));
#else
	event_t levent;
	event_t *event = &levent;
#endif
	event->source = source;
	event->destination = EVENT_TARGET_LED;
	event->type = action;
	event->argument = led;
	return dispatch_enqueue(event);
}

bool led_handle(event_t *event) {
	bool handled = false;
	switch (event->type) {
		case LED_EVENT_TYPE_ON:
			switch (event->argument) {
				case 0:
					LED_PORT_A |= _BV(LED_P_A);
					handled = true;
					break;
				case 1:
					LED_PORT_B |= _BV(LED_P_B);
					handled = true;
					break;
				case 2:
					LED_PORT_C |= _BV(LED_P_C);
					handled = true;
					break;
			}
			break;
		case LED_EVENT_TYPE_OFF:
			switch (event->argument) {
				case 0:
					LED_PORT_A &= ~_BV(LED_P_A);
					handled = true;
					break;
				case 1:
					LED_PORT_B &= ~_BV(LED_P_B);
					handled = true;
					break;
				case 2:
					LED_PORT_C &= ~_BV(LED_P_C);
					handled = true;
					break;
			}
			break;
		case LED_EVENT_TYPE_TOGGLE:
			switch (event->argument) {
				case 0:
					LED_PORT_A ^= _BV(LED_P_A);
					handled = true;
					break;
				case 1:
					LED_PORT_B ^= _BV(LED_P_B);
					handled = true;
					break;
				case 2:
					LED_PORT_C ^= _BV(LED_P_C);
					handled = true;
					break;
			}
			break;
	}
#if DYNMEM
	if (handled) {
		free(event);
	}
#endif
	return handled;
}
