#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdint.h>

#define inline __inline

typedef struct {
  uint32_t * reader;
  uint32_t * writer;
  uint32_t * start;
  uint32_t * end;
  uint8_t write_failed;
  uint8_t read_failed;
} ringbuffer_t;

/* Use Array as RingBuffer */
static inline void RingBuffer_InitFromArray(ringbuffer_t * const rb, uint32_t * const array, const uint32_t size)
{
  rb->start = rb->reader = rb->writer = array;
  rb->end = array + size - 1;
  rb->write_failed = rb->read_failed = 0;
}

static inline uint32_t * RingBuffer_GetNextWriterAddress(const ringbuffer_t * const rb)
{
  uint32_t * next_address = rb->writer;

  if (next_address == rb->end)
    next_address = rb->start;
  else
    next_address++;

  return next_address;
}

static inline uint8_t RingBuffer_IsFull(const ringbuffer_t * const rb)
{
  if (RingBuffer_GetNextWriterAddress(rb) == rb->reader)
    return 1;
  return 0;
}

static inline uint8_t RingBuffer_IsEmpty(const ringbuffer_t * const rb)
{
  if (rb->writer == rb->reader)
    return 1;
  return 0;
}

static inline uint8_t RingBuffer_IsFilled(const ringbuffer_t * const rb)
{
  return !RingBuffer_IsEmpty(rb);
}

/* Write elem into RingBuffer. */
static inline void RingBuffer_Write(ringbuffer_t * const rb, const uint32_t elem)
{
  if (!RingBuffer_IsFull(rb))
  {
    *(rb->writer) = elem;

    if (rb->writer == rb->end)
      rb->writer = rb->start;
    else
      rb->writer++;
  }
  else
  {
    rb->write_failed = 1;
  }
}

/* Read value from RingBuffer and returns it. */
static inline uint32_t RingBuffer_Read(ringbuffer_t * const rb)
{
  uint32_t ret = 0;

  if (!RingBuffer_IsEmpty(rb))
  {
    ret = *(rb->reader);

    if (rb->reader == rb->end)
      rb->reader = rb->start;
    else
      rb->reader++;
  }
  else
  {
    rb->read_failed = 1;
  }

  return ret;
}

#endif
