/**
 * @file console.c
 * @brief UART console driver implementation
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

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <aversive/irq_lock.h>
#include <comm/uart/uart.h>
#include <ihm/rdline/rdline.h>
#include "led.h"
#include "memory.h"
#include "bill.h"
#include "main.h"
#include "bank.h"

/** I/O event type */
typedef enum {
	CONSOLE_EVENT_READ,
	CONSOLE_EVENT_WRITE,
	CONSOLE_EVENT_CLOSE,
} console_event_e;

/** I/O event object */
typedef struct {
	/** Event data (self-referenced) */
	struct callout co;
	/** Event type (direction) */
	console_event_e type;
	/** Payload */
	char character;
} console_event_t;

/** Console driver state object */
typedef struct {
	/** Event queue */
	struct callout_mgr *manager;
	/** Command prompt */
	char prompt[RDLINE_PROMPT_SIZE];
	/** Standard input, output and error */
	FILE stdinout;
	/** Readline state */
	struct rdline rdline;
	/** Memory manager for the event queue */
	memory_t *memory;
	/** Managed memory pool */
	uint8_t pool[MEMORY_POOL_SIZE(CONSOLE_QUEUE_SIZE, sizeof(console_event_t))];
} console_t;

/**
 * Validate callback prototype
 * @param buf a 0-terminated string containing the command buffer
 * @param size the length of the command buffer
 */
typedef void (validate_t)(const char *buf, uint8_t size);

/** Data structure describing a command, its arguments, help text and a handler */
typedef struct {
	/** The command string */
	const char *command;
	/** A help text */
	const char *help;
	/** Pointer to a handler callback */
	validate_t *validate;
} command_t;

static int console_putc(char character, FILE *fp);
static int console_getc(FILE *fp);
static void console_read(char character);
static void console_write(char character);
static void console_callback(struct callout_mgr *cm, struct callout *tim, void *arg);
static void console_validate(const char *buf, uint8_t size);
static int8_t console_complete(const char *buf, char *dstbuf, uint8_t dstsize, int16_t *state);
/**
 * Find the first occurence of whitespace in buf.
 * @param buf a string
 * @param maxlen the string length or -1 to scan until '\0'
 * @return the index of the first whitespace character in buf
 * ('\r', '\n', ' ', '\t'), or the length of the string none was found
 */
static size_t console_whitespace(const char *buf, int16_t maxlen);
/**
 * Tokenize a string by whitespace.
 * @param buf a string
 * @param maxlen the string length or -1 to scan until '\0'
 * @param arraylen the size of tokens and lengths (in elements)
 * @param tokens an array of string pointers, used for storing the token pointers
 * @param lengths an array of string lengths, used for storing the token lengths
 * @return the number of tokens found
 */
static size_t console_tokenize(const char *buf, int16_t maxlen, size_t arraylen, const char **tokens, size_t *lengths);
/**
 * Parse a fixed-point signed decimal number with 16 bits of precision to the
 * left of the decimal point and two decimal digits to the right.
 * 
 * May return 32767.99 on overflow or -32768.99 on underflow.
 * @param buf a string
 * @param maxlen the string length
 * @param left a pointer to storage for the left hand side of the decimal point
 * @param right a pointer to storage for the decimal digits
 * @return the number of processed characters
 */
static size_t console_decimal24(const char *buf, int16_t maxlen, int16_t *left, uint8_t *right);
static uint8_t gpio_pins(char port);
static bool gpio_pin(char port, uint8_t pin);
static void gpio_port(char port, uint8_t pin, bool state);
static void gpio_ddr(char port, uint8_t pin, bool state);

static void console_validate_help(const char *buf, uint8_t size);
static void console_validate_gpio(const char *buf, uint8_t size);
static void console_validate_led(const char *buf, uint8_t size);
static void console_validate_exit(const char *buf, uint8_t size);
static void console_validate_bill(const char *buf, uint8_t size);
static void console_validate_reboot(const char *buf, uint8_t size);
static void console_validate_balance(const char *buf, uint8_t size);
static void console_validate_coin(const char *buf, uint8_t size);

