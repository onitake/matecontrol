/**
 * @file memory.h
 * @brief Pool memory manager
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

#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Dynamic memory manager based on fixed-size chunk allocation.
 * 
 * @warning The manager is not thread- or interrupt-safe.
 */
typedef struct {
	/** Pointer to the head chunk */
	void *head;
} memory_t;

/**
 * Calculates the required pool size in bytes for a given number of chunks
 * and a chunk size.
 * 
 * Example usage:
 * 
 *     char pool[MEMORY_POOL_SIZE(16, sizeof(int))];
 */
#define MEMORY_POOL_SIZE(chunks, size) (sizeof(memory_t) + chunks * (size + sizeof(void *)))

/**
 * Initialize a dynamic memory manager.
 * It is recommended to use @ref MEMORY_POOL_SIZE to calculate the required
 * pool size.
 * @param pool a pointer to the memory pool
 * @param size the number of bytes in the pool
 * @param chunksize the size of a single chunk
 * @return an initialized memory manager or NULL on error
 */
memory_t *memory_init(void *pool, size_t size, size_t chunksize);

/**
 * Allocate a chunk from the memory pool.
 * @param manager the memory manager
 * @return a pointer to the allocated memory, or NULL if no free chunk was found
 */
void *memory_allocate(memory_t *manager);

/**
 * Relinquish a chunk back to the memory pool.
 * @param memory a pointer to the allocated memory
 * @return true, if the chunk was released successfully; false if manager or
 * chunk were NULL or if this chunk doesn't belong to manager
 */
bool memory_release(void *memory);

#endif /*_MEMORY_H*/
