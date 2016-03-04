#ifndef PIPE_H_
#define PIPE_H_

#include <stdint.h>
#include "ringbuffer.h"

/* microsoft specific */
#ifdef _MSC_VER
  #define inline __inline
#endif

/* number of connections */
#define PIPE_NUMBER_OF_CONNECTIONS 4

typedef struct pipe_tt
{
	ringbuffer_t * input;
	void * state;
  struct pipe_tt ** connection;
	uint32_t connection_count;
  uint32_t connection_max;
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
  RingBuffer_Write(pipe->input, element);
}

/*
Read an element from the pipe.
Used inside functions of the pipe system.
*/
static inline uint32_t Pipe_Read(pipe_t * pipe)
{
  return RingBuffer_Read(pipe->input);
}

/*
Write an element to connected pipes.
Used inside functions of the pipe system.
*/
static inline void Pipe_Write(pipe_t * pipe, uint32_t element)
{
  for (uint8_t i = 0; i < pipe->connection_count; i++)
	{
    pipe->log_function(pipe, pipe->connection[i], element);
    RingBuffer_Write(pipe->connection[i]->input, element);
	}
}

/*
Check if the pipe contents elements.
Used inside functions of the pipe system.
*/
static inline uint8_t Pipe_isFilled(pipe_t * pipe)
{
	return RingBuffer_IsFilled(pipe->input);
}

/*
Check if the pipe has no place left for new elements.
Usefull for the logging.
*/
static inline uint8_t Pipe_isFull(const pipe_t * pipe)
{
  return RingBuffer_IsFull(pipe->input);
}

/*
Default Logging function.
Shall be set by user.
*/
static inline void Pipe_Log(pipe_t * const source, pipe_t * const target, uint32_t element)
{
  if (Pipe_isFull(target))
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
#define SizeOfArray(arg) ( sizeof(arg) / sizeof(arg[0]) )

/*
Macro for the creation of a pipe.
Automates the creation of a ring buffer and the pipe.
arg_name is the variable name and string name of the pipe.
arg_size is the ring buffer size in bytes.
arg_conn_count is the number of outgoing connections.
arg_state is the given state, which can be used in the function.
arg_log is the log function, called when an element is sent.
*/
#define Pipe_Create(arg_name, arg_size, arg_conn_count, arg_state, arg_log)                        \
static uint32_t Concat(arg_name, Concat(_buffer, __LINE__))[arg_size];                             \
ringbuffer_t arg_name ## _rb;                                                                      \
RingBuffer_InitFromArray(&arg_name ## _rb, Concat(arg_name, Concat(_buffer, __LINE__)), arg_size); \
pipe_t arg_name;                                                                                   \
static pipe_t *Concat(arg_name, Concat(_connection_buffer, __LINE__))[arg_conn_count];             \
Pipe_Init(&arg_name, &arg_name ## _rb, Concat(arg_name, Concat(_connection_buffer, __LINE__)), arg_conn_count, arg_state, #arg_name, arg_log)

/*
Initializes a pipe.
A Ringbuffer is needed to store elements from other pipes.
A State (NULL if function has no state) for the function using the pipe.
A Name and a logging function are usefull to track the dataflow.
*/
static inline void Pipe_Init(
  pipe_t * const pipe,
  ringbuffer_t * const input,
  pipe_t ** pipe_connections,
  uint32_t connection_max,
  void * state,
  char * const name,
  void(*log_function)(struct pipe_tt * source, struct pipe_tt * target, uint32_t element)
  )
{
  pipe->input = input;
  pipe->state = state;
  pipe->connection = pipe_connections;

  for (uint8_t i = 0; i < connection_max; i++)
    pipe->connection[i] = NULL;

  pipe->connection_count = 0;
  pipe->connection_max = connection_max;
  pipe->name = name;

  if (log_function == NULL)
    pipe->log_function = Pipe_Log;
  else
    pipe->log_function = log_function;
}

/* Connect two pipes. Pipe a sends elements to pipe b. */
static inline void Pipe_Connect(pipe_t * const source, pipe_t * const target)
{
  if (source->connection_count < source->connection_max)
  {
    source->connection[source->connection_count] = target;
    source->connection_count++;
  }
}

#endif