/** @cond DOXYGEN_IGNORE */
static const char MESSAGE_LOGIN[] PROGMEM = "\r\nPress return to open session\r\n";
static const char MESSAGE_WELCOME[] PROGMEM = "Matemat Controller (c) 2015 Chaostreff Basel\r\n";
static const char COMMAND_NAME_BILL[] PROGMEM = "bill";
static const char COMMAND_NAME_HELP[] PROGMEM = "help";
static const char COMMAND_NAME_GPIO[] PROGMEM = "gpio";
static const char COMMAND_NAME_LED[] PROGMEM = "led";
static const char COMMAND_NAME_EXIT[] PROGMEM = "exit";
static const char COMMAND_NAME_REBOOT[] PROGMEM = "reboot";
static const char COMMAND_NAME_BALANCE[] PROGMEM = "balance";
static const char COMMAND_NAME_COIN[] PROGMEM = "coin";
static const char COMMAND_HELP_HELP[] PROGMEM = "Matemat Controller (c) 2015 Chaostreff Basel\r\n\r\nCommands:\r\nhelp\r\ngpio\r\nled\r\nexit\r\nbill\r\nbalance\r\nreboot\r\n";
static const char COMMAND_HELP_GPIO[] PROGMEM = "Usage: gpio [A-G] [0-7] [in, out, on, off]\r\nConfigures (in/out), sets the logic level (on/off) or displays the port status (only port name and optionally bit #) of a GPIO port\r\n";
static const char COMMAND_HELP_LED[] PROGMEM = "Usage: led [A,B,C] [on, off, toggle]\r\nSets the status of LED A, B or C\r\n";
static const char COMMAND_HELP_EXIT[] PROGMEM = "Ends the terminal session\r\n";
static const char COMMAND_HELP_BILL[] PROGMEM = "Usage: bill [inhibit, accept, escrow, direct]\r\nChecks the state of the banknote scanner (no arguments),\r\ninhibits/enables reception or enables/disables escrow mode\r\n";
static const char COMMAND_HELP_REBOOT[] PROGMEM = "Usage: reboot\r\n";
static const char COMMAND_HELP_BALANCE[] PROGMEM = "Usage: balance [0.00]\r\nDisplays the current balance or sets it\r\n";
static const char COMMAND_HELP_COIN[] PROGMEM = "Usage: coin\r\nDisplays the state of the coin acceptor\r\n";
/** @endcond */

/* Sorted lexicographically by command */
static const command_t COMMANDS[] PROGMEM = {
	{ COMMAND_NAME_BALANCE, COMMAND_HELP_BALANCE, console_validate_balance },
	{ COMMAND_NAME_BILL, COMMAND_HELP_BILL, console_validate_bill },
	{ COMMAND_NAME_COIN, COMMAND_HELP_COIN, console_validate_coin },
	{ COMMAND_NAME_EXIT, COMMAND_HELP_EXIT, console_validate_exit },
	{ COMMAND_NAME_HELP, COMMAND_HELP_HELP, console_validate_help },
	{ COMMAND_NAME_GPIO, COMMAND_HELP_GPIO, console_validate_gpio },
	{ COMMAND_NAME_LED, COMMAND_HELP_LED, console_validate_led },
	{ COMMAND_NAME_REBOOT, COMMAND_HELP_REBOOT, console_validate_reboot },
};

static console_t console_global  __attribute__((section (".noinit")));

bool console_init(struct callout_mgr *manager, const char *prompt) {
	console_global.memory = memory_init(console_global.pool, sizeof(console_global.pool), sizeof(console_event_t));
	if (console_global.memory) {
		console_global.manager = manager;
		
		strncpy(console_global.prompt, prompt, sizeof(console_global.prompt));
		
		uart_setconf(CONSOLE_UART, NULL);
		
		fdev_setup_stream(&console_global.stdinout, console_putc, console_getc, _FDEV_SETUP_RW);
		stdin = &console_global.stdinout;
		stdout = &console_global.stdinout;
		stderr = &console_global.stdinout;
		
		// Welcome message
		printf_P(MESSAGE_WELCOME);
		
		uart_register_rx_event(CONSOLE_UART, console_read);

		rdline_init(&console_global.rdline, console_write, console_validate, console_complete);
		//rdline_newline(&console_global.rdline, console_global.prompt);
		rdline_stop(&console_global.rdline);
		printf_P(MESSAGE_LOGIN);
		
		return true;
	}
	return false;
}

void console_shutdown(void) {
	rdline_stop(&console_global.rdline);
}

