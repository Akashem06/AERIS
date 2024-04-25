MAKEFILE_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

PROJECT_DIR := $(MAKEFILE_DIR)
SRC_DIR := $(PROJECT_DIR)src
INC_DIR := $(PROJECT_DIR)inc
UTILS_DIR := $(PROJECT_DIR)utils

TEST_DIR := $(PROJECT_DIR)tests
TEST_INC_DIR := $(TEST_DIR)/inc
MOCK_DIR := $(TEST_DIR)/mock
BUILD_DIR := $(PROJECT_DIR)build
SCRIPTS_DIR := $(PROJECT_DIR)scripts

CPPUTEST_HOME := $(PROJECT_DIR)cpputest
CPPUTEST_INCLUDE := $(CPPUTEST_HOME)/include
CPPUTEST_LIB := $(CPPUTEST_HOME)/lib

CC := gcc
CXX := g++
COMMON_FLAGS := -Wall -Werror -I$(INC_DIR) -I$(TEST_INC_DIR) -I$(MOCK_DIR) -I$(CPPUTEST_INCLUDE)
CFLAGS := $(COMMON_FLAGS) --std=c99
CXXFLAGS := $(COMMON_FLAGS) --std=c++20
LD_FLAGS = -L$(CPPUTEST_LIB) -lCppUTest -lCppUTestExt

C_SRCS := $(wildcard $(SRC_DIR)/*.c)
C_OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(filter %.c, $(C_SRCS)))

CPP_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(filter %.cpp, $(CPP_SRCS)))

C_TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
C_TEST_OBJS := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/%.o, $(filter %.c, $(C_TEST_SRCS)))

CPP_TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
CPP_TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(filter %.cpp, $(CPP_TEST_SRCS)))


MOCK_SRCS := $(wildcard $(MOCK_DIR)/*.c) $(wildcard $(MOCK_DIR)/*.cpp)
MOCK_OBJS := $(patsubst $(MOCK_DIR)/%.c,$(BUILD_DIR)/%.o,$(filter %.c, $(MOCK_SRCS))) \
             $(patsubst $(MOCK_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(filter %.cpp, $(MOCK_SRCS)))

$(shell mkdir -p $(BUILD_DIR))

all: $(C_TEST_OBJS) $(CPP_TEST_OBJS) $(C_OBJS) $(CPP_OBJS) $(MOCK_OBJS)
	@echo "Running tests..."
	@$(CXX) $(CXXFLAGS) $^ -o $(BUILD_DIR)/aeris $(LD_FLAGS)
	@echo "Test results:"
	@$(BUILD_DIR)/aeris

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(MOCK_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(MOCK_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
clean:
	rm -rf $(BUILD_DIR)

setup:
	chmod +x $(SCRIPTS_DIR)/setup.sh
	chmod +x $(SCRIPTS_DIR)/format.sh
	chmod +x $(SCRIPTS_DIR)/linter.sh
	$(SCRIPTS_DIR)/setup.sh

format:
	$(SCRIPTS_DIR)/format.sh

lint:
	$(SCRIPTS_DIR)/linter.sh

