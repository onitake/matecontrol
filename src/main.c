#include <stdbool.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include "main.h"
#include "dispatch.h"

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
	
	bool running = true;
	while (running) {
		event_t *event = dispatch_dequeue();
		if (event) {
			switch (event->destination) {
				case EVENT_TARGET_MAIN:
					if (event->type == MAIN_EVENT_TYPE_SHUTDOWN) {
						running = false;
					}
					break;
				default:
					// Ignore
					break;
			}
		}
#if IDLELOOP
		if (running) {
			// TODO sleep here
		}
#endif
	}
	
	// System shutdown
	dispatch_shutdown();
	
	// Perform a software reset by enabling the watchdog at its smallest setting and passing into an infinite loop
	wdt_enable(WDTO_15MS);
	while (1);
}
