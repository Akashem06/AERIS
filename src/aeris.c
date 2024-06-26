#include "aeris.h"

#include <stdio.h>  // FOR DEBUGGING

#include "aeris_prv.h"

static aeris_state_data prv_aeris = {
    .config = NULL,
    .state = AERIS_STATE_UNINITIALIZED,
    .error = AERIS_ERR_NONE,
    .dfu_app_size = 0,
};

aeris_error aeris_bootloader_init(aeris_config *const config) {
    if (config == NULL || config->transmit_data == NULL || config->receive_data == NULL) {
        return AERIS_ERR_INVALID_ARGS;
    } else {
        prv_aeris.config = config;
        prv_aeris.config->pending_data = false;

        prv_aeris.state_request = AERIS_STATE_IDLE;
        prv_aeris.state = AERIS_STATE_IDLE;
        prv_aeris.error = AERIS_ERR_NONE;
        prv_aeris.message_error = AERIS_MSG_ERR_NONE;
        prv_aeris.app_failed_crc32 = false;
        prv_aeris.first_data_byte_received = false;
        memset(prv_aeris.config->message_buffer, 0, sizeof(prv_aeris.config->message_buffer));
        memset(prv_aeris.ack_message_buffer, 0, sizeof(prv_aeris.ack_message_buffer));
    }
    return AERIS_ERR_NONE;
}

// ---------------------------------------------------------------------------------------------- //
// ---------------------------------------- CHANGE STATE ---------------------------------------- //
// ---------------------------------------------------------------------------------------------- //

static aeris_error prv_aeris_bootloader_change_state(aeris_state current_state, aeris_state desired_state) {
    aeris_error ret = AERIS_ERR_NONE;

    switch (current_state) {
        case AERIS_STATE_IDLE:
            switch (desired_state) {
                case AERIS_STATE_JUMP_APP: {
                    prv_aeris.state = AERIS_STATE_JUMP_APP;
                } break;
                case AERIS_STATE_DFU: {
                    prv_aeris.state = AERIS_STATE_DFU;
                } break;
                default: {
                    ret = AERIS_ERR_INVALID_ARGS;
                } break;
            }
            break;

        case AERIS_STATE_DFU:
            switch (desired_state) {
                case AERIS_STATE_JUMP_APP: {
                    prv_aeris.state = AERIS_STATE_JUMP_APP;
                } break;
                default: {
                    ret = AERIS_ERR_INVALID_ARGS;
                } break;
            }
            break;

        case AERIS_STATE_JUMP_APP:
            if (desired_state == AERIS_STATE_IDLE) {
                prv_aeris.state = AERIS_STATE_IDLE;
            }
            break;

        default:
            ret = AERIS_ERR_INVALID_ARGS;
    }

    return ret;
}

// ---------------------------------------------------------------------------------------------- //
// ------------------------------------- DFU STATE FUNCTIONS ------------------------------------ //
// ---------------------------------------------------------------------------------------------- //

static aeris_error prv_aeris_bootloader_dfu_init(void) {
    aeris_error ret = AERIS_ERR_NONE;

    do {
        prv_aeris.bytes_written = 0;
        prv_aeris.first_data_byte_received = 0;
        // get the start and end addresses from NVM

        // Erase the flash
        ret = prv_aeris_bootloader_change_state(AERIS_STATE_IDLE, AERIS_STATE_DFU);
    } while (false);

    return ret;
}

static aeris_error prv_aeris_dfu_complete(void) {
    aeris_error ret = AERIS_ERR_NONE;
    do {
        ret = prv_aeris_bootloader_change_state(AERIS_STATE_DFU, AERIS_STATE_JUMP_APP);
    } while (false);

    return ret;
}

// ---------------------------------------------------------------------------------------------- //
// --------------------------------- MESSAGE HANDLING FUNCTIONS --------------------------------- //
// ---------------------------------------------------------------------------------------------- //

