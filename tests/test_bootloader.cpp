#include <CppUTestExt/MockSupport.h>

#include "CppUTest/TestHarness.h"

extern "C" {
#include "aeris.h"
#include "aeris_prv.h"
#include "mock_config.h"
}

// little endian
// SOF, ID, 128kb IN HEX (3BYTES), 12345 IN HEX (4BYTES), 123 IN HEX (2BYTES), EOF
uint8_t test_valid_start_message[12] = {0xAA, 0x01, 0x80, 0x00, 0x00, 0x39, 0x30, 0x00, 0x00, 0x00, 0x7B, 0xBB};

// ID, 128kb IN HEX (3BYTES), 12345 IN HEX (4BYTES), 123 IN HEX (2BYTES), EOF
uint8_t test_sof_start_message[12] = {0x00, 0x01, 0x80, 0x00, 0x00, 0x39, 0x30, 0x00, 0x00, 0x00, 0x7B, 0xBB};
// SOF, ID, 128kb IN HEX (3BYTES), 12345 IN HEX (4BYTES), 123 IN HEX (2BYTES)
uint8_t test_eof_start_message[12] = {0xAA, 0x01, 0x80, 0x00, 0x00, 0x39, 0x30, 0x00, 0x00, 0x00, 0x7B, 0xAA};

// SOF, ID, 512kb IN HEX (3BYTES), 12345 IN HEX (4BYTES), 123 IN HEX (2BYTES), EOF
uint8_t test_oversized_start_message[12] = {0xAA, 0x01, 0x00, 0x20, 0x00, 0x39, 0x30, 0x00, 0x00, 0x00, 0x7B, 0xBB};

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
    set_mock_receive_data(test_oversized_start_message, 12);
    uart_receive(default_test_config.message_buffer, 12);
    default_test_config.pending_data = true;
    
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_OVERSIZED, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state());

    // Send start msg that has no sof
    set_mock_receive_data(test_sof_start_message, 12);
    uart_receive(default_test_config.message_buffer, 12);
    default_test_config.pending_data = true;
    
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_INVALID_SOF, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state());

    // Send start msg that has no eof
    set_mock_receive_data(test_eof_start_message, 12);
    uart_receive(default_test_config.message_buffer, 12);
    default_test_config.pending_data = true;
    
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_INVALID_EOF, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state());

    set_mock_receive_data(test_valid_start_message, 12);
    uart_receive(default_test_config.message_buffer, 12);
    default_test_config.pending_data = true;
    
    aeris_bootloader_run();

    CHECK_EQUAL(AERIS_MSG_ERR_NONE, aeris_get_message_error());
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_get_error());
    CHECK_EQUAL(AERIS_STATE_DFU, aeris_get_state());
}

TEST(test_bootloader_state_machine, test_bootloader_is_off_after_init) {
    // Bootloader is not initialized before init
    CHECK_EQUAL(AERIS_STATE_UNINITIALIZED, aeris_get_state());

    // Init function does not return an error
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_bootloader_init(&default_test_config));
    
    // Bootloader is off after init
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state());
}
