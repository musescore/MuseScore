#cmake_minimum_required(VERSION 2.8.12)

#project(breakpad)

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
    common/windows/http_upload.h
    common/windows/http_upload.cc
    common/windows/string_utils-inl.h
    common/windows/guid_string.h
    client/windows/handler/exception_handler.h
    google_breakpad/common/minidump_format.h
    google_breakpad/common/breakpad_types.h
    client/windows/crash_generation/crash_generation_client.h
    common/scoped_ptr.h
)

target_link_libraries(breakpad
    "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.14393.0/um/x86/wininet.lib"
    )


set_target_properties ( breakpad
    PROPERTIES
        COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch -Wno-unknown-pragmas"
    )

xcode_pch(breakpad all)

ADD_DEFINITIONS(-DUNICODE)

ADD_DEPENDENCIES(breakpad mops1)
ADD_DEPENDENCIES(breakpad mops2)

add_executable(crashReporter crashReporter.cpp)


set_target_properties ( crashReporter
    PROPERTIES
        COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch -Wno-unknown-pragmas"
    )



