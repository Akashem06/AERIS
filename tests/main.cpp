#include "CppUTest/CommandLineTestRunner.h"

IMPORT_TEST_GROUP(dummy_test);
IMPORT_TEST_GROUP(test_bootloader_state_machine);

int main(int ac, char** av)
{
   return CommandLineTestRunner::RunAllTests(ac, av);
}