void console_read(char character) {
	uint8_t flags;
	IRQ_LOCK(flags);
	console_event_t *event = (console_event_t *) memory_allocate(console_global.memory);
	IRQ_UNLOCK(flags);
	if (event) {
		event->type = CONSOLE_EVENT_READ;
		event->character = character;
		callout_init(&event->co, console_callback, event, CONSOLE_PRIORITY);
		callout_schedule(console_global.manager, &event->co, 0);
	}
}

int console_putc(char character, FILE *fp) {
	console_write(character);
	return character;
}

int console_getc(FILE *fp) {
	return uart_recv_nowait(CONSOLE_UART);
}

void console_write(char character) {
	//if (character == '\n') uart_send_nowait(CONSOLE_UART, '\r');
	uart_send_nowait(CONSOLE_UART, character);
}

void console_callback(struct callout_mgr *cm, struct callout *tim, void *arg) {
	if (arg) {
		console_event_t *priv = (console_event_t *) arg;
		if (priv->type == CONSOLE_EVENT_READ) {
			if (console_global.rdline.status == RDLINE_RUNNING) {
				int8_t ret = rdline_char_in(&console_global.rdline, priv->character);
				if (ret == 1) {
					// Evaluate again so the prompt isn't shown when the user exits
					if (console_global.rdline.status == RDLINE_RUNNING) {
						rdline_newline(&console_global.rdline, console_global.prompt);
					}
				} else if (ret == -2) {
					rdline_stop(&console_global.rdline);
					printf_P(MESSAGE_LOGIN);
				}
			} else {
				if (priv->character == '\r' || priv->character == '\n') {
					printf_P(MESSAGE_WELCOME);
					printf_P(PSTR("# of commands: %u\r\n"), sizeof(COMMANDS) / sizeof(COMMANDS[0]));
					rdline_restart(&console_global.rdline);
					rdline_newline(&console_global.rdline, console_global.prompt);
				}
			}
		} else if (priv->type == CONSOLE_EVENT_CLOSE) {
			rdline_stop(&console_global.rdline);
			printf_P(MESSAGE_LOGIN);
		}
		memory_release(arg);
	}
}

size_t console_whitespace(const char *buf, int16_t maxlen) {
	// Similar to strcspn, but with a string length argument
	size_t ws;
	for (ws = 0; buf[ws] && (maxlen < 0 || ws < maxlen); ws++) {
		if (buf[ws] == '\r' || buf[ws] == '\n' || buf[ws] == ' ' || buf[ws] == '\t') {
			break;
		}
	}
	return ws;
}

size_t console_tokenize(const char *buf, int16_t maxlen, size_t arraylen, const char **tokens, size_t *lengths) {
	size_t i;
	for (i = 0; *buf && maxlen != 0; i++) {
		tokens[i] = buf;
		size_t ws = console_whitespace(buf, maxlen);
		if (ws < maxlen) {
			maxlen -= ws + 1;
			buf += ws + 1;
			lengths[i] = ws;
		} else {
			maxlen = 0;
			buf += ws;
			lengths[i] = ws;
		}
	}
	return i;
}

size_t console_decimal24(const char *buf, int16_t maxlen, int16_t *left, uint8_t *right) {
	if (buf && maxlen > 0) {
		size_t i = 0;
		bool negative = false;
		if (maxlen > i) {
			if (buf[i] == '-') {
				negative = true;
				i++;
			}
			if (buf[i] == '+') {
				i++;
			}
		}
		int16_t v = 0;
		while (maxlen > i && buf[i] != '.') {
			if (buf[i] >= '0' && buf[i] <= '9') {
				uint8_t digit = buf[i] - '0';
				if (v >= 3276) {
					if (negative) {
						if (digit > 7) {
							// Overflow, stop parsing
							*left = 32767;
							*right = 99;
							return i + 1;
						}
					} else {
						if (digit > 8) {
							// Underflow, stop parsing
							*left = -32768;
							*right = 99;
							return i + 1;
						}
					}
				}
				v *= 10;
				i++;
			}
		}
		uint8_t d = 0;
		if (maxlen > i + 1 && buf[i] == '.') {
			i++;
			if (buf[i] >= '0' && buf[i] <= '9') {
				d = (buf[i] - '0') * 10;
				i++;
				if (maxlen > i && buf[i] == '.') {
					if (buf[i] >= '0' && buf[i] <= '9') {
						d += buf[i] - '0';
						i++;
					}
				}
			}
		}
		if (negative) {
			*left = -v;
		} else {
			*left = v;
		}
		*right = d;
		return i;
	}
	*left = 0;
	*right = 0;
	return 0;
}

