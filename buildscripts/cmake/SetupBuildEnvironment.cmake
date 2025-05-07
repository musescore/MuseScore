# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

include(GetCompilerInfo)
include(GetBuildType)

# Unity build defaults
if(NOT DEFINED CMAKE_UNITY_BUILD_BATCH_SIZE)
    set(CMAKE_UNITY_BUILD_BATCH_SIZE 12)
endif()

# Debug/no-debug options
add_compile_definitions($<$<CONFIG:Debug>:QT_QML_DEBUG>)

add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:QT_NO_DEBUG>)

# String debug hack
if(MUSE_COMPILE_STRING_DEBUG_HACK AND CC_IS_CLANG)
    add_compile_definitions($<$<CONFIG:Debug>:MUSE_STRING_DEBUG_HACK>)
endif()

# Shared libraries
set(BUILD_SHARED_LIBS OFF)
set(SHARED_LIBS_INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

if(MUSE_COMPILE_USE_SHARED_LIBS_IN_DEBUG AND BUILD_IS_DEBUG)
    if(CC_IS_GCC OR CC_IS_MINGW)
        set(BUILD_SHARED_LIBS ON)
    endif()
endif()

# Address Sanitizer
if(MUSE_COMPILE_ASAN)
    if(CC_IS_CLANG OR CC_IS_GCC OR CC_IS_MINGW)
        add_compile_options("-fsanitize=address")
        add_compile_options("-fno-omit-frame-pointer")
        add_link_options("-fsanitize=address")
    elseif(CC_IS_MSVC)
        add_compile_options("/fsanitize=address")
    endif()
endif()

# Mac-specific
if(OS_IS_MAC)
    set(MACOSX_DEPLOYMENT_TARGET 10.15)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
endif(OS_IS_MAC)

# MSVC-specific
if(CC_IS_MSVC)
    add_compile_options("/EHsc")
    add_compile_options("/utf-8")
    add_compile_options("/MP")
    add_compile_options("/bigobj")

    add_compile_definitions(WIN32 _WINDOWS)
    add_compile_definitions(_UNICODE UNICODE)
    add_compile_definitions(_USE_MATH_DEFINES)
    add_compile_definitions(NOMINMAX)
    
    add_link_options("/NODEFAULTLIB:LIBCMT")
endif()

# MinGW-specific
if(CC_IS_MINGW)
    # https://musescore.org/node/22048
    add_compile_options(-mno-ms-bitfields)

    if(NOT MUSE_COMPILE_BUILD_64)
        add_link_options("-Wl,--large-address-aware")
    endif()

    add_compile_definitions(_UNICODE UNICODE)
endif()

# Wasm-specific
if(CC_IS_EMCC)

    # set(EMCC_COMPILE_FLAGS "--bind -o .html --preload-file ../../files")

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/public_html)

    set(EMCC_COMPILE_FLAGS "-s USE_ZLIB=1")
    if (NOT BUILD_IS_DEBUG)
        set(EMCC_COMPILE_FLAGS "${EMCC_COMPILE_FLAGS} -O3")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMCC_COMPILE_FLAGS}")

    #!! NOTE Should be set as Qt6::Platform PROPERTIES INTERFACE_LINK_OPTIONS
    #!! see SetupQt.cmake

    set(EMCC_LINKER_FLAGS "SHELL:-s ASYNCIFY;SHELL:-s USE_ZLIB=1;SHELL:-s USE_LIBPNG=1;SHELL:-s ENVIRONMENT=web;SHELL:-s WASM=1;SHELL:-s EXIT_RUNTIME=1;SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=1;SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=[UTF16ToString,stringToUTF16];SHELL:-s USE_WEBGL2=1;--bind;SHELL:-s FETCH=1;SHELL:-s INITIAL_MEMORY=32MB;SHELL:-s FULL_ES2=1;SHELL:-s DISABLE_EXCEPTION_CATCHING=1;SHELL:-s ALLOW_MEMORY_GROWTH=1;")

    # debug
    if (BUILD_IS_DEBUG)
        set(EMCC_LINKER_FLAGS "${EMCC_LINKER_FLAGS}SHELL:-s DEMANGLE_SUPPORT=1;SHELL:-s GL_DEBUG=1;SHELL:-s ASSERTIONS=2;--profiling-funcs")
    # release
    else()
        # 95.1 Mb
        set(EMCC_LINKER_FLAGS "${EMCC_LINKER_FLAGS}-O2;-g0;")
        # 94.3 Mb
        #set(EMCC_LINKER_FLAGS "${EMCC_LINKER_FLAGS}-Oz;-g0;-flto;SHELL:--closure 0;SHELL:-s ASSERTIONS=0")
    endif()

endif(CC_IS_EMCC)

# Warnings
include(SetupCompileWarnings)

# User overrides
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/SetupBuildEnvironment.user.cmake")
    include(${CMAKE_CURRENT_LIST_DIR}/SetupBuildEnvironment.user.cmake)
endif()
