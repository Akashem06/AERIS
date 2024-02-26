#include "test_settings.hpp"

const aeris_config default_config {
    .jump_app_if_dfu = true,
    .app_stack_start_addr = 0x08010000,
    .app_reset_handler_offset = 4,
    .aeris_custom_uart_transmit = ,
    .aeris_custom_uart_receive = 
};