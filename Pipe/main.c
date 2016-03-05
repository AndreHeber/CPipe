#include <stdio.h>
#include <stdint.h>
#include <strsafe.h>
#include <windows.h>

#include "ringbuffer.h"
#include "pipe.h"

#ifdef _MSC_VER
#define inline __inline
#endif

/* Integrate every element of the signal. */
void increment(pipe_t * const p)
{
	while (Pipe_isFilled(p))
	{
		uint32_t item = Pipe_Read(p);
		item++;
		Pipe_Write(p, item);
	}
}

/* Square every element of the signal. */
void square(pipe_t * const p)
{
	while (Pipe_isFilled(p))
	{
		uint32_t item = Pipe_Read(p);
		item = item * item;
		Pipe_Write(p, item);
	}
}

/* Integrate over every element of the signal. */
void integrate(pipe_t * const pipe)
{
	uint32_t state = *((uint32_t*)pipe->state);

	while (Pipe_isFilled(pipe))
	{
		uint32_t item = Pipe_Read(pipe);
		state = state + item;
		Pipe_Write(pipe, state);
	}

	*((uint32_t*)pipe->state) = state;
}

/* Build the sum of all elements of the signal. */
void sum(pipe_t * const pipe)
{
	uint32_t sum = 0;

	while (Pipe_isFilled(pipe))
		sum += Pipe_Read(pipe);
	Pipe_Write(pipe, sum);
}

/* Build the average of all elements of the signal. */
void average(pipe_t * const pipe)
{
	uint32_t sum = 0;
	uint32_t element_counter = 0;
	uint32_t average = 0;

	while (Pipe_isFilled(pipe))
	{
		sum += Pipe_Read(pipe);
		element_counter++;
	}
	average = sum / element_counter;
	Pipe_Write(pipe, average);
}

/* Print the signal. */
void print(pipe_t * const pipe)
{
	printf("\nOutput:\n");
	while (Pipe_isFilled(pipe))
		printf("%d\n", Pipe_Read(pipe));
}

/* Logging function. Set by user. */
void log(pipe_t * const source, pipe_t * const target, uint32_t element)
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

extern void threads(void);
int main(int argc, char *argv[])
{
	uint32_t counter = 0;

	if (argc == 1)
	{
		/* Create pipes and connect them */
		Pipe_Create(increment_pipe, 4, 1, NULL, log);
		Pipe_Create(square_pipe, 4, 1, NULL, log);
		Pipe_Create(integrate_pipe, 8, 2, &counter, log);
		Pipe_Create(sum_pipe, 8, 2, NULL, log);
		Pipe_Create(average_pipe, 8, 2, NULL, log);
		Pipe_Create(print_pipe, 4, 2, NULL, log);

		Pipe_Connect(&increment_pipe, &integrate_pipe);
		Pipe_Connect(&square_pipe, &integrate_pipe);
		Pipe_Connect(&integrate_pipe, &sum_pipe);
		Pipe_Connect(&integrate_pipe, &average_pipe);
		Pipe_Connect(&sum_pipe, &print_pipe);
		Pipe_Connect(&average_pipe, &print_pipe);

		/* Create Input */
		Pipe_Insert(&increment_pipe, 1);
		Pipe_Insert(&increment_pipe, 3);
		Pipe_Insert(&increment_pipe, 5);

		Pipe_Insert(&square_pipe, 2);
		Pipe_Insert(&square_pipe, 4);
		Pipe_Insert(&square_pipe, 6);

		/* run the functions (each can run in an own thread) */
		increment(&increment_pipe);
		square(&square_pipe);
		integrate(&integrate_pipe);
		sum(&sum_pipe);
		average(&average_pipe);
		print(&print_pipe);

		getchar();
	}
	else
	{
		threads();
	}

	return 0;
}
