/**
 * @file main.h
 * @brief Main process interface
 * 
 * @par Configurable options
 * 
 * Macro               | Default  | Values         | Description
 * --------------------|----------|----------------|-----------------------------------------------
 * MAIN_QUEUE_SIZE     | [undef]  | 0..255         | Size of the event pool
 * MAIN_PRIORITY       | [undef]  | 0..127         | Event queue priority
 * 
 * @copyright Matemat controller firmware
 * Copyright © 2014 Chaostreff Basel
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

#ifndef _MAIN_H
#define _MAIN_H

/**
 * Get the current system time (ticks)
 */
uint16_t main_time(void);

/**
 * Signal the main process to shut down.
 */
void main_shutdown(void);

#endif /*_MAIN_H*/
