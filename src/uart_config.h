/**
 * @file uart_config.h
 * @brief UART module configuration
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

/** Compile in support for UART0 */
#define UART0_COMPILE
/** Enable UART0 */
#define UART0_ENABLED 1
/** Enable UART0 interrupt handlers */
#define UART0_INTERRUPT_ENABLED 1
/** Set default baud rate */
#define UART0_BAUDRATE 38400
/** Disable double speed mode */
#define UART0_USE_DOUBLE_SPEED 0
/** Set static input buffer size */
#define UART0_RX_FIFO_SIZE 16
/** Set static output buffer size */
#define UART0_TX_FIFO_SIZE 256
/** Set default number of data bits */
#define UART0_NBITS 8
/** Set default parity */
#define UART0_PARITY UART_PARTITY_NONE
/** Set default number of stop bits */
#define UART0_STOP_BIT UART_STOP_BITS_1

/** Compile in support for UART1 */
#define UART1_COMPILE
/** Enable UART1 */
#define UART1_ENABLED 1
/** Enable UART1 interrupt handlers */
#define UART1_INTERRUPT_ENABLED 1
/** Set default baud rate */
#define UART1_BAUDRATE 38400
/** Disable double speed mode */
#define UART1_USE_DOUBLE_SPEED 0
/** Set static input buffer size */
#define UART1_RX_FIFO_SIZE 16
/** Set static output buffer size */
#define UART1_TX_FIFO_SIZE 128
/** Set default number of data bits */
#define UART1_NBITS 8
/** Set default parity */
#define UART1_PARITY UART_PARTITY_NONE
/** Set default number of stop bits */
#define UART1_STOP_BIT UART_STOP_BITS_1
