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

bool led_send(dispatch_t *dispatch, event_target_e source, uint8_t led, uint8_t action) {
	event_t event;
	led_event_t *priv = EVENT_PRIVATE(&event, led_event_t);
	event.source = source;
	event.destination = EVENT_TARGET_LED;
	priv->type = action;
	priv->name = led;
	return dispatch_enqueue(dispatch, &event);
}

bool led_handle(event_t *event) {
	led_event_t *priv = EVENT_PRIVATE(event, led_event_t);
	bool handled = false;
	switch (priv->type) {
		case LED_EVENT_TYPE_ON:
			switch (priv->name) {
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
			switch (priv->name) {
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
			switch (priv->name) {
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
	return handled;
}
