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

set(Z_LIB )
if (CC_IS_MSVC)
    include(FindStaticLibrary)
    set(Z_LIB zlibstat)
    set(Z_INCLUDE ${PROJECT_SOURCE_DIR}/dependencies/include/zlib)
elseif (CC_IS_EMSCRIPTEN)
    #zlib included in main linker
else ()
    set(Z_LIB z)
endif ()

set(MODULE_INCLUDE
    ${Z_INCLUDE}
    )

set(MODULE_LINK
    ${Z_LIB}
    )

set(MODULE_NOT_LINK_GLOBAL ON)
include(SetupModule)

target_no_warning(${MODULE} -Wimplicit-fallthrough=0)