static aeris_message_t prv_aeris_bootloader_unpack_message(uint8_t *const buffer, size_t buffer_size) {
    aeris_message_t message = {0};
    message.packet_sof = buffer[0];
    message.packet_id = buffer[1];

    if (message.packet_id == AERIS_MESSAGE_TYPE_START) {
        message.packet_payload.start_packet.app_size |= ((uint32_t)buffer[2]);
        message.packet_payload.start_packet.app_size |= ((uint32_t)buffer[3]) << 8;
        message.packet_payload.start_packet.app_size |= ((uint32_t)buffer[4]) << 16;

        message.packet_payload.start_packet.app_crc |= ((uint32_t)buffer[5]);
        message.packet_payload.start_packet.app_crc |= ((uint32_t)buffer[6]) << 8;
        message.packet_payload.start_packet.app_crc |= ((uint32_t)buffer[7]) << 16;
        message.packet_payload.start_packet.app_crc |= ((uint32_t)buffer[8]) << 24;

        message.packet_eof = buffer[9];
        return message;
    }

    else if (prv_aeris.first_data_byte_received) {
        message.packet_id = AERIS_MESSAGE_TYPE_DATA;
        message.packet_payload.data_packet.app_data = buffer;
        if (prv_aeris.dfu_app_size - prv_aeris.bytes_written < AERIS_MAX_PACKET_SIZE) {
            prv_aeris.bytes_written += buffer_size - 1;
            message.packet_eof = buffer[buffer_size-1];
        }
        printf("bytes written %d\n", prv_aeris.bytes_written);
        return message;
    }

    else if (message.packet_id == AERIS_MESSAGE_TYPE_DATA) {
        prv_aeris.first_data_byte_received = true;
        message.packet_payload.data_packet.app_data = &buffer[2];
        // Data start + Data sizes
        switch (prv_aeris.dfu_app_size < AERIS_MAX_PACKET_SIZE) {
            case true: {
                prv_aeris.bytes_written += buffer_size - 3;
                message.packet_eof |= buffer[prv_aeris.dfu_app_size + 2];
            } break;
            case false: {
                prv_aeris.bytes_written += buffer_size - 2;
                printf("bytes written %d\n", prv_aeris.bytes_written);
            } break;
        }
        return message;
    }
    return message;
}

static aeris_error prv_aeris_bootloader_process_start_packet(const aeris_message_t *const message) {
    aeris_error ret = AERIS_ERR_NONE;

    do {
        // CHECKS SOF AND EOF FOR START PACKETS
        if (message->packet_sof != AERIS_SOF) {
            ret = AERIS_ERR_MSG_FAILURE;
            // Send Ack with error
            aeris_bootloader_ack_message(AERIS_MSG_ERR_INVALID_SOF, AERIS_ACK, prv_aeris.ack_message_buffer);
            break;
        }
        if (message->packet_eof != AERIS_EOF) {
            ret = AERIS_ERR_MSG_FAILURE;
            // Send Ack with error
            aeris_bootloader_ack_message(AERIS_MSG_ERR_INVALID_EOF, AERIS_ACK, prv_aeris.ack_message_buffer);
            break;
        }
        prv_aeris.dfu_app_size = message->packet_payload.start_packet.app_size;
        prv_aeris.dfu_app_crc = message->packet_payload.start_packet.app_crc;

        if (prv_aeris.dfu_app_size > AERIS_MAX_APP_SIZE) {
            ret = AERIS_ERR_MSG_FAILURE;
            // Ack wtih error
            aeris_bootloader_ack_message(AERIS_MSG_ERR_OVERSIZED, AERIS_ACK, prv_aeris.ack_message_buffer);
            break;
        }

        // check crc
        // make function to essentially memset the size into flash memory
        aeris_bootloader_ack_message(AERIS_MSG_ERR_NONE, AERIS_ACK, prv_aeris.ack_message_buffer);
        prv_aeris_bootloader_dfu_init();
    } while (false);

    return ret;
}

static aeris_error prv_aeris_bootloader_process_data_packet(const aeris_message_t *const message) {
    aeris_error ret = AERIS_ERR_NONE;

    do {
        if (!prv_aeris.first_data_byte_received && message->packet_sof != AERIS_SOF) {
            ret = AERIS_ERR_MSG_FAILURE;
            // Send Ack with error
            aeris_bootloader_ack_message(AERIS_MSG_ERR_INVALID_SOF, AERIS_ACK, prv_aeris.ack_message_buffer);
            break;
        }
        if (prv_aeris.dfu_app_size == prv_aeris.bytes_written && message->packet_eof != AERIS_EOF) {
            ret = AERIS_ERR_MSG_FAILURE;
            // Send Ack with error
            aeris_bootloader_ack_message(AERIS_MSG_ERR_INVALID_EOF, AERIS_ACK, prv_aeris.ack_message_buffer);
            break;
        }
        // uint8_t *data = message->packet_payload.data_packet.app_data;
        // make function that uses app_starting_addr, vector table offset + size.
        // make function that flashes
        // finally end by requesting JUMP TO APP
        if (prv_aeris.dfu_app_size == prv_aeris.bytes_written) {
            prv_aeris_dfu_complete();
        }

    } while (false);

    return ret;
}

