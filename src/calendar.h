/**
 * @file calendar.h
 * @brief Calendar and real time clock API
 * 
 * This module implements a calendar, suitable for representing timestamps,
 * Gregorian calendar dates, but also as a real time clock. The clock
 * may be updated from a timer event or an external clock source, through one
 * of the `calendar_add_*` methods. The implementation is not thread or
 * interrupt safe, external locking is recommended if used as a realtime clock.
 * 
 * Timestamps are stored as a second-nanosecond pair, with configurable
 * data types. uint32_t is used by default.
 * 
 * Use these configuration macros to customise:
 * 
 * Macro                   | Default  | Values         | Description
 * ------------------------|----------|----------------|-----------------------------------------------
 * CALENDAR_SIZE_SECONDS   | 32       | 32, 64         | Number of bits for representing seconds
 * CALENDAR_SIZE_NANOS     | 32       | 0, 32          | Number of bits for representing nanoseconds
 * CALENDAR_DATE_REFERENCE | 19700101 | [any date]     | Reference date to use for timestamp [0,0]
 * CALENDAR_COMPATIBILITY  | [undef]  | [undef], [def] | If defined, enable base/time API compatibility
 * 
 * The reference date is the earliest representable date. You may use
 * `calendar_init()` to initialise a date object to any later reference
 * date, at your convenience. This is also helpful when you need to convert
 * between different epochs.
 *
 * Possible reference dates are:
 *
 * Setting  | Enum                      | Date
 * ---------|---------------------------|-----------------------------------------------
 * -        | CALENDAR_DATEREF_NATIVE   | Same as the reference date set at compile time
 * 00000101 | CALENDAR_DATEREF_ZERO     | Jan 1 0 00:00:00.0
 * 16010101 | CALENDAR_DATEREF_WIN32    | Jan 1 1601 00:00:00.0
 * 19000101 | CALENDAR_DATEREF_NINETEEN | Jan 1 1900 00:00:00.0
 * 19700101 | CALENDAR_DATEREF_UNIX     | Jan 1 1970 00:00:00.0
 * 20000101 | CALENDAR_DATEREF_AVR      | Jan 1 2000 00:00:00.0
 * 
 * @cond DOXYGEN_IGNORE
 * 
 * This driver supports several modes of operation. It can automatically
 * configure and update the avr-libc system timer for you, or it can be used
 * to drive an internal 64bit timer. The clock source is also configurable.
 * You may use predefined hardware timer configurations, or drive the update
 * from your own clock source routine. The update frequency should be
 * precisely 1 Hz. If using a timer interrupt, both update on overflow or
 * on compare match are possible. In compare match mode, precise calibration
 * is possible. The prescaler value will be configured as well.
 * 
 * You should not use the timer configured here for any other purposes.
 * If you use hardware/timer, the respective timer should be disabled there,
 * or you should update the clock through `clock_tick()`.
 * 
 * Notes:
 * 
 * - Using the overflow interrupt results in an additional prescaler of
 *   TIMER_MAX+1 (256 or 65536)
 * - Setting the prescaler has currently no effect if `CLOCK_SOURCE_EXT` is used
 * 
 * The driver is configurable through a series of preprocessor macros:
 * 
 * Macro              | Default  | Values             | Description
 * -------------------|----------|--------------------|-----------------------------------------------------------------------
 * CLOCK_USE_TIMER    | [undef]  | [undef], 0..3      | Use TIMERn as the clock tick reference or none at all
 * CLOCK_PRESCALER    | 1        | [depends on timer] | Use the default 1 to allow maximum precision
 * CLOCK_SOURCE_EXT   | [undef]  | [undef], [def]     | Use an external clock reference (only some timers)
 * CLOCK_INT_COMPARE  | [undef]  | [undef], [def]     | Use the compare match interrupt (instead of overlow) as the tick event
 * CLOCK_COMP_VALUE   | [undef]  | [undef], 0-65535   | Set the compare register value
 * CLOCK_ISO_TIME     | [undef]  | [undef], [def]     | Enable support for `<time.h>` (requires avr-libc 1.81)
 * 
 * @bug We should use the EEPROM and/or an external RTC controller to allow
 * periodic and startup updates of the system clock, and to store timezone/DST
 * info. The user must do that with `clock_set()` and/or the `<time.h>` API
 * for now. If `CLOCK_USE_TIMER` is not set, the API is not thread safe.
 * 
 * @bug There is no support for timezones or other calendars besides the
 * Gregorian one.
 * 
 * @endcond DOXYGEN_IGNORE
 * 
 * @author Gregor Riepl <onitake@gmail.com>
 * 
 * @copyright © 2015 Chaostreff Basel
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

#ifndef _CALENDAR_H
#define _CALENDAR_H

#include <stdint.h>
#include <stdbool.h>

/** @cond DOXYGEN_IGNORE */

