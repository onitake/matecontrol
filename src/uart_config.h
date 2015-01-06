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

#define UART0_COMPILE  
#define UART0_ENABLED 1
#define UART0_INTERRUPT_ENABLED 1
#define UART0_BAUDRATE 38400
#define UART0_USE_DOUBLE_SPEED 0
#define UART0_RX_FIFO_SIZE 4
#define UART0_TX_FIFO_SIZE 4
#define UART0_NBITS 8
#define UART0_PARITY UART_PARTITY_NONE
#define UART0_STOP_BIT UART_STOP_BITS_1
