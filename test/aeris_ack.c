#include "../inc/aeris_prv.h"
#include "../inc/aeris.h"

#include "../inc/unity.h"


void uart_transmit(const uint8_t* data, size_t size) {} 


aeris_config mock_config = {
    .uart_transmit = uart_transmit
};

void setUp(void) {}

void tearDown(void) {}

void test_aeris_bootloader_ack_message_valid(void) {
    uint8_t buffer[AERIS_ACK_MESSAGE_SIZE];
    aeris_error result = aeris_bootloader_ack_message(NULL, AERIS_ACK, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL(AERIS_ERR_NONE, result);
}

void test_aeris_bootloader_ack_message_invalid_buffer(void) {
    uint8_t buffer[AERIS_ACK_MESSAGE_SIZE - 1]; d
    aeris_error result = aeris_bootloader_ack_message(NULL, AERIS_ACK, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL(AERIS_ERR_INVALID_ARGS, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_aeris_bootloader_ack_message_valid);
    RUN_TEST(test_aeris_bootloader_ack_message_invalid_buffer);
    return UNITY_END();
}
