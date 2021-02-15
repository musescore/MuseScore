
# Compiler definition
# MuseScore-specific, defines typical configurations

include(GetPlatformInfo)

if (OS_IS_WASM)

    set(CC_IS_EMSCRIPTEN 1)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")

    set(CC_IS_CLANG 1)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    if (OS_IS_WIN)

        set(CC_IS_MINGW 1)

    else(OS_IS_WIN)

        set(CC_IS_GCC 1)

    endif(OS_IS_WIN)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

  set(CC_IS_MSVC 1)

else()
    message(FATAL_ERROR "Unsupported compiller: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Define MINGW for VS, as it appears not to be defined
if (MSVC)
   set (MINGW false)
endif (MSVC)
