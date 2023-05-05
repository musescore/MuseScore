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

set(MODULE engraving)
set(MODULE_QRC engraving.qrc)

include(${CMAKE_CURRENT_LIST_DIR}/libmscore/libmscore.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/compat/midi/compatmidi.cmake)

set(MODULE_SRC

    ${CMAKE_CURRENT_LIST_DIR}/engravingmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/engravingerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/engravingproject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingproject.h
    ${CMAKE_CURRENT_LIST_DIR}/iengravingconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iengravingfont.h
    ${CMAKE_CURRENT_LIST_DIR}/iengravingfontsprovider.h

    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/messagebox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/messagebox.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/imimedata.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/mscio.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/mscreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/mscreader.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/mscwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/mscwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/htmlparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/htmlparser.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/ifileinfoprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/localfileinfoprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/localfileinfoprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/paint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/paint.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/debugpaint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/debugpaint.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/paintdebugger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/paintdebugger.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/smufl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/smufl.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/rtti.h

    ${LIBMSCORE_SRC}

    ${CMAKE_CURRENT_LIST_DIR}/rw/iscorereader.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/readoutdata.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/linksindexer.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/xmlreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/xmlreader.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/xmlwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/xmlwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/mscloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/mscloader.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/114/read114.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/114/read114.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/206/read206.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/206/read206.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/302/read302.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/302/read302.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/400/read400.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/read400.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/staffrw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/staffrw.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/measurerw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/measurerw.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/readcontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/readcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/writecontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/writecontext.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/tread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/tread.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/twrite.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/twrite.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/connectorinfowriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/connectorinfowriter.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/connectorinforeader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/400/connectorinforeader.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/readstyle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/readstyle.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/readchordlisthook.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/readchordlisthook.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/compatutils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/compatutils.h

    ${CMAKE_CURRENT_LIST_DIR}/compat/scoreaccess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/scoreaccess.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/mscxcompat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/mscxcompat.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/pageformat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/pageformat.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/writescorehook.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/writescorehook.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/dummyelement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/dummyelement.h

    ${COMPAT_MIDI_SRC}

    ${CMAKE_CURRENT_LIST_DIR}/types/constants.h
    ${CMAKE_CURRENT_LIST_DIR}/types/types.h
    ${CMAKE_CURRENT_LIST_DIR}/types/dimension.h
    ${CMAKE_CURRENT_LIST_DIR}/types/fraction.h
    ${CMAKE_CURRENT_LIST_DIR}/types/typesconv.cpp
    ${CMAKE_CURRENT_LIST_DIR}/types/typesconv.h
    ${CMAKE_CURRENT_LIST_DIR}/types/symid.h
    ${CMAKE_CURRENT_LIST_DIR}/types/symid_p.h
    ${CMAKE_CURRENT_LIST_DIR}/types/symnames.cpp
    ${CMAKE_CURRENT_LIST_DIR}/types/symnames.h
    ${CMAKE_CURRENT_LIST_DIR}/types/pitchvalue.h
    ${CMAKE_CURRENT_LIST_DIR}/types/bps.h
    ${CMAKE_CURRENT_LIST_DIR}/types/groupnode.h
    ${CMAKE_CURRENT_LIST_DIR}/types/propertyvalue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/types/propertyvalue.h

    ${CMAKE_CURRENT_LIST_DIR}/style/styledef.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/styledef.h
    ${CMAKE_CURRENT_LIST_DIR}/style/textstyle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/textstyle.h
    ${CMAKE_CURRENT_LIST_DIR}/style/pagestyle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/pagestyle.h
    ${CMAKE_CURRENT_LIST_DIR}/style/style.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/style.h
    ${CMAKE_CURRENT_LIST_DIR}/style/defaultstyle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/defaultstyle.h

    ${CMAKE_CURRENT_LIST_DIR}/layout/tlayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/tlayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutoptions.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutcontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/lyricslayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/lyricslayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutmeasure.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutmeasure.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/beamlayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/beamlayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/beamtremololayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/beamtremololayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/chordlayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/chordlayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layouttuplets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layouttuplets.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/verticalgapdata.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/verticalgapdata.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutsystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutsystem.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutharmonies.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutharmonies.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layouttremolo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layouttremolo.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutpage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutpage.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/slurtielayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/slurtielayout.h

    ${CMAKE_CURRENT_LIST_DIR}/playback/renderingcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/playbackcontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/playbackcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/playbackmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/playbackmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/playbackeventsrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/playbackeventsrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/playbacksetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/playbacksetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/renderbase.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/ornamentsrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/ornamentsrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/glissandosrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/glissandosrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/arpeggiorenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/arpeggiorenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/gracechordsrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/gracechordsrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/tremolorenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/tremolorenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/chordarticulationsrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/renderers/chordarticulationsrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/symbolsmetaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/symbolsmetaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/annotationsmetaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/annotationsmetaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/spannersmetaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/spannersmetaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/tremolometaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/tremolometaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/arpeggiometaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/arpeggiometaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/gracenotesmetaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/gracenotesmetaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/chordlinemetaparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/internal/chordlinemetaparser.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/notearticulationsparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/notearticulationsparser.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/chordarticulationsparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/chordarticulationsparser.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/metaparsers/metaparserbase.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/setupresolverbase.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/keyboardssetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/keyboardssetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/stringssetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/stringssetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/windssetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/windssetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/percussionssetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/percussionssetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/voicessetupdataresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/mapping/voicessetupdataresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/filters/filterbase.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/filters/chordfilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/filters/chordfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/filters/spannerfilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/filters/spannerfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/filters/internal/tremolofilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playback/filters/internal/tremolofilter.h

    ${CMAKE_CURRENT_LIST_DIR}/playback/utils/pitchutils.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/utils/expressionutils.h
    ${CMAKE_CURRENT_LIST_DIR}/playback/utils/arrangementutils.h
    )

