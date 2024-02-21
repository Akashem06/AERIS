#include "aeris.h"
#include "aeris_prv.h"

static aeris_state_data prv_aeris = {
    .config = NULL,
    .state = AERIS_STATE_UNINITIALIZED,
    .error = AERIS_ERR_NONE,
    .dfu_app_size = 0
};

aeris_error aeris_bootloader_init(aeris_config const *const config) {
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

aeris_error aeris_bootloader_run(void) {
    aeris_error ret = AERIS_ERR_NONE;

    do {
    // make the state change
    ret = aeris_bootloader_change_state(prv_aeris.state, prv_aeris.state_request);

    if (ret != AERIS_ERR_NONE) {
        break;
    }

    // run the new state
    aeris_bootloader_run_state(prv_aeris.state);
    } while (0);

    return ret;
}

aeris_error aeris_request_state(aeris_state_request desired_state) {
    aeris_error ret = AERIS_ERR_NONE;

    do {
        if (desired_state < 0 || desired_state >= NUM_AERIS_STATE_REQUESTS) {
            ret = AERIS_ERR_INVALID_ARGS;
            break;
        }
        prv_aeris.state_request = desired_state;
    } while (false);
    
    return ret;
}

aeris_state aeris_get_state(void) {return prv_aeris.state;}

aeris_error aeris_bootloader_change_state(aeris_state current_state, aeris_state_request desired_state) {
    aeris_error ret = AERIS_ERR_NONE;

    switch (current_state) {
        case AERIS_STATE_IDLE:
            switch (desired_state) {
                case AERIS_REQUEST_JUMP_APP: {
                    prv_aeris.state = AERIS_STATE_JUMP_APP;
                } break;
                case AERIS_REQUEST_DFU: {
                    prv_aeris.state = AERIS_STATE_DFU;
                } break;
                default: {
                    ret = AERIS_ERR_INVALID_ARGS;
                } break;
            }
            break;

        case AERIS_STATE_DFU:
            switch (desired_state) {
                case AERIS_REQUEST_JUMP_APP: {
                    prv_aeris.state = AERIS_STATE_JUMP_APP;
                } break;
                case AERIS_REQUEST_IDLE: {
                    prv_aeris.state = AERIS_STATE_IDLE;
                } break;
                default: {
                    ret = AERIS_ERR_INVALID_ARGS;
                } break;
            }
            break;

        case AERIS_STATE_JUMP_APP:
            if (desired_state == AERIS_REQUEST_IDLE) {
                prv_aeris.state = AERIS_STATE_IDLE;
            }
            break;

        default:
            ret = AERIS_ERR_INVALID_ARGS;
    }

    return ret;
}

aeris_error aeris_bootloader_run_state(aeris_state current_state) {
    aeris_error ret = AERIS_ERR_NONE;
    
    switch (current_state) {
        case (AERIS_STATE_IDLE): {
            ret = aeris_bootloader_run_idle();
        } break;
        case (AERIS_STATE_DFU): {
            ret = aeris_bootloader_run_dfu();
        } break;
        case (AERIS_STATE_JUMP_APP): {
            ret = aeris_bootloader_jump_app();
        } break;
        default: {
            ret = AERIS_ERR_INVALID_STATE;
        } break;
    }
    return ret;
}

aeris_error aeris_bootloader_run_idle(void) {

}

aeris_error aeris_bootloader_run_dfu(void) {
    aeris_error ret = AERIS_ERR_NONE;
    aeris_message_error message_error = AERIS_MSG_ERR_NONE;
    aeris_message_t received_message;
    memset(prv_aeris.ack_message_buffer, 0, sizeof(prv_aeris.ack_message_buffer));
    
    if (ret == AERIS_ERR_NONE) {
        do {
            memset(prv_aeris.message_buffer, 0, sizeof(prv_aeris.message_buffer));

            ret = prv_aeris.config->uart_receive(prv_aeris.message_buffer, sizeof(prv_aeris.message_buffer));
            if (ret != AERIS_ERR_NONE) {
                break;
            } 
            aeris_message_t received_message = aeris_bootloader_unpack_message(prv_aeris.message_buffer, sizeof(prv_aeris.message_buffer));
            if (prv_aeris.dfu_app_size > AERIS_MAX_APP_SIZE) {
                ret = AERIS_ERR_MSG_FAILURE;
                message_error = AERIS_MSG_ERR_OVERSIZED;
                break;
            }

            if (received_message.packet_sof != AERIS_SOF) {
                ret = AERIS_ERR_MSG_FAILURE;
                message_error = AERIS_MSG_ERR_INVALID_SOF;
                break;
            }
            if (received_message.packet_id < 0 || received_message.packet_id >= NUM_AERIS_MESSAGE_ID) {
                ret = AERIS_ERR_MSG_FAILURE;
                message_error = AERIS_MSG_ERR_INVALID_ID;
                break;
            }
            if (received_message.packet_eof != AERIS_EOF) {
                ret = AERIS_ERR_MSG_FAILURE;
                message_error = AERIS_MSG_ERR_INVALID_EOF;
                break;
            }

            switch(received_message.packet_id) {
                case (AERIS_MESSAGE_TYPE_START): {

                } break;
            }

        } while (false);
    }
    prv_aeris.message_error = message_error;
    return ret;
}

aeris_error aeris_bootloader_run_jump_app(void) {}


aeris_message_t aeris_bootloader_unpack_message(uint8_t *const buffer, size_t buffer_size) {
    aeris_message_t message = {0};
    message.packet_sof = buffer[0];
    message.packet_id = buffer[1];

    switch (message.packet_id) {
        case AERIS_MESSAGE_TYPE_START: {
            message.packet_payload.start_packet.app_size |= ((uint32_t) buffer[2]) << 16;
            message.packet_payload.start_packet.app_size |= ((uint32_t) buffer[3]) << 8;
            message.packet_payload.start_packet.app_size |= ((uint32_t) buffer[4]);
            
            prv_aeris.dfu_app_size = message.packet_payload.start_packet.app_size;

            message.packet_payload.start_packet.app_crc |= ((uint32_t) buffer[5]) << 24;
            message.packet_payload.start_packet.app_crc |= ((uint32_t) buffer[6]) << 16;
            message.packet_payload.start_packet.app_crc |= ((uint32_t) buffer[7]) << 8;
            message.packet_payload.start_packet.app_crc |= ((uint32_t) buffer[8]);
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

// Custom CRC calculation

static uint16_t prv_aeris_package_crc16(aeris_message_t *message, int nBytes) {

} 

static uint32_t prv_aeris_package_crc32(aeris_message_t *message, int nBytes) {
    
} 