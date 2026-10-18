cmake_minimum_required(VERSION 3.10)
project(mini_compiler LANGUAGES CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directory for header files
include_directories(include)

# --- Main executable ---
add_executable(mini_compiler
    src/main.cpp
    src/lexer.cpp
)

# --- Test executable ---
add_executable(lexer_tests
    tests/lexer_tests.cpp
    src/lexer.cpp
)

# Enable CTest
enable_testing()
add_test(NAME LexerTests COMMAND lexer_tests)
