/**
 * @file bill.h
 * @brief Banknote scanner interface driver
 * 
 * To customise denominations and bit patterns, change `BILL_DENOMINATIONS` in
 * bill.c
 * 
 * @par Configurable options
 * 
 * Macro               | Default  | Values         | Description
 * --------------------|----------|----------------|-----------------------------------------------
 * BILL_QUEUE_SIZE     | [undef]  | 0..255         | Size of the event pool
 * BILL_PRIORITY       | [undef]  | 0..127         | Event queue priority
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

#ifndef _BILL_H
#define _BILL_H

#include <stdbool.h>
#include <stdint.h>
#include <base/callout/callout.h>

/**
 * Error codes
 */
typedef enum {
	/** Internal error */
	BILL_ERROR_INTERNAL,
	/** Scan error, fake banknote, or jam */
	BILL_ERROR_SCAN,
	/** Stacker error */
	BILL_ERROR_STACK,
	/** Banknote holder full */
	BILL_ERROR_FULL,
	/** Unknown banknote type */
	BILL_ERROR_UNKNOWN,
} bill_error_t;

/**
 * Scanner state
 */
typedef enum {
	/** Pin state unknown */
	BILL_STATE_UNINITIALIZED,
	/** Self-test at startup */
	BILL_STATE_SELFTEST,
	/** Standby */
	BILL_STATE_IDLE,
	/** Scanner is validation */
	BILL_STATE_VALIDATION,
	/** Validation has ended, or error */
	BILL_STATE_END,
	/** Banknote was validated, but has not been stacked yet */
	BILL_STATE_ACCEPT,
} bill_state_t;


/**
 * Successful scan event handler.
 * 
 * This handler will be called directly, not via the event queue.
 * 
 * In case the bill holder is full after accepting a banknote, the
 * error handler will be called after the report handler.
 * @param denomination the value of the scanned banknote
 */
typedef void (bill_report_cb)(uint16_t denomination);
/**
 * Scan error event handler.
 * 
 * This error handler will be called directly, not via the event queue.
 * @param error an error code
 * @param denomination the value of the banknote being scanned, if applicable
 */
typedef void (bill_error_cb)(bill_error_t error, uint16_t denomination);

/**
 * Initialise the (global) banknote scanner driver.
 * @param manager the callout queue to use for passing events
 * @param report a function to call when a banknote was successfully scanned
 * (may be NULL)
 * @param error a function to call when an error occurs (may be NULL)
 * @return true, if initialisation was successful
 */
bool bill_init(struct callout_mgr *manager, bill_report_cb *report, bill_error_cb *error);

/**
 * Shut the banknote scanner driver down.
 */
void bill_shutdown(void);

/**
 * Enables or disables the banknote scanner
 * @param inhibit true = reject all banknotes, false = enable scanner
 */
void bill_inhibit(bool inhibit);


/**
 * Enables or disables escrow mode (default is off)
 * @param escrow true = validate before accept, false = accept directly
 */
void bill_escrow(bool escrow);

/**
 * Checks the state of the scanner
 * @return the current state of the state machine
 */
bill_state_t bill_state(void);

#endif /*_BILL_H*/
