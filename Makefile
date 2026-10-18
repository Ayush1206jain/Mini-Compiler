# Makefile for Mini Compiler — Day 1
#
# This Makefile is provided as an alternative to CMake for environments
# where CMake is not installed. It uses g++ directly.
#
# Usage:
#   make              — Build both the compiler and tests
#   make mini_compiler — Build only the compiler
#   make lexer_tests  — Build only the tests
#   make test         — Build and run the tests
#   make run          — Build and run the compiler on the example file
#   make clean        — Remove built files

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
BUILD_DIR = build

# Default target: build everything
all: $(BUILD_DIR)/mini_compiler $(BUILD_DIR)/lexer_tests

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the main compiler executable
$(BUILD_DIR)/mini_compiler: src/main.cpp src/lexer.cpp include/token.hpp include/lexer.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) src/main.cpp src/lexer.cpp -o $(BUILD_DIR)/mini_compiler

# Build the test executable
$(BUILD_DIR)/lexer_tests: tests/lexer_tests.cpp src/lexer.cpp include/token.hpp include/lexer.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) tests/lexer_tests.cpp src/lexer.cpp -o $(BUILD_DIR)/lexer_tests

# Run tests
test: $(BUILD_DIR)/lexer_tests
	./$(BUILD_DIR)/lexer_tests

# Run the compiler on the example file
run: $(BUILD_DIR)/mini_compiler
	./$(BUILD_DIR)/mini_compiler examples/day1_example.mc

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test run clean
