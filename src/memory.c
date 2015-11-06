/**
 * @file memory.c
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

#include "memory.h"

memory_t *memory_init(void *pool, size_t size, size_t chunksize) {
	if (pool && size > sizeof(memory_t)) {
		memory_t *manager = (memory_t *) pool;
		void *prev = (uint8_t *) pool + sizeof(memory_t);
		manager->head = prev;
		size_t blocksize = chunksize + sizeof(void *);
		size -= sizeof(memory_t);
		size_t i;
		for (i = 0; size >= (blocksize << 1); i++, size -= blocksize) {
			void *next = (uint8_t *) prev + blocksize;
			*(void **) prev = next;
			prev = next;
		}
		if (size >= blocksize) {
			*(void **) prev = NULL;
			return manager;
		}
	}
	return NULL;
}

void *memory_allocate(memory_t *manager) {
	if (manager) {
		if (manager->head) {
			void *chunk = manager->head;
			manager->head = *(void **) chunk;
			*(void **) chunk = manager;
			return (uint8_t *) chunk + sizeof(void *);
		}
	}
	return NULL;
}

bool memory_release(void *memory) {
	if (memory) {
		memory_t **chunk = (memory_t **) memory - 1;
		memory_t *manager = *chunk;
		*(void **) chunk = manager->head;
		manager->head = chunk;
		return true;
	}
	return false;
}