void console_validate(const char *buf, uint8_t size) {
	//printf_P(PSTR("Validating '%s'\n"), buf);
	size_t ws = console_whitespace(buf, size);
	//printf_P(PSTR("Whitespace at %d\r\n"), ws);
	if (ws > 0) {
		// TODO Do a binary search instead of a linear search
		size_t i;
		for (i = 0; i < sizeof(COMMANDS) / sizeof(COMMANDS[0]); i++) {
			PGM_P command = (PGM_P) pgm_read_ptr(&COMMANDS[i].command);
			//printf_P(PSTR("Comparing %u characters of '%s' against '%S'\r\n"), ws, buf, command);
			if (strncmp_P(buf, command, ws) == 0) {
				//printf_P(PSTR("Found command %S\r\n"), command);
				validate_t *validate = (validate_t *) pgm_read_ptr(&COMMANDS[i].validate);
				validate(buf, size);
				//rdline_add_history(&console_global.rdline, buf);
				return;
			}
		}
	}
}

void console_validate_help(const char *buf, uint8_t size) {
	const char *arguments[2];
	size_t lengths[2];
	size_t count = console_tokenize(buf, size, 2, arguments, lengths);
	if (count == 1) {
		printf_P(COMMAND_HELP_HELP);
	} else if (count == 2) {
		// TODO Do a binary search instead of a linear search
		size_t i;
		for (i = 0; i < sizeof(COMMANDS) / sizeof(COMMANDS[0]); i++) {
			PGM_P command = (PGM_P) pgm_read_ptr(&COMMANDS[i].command);
			if (strncmp_P(arguments[1], command, lengths[1]) == 0) {
				printf_P((PGM_P) pgm_read_ptr(&COMMANDS[i].help));
			}
		}
	}
}

uint8_t gpio_pins(char port) {
	switch (port) {
		case 'A':
			return PINA;
		case 'B':
			return PINB;
		case 'C':
			return PINC;
		case 'D':
			return PIND;
		case 'E':
			return PINE;
		case 'F':
			return PINF;
		case 'G':
			return PING;
	}
	return 0;
}

bool gpio_pin(char port, uint8_t pin) {
	return (gpio_pins(port) & _BV(pin)) != 0;
}

void gpio_port(char port, uint8_t pin, bool state) {
	switch (port) {
		case 'A':
			if (state) PORTA |= _BV(pin); else PORTA &= _BV(pin);
			break;
		case 'B':
			if (state) PORTB |= _BV(pin); else PORTB &= _BV(pin);
			break;
		case 'C':
			if (state) PORTC |= _BV(pin); else PORTC &= _BV(pin);
			break;
		case 'D':
			if (state) PORTD |= _BV(pin); else PORTD &= _BV(pin);
			break;
		case 'E':
			if (state) PORTE |= _BV(pin); else PORTE &= _BV(pin);
			break;
		case 'F':
			if (state) PORTF |= _BV(pin); else PORTF &= _BV(pin);
			break;
		case 'G':
			if (state) PORTG |= _BV(pin); else PORTG &= _BV(pin);
			break;
	}
}

void gpio_ddr(char port, uint8_t pin, bool state) {
	switch (port) {
		case 'A':
			if (state) DDRA |= _BV(pin); else DDRA &= _BV(pin);
			break;
		case 'B':
			if (state) DDRB |= _BV(pin); else DDRB &= _BV(pin);
			break;
		case 'C':
			if (state) DDRC |= _BV(pin); else DDRC &= _BV(pin);
			break;
		case 'D':
			if (state) DDRD |= _BV(pin); else DDRD &= _BV(pin);
			break;
		case 'E':
			if (state) DDRE |= _BV(pin); else DDRE &= _BV(pin);
			break;
		case 'F':
			if (state) DDRF |= _BV(pin); else DDRF &= _BV(pin);
			break;
		case 'G':
			if (state) DDRG |= _BV(pin); else DDRG &= _BV(pin);
			break;
	}
}

