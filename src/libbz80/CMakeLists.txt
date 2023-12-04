qt_add_library(libbz80 STATIC
    mmioDevice.hpp
    mmioRam.hpp
    mmioROM.hpp
    mmioDeviceManager.hpp
    registerpairtype.cpp
    registerpairtype.hpp
    z80basecpu.hpp
    z80basecpu.cpp
    z80BaseCpuInstructions.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Core)
target_link_libraries(libbz80 PRIVATE Qt6::Core)

target_include_directories(libbz80 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
