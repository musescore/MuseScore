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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(engraving)

# TODO: convert to target-based variant of qt_add_resources
# (Not easy because of heavy use of aliases, which is much more cumbersome 
# in CMake than in .qrc files)
muse_module_add_qrc(engraving 
    engraving.qrc
    data/fonts/fonts_Smufl.qrc
)

# TODO: convert to target-based variant of qt_add_resources
muse_module_add_qrc(engraving BIG_RESOURCES
    data/fonts/fonts_Bravura.qrc
    data/fonts/fonts_Campania.qrc
    data/fonts/fonts_Edwin.qrc
    data/fonts/fonts_FreeSans.qrc
    data/fonts/fonts_FreeSerif.qrc
    data/fonts/fonts_Gootville.qrc
    data/fonts/fonts_Leland.qrc
    data/fonts/fonts_MScore.qrc
    data/fonts/fonts_MuseJazz.qrc
    data/fonts/fonts_Petaluma.qrc
    data/fonts/fonts_FinaleMaestro.qrc
    data/fonts/fonts_FinaleBroadway.qrc
    data/fonts/fonts_Tabulature.qrc
)

include(dom/dom.cmake)
include(compat/midi/compatmidi.cmake)
include(rendering/score/rendering.cmake)
include(rendering/single/single.cmake)
include(rendering/editmode/editmode.cmake)

if (MUE_BUILD_ENGRAVING_DEVTOOLS)
    include(devtools/devtools.cmake)
endif()

if (MUE_BUILD_ENGRAVING_PLAYBACK)
    include(playback/playback.cmake)
endif()

