#include "ring_buffer.h"

#define RING_BUFFER_INDEX_MASK (RING_BUFFER_CAPACITY - 1U)

_Static_assert((RING_BUFFER_CAPACITY & RING_BUFFER_INDEX_MASK) == 0U,
               "Ring buffer capacity must be a power of two");

void ring_buffer_init(ring_buffer_t *buffer)
{
    atomic_init(&buffer->head, 0U);
    atomic_init(&buffer->tail, 0U);
}

bool ring_buffer_push(ring_buffer_t *buffer, uint8_t value)
{
    uint8_t head = atomic_load_explicit(&buffer->head, memory_order_relaxed);
    uint8_t next_head = (uint8_t)((head + 1U) & RING_BUFFER_INDEX_MASK);
    uint8_t tail = atomic_load_explicit(&buffer->tail, memory_order_acquire);

    if (next_head == tail) {
        return false;
    }

    buffer->data[head] = value;
    atomic_store_explicit(&buffer->head, next_head, memory_order_release);
    return true;
}

bool ring_buffer_pop(ring_buffer_t *buffer, uint8_t *value)
{
    uint8_t tail = atomic_load_explicit(&buffer->tail, memory_order_relaxed);
    uint8_t head = atomic_load_explicit(&buffer->head, memory_order_acquire);

    if (tail == head) {
        return false;
    }

    *value = buffer->data[tail];
    atomic_store_explicit(
        &buffer->tail,
        (uint8_t)((tail + 1U) & RING_BUFFER_INDEX_MASK),
        memory_order_release);
    return true;
}