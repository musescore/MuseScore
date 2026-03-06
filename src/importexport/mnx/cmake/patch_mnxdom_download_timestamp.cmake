cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED mnxdom_SOURCE_DIR)
    message(FATAL_ERROR "mnxdom_SOURCE_DIR is not set")
endif()

set(_target_file "${mnxdom_SOURCE_DIR}/CMakeLists.txt")
if(NOT EXISTS "${_target_file}")
    message(FATAL_ERROR "mnxdom patch: file not found: ${_target_file}")
endif()

file(READ "${_target_file}" _content)
set(_original "${_content}")

string(REPLACE "        DOWNLOAD_EXTRACT_TIMESTAMP TRUE\n" "" _content "${_content}")
string(REPLACE "        DOWNLOAD_EXTRACT_TIMESTAMP TRUE\r\n" "" _content "${_content}")

if(_content STREQUAL _original)
    message(STATUS "mnxdom patch: DOWNLOAD_EXTRACT_TIMESTAMP lines already removed")
else()
    file(WRITE "${_target_file}" "${_content}")
    message(STATUS "mnxdom patch: removed DOWNLOAD_EXTRACT_TIMESTAMP from mnxdom CMakeLists.txt")
endif()
