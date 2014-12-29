/**
 * Matemat controller firmware
 * Copyright (C) 2014 Chaostreff Basel
 * 
 * led.h
 * LED port driver interface
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

#include "event.h"

/**
 * LED driver event types
 */
enum led_event_type_e {
	LED_EVENT_TYPE_ON,
	LED_EVENT_TYPE_OFF,
	LED_EVENT_TYPE_TOGGLE,
};

/**
 * LED identifiers
 */
enum led_name_e {
	LED_A,
	LED_B,
	LED_C,
};

/**
 * Initialises the LED driver.
 */
void led_init(void);
/**
 * Shuts the LED driver down.
 * @parm off If true, also turns the LEDs off before shutdown.
 */
void led_shutdown(bool off);

/**
 * Send a LED event.
 * @parm source The module sending the event.
 * @parm led The LED to act upon (LED_A, LED_B or LED_C).
 * @parm action The action to take (LED_EVENT_TYPE_ON, LED_EVENT_TYPE_OFF or LED_EVENT_TYPE_TOGGLE)
 * @return true, if the event was queued successfully.
 */
bool led_send(event_target_t source, uint8_t led, uint8_t action);

/**
 * Handle a LED event.
 * @parm event The event
 * @return true, if the event was handled successfully.
 */
bool led_handle(event_t *event);

#endif /*_LED_H*/
