/**
 * @file console.h
 * @brief Serial console
 * 
 * This module implements a simple shell with basic command line editing and
 * autocompletion support. The Aversive rdline module is used for this purpose.
 * 
 * Console output is done through stdio, with asynchronous serial communication.
 * You should set the serial port buffer to the maximum line length.
 * 
 * @par Configurable options
 * 
 * Macro                   | Default  | Values         | Description
 * ------------------------|----------|----------------|-----------------------------------------------
 * CONSOLE_QUEUE_SIZE      | [undef]  | 0..255         | Size of the event pool
 * CONSOLE_PRIORITY        | [undef]  | 0..127         | Event queue priority
 * CONSOLE_UART            | [undef]  | 0..N           | UART port number
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

#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stdbool.h>
#include <base/callout/callout.h>

/**
 * Initialise the (global) UART console driver.
 * @param manager the callout queue to use for passing events
 * @param prompt the command prompt to display
 * @return true, if initialisation was successful
 */
bool console_init(struct callout_mgr *manager, const char *prompt);

/**
 * Shut the consolde driver down.
 */
void console_shutdown(void);

#endif /*_CONSOLE_H*/