target_sources(engraving PRIVATE
    ${RESOURCE_SOURCES}
    ${BIG_RESOURCE_SOURCES}
    engravingmodule.cpp
    engravingmodule.h
    engravingerrors.h
    engravingproject.cpp
    engravingproject.h
    iengravingpluginapihelper.h
    iengravingconfiguration.h
    iengravingfont.h
    iengravingfontsprovider.h

    infrastructure/messagebox.cpp
    infrastructure/messagebox.h
    infrastructure/imimedata.h
    infrastructure/mscio.h
    infrastructure/mscreader.cpp
    infrastructure/mscreader.h
    infrastructure/mscwriter.cpp
    infrastructure/mscwriter.h
    infrastructure/htmlparser.cpp
    infrastructure/htmlparser.h
    infrastructure/ifileinfoprovider.h
    infrastructure/localfileinfoprovider.cpp
    infrastructure/localfileinfoprovider.h
    infrastructure/smufl.cpp
    infrastructure/smufl.h
    infrastructure/rtti.h
    infrastructure/ld_access.h
    infrastructure/shape.cpp
    infrastructure/shape.h
    infrastructure/skyline.cpp
    infrastructure/skyline.h
    infrastructure/eid.cpp
    infrastructure/eid.h
    infrastructure/eidregister.cpp
    infrastructure/eidregister.h

    ${DOM_SRC}

    editing/addremoveelement.cpp
    editing/addremoveelement.h
    editing/cmd.cpp
    editing/cmd.h
    editing/edit.cpp
    editing/editbrackets.cpp
    editing/editbrackets.h
    editing/editchord.cpp
    editing/editchord.h
    editing/editclef.cpp
    editing/editclef.h
    editing/editharppedaldiagram.cpp
    editing/editharppedaldiagram.h
    editing/editinstrumentchange.cpp
    editing/editinstrumentchange.h
    editing/editdata.cpp
    editing/editdata.h
    editing/editexcerpt.cpp
    editing/editexcerpt.h
    editing/editfretboarddiagram.cpp
    editing/editfretboarddiagram.h
    editing/editkeysig.cpp
    editing/editkeysig.h
    editing/editmeasures.cpp
    editing/editmeasures.h
    editing/editnote.cpp
    editing/editnote.h
    editing/editpart.cpp
    editing/editpart.h
    editing/editproperty.cpp
    editing/editproperty.h
    editing/editscoreproperties.cpp
    editing/editscoreproperties.h
    editing/editsoundflag.cpp
    editing/editsoundflag.h
    editing/editspanner.cpp
    editing/editspanner.h
    editing/editstaff.cpp
    editing/editstaff.h
    editing/editstyle.cpp
    editing/editstyle.h
    editing/editsystemlocks.cpp
    editing/editsystemlocks.h
    editing/edittie.cpp
    editing/edittie.h
    editing/edittremolo.cpp
    editing/edittremolo.h
    editing/editvoicing.cpp
    editing/editvoicing.h
    editing/inserttime.cpp
    editing/inserttime.h
    editing/elementeditdata.h
    editing/mscoreview.cpp
    editing/mscoreview.h
    editing/splitjoinmeasure.cpp
    editing/splitjoinmeasure.h
    editing/textedit.cpp
    editing/textedit.h
    editing/transpose.cpp
    editing/transpose.h
    editing/undo.cpp
    editing/undo.h

    rw/ireader.h
    rw/iwriter.h
    rw/rwregister.cpp
    rw/rwregister.h
    rw/inoutdata.h
    rw/linksindexer.h
    rw/xmlreader.cpp
    rw/xmlreader.h
    rw/xmlwriter.cpp
    rw/xmlwriter.h
    rw/mscloader.cpp
    rw/mscloader.h
    rw/mscsaver.cpp
    rw/mscsaver.h

    rw/harmonytodiagramreader.cpp
    rw/harmonytodiagramreader.h

    rw/write/writer.cpp
    rw/write/writer.h
    rw/write/writecontext.cpp
    rw/write/writecontext.h
    rw/write/twrite.cpp
    rw/write/twrite.h
    rw/write/connectorinfowriter.cpp
    rw/write/connectorinfowriter.h
    rw/write/staffwrite.cpp
    rw/write/staffwrite.h
    rw/write/measurewrite.cpp
    rw/write/measurewrite.h

    rw/read114/read114.cpp
    rw/read114/read114.h

    rw/read206/read206.cpp
    rw/read206/read206.h

    rw/read302/read302.cpp
    rw/read302/read302.h

    rw/read400/read400.cpp
    rw/read400/read400.h
    rw/read400/staffrw.cpp
    rw/read400/staffrw.h
    rw/read400/measurerw.cpp
    rw/read400/measurerw.h
    rw/read400/readcontext.cpp
    rw/read400/readcontext.h
    rw/read400/tread.cpp
    rw/read400/tread.h
    rw/read400/connectorinforeader.cpp
    rw/read400/connectorinforeader.h

    rw/read410/read410.cpp
    rw/read410/read410.h
    rw/read410/staffread.cpp
    rw/read410/staffread.h
    rw/read410/measureread.cpp
    rw/read410/measureread.h
    rw/read410/readcontext.cpp
    rw/read410/readcontext.h
    rw/read410/tread.cpp
    rw/read410/tread.h
    rw/read410/connectorinforeader.cpp
    rw/read410/connectorinforeader.h

    rw/read460/read460.cpp
    rw/read460/read460.h
    rw/read460/staffread.cpp
    rw/read460/staffread.h
    rw/read460/measureread.cpp
    rw/read460/measureread.h
    rw/read460/readcontext.cpp
    rw/read460/readcontext.h
    rw/read460/tread.cpp
    rw/read460/tread.h
    rw/read460/connectorinforeader.cpp
    rw/read460/connectorinforeader.h

    rw/compat/readstyle.cpp
    rw/compat/readstyle.h
    rw/compat/readchordlisthook.cpp
    rw/compat/readchordlisthook.h
    rw/compat/compatutils.cpp
    rw/compat/compatutils.h
    rw/compat/tremolocompat.h

    compat/scoreaccess.cpp
    compat/scoreaccess.h
    compat/mscxcompat.cpp
    compat/mscxcompat.h
    compat/pageformat.cpp
    compat/pageformat.h
    compat/writescorehook.cpp
    compat/writescorehook.h
    compat/dummyelement.cpp
    compat/dummyelement.h
    compat/engravingcompat.cpp
    compat/engravingcompat.h

    ${COMPAT_MIDI_SRC}

    types/constants.h
    types/types.h
    types/dimension.h
    types/fraction.h
    types/typesconv.cpp
    types/typesconv.h
    types/symid.h
    types/symid_p.h
    types/symnames.cpp
    types/symnames.h
    types/pitchvalue.h
    types/bps.h
    types/groupnode.h
    types/propertyvalue.cpp
    types/propertyvalue.h

    style/styledef.cpp
    style/styledef.h
    style/textstyle.cpp
    style/textstyle.h
    style/pagestyle.cpp
    style/pagestyle.h
    style/style.cpp
    style/style.h
    style/defaultstyle.cpp
    style/defaultstyle.h

    rendering/iscorerenderer.h
    rendering/isinglerenderer.h
    rendering/ieditmoderenderer.h
    rendering/layoutoptions.h
    rendering/paddingtable.cpp
    rendering/paddingtable.h

    ${RENDERING_SCORE_SRC}
    ${RENDERING_SINGLE_SRC}
    ${RENDERING_EDITMODE_SRC}

    ${PLAYBACK_SRC}
    ${DEVTOOLS_SRC}
)

