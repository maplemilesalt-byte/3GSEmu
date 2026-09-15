cmake_minimum_required(VERSION 3.16)
project(3GSEmu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(3gs-emu
    src/main.cpp
    src/cpu/armv7.cpp
    src/memory/memory.cpp
)

target_include_directories(3gs-emu PRIVATE src)

if (MSVC)
    target_compile_options(3gs-emu PRIVATE /W4)
else()
    target_compile_options(3gs-emu PRIVATE -Wall -Wextra -Wpedantic)
endif()