/* Defaults */

#ifndef CALENDAR_SIZE_SECONDS
#define CALENDAR_SIZE_SECONDS 32
#endif
#ifndef CALENDAR_SIZE_NANOS
#define CALENDAR_SIZE_NANOS 32
#endif

#ifndef CALENDAR_DATE_REFERENCE
#define CALENDAR_DATE_REFERENCE 19700101
#endif


/* Macros */

#define CALENDAR_DATE_REFERENCE_YEAR (CALENDAR_DATE_REFERENCE / 10000UL)
#define CALENDAR_DATE_REFERENCE_MONTH (CALENDAR_DATE_REFERENCE / 100UL - CALENDAR_DATE_REFERENCE_YEAR * 100UL)
#define CALENDAR_DATE_REFERENCE_DAY (CALENDAR_DATE_REFERENCE - CALENDAR_DATE_REFERENCE_YEAR * 10000UL - CALENDAR_DATE_REFERENCE_MONTH * 100UL)
#define CALENDAR_DATE_REFERENCE_HOUR 0
#define CALENDAR_DATE_REFERENCE_MINUTE 0
#define CALENDAR_DATE_REFERENCE_SECOND 0
#define CALENDAR_DATE_REFERENCE_NANO 0

/** @endcond DOXYGEN_IGNORE */


/* Types */

/**
 * A date object.
 */
typedef struct {
#if CALENDAR_SIZE_SECONDS == 32
	/** Seconds (32bit) */
	uint32_t seconds;
#elif CALENDAR_SIZE_SECONDS == 64
	/** Seconds (64bit) */
	uint64_t seconds;
#endif
#if CALENDAR_SIZE_NANOS == 32
	/** Nanoseconds (32bit) */
	uint32_t nanos;
#endif
} calendar_t;

/**
 * Reference dates.
 */
typedef enum {
	/** The calendar reference date that was set at compile time */
	CALENDAR_DATEREF_NATIVE,
	/** Jan 1 0 00:00:00.0 */
	CALENDAR_DATEREF_ZERO,
	/** Jan 1 1900 00:00:00.0 */
	CALENDAR_DATEREF_NINETEEN,
	/** Jan 1 1970 00:00:00.0 */
	CALENDAR_DATEREF_UNIX,
	/** Jan 1 1601 00:00:00.0 */
	CALENDAR_DATEREF_WIN32,
	/** Jan 1 2000 00:00:00.0 */
	CALENDAR_DATEREF_AVR,
} calendar_ref_t;

/**
 * Date parts.
 * */
typedef enum {
	/** Nanoseconds */
	CALENDAR_DATEPART_NANOS,
	/** Microseconds */
	CALENDAR_DATEPART_MICROS,
	/** Milliseconds */
	CALENDAR_DATEPART_MILLIS,
	/** Seconds */
	CALENDAR_DATEPART_SECS,
	/** Minutes */
	CALENDAR_DATEPART_MINS,
	/** Hours */
	CALENDAR_DATEPART_HOURS,
	/** Days */
	CALENDAR_DATEPART_DAYS,
	/** Weeks */
	CALENDAR_DATEPART_WEEKS,
	/** Months */
	CALENDAR_DATEPART_MONTHS,
	/** Years */
	CALENDAR_DATEPART_YEARS,
} calendar_tag_t;

/**
 * Date part type
 */
typedef int16_t calendar_inc_t;

/**
 * Initialise a date object.
 * @param calendar a pointer to an uninitialised date object
 * @param reference the date to set (relative to the reference date)
 */
void calendar_init(calendar_t *calendar, calendar_ref_t reference);

/**
 * Copy a date
 * 
 * Result: `a = b`
 * @param a the destination date
 * @param b the source date
 */
void calendar_copy(calendar_t *a, calendar_t *b);

/**
 * Add one date to another.
 * 
 * Result: `a = a + b`
 * @param a the destination date and first addend
 * @param b the second addend
 */
void calendar_add(calendar_t *a, calendar_t *b);

/**
 * Subtract one date from another.
 * 
 * Result: `a = a - b`
 * @param a the destination date and first subtrahend
 * @param b the second subtrahend
 */
void calendar_sub(calendar_t *a, calendar_t *b);

/**
 * Increment part of a date by a certain value.
 * Negative values decrement the date.
 * @param a the date to modify
 * @param tag the part of the date
 * @param value the value to add (or subtract, if negative)
 */
void calendar_inc(calendar_t *a, calendar_tag_t tag, calendar_inc_t value);

#endif /*_CALENDAR_H*/

/* endcond DOXYGEN_IGNORE */
