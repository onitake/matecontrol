/**
 * Matemat controller firmware
 * Copyright (C) 2014 Chaostreff Basel
 * 
 * main.c
 * Main program and event dispatch
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

#include <stdbool.h>
#if DYNMEM
#include <stdlib.h>
#endif
#include <avr/io.h>
#include <avr/wdt.h>
#include "main.h"
#include "dispatch.h"
#include "event.h"
#include "led.h"

/**
 * Disables the watchdog upon system startup.
 * Do not call this function. It will be executed during CPU initialisation automatically.
 */
void watchdog_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void watchdog_init() {
#ifdef MCUCSR
	MCUCSR = 0;
#else
	MCUSR = 0;
#endif
	wdt_disable();
	return;
}

int main(void) {
	// System initialisation
	dispatch_init(16);
	led_init();
	
	// Turn the first LED on
	led_send(EVENT_TARGET_MAIN, LED_A, LED_EVENT_TYPE_ON);
	
	bool running = true;
	while (running) {
		event_t *event = dispatch_dequeue();
		if (event) {
			bool handled = false;
			switch (event->destination) {
				case EVENT_TARGET_MAIN:
					if (event->type == MAIN_EVENT_TYPE_SHUTDOWN) {
						running = false;
					}
					handled = true;
					break;
				case EVENT_TARGET_LED:
					handled = led_handle(event);
					break;
				default:
					// Ignore
					break;
			}
#if DYNMEM
			if (!handled) {
				free(event);
			}
#endif
		}
#if IDLELOOP
		if (running) {
			// TODO sleep here
		}
#endif
	}
	
	// System shutdown
	dispatch_shutdown();
	
	// Perform a software reset by enabling the watchdog at its smallest setting, then pass into an infinite loop
	wdt_enable(WDTO_15MS);
	while (1);
}
