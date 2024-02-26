#include "mock_uart.h"
#include <stdio.h>

uint8_t mock_tx_buffer[BUFFER_SIZE];
uint8_t mock_rx_buffer[BUFFER_SIZE];

void uart_transmit(const uint8_t data, size_t length) {
    printf("Emulating UART Transmit: %c\n", data);
}

void uart_receive(uint8_t *buffer, size_t buffer_size) {
    // Emulate UART reception (you can set a predefined value for testing)
    printf("Emulating UART Receive\n");
    return mock_receive_data;
}

// Set the mock receive data (useful for testing scenarios)
void set_mock_receive_data(uint8_t data) {
    mock_receive_data = data;
}