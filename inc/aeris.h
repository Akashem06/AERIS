#ifndef AERIS_H
#define AERIS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define AERIS_SOF (0xAA)    // Start of Frame byte
#define AERIS_EOF (0xBB)    // End of Frame byte
#define AERIS_ACK_MESSAGE_SIZE 8U

#define AERIS_MAX_APP_SIZE 1024

// Status code
typedef enum {
  AERIS_ACK,     
	AERIS_NACK,   
} aeris_message_status;

// Error data [FOR STATE CHANGES]
typedef enum {
  AERIS_ERR_NONE,
  AERIS_ERR_INVALID_ARGS,
  AERIS_ERR_INVALID_STATE,
  AERIS_ERR_MSG_FAILURE,
  AERIS_ERR_OUT_OF_RANGE
} aeris_error;

// Error data [FOR MESSAGES]
typedef enum {
  AERIS_MSG_ERR_NONE,
  AERIS_MSG_ERR_INVALID_SOF,
  AERIS_MSG_ERR_INVALID_EOF,
  AERIS_MSG_ERR_INVALID_ID,
  AERIS_MSG_ERR_OVERSIZED,
  AERIS_MSG_DATA_TRANSMISSION,
  AERIS_MSG_ERR_INTERNAL_ERR,
} aeris_message_error;

// STATE MACHINE
typedef enum {
  AERIS_STATE_UNINITIALIZED,
  AERIS_STATE_IDLE,
  AERIS_STATE_DFU,
  AERIS_STATE_JUMP_APP
} aeris_state;

// STATE CHANGE REQUESTS
typedef enum {
  AERIS_REQUEST_UNINITIALIZED, // User should never request this
  AERIS_REQUEST_IDLE,
  AERIS_REQUEST_DFU,
  AERIS_REQUEST_JUMP_APP,
  NUM_AERIS_STATE_REQUESTS
} aeris_state_request;

typedef aeris_message_error (*aeris_custom_uart_transmit) (const uint8_t *data, size_t length);
typedef aeris_message_error (*aeris_custom_uart_receive) (uint8_t *buffer, size_t buffer_size);
// typedef uintptr_t aeris_bootloader_addr_size_t;
// typedef uint32_t aeris_bootloader_crc;
// typedef uint32_t aeris_bootloader_size;

typedef struct {
  bool jump_app_if_dfu;
  uint32_t app_stack_start_addr;
  uint8_t app_reset_handler_offset;
  aeris_custom_uart_transmit uart_transmit;
  aeris_custom_uart_receive uart_receive;
} aeris_config;

/**
 * @brief Initialize the aeris data struct. Uninitialized -> Idle
 * @returns AERIS_ERR_NONE if success, AERIS_ERR_INVALID_ARGS if failure
 */
aeris_error aeris_bootloader_init(void);

/**
 * @brief Recieves data using custom communication protcol defined in aeris_config
 * @param Takes in pointer to recieving buffer and the size of buffer
 * @returns AERIS_ERR_NONE if success, AERIS_EX_ERR otherwise
 */
aeris_error aeris_bootloader_receive(uint8_t *buffer, size_t buffer_size);

/**
 * @brief Interpets the recieved data and causes state change to occur
 * @returns AERIS_ERR_NONE if success, AERIS_EX_ERR otherwise
 */
aeris_error aeris_bootloader_process(uint8_t *buffer, size_t buffer_size);

/**
 * @brief Initialize the OTA process by downloading and flashing
 * @returns AERIS_ERR_NONE if success, AERIS_EX_ERR otherwise
 */
aeris_error aeris_bootloader_run_dfu(void);

/**
 * @brief Initialize the OTA process by downloading and flashing
 * @returns AERIS_EX_OK if success, AERIS_EX_ERR otherwise
 */
aeris_error AERIS_download_and_flash(void);

#endif