#include "mock_uart.h"
#include "mock_config.h"

// rx buffer = message buffer
uint8_t mock_tx_buffer[BUFFER_SIZE] = {0};
uint8_t mock_rx_buffer[BUFFER_SIZE] = {0};
static size_t mock_rx_index = 0;

aeris_message_error uart_transmit(const uint8_t *data, size_t length) {
    printf("MOCK_UART: Emulating UART Transmit\n");
    memcpy(mock_tx_buffer, data, length);
    return AERIS_MSG_ERR_NONE;
}

void view_transmit_data(void) {
    printf("MOCK_UART: Reading...");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%02X ", mock_tx_buffer[i]);
    }
    printf("\n");
}

aeris_message_error uart_receive(uint8_t *buffer, size_t length) {
    printf("MOCK_UART: Emulating UART Receive ");
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = mock_rx_buffer[mock_rx_index++];
        printf("%02X ", buffer[i]);

        if (mock_rx_index >= BUFFER_SIZE) {
            mock_rx_index = 0;
        }
    }
    printf("\n");
    default_test_config.pending_data = true;
    return AERIS_MSG_ERR_NONE;
}

void set_mock_receive_data(uint8_t *data, size_t length) {
    memcpy(mock_rx_buffer, data, length);
    mock_rx_index = 0;
}