cmake_minimum_required(VERSION 3.10)
project(zelda-lttp-decomp VERSION 1.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/main.cpp
    src/snes/hardware.cpp
    src/snes/ppu.cpp
    src/snes/dma.cpp
    src/snes/input.cpp
    src/game/state.cpp
    src/game/sprites.cpp
    src/game/collision.cpp
    src/game/room.cpp
    src/audio/apu.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Add any external libraries if needed (SDL2, etc.)
# find_package(SDL2 REQUIRED)
# target_link_libraries(${PROJECT_NAME} SDL2::SDL2)

# Print build info
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")