void console_validate_gpio(const char *buf, uint8_t size) {
	const char *arguments[4];
	size_t lengths[4];
	size_t count = console_tokenize(buf, size, 4, arguments, lengths);
	if (count == 2) {
		if (((arguments[1][0] >= 'a' && arguments[1][0] <= 'g') || (arguments[1][0] >= 'A' && arguments[1][0] <= 'G')) && lengths[1] == 1) {
			char port;
			if (arguments[1][0] >= 'a' && arguments[1][0] <= 'g') {
				port = arguments[1][0] - 'a' + 'A';
			} else {
				port = arguments[1][0];
			}
			uint8_t pins = gpio_pins(port);
			char portb[9];
			uint8_t i;
			for (i = 0; i < 8; i++) {
				portb[i] = (pins & (0x80 >> i)) ? '1' : '0';
			}
			portb[8] = '\0';
			printf_P(PSTR("Status of GPIO port %c is %s\r\n"), port, portb);
		}
	} else if (count >= 3) {
		if (arguments[2][0] >= '0' && arguments[2][0] <= '7' && lengths[2] == 1) {
			char pin = arguments[2][0] - '0';
			if (((arguments[1][0] >= 'a' && arguments[1][0] <= 'g') || (arguments[1][0] >= 'A' && arguments[1][0] <= 'G')) && lengths[1] == 1) {
				char port;
				if (arguments[1][0] >= 'a' && arguments[1][0] <= 'g') {
					port = arguments[1][0] - 'a' + 'A';
				} else {
					port = arguments[1][0];
				}
				if (count == 3) {
					printf_P(PSTR("Status of GPIO pin %c%u is %S\r\n"), port, pin, gpio_pin(port, pin) ? PSTR("high") : PSTR("low"));
				} else {
					uint8_t flags;
					if (strncasecmp_P(arguments[3], PSTR("on"), lengths[3]) == 0) {
						printf_P(PSTR("Turning GPIO pin %c%u on\r\n"), port, pin);
						IRQ_LOCK(flags);
						gpio_port(port, pin, true);
						IRQ_UNLOCK(flags);
					} else if (strncasecmp_P(arguments[3], PSTR("off"), lengths[3]) == 0) {
						printf_P(PSTR("Turning GPIO pin %c%u off\r\n"), port, pin);
						IRQ_LOCK(flags);
						gpio_port(port, pin, false);
						IRQ_UNLOCK(flags);
					} else if (strncasecmp_P(arguments[3], PSTR("in"), lengths[3]) == 0) {
						printf_P(PSTR("Setting GPIO pin %c%u direction to input\r\n"), port, pin);
						IRQ_LOCK(flags);
						gpio_ddr(port, pin, false);
						IRQ_UNLOCK(flags);
					} else if (strncasecmp_P(arguments[3], PSTR("out"), lengths[3]) == 0) {
						printf_P(PSTR("Setting GPIO pin %c%u direction to output\r\n"), port, pin);
						IRQ_LOCK(flags);
						gpio_ddr(port, pin, true);
						IRQ_UNLOCK(flags);
					}
				}
			}
		}
	}
}

void console_validate_led(const char *buf, uint8_t size) {
	const char *arguments[3];
	size_t lengths[3];
	size_t count = console_tokenize(buf, size, 3, arguments, lengths);
	if (count == 3) {
		printf_P(PSTR("Turning LED "));
		led_name_e led;
		if (strncasecmp_P(arguments[1], PSTR("A"), lengths[1]) == 0) {
			printf_P(PSTR("A "));
			led = LED_A;
		} else if (strncasecmp_P(arguments[1], PSTR("B"), lengths[1]) == 0) {
			printf_P(PSTR("B "));
			led = LED_B;
		} else if (strncasecmp_P(arguments[1], PSTR("C"), lengths[1]) == 0) {
			printf_P(PSTR("C "));
			led = LED_C;
		} else {
			printf_P(PSTR("oops\r\n"));
			return;
		}
		led_event_type_e action;
		if (strncasecmp_P(arguments[2], PSTR("on"), lengths[2]) == 0) {
			printf_P(PSTR("on\r\n"));
			action = LED_EVENT_TYPE_ON;
		} else if (strncasecmp_P(arguments[2], PSTR("off"), lengths[2]) == 0) {
			printf_P(PSTR("off\r\n"));
			action = LED_EVENT_TYPE_OFF;
		} else if (strncasecmp_P(arguments[2], PSTR("toggle"), lengths[2]) == 0) {
			printf_P(PSTR("around\r\n"));
			action = LED_EVENT_TYPE_TOGGLE;
		} else {
			printf_P(PSTR("oops\r\n"));
			return;
		}
		led_action(led, action);
	}
}

