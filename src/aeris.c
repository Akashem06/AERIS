#include "aeris.h"

#include "aeris_prv.h"

static aeris_state_data prv_aeris = {
    .config = NULL,
    .state = AERIS_STATE_UNINITIALIZED,
    .error = AERIS_ERR_NONE,
    .dfu_app_size = 0,
};

aeris_error aeris_bootloader_init(aeris_config *const config) {
    if (config == NULL || config->uart_transmit == NULL || config->uart_receive == NULL) {
        return AERIS_ERR_INVALID_ARGS;
    } else {
        prv_aeris.config = config;
        prv_aeris.state = AERIS_STATE_IDLE;
        prv_aeris.error = AERIS_ERR_NONE;
        prv_aeris.message_error = AERIS_MSG_ERR_NONE;
        prv_aeris.app_failed_crc32 = 0;
        prv_aeris.message_failed_crc16 = 0;
        memset(prv_aeris.message_buffer, 0, sizeof(prv_aeris.message_buffer));
        memset(prv_aeris.ack_message_buffer, 0, sizeof(prv_aeris.ack_message_buffer));
    }
    return AERIS_ERR_NONE;
}

// ---------------------------------------------------------------------------------------------- //
// --------------------------------- MESSAGE HANDLING FUNCTIONS --------------------------------- //
// ---------------------------------------------------------------------------------------------- //

static aeris_message_t prv_aeris_bootloader_unpack_message(uint8_t *const buffer,
                                                           size_t buffer_size) {
    aeris_message_t message = {0};
    message.packet_sof = buffer[0];
    message.packet_id = buffer[1];
    // LITTLE ENDIAN
    switch (message.packet_id) {
        case AERIS_MESSAGE_TYPE_START: {
            message.packet_payload.start_packet.app_size |= ((uint32_t)buffer[2]);
            message.packet_payload.start_packet.app_size |= ((uint32_t)buffer[3]) << 8;
            message.packet_payload.start_packet.app_size |= ((uint32_t)buffer[4]) << 16;

            prv_aeris.dfu_app_size = message.packet_payload.start_packet.app_size;

            message.packet_payload.start_packet.app_crc |= ((uint32_t)buffer[5]);
            message.packet_payload.start_packet.app_crc |= ((uint32_t)buffer[6]) << 8;
            message.packet_payload.start_packet.app_crc |= ((uint32_t)buffer[7]) << 16;
            message.packet_payload.start_packet.app_crc |= ((uint32_t)buffer[8]) << 24;
            // TODO: Add crc16, buffer[9] and buffer[10]
            message.packet_eof = buffer[11];
        } break;
        case AERIS_MESSAGE_TYPE_DATA: {
            message.packet_payload.data_packet.app_data = &buffer[2];
            // Data start + Data size + crc16 size
            message.packet_eof |= buffer[2 + prv_aeris.dfu_app_size + 2];
        } break;
        case AERIS_MESSAGE_TYPE_ACK:
            break;
        default:
            break;
    }

    return message;
}

static aeris_error prv_aeris_bootloader_process_start_packet(const aeris_message_t *const message) {
    aeris_error ret = AERIS_ERR_NONE;

    do {
        if (message->packet_payload.start_packet.app_size > AERIS_MAX_APP_SIZE) {
            ret = AERIS_ERR_MSG_FAILURE;
            prv_aeris.message_error = AERIS_MSG_ERR_OVERSIZED;
            // Ack with error
            aeris_bootloader_ack_message(AERIS_MSG_ERR_OVERSIZED, AERIS_ACK,
                                         prv_aeris.ack_message_buffer);
            break;
        }
        prv_aeris.dfu_app_size = message->packet_payload.start_packet.app_size;
        prv_aeris.dfu_app_crc = message->packet_payload.start_packet.app_crc;

        // make function to essentially memset the size into flash memory

    } while (false);

    return ret;
}

static aeris_error prv_aeris_bootloader_process_data_packet(const aeris_message_t *const message) {
    aeris_error ret = AERIS_ERR_NONE;

    do {
        // uint8_t *data = message->packet_payload.data_packet.app_data;
        // make function that uses app_starting_addr, vector table offset + size.
        // make function that flashes
        // finally end by requesting JUMP TO APP

    } while (false);

    return ret;
}

// ---------------------------------------------------------------------------------------------- //
// ------------------------------------ STATE RUN FUNCTIONS ------------------------------------ //
// ---------------------------------------------------------------------------------------------- //

static aeris_error prv_aeris_bootloader_run_idle(void) {
    aeris_error ret = AERIS_ERR_NONE;
    return ret;
}

