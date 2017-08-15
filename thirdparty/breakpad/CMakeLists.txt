include (${PROJECT_SOURCE_DIR}/build/gch.cmake)
if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
else (APPLE)
        set(INCS "")
endif (APPLE)

add_library (
    breakpad STATIC
    ${PROJECT_BINARY_DIR}/all.h
    ${INCS}
    crash_handler.cpp
    crash_handler.h
    client/windows/common/ipc_protocol.h
    client/windows/handler/exception_handler.cc
    common/windows/string_utils.cc
    common/windows/guid_string.cc
    client/windows/crash_generation/crash_generation_client.cc
    crash_handler.h
    crash_handler.cpp
    common/windows/string_utils-inl.h
    common/windows/guid_string.h
    client/windows/handler/exception_handler.h
    google_breakpad/common/minidump_format.h
    google_breakpad/common/breakpad_types.h
    client/windows/crash_generation/crash_generation_client.h
    common/scoped_ptr.h
)


set_target_properties ( breakpad
    PROPERTIES
        COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch -Wno-unknown-pragmas"
    )

xcode_pch(breakpad all)

ADD_DEFINITIONS(-DUNICODE)

ADD_DEPENDENCIES(breakpad mops1)
ADD_DEPENDENCIES(breakpad mops2)

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
        COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch -Wno-unknown-pragmas"
    )

xcode_pch(musescore_crashreporter all)

ADD_DEPENDENCIES(musescore_crashreporter mops1)
ADD_DEPENDENCIES(musescore_crashreporter mops2)
