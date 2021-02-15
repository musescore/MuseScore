#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2014 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

set(MODULE qzip)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/qzip.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qzipreader_p.h
    ${CMAKE_CURRENT_LIST_DIR}/qzipwriter_p.h
    )

include(GetPlatformInfo)
include(GetCompilerInfo)

if (OS_IS_WIN)
    set(MODULE_INCLUDE ${PROJECT_SOURCE_DIR}/dependencies/include/zlib)
endif(OS_IS_WIN)

if (CC_IS_EMSCRIPTEN)
    #zlib included in main linker
elseif (NOT CC_IS_MSVC)
    set(MODULE_LINK z)
endif()

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

target_no_warning(${MODULE} -Wimplicit-fallthrough=0)
