/**
 * @file clock.h
 * @brief Real time clock driver
 * 
 * This driver provides basic hardware support to drive the avr-libc
 * time module. If `time.h` is not available, a simple API consisting of
 * `time_t`, `time()`, `difftime()` and `set_system_time()` is provided.
 * In this case, you cannot use the calendar functionality of `time.h`.
 * 
 * `time.h` is available in avr-libc 1.8.1 and later. The version is detected
 * automatically and HAVE_TIME_H is set accordingly.
 * 
 * The driver is currently statically configured to use TIMER1 in Fast PWM
 * mode, with overflow occuring at TOP. TOP is set to
 * `CONFIG_QUARTZ / CLOCK_PRESCALER`, with `CLOCK_PRESCALER` equal to 1024.
 * 
 * With a 16MHz quartz, this should result in a precise 1-second interval.
 * 
 * If you prefer to use the builtin API, even if `time.h` is available, define
 * the preprocessor macro `CLOCK_DISABLE_TIME_H`.
 * 
 * @note You should not use timer 1 for other purposes.
 * 
 * @bug The timer configuration is currently static. Allow more control
 * with external defines.
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

#ifndef _CLOCK_H
#define _CLOCK_H

#include <stdint.h>
#include <avr/version.h>

/* Allow overriding the default */
#ifndef CLOCK_DISABLE_TIME_H
#if __AVR_LIBC_VERSION__ >= 10801UL
/**
 * Defined if the C library has time.h (avr-libc 1.8.1 and later).
 * 
 * May be disabled by defining `CLOCK_DISABLE_TIME_H`.
 */
#define HAVE_TIME_H
#else
#warning time.h not available. Realtime clock is limited to basic API.
#endif
#endif

/**
 * Configure and start the realtime clock.
 * 
 * This should be called after interrupts are enabled.
 */
void clock_start(void);

#ifdef HAVE_TIME_H
/* Include libc time.h */

#include <time.h>

#else
/* Alternative implementation */

/**
 * System time type.
 * Only declared when no time.h is available.
 */
typedef int64_t time_t;
/**
 * Get the current system time.
 * Only declared when no time.h is available.
 */
time_t time(time_t *timer);
/**
 * Calculate the difference between to points of time.
 * Only declared when no time.h is available.
 */
time_t difftime(time_t time1, time_t time0);
/**
 * Set the current system time.
 * Only declared when no time.h is available.
 */
void set_system_time(time_t timestamp);

#endif /*HAVE_TIME_H*/

#endif /*_CLOCK_H*/
