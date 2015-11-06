/**
 * @file timer_config.h
 * @brief hardware/timer configuration
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

/**
 * Enable TIMER2 (8bit, PWM capable).
 */
#define TIMER2_ENABLED
/**
 * Set prescaler of TIMER2.
 * 
 * Possible options for ATmega128: 1, 8, 64, 256, 1024
 * 
 * With a 16MHz quartz and an overflow driven 16bit counter (8bit counter +
 * 8bit timer), the following tick durations, scheduling precisions
 * and maxium timer lengths (callout uses signed comparison, limiting them to
 * half the timer range) are possible:
 * 
 * Prescaler | Tick length | Precision | Maximum length | Scheduling frequency
 * ----------|-------------|-----------|----------------|---------------------
 * 1         | 62.5ns      | 16us      | 2.048ms        | 62.5kHz
 * 8         | 500ns       | 128µs     | 16.384ms       | 7.8125kHz
 * 64        | 4µs         | 1.024ms   | 131.072ms      | 976.5625Hz
 * 1024      | 64µs        | 16.384ms  | 2.097152s      | 61.03515625Hz
 */
#define TIMER2_PRESCALER_DIV 1024

//#define TIMER0_ENABLED
/* some archs have TIMER0A_ENABLED or TIMER0B_ENABLED */

//#define TIMER1_ENABLED
//#define TIMER1A_ENABLED
//#define TIMER1B_ENABLED
//#define TIMER1C_ENABLED

/* some archs have TIMER2A_ENABLED or TIMER2B_ENABLED */

//#define TIMER3_ENABLED
//#define TIMER3A_ENABLED
//#define TIMER3B_ENABLED
//#define TIMER3C_ENABLED

//#define TIMER4_ENABLED
//#define TIMER4A_ENABLED
//#define TIMER4B_ENABLED
//#define TIMER4C_ENABLED

//#define TIMER5_ENABLED
//#define TIMER5A_ENABLED
//#define TIMER5B_ENABLED
//#define TIMER5C_ENABLED

//#define TIMER0_PRESCALER_DIV 1
//#define TIMER1_PRESCALER_DIV 1
//#define TIMER3_PRESCALER_DIV 1
//#define TIMER4_PRESCALER_DIV 1
//#define TIMER5_PRESCALER_DIV 1