// ---------------------------------------------------------------------------------------------- //
// ------------------------------------ STATE RUN FUNCTIONS ------------------------------------ //
// ---------------------------------------------------------------------------------------------- //

static aeris_error prv_aeris_bootloader_run_idle(void) {
    aeris_error ret = AERIS_ERR_NONE;
    memset(prv_aeris.ack_message_buffer, 0, sizeof(prv_aeris.ack_message_buffer));
    if (prv_aeris.config->pending_data) {
        do {
            // SHOULD ONLY RECEIVE START MSG. IF NOT ISSUE ERROR (START NOT RECEIVED)
            const aeris_message_t received_message = prv_aeris_bootloader_unpack_message(prv_aeris.config->message_buffer, AERIS_START_MESSAGE_SIZE);

            if (received_message.packet_id == AERIS_MESSAGE_TYPE_DATA) {
                ret = AERIS_ERR_MSG_FAILURE;
                // Send Ack with error
                aeris_bootloader_ack_message(AERIS_ERR_START_NOT_RECEIVED, AERIS_ACK, prv_aeris.ack_message_buffer);
                break;
            }

            if (received_message.packet_id == AERIS_MESSAGE_TYPE_START) {
                ret = prv_aeris_bootloader_process_start_packet(&received_message);
            } else {
                ret = AERIS_ERR_MSG_FAILURE;
                // Send Ack with error
                aeris_bootloader_ack_message(AERIS_MSG_ERR_INVALID_ID, AERIS_ACK, prv_aeris.ack_message_buffer);
                break;
            }

        } while (false);
    }
    return ret;
}

static aeris_error prv_aeris_bootloader_run_dfu(void) {
    aeris_error ret = AERIS_ERR_NONE;
    uint16_t data_read_size = AERIS_MAX_PACKET_SIZE;
    memset(prv_aeris.ack_message_buffer, 0, sizeof(prv_aeris.ack_message_buffer));

    if (prv_aeris.dfu_app_size < AERIS_MAX_PACKET_SIZE) {
        data_read_size = prv_aeris.dfu_app_size + 3;
    } else if (prv_aeris.dfu_app_size - prv_aeris.bytes_written < AERIS_MAX_PACKET_SIZE) {
        data_read_size = prv_aeris.dfu_app_size - prv_aeris.bytes_written + 1;
    } else {
        data_read_size = AERIS_MAX_PACKET_SIZE;
    }

    data_read_size = data_read_size == 0? AERIS_MAX_PACKET_SIZE : data_read_size;

    prv_aeris.config->receive_data(prv_aeris.config->message_buffer, data_read_size);
    printf("data_read_size %d\n", data_read_size);
    do {
        const aeris_message_t received_message =    
            prv_aeris_bootloader_unpack_message(prv_aeris.config->message_buffer, data_read_size);

        if (received_message.packet_id == AERIS_MESSAGE_ID_UNKNOWN || received_message.packet_id >= NUM_AERIS_MESSAGE_ID) {
            ret = AERIS_ERR_MSG_FAILURE;
            // Send Ack wtih error
            aeris_bootloader_ack_message(AERIS_MSG_ERR_INVALID_ID, AERIS_ACK, prv_aeris.ack_message_buffer);
            break;
        }

        switch (received_message.packet_id) {
            case (AERIS_MESSAGE_TYPE_START): {
                ret = prv_aeris_bootloader_process_start_packet(&received_message);
            } break;
            case (AERIS_MESSAGE_TYPE_DATA): {
                ret = prv_aeris_bootloader_process_data_packet(&received_message);
            } break;
            case (AERIS_MESSAGE_TYPE_ACK):
                break;
            default:
                break;
        }

        if (ret != AERIS_ERR_NONE) {
            break;
        }
        // At this point everything works! So no error should be present
        prv_aeris.error = AERIS_ERR_NONE;
        ret = aeris_bootloader_ack_message(AERIS_MSG_ERR_NONE, AERIS_ACK, prv_aeris.ack_message_buffer);

    } while (false);

    return ret;
}

