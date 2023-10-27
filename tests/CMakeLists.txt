cmake_minimum_required(VERSION 3.5)

project(Bz80Tests LANGUAGES CXX)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Bz80Tests tst_mmioramtest.cpp)
add_test(NAME MmioRamTest COMMAND MmioRamTest)

target_link_libraries(Bz80Tests
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    libbz80
)
