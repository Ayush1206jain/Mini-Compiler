# Makefile for Mini Compiler — Day 4
#
# Usage:
#   make              — Build everything (compiler and all tests)
#   make mini_compiler — Build the compiler binary
#   make test         — Build and run all test suites
#   make clean        — Clean build artifacts

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
BUILD_DIR = build

SOURCES = src/lexer.cpp src/parser.cpp src/ir.cpp src/ir_generator.cpp src/ir_interpreter.cpp src/ir_optimizer.cpp

all: $(BUILD_DIR)/mini_compiler $(BUILD_DIR)/lexer_tests $(BUILD_DIR)/parser_tests $(BUILD_DIR)/ir_tests $(BUILD_DIR)/ir_optimizer_tests

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/mini_compiler: src/main.cpp $(SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) src/main.cpp $(SOURCES) -o $(BUILD_DIR)/mini_compiler

$(BUILD_DIR)/lexer_tests: tests/lexer_tests.cpp src/lexer.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) tests/lexer_tests.cpp src/lexer.cpp -o $(BUILD_DIR)/lexer_tests

$(BUILD_DIR)/parser_tests: tests/parser_tests.cpp src/lexer.cpp src/parser.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) tests/parser_tests.cpp src/lexer.cpp src/parser.cpp -o $(BUILD_DIR)/parser_tests

$(BUILD_DIR)/ir_tests: tests/ir_tests.cpp $(SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) tests/ir_tests.cpp $(SOURCES) -o $(BUILD_DIR)/ir_tests

$(BUILD_DIR)/ir_optimizer_tests: tests/ir_optimizer_tests.cpp $(SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) tests/ir_optimizer_tests.cpp $(SOURCES) -o $(BUILD_DIR)/ir_optimizer_tests

test: $(BUILD_DIR)/lexer_tests $(BUILD_DIR)/parser_tests $(BUILD_DIR)/ir_tests $(BUILD_DIR)/ir_optimizer_tests
	./$(BUILD_DIR)/lexer_tests
	./$(BUILD_DIR)/parser_tests
	./$(BUILD_DIR)/ir_tests
	./$(BUILD_DIR)/ir_optimizer_tests

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean
