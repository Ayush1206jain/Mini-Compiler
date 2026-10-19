cmake_minimum_required(VERSION 3.10)
project(mini_compiler LANGUAGES CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directory for header files
include_directories(include)

# Common compiler source files
set(COMPILER_SOURCES
    src/lexer.cpp
    src/parser.cpp
    src/ir.cpp
    src/ir_generator.cpp
    src/ir_interpreter.cpp
    src/ir_optimizer.cpp
)

# --- Main executable ---
add_executable(mini_compiler
    src/main.cpp
    ${COMPILER_SOURCES}
)

# --- Test executables ---
add_executable(lexer_tests
    tests/lexer_tests.cpp
    src/lexer.cpp
)

add_executable(parser_tests
    tests/parser_tests.cpp
    src/lexer.cpp
    src/parser.cpp
)

add_executable(ir_tests
    tests/ir_tests.cpp
    ${COMPILER_SOURCES}
)

add_executable(ir_optimizer_tests
    tests/ir_optimizer_tests.cpp
    ${COMPILER_SOURCES}
)

# Enable CTest
enable_testing()
add_test(NAME LexerTests COMMAND lexer_tests)
add_test(NAME ParserTests COMMAND parser_tests)
add_test(NAME IRTests COMMAND ir_tests)
add_test(NAME IROptimizerTests COMMAND ir_optimizer_tests)
