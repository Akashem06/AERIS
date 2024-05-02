#ifndef AERIS_H
#define AERIS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define AERIS_SOF (0xAA)  // Start of Frame byte
#define AERIS_EOF (0xBB)  // End of Frame byte
#define AERIS_ACK_MESSAGE_SIZE 8U

#define AERIS_START_MESSAGE_SIZE 12U

#define AERIS_MAX_APP_SIZE 16000000 // 16 MB
#define AERIS_MAX_PACKET_SIZE 512

// Status code
typedef enum {
    AERIS_ACK,
    AERIS_NACK
} aeris_message_status;

// Error data [FOR STATE CHANGES]
typedef enum {
    AERIS_ERR_NONE,
    AERIS_ERR_INVALID_ARGS,
    AERIS_ERR_INVALID_STATE,
    AERIS_ERR_MSG_FAILURE,
    AERIS_ERR_OUT_OF_RANGE,
    AERIS_ERR_START_NOT_RECEIVED
} aeris_error;

// Error data [FOR MESSAGES]
typedef enum {
    AERIS_MSG_ERR_NONE,
    AERIS_MSG_ERR_INVALID_SOF,
    AERIS_MSG_ERR_INVALID_EOF,
    AERIS_MSG_ERR_INVALID_ID,
    AERIS_MSG_ERR_OVERSIZED,
    AERIS_MSG_DATA_TRANSMISSION,
    AERIS_MSG_ERR_INTERNAL_ERR
} aeris_message_error;

// STATE MACHINE
typedef enum {
    AERIS_STATE_UNINITIALIZED,
    AERIS_STATE_IDLE,
    AERIS_STATE_DFU,
    AERIS_STATE_JUMP_APP,
    NUM_AERIS_STATES
} aeris_state;

typedef enum {
    AERIS_STATE_DFU_START,
    AERIS_STATE_DFU_DATA,
    AERIS_STATE_DFU_COMPLETE,
    NUM_AERIS_DFU_STATES
} aeris_dfu_states;

typedef aeris_message_error (*aeris_custom_transmit)(const uint8_t *data, size_t length);
typedef aeris_message_error (*aeris_custom_receive)(uint8_t *buffer, size_t buffer_size);
// typedef uintptr_t aeris_bootloader_addr_size_t;
// typedef uint32_t aeris_bootloader_crc;
// typedef uint32_t aeris_bootloader_size;

typedef struct {
    bool jump_app_if_dfu;
    uint32_t app_stack_start_addr;
    uint8_t app_reset_handler_offset;
    aeris_custom_transmit transmit_data;
    aeris_custom_receive receive_data;

    bool pending_data;
    uint8_t message_buffer[AERIS_MAX_PACKET_SIZE];
} aeris_config;

/**
 * @brief Initialize the aeris data struct. Uninitialized -> Idle
 * @returns AERIS_ERR_NONE if success, AERIS_ERR_INVALID_ARGS if failure
 */
aeris_error aeris_bootloader_init(aeris_config *const config);

/**
 * @brief Requests for the bootloader to change states
 * @param The desired state for the bootloader
 * @returns AERIS_ERR_NONE if success, AERIS_ERR_INVALID_ARGS if failure
 */
aeris_error aeris_request_state(aeris_state desired_state);

/**
 * @brief Runs the state and handles state changes
 * @returns AERIS_ERR_NONE if success, relevant AERIS_ERR if failure
 */
aeris_error aeris_bootloader_run(void);

/**
 * @brief Retrieves current state
 * @returns Gets the current state of the bootloader
 */
aeris_state aeris_get_state(void);

/**
 * @brief Retrieves most recent message error
 * @returns Returns message error
 */
uint8_t aeris_get_message_error(void);

/**
 * @brief Retrieves state machines error
 * @returns Returns state error
 */
uint8_t aeris_get_error(void);

/**
 * @brief Retrieves if first data byte received
 * @returns Returns if first data byte received
 */
bool aeris_get_first_data_byte_received(void);

/**
 * @brief Sends ACK message
 * @returns AERIS_ERR_NONE if success, relevant AERIS_ERR_INVALID_ARGS if failure
 */
aeris_error aeris_bootloader_ack_message(aeris_message_error msg_error, aeris_message_status status, uint8_t *buffer);

#endif