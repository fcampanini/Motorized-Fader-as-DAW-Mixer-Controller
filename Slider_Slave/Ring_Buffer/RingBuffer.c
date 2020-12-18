#include "RingBuffer.h"

/*
* - Write data into the ring buffer -
*
* param *buffer: pointer to the struct Buffer ring buffer
* param byte: byte to write inside the ring buffer
* return: enum Buffer status BUFFER_OK or BUFFER_FULL
*/
enum BufferStatus bufferWrite(volatile struct Buffer *buffer, uint8_t byte) {
	uint8_t next_index = (((buffer->newest_index) + 1) % BUFFER_SIZE);

	if (next_index == buffer->oldest_index) {
		return BUFFER_FULL;
	}

	buffer->data[buffer->newest_index] = byte;
	buffer->newest_index = next_index;
	return BUFFER_OK;
}


/*
* - Read last byte in the tail from the buffer -
*
* param *buffer: pointer to the struct Buffer ring buffer
* param byte: byte to read from the ring buffer
* return: enum Buffer status BUFFER_OK or BUFFER_EMPTY
*/
enum BufferStatus bufferRead(volatile struct Buffer *buffer, uint8_t *byte) {

	if (buffer->newest_index == buffer->oldest_index) {
		return BUFFER_EMPTY;
	}

	*byte = buffer->data[buffer->oldest_index];
	buffer->oldest_index = ((buffer->oldest_index + 1) % BUFFER_SIZE);
	return BUFFER_OK;
}


/*
* - Looks at the most recently added character, 
* but doesn’t change either of the index variables -
*
* param *buffer: pointer to the struct Buffer ring buffer
* param byte: byte to search for inside the buffer
* return: enum Buffer status BUFFER_OK or BUFFER_EMPTY
*/
enum BufferStatus bufferPeek(volatile struct Buffer *buffer, uint8_t *byte) {

	uint8_t last_index = ((BUFFER_SIZE + (buffer->newest_index) - 1) % BUFFER_SIZE);
	if (buffer->newest_index == buffer->oldest_index) {
		return BUFFER_EMPTY;
	}
	*byte = buffer->data[last_index];
	return BUFFER_OK;
}


/*
* - Read last byte in the tail from the buffer 
* without updating any index -
*
* param *buffer: pointer to the struct Buffer ring buffer
* param byte: byte to read from the ring buffer
* return: enum Buffer status BUFFER_OK or BUFFER_EMPTY
*/
enum BufferStatus bufferCheck(volatile struct Buffer *buffer, uint8_t *byte) {

	if (buffer->newest_index == buffer->oldest_index) {
		return BUFFER_EMPTY;
	}

	*byte = buffer->data[buffer->oldest_index];
	return BUFFER_OK;
}

/*
* - Flash the ring buffer -
*
* param *buffer: pointer to the ring buffer to flash
*/
void bufferFlash(volatile struct Buffer *buffer) {

	buffer->newest_index = buffer->oldest_index;
}

/*
* - Check if the buffer is empty
*	without altering the indexes -
*
* param *buffer: pointer to the ring buffer to flash
*/
bool isBufferEmpty(volatile struct Buffer *buffer){
	if (buffer->newest_index == buffer->oldest_index)
	{
		return true;
	}
	else
	{
		return false;
	}
}
