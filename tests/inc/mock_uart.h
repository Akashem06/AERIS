#ifndef MOCK_UART_H
#define MOCK_UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "aeris.h"
#include "aeris_prv.h"

#define BUFFER_SIZE 1024

aeris_message_error uart_transmit(const uint8_t *data, size_t length);
aeris_message_error uart_receive(uint8_t *buffer, size_t buffer_size);

void set_mock_receive_data(uint8_t *data, size_t length);

#endif