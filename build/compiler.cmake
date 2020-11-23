
# Compiler definition
# MuseScore-specific, defines typical configurations

include(${CMAKE_CURRENT_LIST_DIR}/platform.cmake)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

    set(COMPILER_IS_CLANG 1)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    if (PLATFORM_IS_WINDOWS)

        set(COMPILER_IS_MINGW 1)

    else(PLATFORM_IS_WINDOWS)

        set(COMPILER_IS_GCC 1)

    endif(PLATFORM_IS_WINDOWS)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

  set(COMPILER_IS_MSVC 1)

endif()
