#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdint.h>

#ifdef _MSC_VER
#define inline __inline
#endif

/*
A thread-safe ring buffer, which uses an pre-declared array as buffer.
But you have to declare a buffer with one more item than you really need.
For instance, if you need a ring buffer for 10 items, the array must be declared for 11 items.
*/

typedef struct {
	uint32_t * reader;
	uint32_t * writer;
	uint32_t * start;
	uint32_t * end;
} ringbuffer_t;

/* Use Array as RingBuffer */
static inline void RingBuffer_Init(ringbuffer_t * const ring_buffer, uint32_t * const array, uint32_t size)
{
	ring_buffer->start = ring_buffer->reader = ring_buffer->writer = &array[0];
	ring_buffer->end = &array[0] + size - 1;
}

static inline uint32_t * RingBuffer_NextAddress(ringbuffer_t * const ring_buffer, uint32_t * const pointer)
{
	if (pointer == ring_buffer->end)
		return ring_buffer->start;
	else
		return pointer + 1;
}

static inline uint8_t RingBuffer_IsFull(ringbuffer_t * const ring_buffer)
{
	if (RingBuffer_NextAddress(ring_buffer, ring_buffer->writer) == ring_buffer->reader)
		return 1;
	return 0;
}

static inline uint8_t RingBuffer_IsEmpty(ringbuffer_t * const ring_buffer)
{
	if (ring_buffer->writer == ring_buffer->reader)
		return 1;
	return 0;
}

static inline uint8_t RingBuffer_IsFilled(ringbuffer_t * const ring_buffer)
{
	return !RingBuffer_IsEmpty(ring_buffer);
}

static inline void RingBuffer_Write(ringbuffer_t * const ring_buffer, uint32_t element)
{
	if (!RingBuffer_IsFull(ring_buffer))
	{
		*(ring_buffer->writer) = element;
		ring_buffer->writer = RingBuffer_NextAddress(ring_buffer, ring_buffer->writer);
	}
}

static inline uint32_t RingBuffer_Read(ringbuffer_t * const ring_buffer)
{
	uint32_t element = 0;

	if (!RingBuffer_IsEmpty(ring_buffer))
	{
		element = *(ring_buffer->reader);
		ring_buffer->reader = RingBuffer_NextAddress(ring_buffer, ring_buffer->reader);
	}

	return element;
}


#define Concat2(a, b) a ## b
#define Concat(a, b) Concat2(a, b)
#define UniqueBuffer(name) Concat(name, Concat(_buffer, __LINE__))

/*
Macro for the creation of a ring buffer.
*/
#define RingBuffer_Create(name, size)             \
uint32_t UniqueBuffer(name)[size];                \
ringbuffer_t name;                                \
RingBuffer_Init(&name, UniqueBuffer(name), size)  \

#define RingBuffer(name, size)                    \
uint32_t UniqueBuffer(name)[size];                \
ringbuffer_t name = {                             \
	.reader = &UniqueBuffer(name)[0],             \
	.writer = &UniqueBuffer(name)[0],             \
	.start = &UniqueBuffer(name)[0],              \
	.end = &UniqueBuffer(name)[0] + sizeof(UniqueBuffer(name)) / sizeof(UniqueBuffer(name)[0]) \
}

#endif
