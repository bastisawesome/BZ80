qt_add_executable(bz80
    main.cpp
    systeminterface.hpp
    systeminterface.cpp
)

target_link_libraries(bz80 PRIVATE
    libbz80
    Qt6::Core
)

set_target_properties(bz80 PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
