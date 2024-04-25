#include <CppUTestExt/MockSupport.h>

#include "CppUTest/TestHarness.h"
#include "mock_config.hpp"

extern "C" {
#include "aeris.h"
#include "aeris_prv.h"
}

TEST_GROUP(test_bootloader_state_machine){

    void setup(){
        mock().clear();
    }

    void teardown() { 
        mock().clear();
    }
};

TEST(test_bootloader_state_machine, test_bootloader_is_off_after_init) {
    // Bootloader is not initialized before init
    CHECK_EQUAL(AERIS_STATE_UNINITIALIZED, aeris_get_state());

    // Init function does not return an error
    CHECK_EQUAL(AERIS_ERR_NONE, aeris_bootloader_init(&default_test_config));

    // Bootloader is off after init
    CHECK_EQUAL(AERIS_STATE_IDLE, aeris_get_state());
}