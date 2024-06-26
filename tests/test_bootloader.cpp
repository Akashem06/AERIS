#include <CppUTestExt/MockSupport.h>

#include "CppUTest/TestHarness.h"

extern "C" {
#include "aeris.h"
#include "aeris_prv.h"
#include "mock_config.h"
}

// ALL MESSAGES IN LITTLE ENDIAN

// START MESSAGES
// SOF, ID, 128b IN HEX (3BYTES), 12345 IN HEX (4BYTES), EOF
uint8_t test_valid_start_message[10] = {0xAA, 0x01, 0x80, 0x00, 0x00, 0x39, 0x30, 0x00, 0x00, 0xBB};
// ID, 128kb IN HEX (3BYTES), 12345 IN HEX (4BYTES), 123 IN HEX (2BYTES), EOF
uint8_t test_sof_start_message[10] = {0x00, 0x01, 0x80, 0x00, 0x00, 0x39, 0x30, 0x00, 0x00, 0xBB};
// SOF, ID, 128kb IN HEX (3BYTES), 12345 IN HEX (4BYTES), 123 IN HEX (2BYTES)
uint8_t test_eof_start_message[10] = {0xAA, 0x01, 0x80, 0x00, 0x00, 0x39, 0x30, 0x00, 0x00, 0xAA};
// SOF, ID, 16777215b IN HEX (3BYTES), 12345 IN HEX (4BYTES), 123 IN HEX (2BYTES), EOF
uint8_t test_oversized_start_message[10] = {0xAA, 0x01, 0xFF, 0xFF, 0xFF, 0x39, 0x30, 0x00, 0x00, 0xBB};

uint8_t test_valid_short_data_message[130];

// SOF, ID, 913b IN HEX (3BYTES), 12345 IN HEX (4BYTES), EOF
uint8_t test_valid_long_start_message[10] = {0xAA, 0x01, 0x91, 0x03, 0x00, 0x39, 0x30, 0x00, 0x00, 0xBB};
uint8_t test_valid_long_data_message[915];

void fill_random_data(uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buffer[i] = rand() % 256; // Generate a random number between 0 and 255
    }
}

TEST_GROUP(test_bootloader_state_machine){

    void setup(){
        mock().clear();
    }

    void teardown() { 
        mock().clear();
    }
};

TEST(test_bootloader_state_machine, test_bootloader_enters_dfu_after_start_message) {
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_bootloader_init(&default_test_config));
    
    // Bootloader is off after init
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state());

    // Send start msg that has an oversized app size
    set_mock_receive_data(test_oversized_start_message, 10);
    uart_receive(default_test_config.message_buffer, 10);
    
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_OVERSIZED, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_MSG_FAILURE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state());

    // Send start msg that has no sof
    set_mock_receive_data(test_sof_start_message, 10);
    uart_receive(default_test_config.message_buffer, 10);
    
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_INVALID_SOF, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_MSG_FAILURE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state());

    // Send start msg that has no eof
    set_mock_receive_data(test_eof_start_message, 10);
    uart_receive(default_test_config.message_buffer, 10);
    
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_INVALID_EOF, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_MSG_FAILURE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state());

    // Send valid start msg but pending data is not set
    set_mock_receive_data(test_valid_start_message, 10);
    uart_receive(default_test_config.message_buffer, 10);
    default_test_config.pending_data = false;
    
    aeris_bootloader_run();
    
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state()); // NOTHING SHOULD HAPPEN

    // Send valid start message
    set_mock_receive_data(test_valid_start_message, 10);
    uart_receive(default_test_config.message_buffer, 10);
    
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_NONE, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_DFU, aeris_get_state());
    CHECK_EQUAL(false, aeris_get_first_data_byte_received());
}

TEST(test_bootloader_state_machine, test_bootloader_dfu_short_data_receiving) {
    // Send valid start message
    set_mock_receive_data(test_valid_start_message, 10);
    uart_receive(default_test_config.message_buffer, 10);
    
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_NONE, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_DFU, aeris_get_state());
    CHECK_EQUAL(false, aeris_get_first_data_byte_received());

    test_valid_short_data_message[0] = 0xAA;
    test_valid_short_data_message[1] = 0x02;
    test_valid_short_data_message[130] = 0xBB;
    fill_random_data(&test_valid_short_data_message[2], 128);
    set_mock_receive_data(test_valid_short_data_message, 131);
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_NONE, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_JUMP_APP, aeris_get_state());
    CHECK_EQUAL(true, aeris_get_first_data_byte_received()); // fix
}

TEST(test_bootloader_state_machine, test_bootloader_dfu_long_data_receiving) {
    // Send valid start message
    set_mock_receive_data(test_valid_long_start_message, 10);
    uart_receive(default_test_config.message_buffer, 10);
    
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_NONE, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_DFU, aeris_get_state());
    CHECK_EQUAL(false, aeris_get_first_data_byte_received());

    test_valid_long_data_message[0] = 0xAA;
    test_valid_long_data_message[1] = 0x02;
    test_valid_long_data_message[915] = 0xBB;
    fill_random_data(&test_valid_long_data_message[2], 913);
    set_mock_receive_data(test_valid_long_data_message, 512);

    while (aeris_get_state() == AERIS_STATE_DFU) {
        aeris_bootloader_run();
        set_mock_receive_data(&test_valid_long_data_message[512], 512);
    };

    CHECK_EQUAL(AERIS_MSG_ERR_NONE, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_JUMP_APP, aeris_get_state());
    CHECK_EQUAL(true, aeris_get_first_data_byte_received());
}

TEST(test_bootloader_state_machine, test_bootloader_is_off_after_init) {
    // Bootloader is not initialized before init
    CHECK_EQUAL(AERIS_STATE_UNINITIALIZED, aeris_get_state());

    // Init function does not return an error
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_bootloader_init(&default_test_config));
    
    // Bootloader is off after init
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state());
}
