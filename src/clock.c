/**
 * @file clock.c
 * @brief Real time clock driver
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

#include <avr/interrupt.h>
#include <avr/io.h>
#include <aversive/irq_lock.h>
#include "clock.h"
#include "autoconf.h"

/**
 * Default prescaler (1024)
 */
#define CLOCK_PRESCALER 1024
/**
 * Clock interrupt (timer 1 overflow)
 */
#define CLOCK_ISR TIMER1_OVF_vect

/**
 * Comparator value corresponding to a 1s period
 */
#define RTC_COMPARE_SECOND (CONFIG_QUARTZ / CLOCK_PRESCALER)
#if CONFIG_QUARTZ % CLOCK_PRESCALER != 0
#warning Realtime clock comparator is not integral. Clock will not be precise.
#endif

/** @cond DOXYGEN_IGNORE */
/** @endcond DOXYGEN_IGNORE */

void clock_start(void) {
	/* Initialize time to C library epoch (Jan 1 2000) */
	/* TODO use RTC timer instead */
	set_system_time(0);
	// compare register (= input capture)
	ICR1 = RTC_COMPARE_SECOND;
	// enable overflow interrupt
	TIMSK |= _BV(TOIE1);
	// WGM10:1 = 0b10 (Fast PWM, ICR = overflow), output waveform off
	TCCR1A = _BV(WGM11);
	// WGM12:3 = 0b11 (Fast PWM, ICR = overflow), CS10:2 = 0b101 (1024), noise canceler off, edge select off
	TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10) | _BV(CS12);
}

#ifdef HAVE_TIME_H

/**
 * Timer overflow interrupt
 */
ISR(CLOCK_ISR, ISR_NAKED) {
	system_tick();
	PORTG ^= _BV(PG0);
	reti();
}

#else /*HAVE_TIME_H*/

static time_t clock_global;

/**
 * Timer overflow interrupt
 */
ISR(CLOCK_ISR) {
	clock_global++;
}

time_t time(time_t *timer) {
	uint8_t flags;
	IRQ_LOCK(flags);
	time_t temp = clock_global;
	IRQ_UNLOCK(flags);
	if (timer) *timer = temp;
	return temp;
}

void set_system_time(time_t timestamp) {
	uint8_t flags;
	IRQ_LOCK(flags);
	clock_global = timestamp;
	IRQ_UNLOCK(flags);
}

time_t difftime(time_t time1, time_t time0) {
	return time1 - time0;
}

#endif /*HAVE_TIME_H*/