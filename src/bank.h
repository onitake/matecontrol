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
 * Balance change event.
 * 
 * This handler will be called directly, not via the event queue.
 * 
 * @param balance the current balance
 */
typedef void (bank_balance_cb)(currency_t balance);

/**
 * Bank state structure
 */
typedef struct {
	/** The current balance */
	currency_t balance;
	/** The balance change event handler */
	bank_balance_cb *report;
} bank_t;

/**
 * Add two (signed) monetary quantities, saturating to 32767.99 on overflow and
 * -32768.99 on underflow.
 * @return a + b
 */
currency_t currency_add(currency_t a, currency_t b);
/**
 * Subtract two (signed) monetary quantities, saturating to 32767.99 on overflow and
 * -32768.99 on underflow.
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
 * Initialise a balance and account manager.
 * @param report a function to call when the balance changes (may be NULL)
 * @param bank the balance manager
 * @return true, if initialisation was successful
 */
bool bank_init(bank_t *bank, bank_balance_cb *report);

/**
 * Shut a balance manager down.
 * @param bank the balance manager
 */
void bank_shutdown(bank_t *bank);

/**
 * Get the current account balance
 * @param bank the balance manager to get data from
 */
currency_t bank_get_balance(bank_t *bank);
/**
 * Set the current account balance
 * @param bank the balance manager to access
 * @param balance the balance to set
 */
void bank_set_balance(bank_t *bank, currency_t balance);
/**
 * (Atomically) add to the balance
 * 
 * Warning: Exceeding the balance limit will set the balance to its maximum value
 * @param bank the balance manager to access
 * @param amount the amount of credits to add
 */
void bank_deposit(bank_t *bank, currency_t amount);
/**
 * (Atomically) subtract from the balance
 * 
 * Warning: Exceeding the balance limit will set the balance to its minimum value
 * @param bank the balance manager to access
 * @param amount the amount of credits to subtract
 */
void bank_withdraw(bank_t *bank, currency_t amount);

#endif /*_BANK_H*/