if (ENGRAVING_NO_ACCESSIBILITY)
    target_compile_definitions(engraving PRIVATE ENGRAVING_NO_ACCESSIBILITY)
else()
    target_sources(engraving PRIVATE
        accessibility/accessibleitem.cpp
        accessibility/accessibleitem.h
        accessibility/accessibleroot.cpp
        accessibility/accessibleroot.h
        accessibility/accessiblenote.cpp
        accessibility/accessiblenote.h
    )
endif()

if (ENGRAVING_NO_INTERNAL)
    target_compile_definitions(engraving PRIVATE ENGRAVING_NO_INTERNAL ENGRAVING_NO_INTERACTIVE ENGRAVING_NO_API)
else()
    include(${CMAKE_CURRENT_LIST_DIR}/api/v1/api_v1.cmake)
    target_sources(engraving PRIVATE
        internal/engravingconfiguration.cpp
        internal/engravingconfiguration.h
        internal/qmimedataadapter.cpp
        internal/qmimedataadapter.h
        internal/engravingfontsprovider.cpp
        internal/engravingfontsprovider.h
        internal/engravingfont.cpp
        internal/engravingfont.h
        ${API_V1_SRC}
    )
endif()

set_source_files_properties( 
    # For these files, Unity Build does not work
    dom/excerpt.cpp
    #api/vi/excerpt.cpp
    dom/instrument.cpp
    #api/vi/instrument.cpp
    dom/part.cpp
    #api/vi/part.cpp
    dom/score.cpp
    #api/vi/score.cpp
    dom/transpose.cpp
    editing/transpose.cpp
    style/style.cpp
    #${CMAKE_CURRENT_LIST_DIR}/api/vi/style.cpp

    infrastructure/mscreader.cpp
    infrastructure/mscwriter.cpp
    #rw/read114/read114.cpp
    rw/read206/read206.cpp
    #rw/read302/read302.cpp

    #rw/read400/read400.cpp
    #rw/read400/staffrw.cpp
    #rw/read400/measurerw.cpp
    rw/read400/readcontext.cpp
    rw/read400/tread.cpp
    rw/read400/connectorinforeader.cpp

    #rw/read410/read410.cpp
    rw/read410/staffread.cpp
    rw/read410/measureread.cpp
    rw/read410/readcontext.cpp
    rw/read410/tread.cpp
    rw/read410/connectorinforeader.cpp

    #rw/read460/read460.cpp
    rw/read460/staffread.cpp
    rw/read460/measureread.cpp
    rw/read460/readcontext.cpp
    rw/read460/tread.cpp
    rw/read460/connectorinforeader.cpp

    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
)

if (MUE_ENABLE_ENGRAVING_RENDER_DEBUG)
    target_compile_definitions(engraving PRIVATE MUE_ENABLE_ENGRAVING_RENDER_DEBUG)
endif()

if (MUE_ENABLE_ENGRAVING_LD_ACCESS)
    target_compile_definitions(engraving PRIVATE MUE_ENABLE_ENGRAVING_LD_ACCESS)
endif()

if (MUE_ENABLE_ENGRAVING_LD_PASSES)
    target_compile_definitions(engraving PRIVATE MUE_ENABLE_ENGRAVING_LD_PASSES)
endif()

if (MUE_BUILD_ENGRAVING_DEVTOOLS)
    target_compile_definitions(engraving PRIVATE MUE_BUILD_ENGRAVING_DEVTOOLS)
endif()


target_include_directories(engraving PRIVATE
    infrastructure
    ${PROJECT_SOURCE_DIR}/thirdparty/dtl
)

target_link_libraries(engraving PRIVATE muse::draw)

if (QT_SUPPORT)
    target_link_libraries(engraving PRIVATE Qt::Quick)
endif()

set_property(TARGET engraving APPEND PROPERTY AUTOMOC_MACRO_NAMES "BEGIN_QT_REGISTERED_ENUM")

#target_compile_options(engraving PUBLIC -Wconversion)

if (MUE_BUILD_ENGRAVING_TESTS)
    add_subdirectory(tests)
    add_subdirectory(api/tests)
endif()

if (QT_SUPPORT)
    add_subdirectory(qml/MuseScore/Engraving)
endif()
