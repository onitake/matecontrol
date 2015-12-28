/**
 * @file main.c
 * @brief Main program and event dispatch
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

#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/version.h>
#include <avr/pgmspace.h>
#include <aversive/irq_lock.h>
#include <base/callout/callout.h>
#include <hardware/timer/timer.h>
#include "main.h"
#include "memory.h"
#include "led.h"
#include "clock.h"
#include "util.h"
#include "console.h"
#include "bill.h"
#include "coin.h"
#include "bank.h"

/**
 * Main process event types
 */
typedef enum {
	/** System shutdown event */
	MAIN_EVENT_TYPE_SHUTDOWN,
} main_event_type_e;

/**
 * Main process event argument
 */
typedef struct {
	/** Event data (self-referenced) */
	struct callout co;
	/** Event type */
	main_event_type_e type;
} main_event_t;

/**
 * Main process object
 */
typedef struct {
	/** Global running state */
	bool running;
	/** System time (in ticks) */
	uint16_t time;
	/** Global event queue manager */
	struct callout_mgr manager;
	/** Global credit store */
	bank_t bank;
	/** Main process event memory manager */
	memory_t *memory;
	/** Main process event memory pool */
	uint8_t pool[MEMORY_POOL_SIZE(MAIN_QUEUE_SIZE, sizeof(main_event_t))];
} main_t;

/**
 * Global main process object
 */
static main_t main_global __attribute__((section (".noinit")));

/**
 * Main entry point
 */
int main(void) __attribute__((noreturn));

/**
 * Disables the watchdog upon system startup.
 * Do not call this function directly.
 * `__attribute__((section(".init3")))` guarantees that it will be called
 * implicitly on system startup.
 */
void watchdog_init(void) __attribute__((naked)) __attribute__((section(".init3")));

/**
 * Handle a main process event.
 * @param cm the event queue manager
 * @param tim the event information
 * @param arg a private argument
 */
static void main_callback(struct callout_mgr *cm, struct callout *tim, void *arg);

/**
 * Update the system timer and call the event queue manager
 */
static void main_systick(void);

/**
 * Add a scanned banknote value to the piggybank (callback)
 */
static void main_bill_report(uint16_t denomination);
/**
 * Report a banknote scanning error to the user (callback)
 */
static void main_bill_error(bill_error_t error, uint16_t denomination);
/**
 * Add a scanned banknote value to the piggybank (callback)
 */
static void main_coin_report(currency_t denomination);
/**
 * Report a change in account balance
 */
static void main_balance_report(currency_t balance);

void watchdog_init(void) {
#ifdef MCUCSR
	MCUCSR = 0;
#else
	MCUSR = 0;
#endif
	wdt_disable();
	return;
}

void main_shutdown(void) {
	uint8_t flags;
	IRQ_LOCK(flags);
	main_event_t *event = (main_event_t *) memory_allocate(main_global.memory);
	IRQ_UNLOCK(flags);
	if (event) {
		event->type = MAIN_EVENT_TYPE_SHUTDOWN;
		callout_init(&event->co, main_callback, event, MAIN_PRIORITY);
		callout_schedule(&main_global.manager, &event->co, 0);
	}
}

static void main_callback(struct callout_mgr *cm, struct callout *tim, void *arg) {
	uint8_t flags;
	if (arg) {
		main_event_t *priv = (main_event_t *) arg;
		switch (priv->type) {
			case MAIN_EVENT_TYPE_SHUTDOWN:
				main_global.running = false;
				break;
		}
		IRQ_LOCK(flags);
		memory_release(arg);
		IRQ_UNLOCK(flags);
	}
}

static void main_systick(void) {
	uint8_t flags;
	IRQ_LOCK(flags);
	main_global.time += 0x100;
	IRQ_UNLOCK(flags);
	callout_manage(&main_global.manager);
}

uint16_t main_time(void) {
	uint8_t flags;
	IRQ_LOCK(flags);
	uint16_t time = main_global.time | timer2_get();
	IRQ_UNLOCK(flags);
	return time;
}

static void main_bill_report(uint16_t denomination) {
	printf_P(PSTR("Scanned banknote: %d\r\n"), denomination);
	currency_t deposit;
	deposit.base = denomination;
	deposit.cents = 0;
	bank_deposit(&main_global.bank, deposit);
}

static void main_bill_error(bill_error_t error, uint16_t denomination) {
	PGM_P errstr = PSTR("");
	switch (error) {
		case BILL_ERROR_INTERNAL:
			errstr = PSTR("Internal error");
			break;
		case BILL_ERROR_SCAN:
			errstr = PSTR("Scan error, fake banknote, or jam");
			break;
		case BILL_ERROR_STACK:
			errstr = PSTR("Stacker error");
			break;
		case BILL_ERROR_FULL:
			errstr = PSTR("Holder full");
			break;
		case BILL_ERROR_UNKNOWN:
			errstr = PSTR("Unknown banknote");
			break;
	}
	printf_P(PSTR("Banknote scan error: %S\r\n"), errstr);
}

static void main_balance_report(currency_t balance) {
	printf_P(PSTR("Current balance: %d.%d\r\n"), balance.base, balance.cents);
}

static void main_coin_report(currency_t denomination) {
	printf_P(PSTR("Scanned coin: %d.%d\r\n"), denomination.base, denomination.cents);
	bank_deposit(&main_global.bank, denomination);
}

bank_t *main_get_bank(void) {
	return &main_global.bank;
}

int main(void) {
	// System initialisation
	main_global.memory = memory_init(main_global.pool, sizeof(main_global.pool), sizeof(main_event_t));
	callout_mgr_init(&main_global.manager, main_time);
	main_global.time = 0;
	
	// Initialize timers
	timer_init();
	// System timer
	timer2_register_OV_intr(main_systick);
	
	// Driver initialisation
	led_init(&main_global.manager);
	bill_init(&main_global.manager, main_bill_report, main_bill_error);
	coin_init(&main_global.manager, main_coin_report);
	
	// I/O layer initialisation
	console_init(&main_global.manager, "$ ");
	
	// Balance manager initialisation
	bank_init(&main_global.bank, main_balance_report);
	
	// Turn the third LED on
	led_action(LED_C, LED_EVENT_TYPE_ON);
	// Make the second LED blink once per second
	led_blink(LED_B, 15625, 15625, true);
	
	// Set idle sleep mode
	set_sleep_mode(SLEEP_MODE_IDLE);
	
	// Enable interrupts
	sei();
	
	// Start timers
	timer2_start();
	// Start real time clock
	clock_start();
	
	main_global.running = true;
	while (main_global.running) {
		// Halt CPU and wait for the next interrupt
		sleep_mode();
	}
	
	// System shutdown
	cli();
	bank_shutdown(&main_global.bank);
	coin_shutdown();
	bill_shutdown();
	led_shutdown(true);
	console_shutdown();
	
	// Perform a software reset by enabling the watchdog at its shortest setting, then go to sleep
	wdt_enable(WDTO_15MS);
	sleep_mode();
	while (1);
}
