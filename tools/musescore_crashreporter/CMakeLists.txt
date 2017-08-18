include (${PROJECT_SOURCE_DIR}/build/gch.cmake)
if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
else (APPLE)
        set(INCS "")
endif (APPLE)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

add_executable(musescore_crashreporter
    crashReporter.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    )

target_link_libraries(musescore_crashreporter
    Qt5::Widgets
    Qt5::Network
    )

set_target_properties (musescore_crashreporter
    PROPERTIES
        COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Wno-invalid-pch -Wno-unknown-pragmas"
    )

xcode_pch(musescore_crashreporter all)

ADD_DEFINITIONS(-DUNICODE)

ADD_DEPENDENCIES(musescore_crashreporter mops1)
ADD_DEPENDENCIES(musescore_crashreporter mops2)
