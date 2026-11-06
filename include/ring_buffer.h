#ifndef EMBEDDED_UART_SHELL_RING_BUFFER_H
#define EMBEDDED_UART_SHELL_RING_BUFFER_H

#include <stdbool.h>
#include <stdatomic.h>
#include <stdint.h>

#define RING_BUFFER_CAPACITY 128U

typedef struct {
    uint8_t data[RING_BUFFER_CAPACITY];
    _Atomic uint8_t head;
    _Atomic uint8_t tail;
} ring_buffer_t;

void ring_buffer_init(ring_buffer_t *buffer);
bool ring_buffer_push(ring_buffer_t *buffer, uint8_t value);
bool ring_buffer_pop(ring_buffer_t *buffer, uint8_t *value);

#endif