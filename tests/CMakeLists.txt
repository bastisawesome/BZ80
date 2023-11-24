cmake_minimum_required(VERSION 3.5)

project(Bz80Tests LANGUAGES CXX)

enable_testing()

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro(addtest test_name)
    string(TOLOWER ${test_name} source_name)
    add_executable(${test_name}
        tst_${source_name}.cpp
    )

    add_test(NAME ${test_name} COMMAND ${test_name})

    target_link_libraries(${test_name}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        libbz80
    )
endmacro()

addtest(MmioRamTest)
addtest(MmioRomTest)
addtest(MmioDeviceManager)
addtest(RegisterPairType)
addtest(Z80BaseCpuInstructions)
