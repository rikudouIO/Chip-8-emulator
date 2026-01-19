cmake_minimum_required(VERSION 3.28)
project(chip8)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Os -s -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,--strip-all")

file(GLOB SOURCES "*.cpp")

add_executable(chip8 ${SOURCES})