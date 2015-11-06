/**
 * @file rdline_config.h
 * @brief ihm/rdline configuration
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

/** Readline line buffer size */
#define RDLINE_BUF_SIZE 64
/** Maximum length of command prompt */
#define RDLINE_PROMPT_SIZE 16
/** VT100 command code buffer */
#define RDLINE_VT100_BUF_SIZE  8
/** Maximum size of history buffer (not used if history is disabled) */
#define RDLINE_HISTORY_BUF_SIZE 256
/** Maximum line length in history buffer (not used if history is disabled) */
#define RDLINE_HISTORY_MAX_LINE 64
