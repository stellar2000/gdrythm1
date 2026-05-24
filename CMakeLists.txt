cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(GDOverlayBridge VERSION 1.0.0)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK.")
else()
    message(STATUS "Found Geode SDK: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode-sdk)

file(GLOB SOURCES
    src/*.cpp
)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} geode-sdk)
setup_geode_mod(${PROJECT_NAME})
