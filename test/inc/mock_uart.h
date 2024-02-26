#ifndef MOCK_UART_H
#define MOCK_UART_H

#include <stdint.h>

#define BUFFER_SIZE 1024

void uart_transmit(const uint8_t data, size_t length);
uint8_t uart_receive(uint8_t *buffer, size_t buffer_size);

#endif