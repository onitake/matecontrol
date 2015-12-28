/**
 * @file coin.c
 * @brief Coin acceptor interface driver implementation
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

#include <stdio.h>
#include <avr/io.h>
#include "coin.h"

/**
 * Driver state structure
 */
typedef struct {
	/** Accept callback */
	coin_report_cb *report;
} coin_t;

/**
 * Global driver state
 */
static coin_t coin_global __attribute__((section(".noinit")));

bool coin_init(coin_report_cb *report) {
	coin_global.report = report;
	return true;
}

void coin_shutdown(void) {
	// Nothing
}
