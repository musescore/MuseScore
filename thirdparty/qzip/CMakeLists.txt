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

if (APPLE)
    file(GLOB_RECURSE INCS "*.h")
else (APPLE)
    set(INCS "")
endif (APPLE)

add_library(qzip STATIC
    qzip.cpp
    ${INCS}
    )

if (MSVC)
    target_include_directories(qzip PRIVATE "${PROJECT_SOURCE_DIR}/dependencies/include/zlib")
endif (MSVC)

if (BUILD_PCH)
    target_link_libraries(qzip global)
    target_precompile_headers(qzip REUSE_FROM global)
endif (BUILD_PCH)
