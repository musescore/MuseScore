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

muse_create_module(notation)

target_sources(notation PRIVATE
    notationmodule.cpp
    notationmodule.h

    imasternotation.h
    iexcerptnotation.h
    inotation.h
    inotationpainting.h
    inotationviewstate.h
    inotationsolomutestate.h
    inotationnoteinput.h
    inotationselection.h
    inotationselectionfilter.h
    inotationinteraction.h
    internal/inotationundostack.h
    inotationstyle.h
    inotationaccessibility.h
    inotationmidiinput.h
    notationtypes.h
    inotationconfiguration.h
    notationerrors.h
    inotationplayback.h
    inotationelements.h
    inotationparts.h
    iinstrumentsrepository.h

    internal/excerptnotation.cpp
    internal/excerptnotation.h
    internal/igetscore.h
    internal/inotationselectionrange.h
    internal/instrumentsrepository.cpp
    internal/instrumentsrepository.h
    internal/masternotation.cpp
    internal/masternotation.h
    internal/masternotationparts.cpp
    internal/masternotationparts.h
    internal/mscnotationwriter.cpp
    internal/mscnotationwriter.h
    internal/mscoreerrorscontroller.cpp
    internal/mscoreerrorscontroller.h
    internal/notation.cpp
    internal/notation.h
    internal/notationaccessibility.cpp
    internal/notationconfiguration.cpp
    internal/notationconfiguration.h
    internal/notationelements.cpp
    internal/notationelements.h
    internal/notationinteraction.cpp
    internal/notationinteraction.h
    internal/notationmidiinput.cpp
    internal/notationmidiinput.h
    internal/notationnoteinput.cpp
    internal/notationnoteinput.h
    internal/notationpainting.cpp
    internal/notationpainting.h
    internal/notationparts.cpp
    internal/notationparts.h
    internal/notationselection.cpp
    internal/notationselection.h
    internal/notationselectionfilter.cpp
    internal/notationselectionfilter.h
    internal/notationselectionrange.cpp
    internal/notationselectionrange.h
    internal/notationsolomutestate.cpp
    internal/notationsolomutestate.h
    internal/notationstyle.cpp
    internal/notationstyle.h
    internal/notationundostack.cpp
    internal/notationundostack.h
    internal/notationviewstate.cpp
    internal/notationviewstate.h
    internal/positionswriter.cpp
    internal/positionswriter.h
    internal/scorecallbacks.cpp
    internal/scorecallbacks.h
    internal/searchcommandsparser.cpp
    internal/searchcommandsparser.h

    utilities/scorerangeutilities.cpp
    utilities/scorerangeutilities.h
)

if(MUE_BUILD_ENGRAVING_PLAYBACK)
    target_sources(notation PRIVATE
        internal/notationplayback.cpp
        internal/notationplayback.h
    )
    target_compile_definitions(notation PRIVATE MUE_BUILD_ENGRAVING_PLAYBACK)
else()
    target_sources(notation PRIVATE
        internal/notationplaybackstub.cpp
        internal/notationplaybackstub.h
    )
endif()

if(NOT OS_IS_WASM)
    target_sources(notation PRIVATE
        internal/engravingfontscontroller.cpp
        internal/engravingfontscontroller.h
    )
    target_compile_definitions(notation PRIVATE MUE_BUILD_ENGRAVING_FONTSCONTROLLER)
endif()

target_link_libraries(notation PRIVATE
    muse::draw
    engraving
)

if(MUE_BUILD_NOTATION_TESTS)
    add_subdirectory(tests)
endif()
