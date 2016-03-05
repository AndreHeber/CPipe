#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <stdint.h>
#include "ringbuffer.h"

/* microsoft specific */
#ifdef _MSC_VER
#define inline __inline
#endif

/* A connection is an array of pointers, pointing to ringbuffers. */
typedef struct
{
	ringbuffer_t ** connection;
	uint32_t connection_count;
	uint32_t connection_max;
} connector_t;

static inline uint8_t Connector_IsFull(connector_t * const connector)
{
	for (uint32_t i = 0; i < connector->connection_count; i++)
	{
		if (RingBuffer_IsFull(connector->connection[i]))
			return 1;
	}
	return 0;
}

static inline uint8_t Connector_IsEmpty(connector_t * const connector)
{
	for (uint32_t i = 0; i < connector->connection_count; i++)
	{
		if (!RingBuffer_IsEmpty(connector->connection[i]))
			return 0;
	}
	return 1;
}

static inline uint8_t Connector_IsFilled(connector_t * const connector)
{
	return !Connector_IsEmpty(connector);
}

static inline uint32_t Connector_Read(connector_t * const connector)
{
	uint32_t i = 0;

	while ( RingBuffer_IsEmpty(connector->connection[i]) && (i < connector->connection_count) )
		i++;

	if (i == connector->connection_count)
		return 0;
	return RingBuffer_Read(connector->connection[i]);
}

static inline void Connector_Write(connector_t * const connector, uint32_t element)
{
	for (uint32_t i = 0; i < connector->connection_count; i++)
		RingBuffer_Write(connector->connection[i], element);
}

static inline void Connector_Insert(connector_t * const connector, uint32_t ring_buffer_index, uint32_t element)
{
	RingBuffer_Write(connector->connection[ring_buffer_index], element);
}

static inline void Connector_Add(connector_t * const connector, ringbuffer_t * ring_buffer)
{
	if (connector->connection_count < connector->connection_max)
	{
		connector->connection[connector->connection_count] = ring_buffer;
		connector->connection_count++;
	}
}

static inline void Connector_Init(connector_t * const connector, ringbuffer_t *buffer[], uint32_t buffer_size)
{
	connector->connection = &buffer[0];
	connector->connection_count = 0;
	connector->connection_max = buffer_size;
}

#define Concat2(a, b) a ## b
#define Concat(a, b) Concat2(a, b)

/*
Macro for the creation of a connector.
*/
#define Connector_Create(name, size)                                   \
ringbuffer_t *Concat(name, Concat(_buffer, __LINE__))[size];           \
connector_t name;                                                      \
Connector_Init(&name, Concat(name, Concat(_buffer, __LINE__)), size);  \

#endif
