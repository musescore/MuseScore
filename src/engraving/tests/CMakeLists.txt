# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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

set(MODULE_TEST engraving_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp

    ${CMAKE_CURRENT_LIST_DIR}/msczfile_tests.cpp

    ${CMAKE_CURRENT_LIST_DIR}/utils/scorerw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/scorerw.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/scorecomp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/scorecomp.h

    ${CMAKE_CURRENT_LIST_DIR}/barline_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/beam_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/box_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/breath_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/chordsymbol_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/clef_courtesy_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/clef_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat114_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat206_tests.cpp
    #${CMAKE_CURRENT_LIST_DIR}/concertpitch_tests.cpp doesn't compile and needs actualization
    ${CMAKE_CURRENT_LIST_DIR}/copypaste_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/copypastesymbollist_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/durationtype_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dynamic_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/earlymusic_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/element_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exchangevoices_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/expression_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/hairpin_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/implodeexplode_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instrumentchange_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/join_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/keysig_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layoutelements_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/links_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/measure_tests.cpp
    #${CMAKE_CURRENT_LIST_DIR}/midimapping_tests.cpp doesn't compile and needs actualization
    ${CMAKE_CURRENT_LIST_DIR}/note_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/parts_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pitchwheelrender_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbackeventsrendering_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbackmodel_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/readwriteundoreset_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/remove_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/repeat_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rhythmicgrouping_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/scantree_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/selectionfilter_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/selectionrangedelete_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spanners_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/split_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/splitstaff_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tempomap_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/textbase_tests.cpp
    #${CMAKE_CURRENT_LIST_DIR}/textedit_tests.cpp doesn't compile and needs actualization
    ${CMAKE_CURRENT_LIST_DIR}/timesig_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tools_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transpose_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tuplet_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/unrollrepeats_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/changevisibility_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/midirenderer_tests.cpp
   

    ${CMAKE_CURRENT_LIST_DIR}/mocks/engravingconfigurationmock.h
)

set(MODULE_TEST_LINK
    engraving
    fonts
)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(${PROJECT_SOURCE_DIR}/src/framework/testing/gtest.cmake)
