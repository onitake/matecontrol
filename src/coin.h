/**
 * @file coin.h
 * @brief Coin acceptor interface driver
 * 
 * @par Configurable options
 * 
 * Macro               | Default  | Values         | Description
 * --------------------|----------|----------------|-----------------------------------------------
 * COIN_QUEUE_SIZE     | [undef]  | 0..255         | Size of the event pool
 * COIN_PRIORITY       | [undef]  | 0..127         | Event queue priority
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

#ifndef _COIN_H
#define _COIN_H

#include <stdbool.h>
#include <stdint.h>
#include <base/callout/callout.h>
#include "bank.h"

/**
 * Error codes
 */
typedef enum {
	/** Alarm state */
	COIN_ERROR_ALARM,
} coin_error_t;

/**
 * Coin acceptance event handler.
 * 
 * This handler will be called directly, not via the event queue.
 * @param denomination the value of the coin
 */
typedef void (coin_report_cb)(currency_t denomination);
/**
 * Coin acceptor error event handler.
 * 
 * This error handler will be called directly, not via the event queue.
 * @param error an error code
 */
typedef void (coin_error_cb)(coin_error_t error);

/**
 * Initialise the (global) coin acceptor driver.
 * @param manager the callout queue to use for passing events
 * @param report a function to call when a banknote was successfully scanned
 * (may be NULL)
 * @param error a function to call when an error occurs (may be NULL)
 * @return true, if initialisation was successful
 */
bool coin_init(struct callout_mgr *manager, coin_report_cb *report, coin_error_cb *error);

/**
 * Shut the coin acceptor driver down.
 */
void coin_shutdown(void);

#endif /*_COIN_H*/
