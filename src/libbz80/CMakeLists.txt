qt_add_library(libbz80 STATIC
    mmioDevice.hpp
    mmioRam.hpp
    mmioROM.hpp
    mmioDeviceManager.hpp
    registerpairtype.cpp
    registerpairtype.hpp
    registerinterface.hpp
    registerinterface.cpp
)

target_include_directories(libbz80 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
