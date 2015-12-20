/**
 * @file bank.c
 * @brief Balance and account manager
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

#include <aversive/irq_lock.h>
#include "bank.h"

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
 * Global state
 */
static bank_t bank_global __attribute__((section(".noinit")));

currency_t currency_add(currency_t a, currency_t b) {
	currency_t ret;
	ret.base = a.base + b.base;
	if (a.base > 0 && b.base > 0 && ret.base < 0) {
		ret.base = 32767;
		ret.cents = 99;
	} else {
		ret.cents = a.cents + b.cents;
		if (ret.cents >= 100) {
			ret.cents -= 100;
			ret.base++;
		}
	}
	return ret;
}

currency_t currency_sub(currency_t a, currency_t b) {
	currency_t ret;
	ret.base = a.base - b.base;
	if (a.base < 0 && b.base < 0 && ret.base > 0) {
		ret.base = -32768;
		ret.cents = 99;
	} else {
		ret.cents = a.cents + b.cents;
		if (ret.cents >= 100) {
			ret.cents -= 100;
			ret.base++;
		}
	}
	return ret;
}

bool bank_init(bank_balance_cb *report) {
	// TODO Read the balance from EEPROM
	bank_global.balance.base = 0;
	bank_global.balance.cents = 0;
	bank_global.report = report;
	return true;
}

void bank_shutdown(void) {
	// TODO Store the balance to EEPROM
}

currency_t bank_get_balance(void) {
	uint8_t flags;
	IRQ_LOCK(flags);
	currency_t ret = bank_global.balance;
	IRQ_UNLOCK(flags);
	return ret;
}

void bank_set_balance(currency_t balance) {
	uint8_t flags;
	IRQ_LOCK(flags);
	bank_global.balance = balance;
	IRQ_UNLOCK(flags);
	if (bank_global.report) {
		bank_global.report(balance);
	}
}

void bank_deposit(currency_t amount) {
	currency_t balance;
	uint8_t flags;
	IRQ_LOCK(flags);
	bank_global.balance = currency_add(bank_global.balance, amount);
	balance = bank_global.balance;
	IRQ_UNLOCK(flags);
	if (bank_global.report) {
		bank_global.report(balance);
	}
}

void bank_withdraw(currency_t amount) {
	currency_t balance;
	uint8_t flags;
	IRQ_LOCK(flags);
	bank_global.balance = currency_sub(bank_global.balance, amount);
	balance = bank_global.balance;
	IRQ_UNLOCK(flags);
	if (bank_global.report) {
		bank_global.report(balance);
	}
}
