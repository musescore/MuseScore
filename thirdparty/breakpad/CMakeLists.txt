#cmake_minimum_required(VERSION 2.8.12)

#project(qt_example LANGUAGES CXX)

include (${PROJECT_SOURCE_DIR}/build/gch.cmake)
if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
else (APPLE)
        set(INCS "")
endif (APPLE)

add_library (
  breakpads STATIC
  ${PROJECT_BINARY_DIR}/all.h
  ${INCS}
  crash_handler.cpp
  crash_handler.h
  client/windows/handler/exception_handler.cc
  common/windows/string_utils.cc
  common/windows/guid_string.cc
  client/windows/crash_generation/crash_generation_client.cc
  crash_handler.h
  crash_handler.cpp
  common/windows/string_utils-inl.h
  common/windows/guid_string.h
  client/windows/handler/exception_handler.h
  client/windows/common/ipc_protocol.h
  google_breakpad/common/minidump_format.h
  google_breakpad/common/breakpad_types.h
  client/windows/crash_generation/crash_generation_client.h
  common/scoped_ptr.h
  )


  
xcode_pch(breakpads all)


ADD_DEFINITIONS(-DUNICODE)

ADD_DEPENDENCIES(breakpads mops1)
ADD_DEPENDENCIES(breakpads mops2)

#set(CMAKE_BUILD_TYPE DEBUG)