void console_validate_bill(const char *buf, uint8_t size) {
	const char *arguments[2];
	size_t lengths[2];
	size_t count = console_tokenize(buf, size, 2, arguments, lengths);
	if (count == 1) {
		printf_P(PSTR("Banknote scanner state: "));
		bill_state_t state = bill_state();
		switch (state) {
			case BILL_STATE_UNINITIALIZED:
				printf_P(PSTR("uninitialized"));
				break;
			case BILL_STATE_SELFTEST:
				printf_P(PSTR("self-test"));
				break;
			case BILL_STATE_IDLE:
				printf_P(PSTR("idle"));
				break;
			case BILL_STATE_VALIDATION:
				printf_P(PSTR("validating"));
				break;
			case BILL_STATE_END:
				printf_P(PSTR("ended"));
				break;
			case BILL_STATE_ACCEPT:
				printf_P(PSTR("accepting"));
				break;
			case BILL_STATE_REJECT:
				printf_P(PSTR("rejecting"));
				break;
			case BILL_STATE_SCANNED:
				printf_P(PSTR("scanned"));
				break;
			case BILL_STATE_ERROR:
				printf_P(PSTR("error"));
				break;
		}
		printf_P(PSTR("\r\n"));
	} else {
		if (strncasecmp_P(arguments[1], PSTR("inhibit"), lengths[1]) == 0) {
			printf_P(PSTR("Banknote scanner inhibit is on\r\n"));
			bill_inhibit(true);
		} else if (strncasecmp_P(arguments[1], PSTR("accept"), lengths[1]) == 0) {
			printf_P(PSTR("Banknote scanner inhibit is off\r\n"));
			bill_inhibit(false);
		} else if (strncasecmp_P(arguments[1], PSTR("escrow"), lengths[1]) == 0) {
			printf_P(PSTR("Banknote scanner escrow mode is on\r\n"));
			bill_escrow(true);
		} else if (strncasecmp_P(arguments[1], PSTR("direct"), lengths[1]) == 0) {
			printf_P(PSTR("Banknote scanner escrow mode is off\r\n"));
			bill_escrow(false);
		} else {
			printf_P(PSTR("oops\r\n"));
			return;
		}
	}
}

void console_validate_coin(const char *buf, uint8_t size) {
	printf_P(PSTR("Coin acceptor is (unknown)\r\n"));
}

void console_validate_exit(const char *buf, uint8_t size) {
	rdline_stop(&console_global.rdline);
	printf_P(MESSAGE_LOGIN);
}

void console_validate_reboot(const char *buf, uint8_t size) {
	main_shutdown();
}

void console_validate_balance(const char *buf, uint8_t size) {
	const char *arguments[2];
	size_t lengths[2];
	size_t count = console_tokenize(buf, size, 2, arguments, lengths);
	if (count == 2) {
		currency_t balance;
		console_decimal24(arguments[1], lengths[1], &balance.base, &balance.cents);
		bank_set_balance(main_get_bank(), balance);
	} else {
		currency_t balance = bank_get_balance(main_get_bank());
		printf_P(PSTR("Current balance: %d.%d\r\n"), balance.base, balance.cents);
	}
}

int8_t console_complete(const char *buf, char *dstbuf, uint8_t dstsize, int16_t *state) {
	//printf_P(PSTR("Completing '%s'\n"), buf);
	size_t ws = console_whitespace(buf, -1);
	// TODO Do a binary search instead of a linear search
	size_t i;
	for (i = *state; i < sizeof(COMMANDS) / sizeof(COMMANDS[0]); i++) {
		PGM_P command = (PGM_P) pgm_read_ptr(&COMMANDS[i].command);
		if (strncmp_P(buf, command, ws) == 0) {
			strncpy_P(dstbuf, command + ws, dstsize);
			/*if (i + 1 < sizeof(COMMANDS) / sizeof(COMMANDS[0])) {
				PGM_P command2 = (PGM_P) pgm_read_ptr(&COMMANDS[i + 1].command);
				if (strncmp_P(buf, command2, ws) == 0) {
					*state = i;
					return 1;
				}
			}*/
			return 2;
		}
	}
	return 0;
}