static aeris_error prv_aeris_bootloader_jump_app(void) {
    // make user input mem location in config. Jump to that
    aeris_error ret = AERIS_ERR_NONE;
    return ret;
}

// ---------------------------------------------------------------------------------------------- //
// ---------------------------------- STATE HANDLING FUNCTIONS ---------------------------------- //
// ---------------------------------------------------------------------------------------------- //

static aeris_error prv_aeris_bootloader_run_state(aeris_state current_state) {
    aeris_error ret = AERIS_ERR_NONE;

    switch (current_state) {
        case (AERIS_STATE_IDLE): {
            ret = prv_aeris_bootloader_run_idle();
        } break;
        case (AERIS_STATE_DFU): {
            ret = prv_aeris_bootloader_run_dfu();
        } break;
        case (AERIS_STATE_JUMP_APP): {
            ret = prv_aeris_bootloader_jump_app();
        } break;
        default: {
            ret = AERIS_ERR_INVALID_STATE;
        } break;
    }
    return ret;
}

// ---------------------------------------------------------------------------------------------- //
// ---------------------------------- SENDING ACK/NAK MESSAGES ---------------------------------- //
// ---------------------------------------------------------------------------------------------- //

aeris_error aeris_bootloader_ack_message(aeris_message_error msg_error, aeris_message_status status, uint8_t *buffer) {
    aeris_error ret = AERIS_ERR_NONE;
    prv_aeris.message_error = msg_error;
    do {
        if (sizeof(buffer) < AERIS_ACK_MESSAGE_SIZE) {
            ret = AERIS_ERR_INVALID_ARGS;
            return ret;
        }
        memset(buffer, 0, AERIS_ACK_MESSAGE_SIZE);
        buffer[0] |= AERIS_SOF;
        buffer[1] |= AERIS_MESSAGE_TYPE_ACK;
        buffer[2] |= status;  // Should be 0 if its an ACK 1 if its a NAK

        memcpy(&buffer[3], &msg_error, 4);

    } while (false);
    prv_aeris.config->pending_data = false;
    prv_aeris.config->transmit_data(prv_aeris.ack_message_buffer, sizeof(prv_aeris.ack_message_buffer));
    return ret;
}

// ---------------------------------------------------------------------------------------------- //
// ----------------------------------- PRIMARY USER FUNCTIONS ----------------------------------- //
// ---------------------------------------------------------------------------------------------- //

aeris_error aeris_request_state(aeris_state desired_state) {
    if (desired_state == AERIS_STATE_UNINITIALIZED) {
        return AERIS_ERR_INVALID_ARGS;
    }
    prv_aeris.state_request = desired_state;

    return AERIS_ERR_NONE;
}

aeris_error aeris_bootloader_run(void) {
    aeris_error ret = AERIS_ERR_NONE;

    if (prv_aeris.state != prv_aeris.state_request) {
        do {
            // make the state change
            ret = prv_aeris_bootloader_change_state(prv_aeris.state, prv_aeris.state_request);

            if (ret != AERIS_ERR_NONE) {
                aeris_bootloader_ack_message(AERIS_MSG_ERR_INTERNAL_ERR, AERIS_NACK, prv_aeris.ack_message_buffer);
                break;
            }
        } while (0);
    }

    // MIGHT MOVE OUT OF THE IF STATEMENT
    ret = prv_aeris_bootloader_run_state(prv_aeris.state);

    prv_aeris.error = ret;

    return ret;
}

aeris_state aeris_get_state(void) { return prv_aeris.state; }

uint8_t aeris_get_message_error(void) { return prv_aeris.message_error; }
uint8_t aeris_get_error(void) { return prv_aeris.error; }
bool aeris_get_first_data_byte_received(void) { return prv_aeris.first_data_byte_received; }

// Custom CRC calculation

// static uint16_t prv_aeris_package_crc16(aeris_message_t *message, int nBytes) {
//     return 0;
// }

// static uint32_t prv_aeris_package_crc32(aeris_message_t *message, int nBytes) {
//     return 0;
// }
