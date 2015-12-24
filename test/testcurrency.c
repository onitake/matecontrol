#include <stdint.h>
#include <stdio.h>
#include <assert.h>

/**
 * Fixed point currency type.
 * 
 * The total range is limited to [-32768.99,32.767.99], precision is one
 * 100th of the base currency.
 * 
 * For display purposes, the structure members may be used directly, but
 * to handle rollover correctly, use currency_add and currency_sub
 * for adding and subtracting quantities, respectively.
 */
typedef struct {
	/** Base coinage, 16 bits, range = [-32768,32767] */
	int16_t base;
	/** 100ths of the base coinage, 8 bits, range = [0,99] */
	uint8_t cents;
} currency_t;

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

void test_add(int ab, int ac, int bb, int bc, int cb, int cc) {
	currency_t a, b, c;
	a.base = ab;
	a.cents = ac;
	printf("a=%d.%d\n", a.base, a.cents);
	assert(a.base == ab && a.cents == ac);
	b.base = bb;
	b.cents = bc;
	printf("b=%d.%d\n", b.base, b.cents);
	assert(b.base == bb && b.cents == bc);
	c = currency_add(a, b);
	printf("c=%d.%d\n", c.base, c.cents);
	assert(c.base == cb && c.cents == cc);
}

void test_sub(int ab, int ac, int bb, int bc, int cb, int cc) {
	currency_t a, b, c;
	a.base = ab;
	a.cents = ac;
	printf("a=%d.%d\n", a.base, a.cents);
	assert(a.base == ab && a.cents == ac);
	b.base = bb;
	b.cents = bc;
	printf("b=%d.%d\n", b.base, b.cents);
	assert(b.base == bb && b.cents == bc);
	c = currency_sub(a, b);
	printf("c=%d.%d\n", c.base, c.cents);
	assert(c.base == cb && c.cents == cc);
}

int main(int argc, char **argv) {
	test_add(10, 99, 0, 0, 10, 99);
	test_add(0, 0, 1, 20, 1, 20);
	test_add(10, 99, 1, 20, 12, 19);
	test_add(10, 20, 1, 99, 12, 19);
	test_add(-10, 99, 1, 20, -9, 79);
	test_add(-10, 20, 1, 99, -8, 21);
	test_add(10, 99, -1, 20, 9, 79);
	test_add(10, 20, -1, 99, 8, 21);
	test_add(-10, 99, -1, 20, -12, 19);
	test_add(-10, 20, -1, 99, -12, 19);
	test_add(20000, 10, 32000, 1, 32767, 99);
	test_add(32767, 80, 0, 19, 32767, 99);
	test_add(32767, 80, 0, 20, 32767, 99);
	return 0;
}
