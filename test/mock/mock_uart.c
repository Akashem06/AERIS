#include "mock_uart.h"
#include <stdio.h>

uint8_t mock_tx_buffer[BUFFER_SIZE];
uint8_t mock_rx_buffer[BUFFER_SIZE];
static size_t mock_rx_index = 0;

void uart_transmit(const uint8_t data, size_t length) {
    printf("Emulating UART Transmit");
    memcpy(mock_tx_buffer, data, length);
}

void view_transmit_data(void) {
    prtinf("Reading...");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%02X ", mock_tx_buffer[i]);
    }
    printf("\n");
}

void uart_receive(uint8_t *buffer, size_t length) {
    printf("Emulating UART Receive\n");
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = mock_rx_buffer[mock_rx_index++];
        printf("%02X ", buffer[i]);
        
        if (mock_rx_index >= BUFFER_SIZE) {
            mock_rx_index = 0;
        }
    }
    printf("\n");
}

// Set the mock receive data. Typical parameter = tx buffer
void set_mock_receive_data(uint8_t data, size_t length) {
    memcpy(mock_rx_buffer, data, length);
    mock_rx_index = 0;
}