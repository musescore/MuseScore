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

set(PLAYBACK_SRC
    ${CMAKE_CURRENT_LIST_DIR}/renderingcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/playbackcontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbackcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/playbackmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbackmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/playbackeventsrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbackeventsrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/playbacksetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbacksetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/renderers/renderbase.h
    ${CMAKE_CURRENT_LIST_DIR}/renderers/ornamentsrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderers/ornamentsrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/renderers/glissandosrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderers/glissandosrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/renderers/arpeggiorenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderers/arpeggiorenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/renderers/gracechordcontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderers/gracechordcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/renderers/tremolorenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderers/tremolorenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/renderers/chordarticulationsrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderers/chordarticulationsrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/renderers/noterenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderers/noterenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/renderers/bendsrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderers/bendsrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/symbolsmetaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/symbolsmetaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/annotationsmetaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/annotationsmetaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/spannersmetaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/spannersmetaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/tremolometaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/tremolometaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/arpeggiometaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/arpeggiometaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/gracenotesmetaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/gracenotesmetaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/chordlinemetaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/internal/chordlinemetaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/notearticulationsparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/notearticulationsparser.h
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/chordarticulationsparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/chordarticulationsparser.h
    ${CMAKE_CURRENT_LIST_DIR}/metaparsers/metaparserbase.h
    ${CMAKE_CURRENT_LIST_DIR}/mapping/setupresolverbase.h
    ${CMAKE_CURRENT_LIST_DIR}/mapping/keyboardssetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mapping/keyboardssetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/mapping/stringssetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mapping/stringssetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/mapping/windssetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mapping/windssetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/mapping/percussionssetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mapping/percussionssetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/mapping/voicessetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mapping/voicessetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/filterbase.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/chordfilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/chordfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/spannerfilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/spannerfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/filters/internal/tremolofilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filters/internal/tremolofilter.h

    ${CMAKE_CURRENT_LIST_DIR}/utils/pitchutils.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/expressionutils.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/arrangementutils.h
)
