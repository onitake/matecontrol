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

currency_t currency_add(currency_t a, currency_t b) {
	currency_t ret;
	ret.base = a.base + b.base;
	if (a.base < 0) {
		if (b.base < 0) {
			// Special case, only happens on underflow or carry
			if (ret.base > 0 || (ret.base == -32768 && a.cents + b.cents >= 100)) {
				ret.base = -32768;
				ret.cents = 99;
			} else {
				ret.cents = a.cents + b.cents;
				if (ret.cents >= 100) {
					ret.cents -= 100;
					ret.base--;
				}
			}
		} else {
			if (b.cents > a.cents) {
				ret.cents = -(b.cents - a.cents) + 100;
				ret.base++;
			} else {
				ret.cents = a.cents - b.cents;
			}
		}
	} else {
		if (b.base < 0) {
			if (b.cents > a.cents) {
				ret.cents = -(b.cents - a.cents) + 100;
				ret.base--;
			} else {
				ret.cents = a.cents - b.cents;
			}
		} else {
			// Special case, only happens on overflow or carry
			if (ret.base < 0 || (ret.base == 32767 && a.cents + b.cents >= 100)) {
				ret.base = 32767;
				ret.cents = 99;
			} else {
				ret.cents = a.cents + b.cents;
				if (ret.cents >= 100) {
					ret.cents -= 100;
					ret.base++;
				}
			}
		}
	}
	return ret;
}

currency_t currency_sub(currency_t a, currency_t b) {
	b.base = -b.base;
	return currency_add(a, b);
}

int16_t currency_base(currency_t c) {
	return c.base;
}

uint8_t currency_cents(currency_t c) {
	return c.cents;
}

bool bank_init(bank_t *bank, bank_balance_cb *report) {
	// TODO Read the balance from EEPROM
	bank->balance.base = 0;
	bank->balance.cents = 0;
	bank->report = report;
	return true;
}

void bank_shutdown(bank_t *bank) {
	// TODO Store the balance to EEPROM
}

currency_t bank_get_balance(bank_t *bank) {
	uint8_t flags;
	IRQ_LOCK(flags);
	currency_t ret = bank->balance;
	IRQ_UNLOCK(flags);
	return ret;
}

void bank_set_balance(bank_t *bank, currency_t balance) {
	uint8_t flags;
	IRQ_LOCK(flags);
	bank->balance = balance;
	IRQ_UNLOCK(flags);
	if (bank->report) {
		bank->report(balance);
	}
}

void bank_deposit(bank_t *bank, currency_t amount) {
	currency_t balance;
	uint8_t flags;
	IRQ_LOCK(flags);
	bank->balance = currency_add(bank->balance, amount);
	balance = bank->balance;
	IRQ_UNLOCK(flags);
	if (bank->report) {
		bank->report(balance);
	}
}

void bank_withdraw(bank_t *bank, currency_t amount) {
	currency_t balance;
	uint8_t flags;
	IRQ_LOCK(flags);
	bank->balance = currency_sub(bank->balance, amount);
	balance = bank->balance;
	IRQ_UNLOCK(flags);
	if (bank->report) {
		bank->report(balance);
	}
}
