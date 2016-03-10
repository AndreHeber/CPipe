#ifndef HUB_H_
#define HUB_H_

#include <stdint.h>
#include "ringbuffer.h"
#include "connector.h"

/* microsoft specific */
#ifdef _MSC_VER
#define inline __inline
#endif

typedef struct hub_tt
{
	connector_t * input_connector;
	connector_t * output_connector;
	void * state;
} hub_t;


/*******************************************/
/* Functions to work with the hub system. */
/*******************************************/

/*
Inserts a element into a hub.
So the signal can inserted to the hub system.
*/
static inline void Hub_Insert(hub_t * const hub, uint32_t element)
{
	Connector_Insert(hub->input_connector, 0, element);
}

/*
Read an element from the hub.
Used inside functions of the hub system.
*/
static inline uint32_t Hub_Read(hub_t * hub)
{
	return Connector_Read(hub->input_connector);
}

/*
Write an element to connected hubs.
Used inside functions of the hub system.
*/
static inline void Hub_Write(hub_t * hub, uint32_t element)
{
	Connector_Write(hub->output_connector, element);
}

/*
Check if the hub contents elements.
Used inside functions of the hub system.
*/
static inline uint8_t Hub_IsFilled(hub_t * hub)
{
	return Connector_IsFilled(hub->input_connector);
}

static inline uint8_t Hub_IsEmpty(hub_t * hub)
{
	return Connector_IsEmpty(hub->input_connector);
}

/*
Check if the hub has no place left for new elements.
*/
static inline uint8_t Hub_IsFull(const hub_t * hub)
{
	return Connector_IsFull(hub->output_connector);
}


/***************************************/
/* Functions to construct hub system. */
/***************************************/

#define Concat2(a, b) a ## b
#define Concat(a, b) Concat2(a, b)

/*
Macro for the creation of a hub.
Automates the creation of a ring buffer and the hub.
arg_name is the variable name and string name of the hub.
arg_size is the ring buffer size in bytes.
arg_conn_count is the number of outgoing connections.
arg_state is the given state, which can be used in the function.
arg_log is the log function, called when an element is sent.
*/
#define Hub_Create(name, input_connection_count, output_connection_count, state)               \
static Connector_Create(Concat(name, Concat(_input_connector, __LINE__)), input_connection_count);   \
static Connector_Create(Concat(name, Concat(_output_connector, __LINE__)), output_connection_count); \
hub_t name;                                                                                         \
Hub_Init(&name, &Concat(name, Concat(_input_connector, __LINE__)), &Concat(name, Concat(_output_connector, __LINE__)), state)

/*
Initializes a hub.
A Ringbuffer is needed to store elements from other hubs.
A State (NULL if function has no state) for the function using the hub.
*/
static inline void Hub_Init(hub_t * const hub, connector_t * const input_connector, connector_t * const output_connector, void * const state)
{
	hub->input_connector = input_connector;
	hub->output_connector = output_connector;
	hub->state = state;
}

/* Connect two hubs. hub a sends elements to hub b. */
static inline void Hub_Connect(hub_t * const source, hub_t * const target, ringbuffer_t * ring_buffer)
{
	Connector_Add(source->output_connector, ring_buffer);
	Connector_Add(target->input_connector, ring_buffer);
}

#define Hub_CreateConnection(source, target, size)                      \
RingBuffer_Create(Concat(connector_ring_buffer, __LINE__), size);       \
Hub_Connect(&source, &target, &Concat(connector_ring_buffer, __LINE__))


/*
If you want to connect the hub with a ring buffer, use Hub_ConnectInputWithRingBuffer
or Hub_ConnectOutputWithRingBuffer.
*/

static inline void Hub_ConnectInputWithRingBuffer(hub_t * const hub, ringbuffer_t * ring_buffer)
{
	Connector_Add(hub->input_connector, ring_buffer);
}

static inline void Hub_ConnectOutputWithRingBuffer(hub_t * const hub, ringbuffer_t * ring_buffer)
{
	Connector_Add(hub->output_connector, ring_buffer);
}

static inline void Hub_AddInputBuffer(hub_t * const hub, ringbuffer_t * input_buffer)
{
	Connector_Add(hub->input_connector, input_buffer);
}

#define Hub_CreateInputBuffer(hub_name, size)                                \
RingBuffer_Create(Concat(connector_ring_buffer, __LINE__), size);              \
Hub_AddInputBuffer(&hub_name, &Concat(connector_ring_buffer, __LINE__))

#endif
