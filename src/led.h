/**
 * @file led.h
 * @brief LED port driver interface
 * 
 * The LED driver supports periodic and one-shot actions. The event queue
 * is used for all of them
 * 
 * led_action() turns a LED on or off, or toggles its state.
 * led_blink() can be used to turn a LED on and off again after a delay,
 * or to make it blink periodically.
 * 
 * @par Configurable options
 * 
 * Macro               | Default  | Values         | Description
 * --------------------|----------|----------------|-----------------------------------------------
 * LED_QUEUE_SIZE      | [undef]  | 0..255         | Size of the event pool
 * LED_PRIORITY        | [undef]  | 0..127         | Event queue priority
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

#ifndef _LED_H
#define _LED_H

#include <stdbool.h>
#include <base/callout/callout.h>

/**
 * LED driver event types
 */
typedef enum {
	/** Turn LED on */
	LED_EVENT_TYPE_ON,
	/** Turn LED off */
	LED_EVENT_TYPE_OFF,
	/** Flip LED state */
	LED_EVENT_TYPE_TOGGLE,
} led_event_type_e;

/**
 * LED identifiers
 */
typedef enum {
	/** LED A */
	LED_A = 0,
	/** LED B */
	LED_B,
	/** LED C */
	LED_C,
	/** Number of LEDs */
	LED_MAX,
} led_name_e;

/**
 * Initialise the (global) LED driver.
 * @param manager the callout queue to use for passing events
 * @return true, if initialisation was successful
 */
bool led_init(struct callout_mgr *manager);

/**
 * Shut the LED driver down.
 * @param off if true, also turns all LEDs off before shutdown
 */
void led_shutdown(bool off);

/**
 * Send a command to the LED driver.
 * @param led the LED to act upon
 * @param action the action to take (LED_EVENT_TYPE_OFF, LED_EVENT_TYPE_ON or
 * LED_EVENT_TYPE_TOGGLE)
 * @return true, if the event was queued successfully.
 */
bool led_action(led_name_e led, led_event_type_e action);

/**
 * Schedule a periodic or one-shot event to make the LED turn off and on
 * @param led the LED to act upon
 * @param ontime the length of the on-time in ticks
 * @param offtime the length of the off-time in ticks
 * @param repeat true, if the blinking should be periodic
 * @return true, if the event was queued successfully.
 */
bool led_blink(led_name_e led, uint16_t ontime, uint16_t offtime, bool repeat);

#endif /*_LED_H*/
