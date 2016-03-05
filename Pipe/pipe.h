#ifndef PIPE_H_
#define PIPE_H_

#include <stdint.h>
#include "ringbuffer.h"
#include "connector.h"

/* microsoft specific */
#ifdef _MSC_VER
#define inline __inline
#endif

typedef struct pipe_tt
{
	connector_t * input_connector;
	connector_t * output_connector;
	void * state;
	char * name;
	void(*log_function)(struct pipe_tt * from, struct pipe_tt * to, uint32_t elem);
} pipe_t;


/*******************************************/
/* Functions to work with the pipe system. */
/*******************************************/

/*
Inserts a element into a pipe.
So the signal can inserted to the pipe system.
*/
static inline void Pipe_Insert(pipe_t * const pipe, uint32_t element)
{
	Connector_Insert(pipe->input_connector, 0, element);
}

/*
Read an element from the pipe.
Used inside functions of the pipe system.
*/
static inline uint32_t Pipe_Read(pipe_t * pipe)
{
	return Connector_Read(pipe->input_connector);
}

/*
Write an element to connected pipes.
Used inside functions of the pipe system.
*/
static inline void Pipe_Write(pipe_t * pipe, uint32_t element)
{
	pipe->log_function(pipe, NULL, element);
	Connector_Write(pipe->output_connector, element);
}

/*
Check if the pipe contents elements.
Used inside functions of the pipe system.
*/
static inline uint8_t Pipe_IsFilled(pipe_t * pipe)
{
	return Connector_IsFilled(pipe->input_connector);
}

/*
Check if the pipe has no place left for new elements.
Usefull for the logging.
*/
static inline uint8_t Pipe_IsFull(const pipe_t * pipe)
{
	return Connector_IsFull(pipe->input_connector);
}

/*
Default Logging function.
Shall be set by user.
*/
static inline void Pipe_Log(pipe_t * const source, pipe_t * const target, uint32_t element)
{
	if (Pipe_IsFull(target))
		printf("Error: Pipe %s is full!\n", target->name);

	if (source->state == NULL && target->state == NULL)
		printf("%s -> %d -> %s\n", source->name, element, target->name);
	else if (source->state != NULL && target->state != NULL)
		printf("%s(%d) -> %d -> %s(%d)\n", source->name, *((uint32_t*)source->state), element, target->name, *((uint32_t*)target->state));
	else if (source->state != NULL)
		printf("%s(%d) -> %d -> %s\n", source->name, *((uint32_t*)source->state), element, target->name);
	else
		printf("%s -> %d -> %s(%d)\n", source->name, element, target->name, *((uint32_t*)target->state));
}


/***************************************/
/* Functions to construct pipe system. */
/***************************************/

#define Concat2(a, b) a ## b
#define Concat(a, b) Concat2(a, b)

/*
Macro for the creation of a pipe.
Automates the creation of a ring buffer and the pipe.
arg_name is the variable name and string name of the pipe.
arg_size is the ring buffer size in bytes.
arg_conn_count is the number of outgoing connections.
arg_state is the given state, which can be used in the function.
arg_log is the log function, called when an element is sent.
*/
#define Pipe_Create(name, input_connection_count, output_connection_count, state, log)               \
static Connector_Create(Concat(name, Concat(_input_connector, __LINE__)), input_connection_count);   \
static Connector_Create(Concat(name, Concat(_output_connector, __LINE__)), output_connection_count); \
pipe_t name;                                                                                         \
Pipe_Init(&name, &Concat(name, Concat(_input_connector, __LINE__)), &Concat(name, Concat(_output_connector, __LINE__)), state, #name, log)

/*
Initializes a pipe.
A Ringbuffer is needed to store elements from other pipes.
A State (NULL if function has no state) for the function using the pipe.
A Name and a logging function are usefull to track the dataflow.
*/
static inline void Pipe_Init(
	pipe_t * const pipe,
	connector_t * input_connector,
	connector_t * output_connector,
	void * state,
	char * const name,
	void(*log_function)(struct pipe_tt * source, struct pipe_tt * target, uint32_t element)
	)
{
	pipe->input_connector = input_connector;
	pipe->output_connector = output_connector;
	pipe->state = state;
	pipe->name = name;

	if (log_function == NULL)
		pipe->log_function = Pipe_Log;
	else
		pipe->log_function = log_function;
}

/* Connect two pipes. Pipe a sends elements to pipe b. */
static inline void Pipe_Connect(pipe_t * const source, pipe_t * const target, ringbuffer_t * ring_buffer)
{
	Connector_Add(source->output_connector, ring_buffer);
	Connector_Add(target->input_connector, ring_buffer);
}

#define Pipe_CreateConnection(source, target, size)                     \
RingBuffer_Create(Concat(connector_ring_buffer, __LINE__), size);       \
Pipe_Connect(&source, &target, &Concat(connector_ring_buffer, __LINE__))

static inline void Pipe_AddInputBuffer(pipe_t * const pipe, ringbuffer_t * input_buffer)
{
	Connector_Add(pipe->input_connector, input_buffer);
}

#define Pipe_CreateInputBuffer(pipe_name, size)                                \
RingBuffer_Create(Concat(connector_ring_buffer, __LINE__), size);              \
Pipe_AddInputBuffer(&pipe_name, &Concat(connector_ring_buffer, __LINE__))

#endif