static aeris_error prv_aeris_bootloader_run_dfu(void) {
    aeris_error ret = AERIS_ERR_NONE;
    memset(prv_aeris.ack_message_buffer, 0, sizeof(prv_aeris.ack_message_buffer));

    if (ret == AERIS_ERR_NONE) {
        do {
            memset(prv_aeris.message_buffer, 0, sizeof(prv_aeris.message_buffer));

            ret = prv_aeris.config->uart_receive(prv_aeris.message_buffer,
                                                 sizeof(prv_aeris.message_buffer));
            if (ret != AERIS_ERR_NONE) {
                break;
            }
            const aeris_message_t received_message = prv_aeris_bootloader_unpack_message(
                prv_aeris.message_buffer, sizeof(prv_aeris.message_buffer));
            // ERROR HANDLE
            if (prv_aeris.dfu_app_size > AERIS_MAX_APP_SIZE) {
                ret = AERIS_ERR_MSG_FAILURE;
                prv_aeris.message_error = AERIS_MSG_ERR_OVERSIZED;
                // Ack wtih error
                aeris_bootloader_ack_message(AERIS_MSG_ERR_OVERSIZED, AERIS_ACK,
                                             prv_aeris.ack_message_buffer);
                break;
            }

            if (received_message.packet_sof != AERIS_SOF) {
                ret = AERIS_ERR_MSG_FAILURE;
                prv_aeris.message_error = AERIS_MSG_ERR_INVALID_SOF;
                // Send Ack wtih error
                aeris_bootloader_ack_message(AERIS_MSG_ERR_INVALID_SOF, AERIS_ACK,
                                             prv_aeris.ack_message_buffer);
                break;
            }
            if (received_message.packet_id < 0 ||
                received_message.packet_id >= NUM_AERIS_MESSAGE_ID) {
                ret = AERIS_ERR_MSG_FAILURE;
                prv_aeris.message_error = AERIS_MSG_ERR_INVALID_ID;
                // Send Ack wtih error
                aeris_bootloader_ack_message(AERIS_MSG_ERR_INVALID_ID, AERIS_ACK,
                                             prv_aeris.ack_message_buffer);
                break;
            }
            if (received_message.packet_eof != AERIS_EOF) {
                ret = AERIS_ERR_MSG_FAILURE;
                prv_aeris.message_error = AERIS_MSG_ERR_INVALID_EOF;
                // Send Ack wtih error
                aeris_bootloader_ack_message(AERIS_MSG_ERR_INVALID_EOF, AERIS_ACK,
                                             prv_aeris.ack_message_buffer);
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
            prv_aeris.message_error = AERIS_MSG_ERR_NONE;  // At this point everything works! So no
                                                           // error should be present
            ret = aeris_bootloader_ack_message(prv_aeris.message_error, AERIS_ACK,
                                               prv_aeris.ack_message_buffer);

        } while (false);
    }
    return ret;
}

static aeris_error prv_aeris_bootloader_jump_app(void) {
    aeris_error ret = AERIS_ERR_NONE;
    return ret;
}

// ---------------------------------------------------------------------------------------------- //
// ---------------------------------- STATE HANDLING FUNCTIONS ---------------------------------- //
// ---------------------------------------------------------------------------------------------- //

static aeris_error prv_aeris_bootloader_change_state(aeris_state current_state,
                                                     aeris_state desired_state) {
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
                case AERIS_STATE_IDLE: {
                    prv_aeris.state = AERIS_STATE_IDLE;
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

aeris_error aeris_bootloader_ack_message(aeris_message_error msg_error, aeris_message_status status,
                                         uint8_t *buffer) {
    aeris_error ret = AERIS_ERR_NONE;
    do {
        if (buffer == NULL || sizeof(buffer) < AERIS_ACK_MESSAGE_SIZE) {
            ret = AERIS_ERR_INVALID_ARGS;
            return ret;
        }
        memset(buffer, 0, AERIS_ACK_MESSAGE_SIZE);
        buffer[0] |= AERIS_SOF;
        buffer[1] |= AERIS_MESSAGE_TYPE_ACK;
        buffer[2] |= status;  // Should be 0 if its an ACK 1 if its a NAK

        memcpy(&buffer[3], &msg_error, 4);

    } while (false);
    prv_aeris.config->uart_transmit(prv_aeris.ack_message_buffer,
                                    sizeof(prv_aeris.ack_message_buffer));
    return ret;
}

// ---------------------------------------------------------------------------------------------- //
// ----------------------------------- PRIMARY USER FUNCTIONS ----------------------------------- //
// ---------------------------------------------------------------------------------------------- //

aeris_error aeris_request_state(aeris_state desired_state) {
    aeris_error ret = AERIS_ERR_NONE;

    do {
        if (desired_state < 0 || desired_state >= NUM_AERIS_STATES) {
            ret = AERIS_ERR_INVALID_ARGS;
            break;
        }
        prv_aeris.state_request = desired_state;
    } while (false);

    return ret;
}

aeris_error aeris_bootloader_run(void) {
    aeris_error ret = AERIS_ERR_NONE;

    if (prv_aeris.state != prv_aeris.state_request) {
        do {
            // make the state change
            ret = prv_aeris_bootloader_change_state(prv_aeris.state, prv_aeris.state_request);

            if (ret != AERIS_ERR_NONE) {
                aeris_bootloader_ack_message(AERIS_MSG_ERR_INTERNAL_ERR, AERIS_NACK,
                                             prv_aeris.ack_message_buffer);
                break;
            }

            // run the new state
            // MIGHT MOVE OUT OF THE IF STATEMENT
            ret = prv_aeris_bootloader_run_state(prv_aeris.state);

            if (ret != AERIS_ERR_NONE) {
                aeris_bootloader_ack_message(AERIS_MSG_ERR_INTERNAL_ERR, AERIS_NACK,
                                             prv_aeris.ack_message_buffer);
                break;
            }

        } while (0);
    }

    return ret;
}

aeris_state aeris_get_state(void) { return prv_aeris.state; }

// Custom CRC calculation

// static uint16_t prv_aeris_package_crc16(aeris_message_t *message, int nBytes) {
//     return 0;
// }

// static uint32_t prv_aeris_package_crc32(aeris_message_t *message, int nBytes) {
//     return 0;
// }