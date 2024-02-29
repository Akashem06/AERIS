MAKEFILE_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

PROJECT_DIR := $(MAKEFILE_DIR)
SRC_DIR := $(PROJECT_DIR)src
INC_DIR := $(PROJECT_DIR)inc
UTILS_DIR := $(PROJECT_DIR)utils

TEST_DIR := $(PROJECT_DIR)test
TEST_INC_DIR := $(TEST_DIR)/inc
MOCK_DIR := $(TEST_DIR)/mock
BUILD_DIR := $(PROJECT_DIR)build
SCRIPTS_DIR := $(PROJECT_DIR)scripts
SHELL = /bin/bash

CC := gcc
CFLAGS := -Wall -Werror -I$(INC_DIR) -I$(TEST_INC_DIR) -I$(MOCK_DIR) -I$(CPPUTEST_HOME)/include --std=c99
LD_FLAGS := -L$(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt

C_SRCS := $(wildcard $(SRC_DIR)/*.c)
C_OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(filter %.c, $(C_SRCS)))

C_TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
C_TEST_OBJS := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/%.o, $(filter %.c, $(C_TEST_SRCS)))

MOCK_SRCS := $(wildcard $(MOCK_DIR)/*.c)
MOCK_OBJS := $(patsubst $(MOCK_DIR)/%.c, $(BUILD_DIR)/%.o, $(filter %.c, $(MOCK_SRCS)))

$(shell mkdir -p $(BUILD_DIR))

all: $(C_OBJS) $(C_TEST_OBJS) $(MOCK_OBJS)
	$(CC) $(C_OBJS) $(C_TEST_OBJS) $(MOCK_OBJS) -o $(BUILD_DIR)/aeris

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(MOCK_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

format:
	$(SCRIPTS_DIR)/format.sh

setup:
	chmod +x $(SCRIPTS_DIR)/format.sh
	chmod +x $(SCRIPTS_DIR)/setup.sh
	$(SCRIPTS_DIR)/setup.sh
