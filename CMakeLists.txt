cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(level_song_offset_2 VERSION 1.0.0)

# Add all source files inside src (recursively)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Set up the mod binary
add_library(${PROJECT_NAME} SHARED ${SOURCES}
        src/data/OffsetData.hpp
        src/offset/OffsetTracker.cpp
        src/offset/OffsetTracker.hpp
        src/offset/QueuedMusicTracker.cpp
        src/offset/QueuedMusicTracker.hpp
        src/hooks/GJBaseGameLayerHooks.cpp
        src/hooks/GJBaseGameLayerHooks.cpp)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

# Add debug switches to control debug logs
if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(RELWITHDEBINFO_BUILD)
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Set up dependencies, resources, and link Geode.
setup_geode_mod(${PROJECT_NAME})
