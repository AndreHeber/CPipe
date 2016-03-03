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
  static uint32_t state = 0;

  while (Pipe_isFilled(p))
  {
    uint32_t item = Pipe_Read(p);
    state = state + item;
    Pipe_Write(p, state);
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

int main(int argc, char * argv[])
{
  /* Create pipes and connect them */
  Pipe_Create(increment_pipe, 4);
  Pipe_Create(square_pipe, 4);
  Pipe_Create(integrate_pipe, 8);
  Pipe_Create(sum_pipe, 8);
  Pipe_Create(average_pipe, 8);

  Pipe_Connect(&increment_pipe, &integrate_pipe);
  Pipe_Connect(&square_pipe, &integrate_pipe);
  Pipe_CreateOutput(integrate_pipe, integrate_output_rb, 8);
  Pipe_Connect(&integrate_pipe, &sum_pipe);
  Pipe_CreateOutput(sum_pipe, sum_output_rb, 4);
  Pipe_Connect(&integrate_pipe, &average_pipe);
  Pipe_CreateOutput(average_pipe, average_output_rb, 4);

  /* Generate input */
  Pipe_Insert(&increment_pipe, 1);
  Pipe_Insert(&increment_pipe, 3);
  Pipe_Insert(&increment_pipe, 5);

  Pipe_Insert(&square_pipe, 2);
  Pipe_Insert(&square_pipe, 4);
  Pipe_Insert(&square_pipe, 6);

  /* run the functions */
  increment(&increment_pipe);
  square(&square_pipe);
  integrate(&integrate_pipe);
  sum(&sum_pipe);
  average(&average_pipe);

  /* print the output */
  printf("Output:\n");
  printf("%d\n", RingBuffer_Read(&integrate_output_rb));
  printf("%d\n", RingBuffer_Read(&integrate_output_rb));
  printf("%d\n", RingBuffer_Read(&integrate_output_rb));
  printf("%d\n", RingBuffer_Read(&integrate_output_rb));
  printf("%d\n", RingBuffer_Read(&integrate_output_rb));
  printf("%d\n", RingBuffer_Read(&integrate_output_rb));
  printf("\n");
  printf("Summe: %d\n", RingBuffer_Read(&sum_output_rb));
  printf("Average: %d\n", RingBuffer_Read(&average_output_rb));

  getchar();

  return 0;
}
