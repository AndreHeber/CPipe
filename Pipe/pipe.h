#ifndef PIPE_H_
#define PIPE_H_

#include <stdint.h>
#include "ringbuffer.h"

/* microsoft specific */
#define inline __inline

/* number of counts */
#define PIPE_OUTPUT_COUNT 4

typedef struct pipe_tt
{
	ringbuffer_t * input;
	void * state;
	struct pipe_tt *output[PIPE_OUTPUT_COUNT];
	uint8_t output_count;
	char * name;
	void(*log)(struct pipe_tt * from, struct pipe_tt * to, uint32_t elem);
} pipe_t;


/***********************************/
/* Functions to contruct pipe mesh */
/***********************************/

#define Concat2(a, b) a ## b
#define Concat(a, b) Concat2(a, b)
#define SizeOfArray(arg) ( sizeof(arg) / sizeof(arg[0]) )

#define Pipe_Create(arg_name, arg_state, arg_size, arg_log)                     \
  static uint32_t Concat(buffer, __LINE__)[arg_size];              \
  ringbuffer_t arg_name ## _rb; \
  RingBuffer_InitFromArray(&arg_name ## _rb, Concat(buffer, __LINE__), SizeOfArray(Concat(buffer, __LINE__))); \
  pipe_t arg_name; \
  Pipe_Init(&arg_name, &arg_name ## _rb, arg_state, #arg_name, arg_log)

static inline void Pipe_Init(
	pipe_t * const p,
	ringbuffer_t * const arg_input,
	void * arg_state,
	char * const arg_name,
	void(*arg_log)(struct pipe_tt * from, struct pipe_tt * to, uint32_t elem)
	)
{
	p->input = arg_input;
	p->state = arg_state;

	for (uint8_t i = 0; i < PIPE_OUTPUT_COUNT; i++)
		p->output[i] = NULL;

	p->output_count = 0;
	p->name = arg_name;
	p->log = arg_log;
}

static inline void Pipe_Connect(pipe_t * const a, pipe_t * const b)
{
	a->output[a->output_count] = b;
	a->output_count++;
}


/************************************/
/* Functions to work with pipe mesh */
/************************************/

static inline void Pipe_Insert(pipe_t * const p, uint32_t elem)
{
	RingBuffer_Write(p->input, elem);
}

static inline uint32_t Pipe_Read(pipe_t * p)
{
	return RingBuffer_Read(p->input);
}

static inline void Pipe_Write(pipe_t * p, uint32_t elem)
{
	for (uint8_t i = 0; i < p->output_count; i++)
	{
		p->log(p, p->output[i], elem);
		RingBuffer_Write(p->output[i]->input, elem);
	}
}

static inline uint8_t Pipe_isFilled(pipe_t * p)
{
	return RingBuffer_IsFilled(p->input);
}

#endif
