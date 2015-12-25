/**
 * @file bank.h
 * @brief Balance and account manager
 * 
 * @par Configurable options
 * 
 * None
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

#ifndef _BANK_H
#define _BANK_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Fixed point currency type.
 * 
 * The total range is limited to [-32768.99,32.767.99], precision is one
 * 100th of the base currency.
 * 
 * For display purposes, the structure members may be used directly, but
 * to handle rollover correctly, use currency_add() and currency_sub()
 * for adding and subtracting quantities, respectively.
 */
typedef struct {
	/** Base coinage, 16 bits, range = [-32768,32767] */
	int16_t base;
	/** 100ths of the base coinage, 8 bits, range = [0,99] */
	uint8_t cents;
} currency_t;

/**
 * Add two monetary quantities, saturating to 32767.99 if overflow occurs.
 * @return a + b
 */
currency_t currency_add(currency_t a, currency_t b);
/**
 * Subtract two monetary quantities, saturating to -32768.99 if underflow occurs.
 * @return a - b
 */
currency_t currency_sub(currency_t a, currency_t b);
/**
 * Get the base credit (100 cent units)
 */
int16_t currency_base(currency_t c);
/**
 * Get the unsigned cent fraction
 */
uint8_t currency_cents(currency_t c);

/**
 * Balance change event.
 * 
 * This handler will be called directly, not via the event queue.
 * 
 * @param balance the current balance
 */
typedef void (bank_balance_cb)(currency_t balance);

/**
 * Initialise the (global) balance and account manager.
 * @param report a function to call when the balance changes (may be NULL)
 * @return true, if initialisation was successful
 */
bool bank_init(bank_balance_cb *report);

/**
 * Shut the balance manager down.
 */
void bank_shutdown(void);

/**
 * Get the current account balance
 */
currency_t bank_get_balance(void);
/**
 * Set the current account balance
 */
void bank_set_balance(currency_t balance);
/**
 * (Atomically) add to the balance
 * 
 * Warning: Exceeding the balance limit will set the balance to its maximum value
 */
void bank_deposit(currency_t amount);
/**
 * (Atomically) subtract from the balance
 * 
 * Warning: Exceeding the balance limit will set the balance to its minimum value
 */
void bank_withdraw(currency_t amount);

#endif /*_BANK_H*/
