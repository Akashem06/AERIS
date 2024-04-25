#include "CppUTest/CommandLineTestRunner.h"

IMPORT_TEST_GROUP(dummy_test);

int main(int ac, char** av)
{
   return CommandLineTestRunner::RunAllTests(ac, av);
}