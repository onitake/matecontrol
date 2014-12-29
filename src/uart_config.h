/**
 * Matemat controller firmware
 * Copyright (C) 2014 Chaostreff Basel
 * 
 * uart_config.h
 * comm/uart configuration
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

/*
 * UART0 definitions 
 */

/* compile uart0 functions, undefine it to pass compilation */
#define UART0_COMPILE  

/* enable uart0 if == 1, disable if == 0 */
#define UART0_ENABLED  1

/* enable uart0 interrupts if == 1, disable if == 0 */
#define UART0_INTERRUPT_ENABLED  1

#define UART0_BAUDRATE 38400

/* 
 * if you enable this, the maximum baudrate you can reach is 
 * higher, but the precision is lower. 
 */
#define UART0_USE_DOUBLE_SPEED 0
//#define UART0_USE_DOUBLE_SPEED 1

#define UART0_RX_FIFO_SIZE 4
#define UART0_TX_FIFO_SIZE 4
//#define UART0_NBITS 5
//#define UART0_NBITS 6
//#define UART0_NBITS 7
#define UART0_NBITS 8
//#define UART0_NBITS 9

#define UART0_PARITY UART_PARTITY_NONE
//#define UART0_PARITY UART_PARTITY_ODD
//#define UART0_PARITY UART_PARTITY_EVEN

#define UART0_STOP_BIT UART_STOP_BITS_1
//#define UART0_STOP_BIT UART_STOP_BITS_2

/* .... same for uart 1, 2, 3 ... */

