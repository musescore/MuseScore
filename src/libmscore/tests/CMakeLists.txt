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

set(MODULE_TEST libmscore_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/testutils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/testutils.h
    ${CMAKE_CURRENT_LIST_DIR}/tst_barline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_beam.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_box.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_breath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_chordsymbol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_clef.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_clef_courtesy.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/tst_compat114.cpp fail
    # ${CMAKE_CURRENT_LIST_DIR}/tst_compat206.cpp fail
    ${CMAKE_CURRENT_LIST_DIR}/tst_concertpitchbenchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_copypaste.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_copypastesymbollist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_durationtype.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_dynamic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_earlymusic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_element.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_exchangevoices.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_hairpin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_implodeExplode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_instrumentchange.cpp
)

set(MODULE_TEST_LINK
    qzip
    libmscore
    fonts
    )

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(${PROJECT_SOURCE_DIR}/src/framework/testing/qtest.cmake)
