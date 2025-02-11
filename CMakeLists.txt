cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GitVersion)

get_version_from_git()

set(VERSION_HEADER_TEMPLATE "${CMAKE_SOURCE_DIR}/version.hpp.in")
set(VERSION_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/version.hpp")

configure_file(
    ${VERSION_HEADER_TEMPLATE}
    ${VERSION_HEADER_FILE}
)

add_custom_target(version_update DEPENDS ${VERSION_HEADER_FILE})

include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

project(BZ80 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS -Wunused)

find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup()

add_subdirectory(src/bz80)
add_subdirectory(src/libbz80)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(tests)
endif()

enable_testing(true)

add_custom_target(${CMAKE_PROJECT_NAME} version_update)
