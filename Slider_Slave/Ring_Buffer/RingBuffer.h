#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE  128

struct Buffer {
	uint8_t data[BUFFER_SIZE];
	uint8_t newest_index;
	uint8_t oldest_index;
};

enum BufferStatus { BUFFER_OK, BUFFER_EMPTY, BUFFER_FULL };

/* Write data into the ring buffer */
enum BufferStatus bufferWrite(volatile struct Buffer *buffer, uint8_t byte);

/*  Read last byte in the tail from the buffer */
enum BufferStatus bufferRead(volatile struct Buffer *buffer, uint8_t *byte);

/*  Looks at the most recently added character, but doesn't change either of the index variables */
enum BufferStatus bufferPeek(volatile struct Buffer *buffer, uint8_t *byte);

/*  Read last byte in the tail from the buffer without updating any index */
enum BufferStatus bufferCheck(volatile struct Buffer *buffer, uint8_t *byte);

/* Flash the buffer */
void bufferFlash(volatile struct Buffer *buffer);

/* Check if the buffer is empty without altering the indexes */
bool isBufferEmpty(volatile struct Buffer *buffer);

#endif // RINGBUFFER_H