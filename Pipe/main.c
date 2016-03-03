#include <stdio.h>
#include <stdint.h>
#include "ringbuffer.h"
#include "pipe.h"

void increment(pipe_t * const p)
{
	while (Pipe_isFilled(p))
	{
		uint32_t item = Pipe_Read(p);
		item++;
		Pipe_Write(p, item);
	}
}

void square(pipe_t * const p)
{
	while (Pipe_isFilled(p))
	{
		uint32_t item = Pipe_Read(p);
		item = item * item;
		Pipe_Write(p, item);
	}
}

void integrate(pipe_t * const p)
{
	uint32_t * state = p->state;

	while (Pipe_isFilled(p))
	{
		uint32_t item = Pipe_Read(p);
		*state = *state + item;
		Pipe_Write(p, *state);
	}
}

void sum(pipe_t * const p)
{
	uint32_t sum = 0;

	while (Pipe_isFilled(p))
		sum += Pipe_Read(p);
	Pipe_Write(p, sum);
}

void average(pipe_t * const p)
{
	uint32_t sum = 0;
	uint32_t element_counter = 0;
	uint32_t average = 0;

	while (Pipe_isFilled(p))
	{
		sum += Pipe_Read(p);
		element_counter++;
	}
	average = sum / element_counter;
	Pipe_Write(p, average);
}

void print(pipe_t * const p)
{
	printf("\nOutput:\n");
	while (Pipe_isFilled(p))
		printf("%d\n", Pipe_Read(p));
}

void log(pipe_t * const from, pipe_t * const to, uint32_t elem)
{
	if (from->state == NULL && to->state == NULL)
		printf("%s -> %d -> %s\n", from->name, elem, to->name);
	else if (from->state != NULL && to->state != NULL)
		printf("%s(%d) -> %d -> %s(%d)\n", from->name, *((uint32_t*)from->state), elem, to->name, *((uint32_t*)to->state));
	else if (from->state != NULL)
		printf("%s(%d) -> %d -> %s\n", from->name, *((uint32_t*)from->state), elem, to->name);
	else
		printf("%s -> %d -> %s(%d)\n", from->name, elem, to->name, *((uint32_t*)to->state));
}

int main(void)
{
	uint32_t counter = 0;

	/* Create pipes and connect them */
	Pipe_Create(increment_pipe, NULL, 4, log);
	Pipe_Create(square_pipe, NULL, 4, log);
	Pipe_Create(integrate_pipe, &counter, 8, log);
	Pipe_Create(sum_pipe, NULL, 8, log);
	Pipe_Create(average_pipe, NULL, 8, log);
	Pipe_Create(print_pipe, NULL, 4, log);

	Pipe_Connect(&increment_pipe, &integrate_pipe);
	Pipe_Connect(&square_pipe, &integrate_pipe);
	Pipe_Connect(&integrate_pipe, &sum_pipe);
	Pipe_Connect(&integrate_pipe, &average_pipe);
	Pipe_Connect(&sum_pipe, &print_pipe);
	Pipe_Connect(&average_pipe, &print_pipe);

	/* Generate input */
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

	return 0;
}
