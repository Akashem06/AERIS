#ifndef MOCK_UART_H
#define MOCK_UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

void uart_transmit(const uint8_t data, size_t length);
void uart_receive(uint8_t *buffer, size_t buffer_size);

#endif