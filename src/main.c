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
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "main.h"
#include "dispatch.h"
#include "event.h"
#include "led.h"

/**
 * Storage pool for the dispatch queue
 */
char main_pool[DISPATCH_POOL_SIZE(DISPATCH_QUEUE_LENGTH)] __attribute__((section (".noinit")));

/**
 * Global dispatch queue
 */
dispatch_t *main_dispatch __attribute__((section (".noinit")));

/**
 * Global running state
 */
bool main_running __attribute__((section (".noinit")));

/**
 * Disables the watchdog upon system startup.
 * Do not call this function directly.
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

bool main_handle(event_t *event) {
	main_event_t *priv = EVENT_PRIVATE(event, main_event_t);
	switch (priv->type) {
		case MAIN_EVENT_TYPE_SHUTDOWN:
			main_running = false;
			return true;
	}
	return false;
}

int main(void) {
	// System initialisation
	main_dispatch = dispatch_init(main_pool, sizeof(main_pool));
	led_init();
	
	// Turn the first LED on
	led_send(main_dispatch, EVENT_TARGET_MAIN, LED_A, LED_EVENT_TYPE_ON);
	
	// Set idle sleep mode
	set_sleep_mode(SLEEP_MODE_IDLE);
	
	// Enable interrupts
	//sei();
	
	main_running = true;
	while (main_running) {
		event_t event;
		if (dispatch_dequeue(main_dispatch, &event)) {
			bool handled = false;
			switch (event.destination) {
				case EVENT_TARGET_MAIN:
					handled = main_handle(&event);
					break;
				case EVENT_TARGET_LED:
					handled = led_handle(&event);
					break;
				default:
					// Ignore
					break;
			}
		}
#if IDLELOOP
	if (main_running) {
			sleep_mode();
		}
#endif
	}
	
	// System shutdown
	cli();
	//led_shutdown();
	dispatch_shutdown(main_dispatch);
	
	// Perform a software reset by enabling the watchdog at its smallest setting, then pass into an infinite loop
	wdt_enable(WDTO_15MS);
	while (1);
}
