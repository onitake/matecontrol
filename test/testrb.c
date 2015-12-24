#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
	uint32_t value;
} event_t;
typedef uint8_t dispatch_size_t;

typedef struct {
	bool initialized;
	dispatch_size_t size;
	dispatch_size_t in;
	dispatch_size_t out;
	event_t **events;
} dispatch_queue_t;

static dispatch_queue_t global_dispatch;

event_t *event_new(uint32_t value) {
	event_t *event = calloc(1, sizeof(event_t));
	event->value = value;
	return event;
}

void event_delete(event_t *event) {
	free(event);
}

dispatch_size_t dispatch_events_count() {
	if (global_dispatch.in >= global_dispatch.out) {
		return global_dispatch.in - global_dispatch.out;
	} else {
		return global_dispatch.size - global_dispatch.out + global_dispatch.in;
	}
}

event_t *dispatch_dequeue() {
	if (global_dispatch.in == global_dispatch.out) {
		return NULL;
	} else {
		event_t *event = global_dispatch.events[global_dispatch.out];
		global_dispatch.out = (global_dispatch.out == global_dispatch.size - 1) ? 0 : global_dispatch.out + 1;
		return event;
	}
}

bool dispatch_enqueue(event_t *event) {
	dispatch_size_t nextin = (global_dispatch.in == global_dispatch.size - 1) ? 0 : global_dispatch.in + 1;
	if (global_dispatch.out == nextin) {
		return false;
	} else {
		global_dispatch.events[global_dispatch.in] = event;
		global_dispatch.in = nextin;
		return true;
	}
}

void dispatch_init(dispatch_size_t size) {
	if (!global_dispatch.initialized) {
		global_dispatch.size = size;
		global_dispatch.events = (event_t **) calloc(size, sizeof(event_t *));
		global_dispatch.in = 0;
		global_dispatch.out = 0;
		global_dispatch.initialized = true;
	}
}

void dispatch_shutdown() {
	if (global_dispatch.initialized) {
		event_t *event;
		do {
			event = dispatch_dequeue();
			if (event) {
				event_delete(event);
			}
		} while (event);
		free(global_dispatch.events);
		global_dispatch.initialized = false;
	}
}

const size_t cases = 16;
const dispatch_size_t ins[] =    { 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3 };
const dispatch_size_t outs[] =   { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3 };
const dispatch_size_t counts[] = { 0, 3, 2, 1, 1, 0, 3, 2, 2, 1, 0, 3, 3, 2, 1, 0 };
const bool nextins[] =           { 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1 };
const bool nextouts[] =          { 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0 };

int main(int argc, char **argv) {
	for (size_t i = 0; i < cases; i++) {
		dispatch_init(4);
		assert(dispatch_events_count() == 0);
		global_dispatch.in = ins[i];
		global_dispatch.out = outs[i];
		dispatch_size_t count = dispatch_events_count();
		if (count != counts[i]) printf("count fail in:%u out:%u count:%u result:%u\n", ins[i], outs[i], counts[i], count);
		dispatch_shutdown();
	}
	for (size_t i = 0; i < cases; i++) {
		dispatch_init(4);
		assert(dispatch_events_count() == 0);
		global_dispatch.in = ins[i];
		global_dispatch.out = outs[i];
		bool nextin = dispatch_enqueue(event_new(i));
		if (nextin != nextins[i]) printf("enqueue fail in:%u out:%u nextin:%u result:%u\n", ins[i], outs[i], nextins[i], nextin);
		dispatch_shutdown();
	}
	for (size_t i = 0; i < cases; i++) {
		dispatch_init(4);
		assert(dispatch_events_count() == 0);
		for (size_t j = 0; j < 4; j++) {
			global_dispatch.events[j] = event_new(i);
		}
		global_dispatch.in = ins[i];
		global_dispatch.out = outs[i];
		event_t *event = dispatch_dequeue(event_new(i));
		bool nextout = event != NULL;
		if (nextout != nextouts[i]) printf("dequeue fail in:%u out:%u nextout:%u result:%u\n", ins[i], outs[i], nextouts[i], nextout);
		free(event);
		dispatch_shutdown();
	}
	return 0;
}