if (ENGRAVING_NO_ACCESSIBILITY)
    set(MODULE_DEF ${MODULE_DEF} -DENGRAVING_NO_ACCESSIBILITY)
else()
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessibleitem.cpp
        ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessibleitem.h
        ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessibleroot.cpp
        ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessibleroot.h
        ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessiblenote.cpp
        ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessiblenote.h
    )
endif()

if (ENGRAVING_NO_INTERNAL)
    set(MODULE_DEF ${MODULE_DEF} -DENGRAVING_NO_INTERNAL -DENGRAVING_NO_QTEXTDOCUMENT -DENGRAVING_NO_INTERACTIVE)
else()
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingconfiguration.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingconfiguration.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qmimedataadapter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qmimedataadapter.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingfontsprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingfontsprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingfont.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingfont.h
        )
endif()

set_source_files_properties( # For these files, Unity Build does not work
    ${CMAKE_CURRENT_LIST_DIR}/libmscore/chordlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/chordlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/read114.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/read206.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
    )

set(MODULE_INCLUDE
    ${PROJECT_SOURCE_DIR}/thirdparty/dtl
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure
    )

set(MODULE_DEF ${MODULE_DEF} ${ENGRAVING_INFRASTRUCTURE_DEF})
set(MODULE_LINK
    ${ENGRAVING_INFRASTRUCTURE_LINK}
    draw
)

if (MUE_ENABLE_ENGRAVING_PAINT_DEBUGGER)
    set(MODULE_DEF ${MODULE_DEF} -DMUE_ENABLE_ENGRAVING_PAINT_DEBUGGER)
endif()

set(MODULE_USE_UNITY_NONE ON)
include(SetupModule)

set_property(TARGET ${MODULE} APPEND PROPERTY AUTOMOC_MACRO_NAMES "BEGIN_QT_REGISTERED_ENUM")

#target_compile_options(${MODULE} PUBLIC -Wconversion)

if (MUE_BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()
