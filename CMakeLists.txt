cmake_minimum_required(VERSION 3.10)
project(asm386 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.c
    src/assembler.c
    src/emit.c
    src/labels.c
    src/parser.c
    src/registers.c
    src/instructions.c
    src/directives.c
)

# Executable
add_executable(asm386 ${SOURCES})

# Installation
install(TARGETS asm386 DESTINATION bin)
