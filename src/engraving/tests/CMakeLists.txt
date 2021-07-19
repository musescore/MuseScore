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
    ${CMAKE_CURRENT_LIST_DIR}/testbase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/testbase.h
    ${CMAKE_CURRENT_LIST_DIR}/tst_all_elements_layout_elements.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/tst_all_elements_tree_model.cpp # fail
    ${CMAKE_CURRENT_LIST_DIR}/tst_barline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_beam.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_box.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_breath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_chordsymbol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_clef.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_clef_courtesy.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/tst_compat114.cpp # fail
    # ${CMAKE_CURRENT_LIST_DIR}/tst_compat206.cpp # fail
    ${CMAKE_CURRENT_LIST_DIR}/tst_concertpitchbenchmark.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_copypaste.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_copypastesymbollist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_durationtype.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_dynamic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_earlymusic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_element.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/tst_exchangevoices.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_hairpin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_implodeExplode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_instrumentchange.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_join.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/tst_keysig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_layout_benchmark.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/tst_links.cpp # fail
#    ${CMAKE_CURRENT_LIST_DIR}/tst_measure.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/tst_midi.cpp not ported
    # ${CMAKE_CURRENT_LIST_DIR}/tst_midimapping.cpp not ported
    ${CMAKE_CURRENT_LIST_DIR}/tst_note.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/tst_parts.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_readwriteundoreset.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_remove.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/tst_repeat.cpp # fail
    ${CMAKE_CURRENT_LIST_DIR}/tst_rhythmicGrouping.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/tst_selectionfilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_selectionrangedelete.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/tst_spanners.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/tst_split.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_splitstaff.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/tst_text.cpp not actual, not compile
    ${CMAKE_CURRENT_LIST_DIR}/tst_timesig.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/tst_tools.cpp # fail
    # ${CMAKE_CURRENT_LIST_DIR}/tst_transpose.cpp # fail
    # ${CMAKE_CURRENT_LIST_DIR}/tst_tuplet.cpp # fail
    # ${CMAKE_CURRENT_LIST_DIR}/tst_unrollrepeats.cpp # fail
    ${CMAKE_CURRENT_LIST_DIR}/tst_utils.cpp
)

set(MODULE_TEST_LINK
    qzip
    engraving
    fonts
    )

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(${PROJECT_SOURCE_DIR}/src/framework/testing/qtest.cmake)
