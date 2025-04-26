# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lyrics)

declare_module(iex_imagesexport)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/imagesexportmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imagesexportmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iimagesexportconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/imagesexportconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/imagesexportconfiguration.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractimagewriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractimagewriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/svgwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/svgwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/svggenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/svggenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pngwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pngwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pdfwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pdfwriter.h
)

set(MODULE_LINK
    engraving
    iex_lyrics
)

setup_module()
