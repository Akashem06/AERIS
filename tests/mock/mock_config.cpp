#include "mock_config.hpp"

aeris_config default_test_config {
    .jump_app_if_dfu = true,
    .app_stack_start_addr = 0x8000000,
    .app_reset_handler_offset = 4,
    .transmit_data = uart_transmit,
    .receive_data = uart_receive
};
