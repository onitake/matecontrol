/**
 * @file coin.h
 * @brief Coin acceptor interface driver
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
#include "bank.h"

/**
 * Successful scan event handler.
 * 
 * This handler will be called directly, not via the event queue.
 * @param denomination the value of the scanned banknote
 */
typedef void (coin_report_cb)(currency_t denomination);

/**
 * Initialise the (global) coin acceptor driver.
 * @param report a function to call when a banknote was successfully scanned
 * (may be NULL)
 * @return true, if initialisation was successful
 */
bool coin_init(coin_report_cb *report);

/**
 * Shut the coin acceptor driver down.
 */
void coin_shutdown(void);

#endif /*_COIN_H*/
