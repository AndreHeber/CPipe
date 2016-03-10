#include <stdio.h>
#include <stdint.h>
#include <strsafe.h>
#include <windows.h>

#include "ringbuffer.h"
#include "connector.h"
#include "hub.h"

#ifdef _MSC_VER
#define inline __inline
#endif

/* Integrate every element of the signal. */
void increment(hub_t * const p)
{
	while (Hub_IsFilled(p))
	{
		uint32_t item = Hub_Read(p);
		item++;
		Hub_Write(p, item);
	}
}

/* Square every element of the signal. */
void square(hub_t * const p)
{
	while (Hub_IsFilled(p))
	{
		uint32_t item = Hub_Read(p);
		item = item * item;
		Hub_Write(p, item);
	}
}

/* Integrate over every element of the signal. */
void integrate(hub_t * const hub)
{
	uint32_t state = *((uint32_t*)hub->state);

	while (Hub_IsFilled(hub))
	{
		uint32_t item = Hub_Read(hub);
		state = state + item;
		Hub_Write(hub, state);
	}

	*((uint32_t*)hub->state) = state;
}

/* Build the sum of all elements of the signal. */
void sum(hub_t * const hub)
{
	uint32_t sum = 0;

	while (Hub_IsFilled(hub))
		sum += Hub_Read(hub);
	Hub_Write(hub, sum);
}

/* Build the average of all elements of the signal. */
void average(hub_t * const hub)
{
	uint32_t sum = 0;
	uint32_t element_counter = 0;
	uint32_t average = 0;

	while (Hub_IsFilled(hub))
	{
		sum += Hub_Read(hub);
		element_counter++;
	}
	average = sum / element_counter;
	Hub_Write(hub, average);
}

/* Print the signal. */
void print(hub_t * const hub)
{
	printf("\nOutput:\n");
	while (Hub_IsFilled(hub))
		printf("%d\n", Hub_Read(hub));
}

/* Logging function. Set by user. */
//void log(hub_t * const source, hub_t * const target, uint32_t element)
//{
	//if (Hub_IsFull(target))
	//	printf("Error: Hub %s is full!\n", target->name);

	//if (source->state == NULL && target->state == NULL)
	//	printf("%s -> %d -> %s\n", source->name, element, target->name);
	//else if (source->state != NULL && target->state != NULL)
	//	printf("%s(%d) -> %d -> %s(%d)\n", source->name, *((uint32_t*)source->state), element, target->name, *((uint32_t*)target->state));
	//else if (source->state != NULL)
	//	printf("%s(%d) -> %d -> %s\n", source->name, *((uint32_t*)source->state), element, target->name);
	//else
	//	printf("%s -> %d -> %s(%d)\n", source->name, element, target->name, *((uint32_t*)target->state));

	//printf("%s: %d\n", source->name, element);
//}

extern void threads(uint32_t loops);
extern void speed(uint32_t items_count);

int main(int argc, char *argv[])
{
	uint32_t counter = 0;

	if (argc == 1)
	{
		/* Create hubs and connect them */
		Hub_Create(increment_hub, 1, 1, NULL);
		Hub_Create(square_hub, 1, 1, NULL);
		Hub_Create(integrate_hub, 2, 2, &counter);
		Hub_Create(sum_hub, 1, 1, NULL);
		Hub_Create(average_hub, 1, 1, NULL);
		Hub_Create(print_hub, 2, 1, NULL);

		Hub_CreateInputBuffer(increment_hub, 4);
		Hub_CreateInputBuffer(square_hub, 4);

		Hub_CreateConnection(increment_hub, integrate_hub, 4);
		Hub_CreateConnection(square_hub, integrate_hub, 4);
		Hub_CreateConnection(integrate_hub, sum_hub, 8);
		Hub_CreateConnection(integrate_hub, average_hub, 8);
		Hub_CreateConnection(sum_hub, print_hub, 4);
		Hub_CreateConnection(average_hub, print_hub, 4);

		/* Create Input */
		Hub_Insert(&increment_hub, 1);
		Hub_Insert(&increment_hub, 3);
		Hub_Insert(&increment_hub, 5);

		Hub_Insert(&square_hub, 2);
		Hub_Insert(&square_hub, 4);
		Hub_Insert(&square_hub, 6);

		/* run the functions (each can run in an own thread) */
		increment(&increment_hub);
		square(&square_hub);
		integrate(&integrate_hub);
		sum(&sum_hub);
		average(&average_hub);
		print(&print_hub);

		getchar();
	}
	else if (argc == 2)
	{
		threads(atoi(argv[1]));
	}
	else
	{
		speed(atoi(argv[2]));
	}

	return 0;
}
