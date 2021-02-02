#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2020 MuseScore BVBA and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

# We are separating the importexport module into separate modules,
# soon there will be nothing left of it
set(MODULE importexport)

include(${CMAKE_CURRENT_LIST_DIR}/internal/ove/ove.cmake)

set(MODULE_SRC
    ${OVE_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/importexportmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/importexportmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iimportexportconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importexportconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importexportconfiguration.h
    
    ${CMAKE_CURRENT_LIST_DIR}/internal/overeader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/overeader.h
    
    ${CMAKE_CURRENT_LIST_DIR}/internal/svgwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/svgwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/svggenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/svggenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pngwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pngwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pdfwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pdfwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mp3writer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mp3writer.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/wavewriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/wavewriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/oggwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/oggwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/flacwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/flacwriter.h
    )

set(MODULE_LINK
    rtf2html    # for capella
    libmscore
    qzip
    notation
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

