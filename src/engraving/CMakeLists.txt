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

declare_module(engraving)
set(MODULE_QRC
    engraving.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_Smufl.qrc
)

set(MODULE_BIG_QRC
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_Bravura.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_Campania.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_Edwin.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_FreeSans.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_FreeSerif.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_Gootville.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_Leland.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_MScore.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_MuseJazz.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_Petaluma.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_FinaleMaestro.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_FinaleBroadway.qrc
    ${CMAKE_CURRENT_LIST_DIR}/data/fonts/fonts_Tabulature.qrc
    )

include(${CMAKE_CURRENT_LIST_DIR}/dom/dom.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/compat/midi/compatmidi.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/rendering/score/rendering.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/rendering/single/single.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/rendering/editmode/editmode.cmake)

if (MUE_BUILD_ENGRAVING_DEVTOOLS)
    include(${CMAKE_CURRENT_LIST_DIR}/devtools/devtools.cmake)
endif()

if (MUE_BUILD_ENGRAVING_PLAYBACK)
    include(${CMAKE_CURRENT_LIST_DIR}/playback/playback.cmake)
endif()

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
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/smufl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/smufl.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/rtti.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/ld_access.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/shape.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/shape.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/skyline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/skyline.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/eid.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/eid.h
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/eidregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure/eidregister.h

    ${DOM_SRC}

    ${CMAKE_CURRENT_LIST_DIR}/rw/README.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/ireader.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/iwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/rwregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/rwregister.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/inoutdata.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/linksindexer.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/xmlreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/xmlreader.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/xmlwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/xmlwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/mscloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/mscloader.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/mscsaver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/mscsaver.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/harmonytodiagramreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/harmonytodiagramreader.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/write/writer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/writer.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/writecontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/writecontext.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/twrite.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/twrite.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/connectorinfowriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/connectorinfowriter.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/staffwrite.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/staffwrite.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/measurewrite.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/write/measurewrite.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/read114/read114.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read114/read114.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/read206/read206.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read206/read206.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/read302/read302.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read302/read302.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/read400.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/read400.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/staffrw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/staffrw.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/measurerw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/measurerw.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/readcontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/readcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/tread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/tread.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/connectorinforeader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read400/connectorinforeader.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/read410.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/read410.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/staffread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/staffread.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/measureread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/measureread.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/readcontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/readcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/tread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/tread.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/connectorinforeader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read410/connectorinforeader.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/read460.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/read460.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/staffread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/staffread.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/measureread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/measureread.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/readcontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/readcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/tread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/tread.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/connectorinforeader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/read460/connectorinforeader.h

    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/readstyle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/readstyle.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/readchordlisthook.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/readchordlisthook.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/compatutils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/compatutils.h
    ${CMAKE_CURRENT_LIST_DIR}/rw/compat/tremolocompat.h

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
    ${CMAKE_CURRENT_LIST_DIR}/compat/engravingcompat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/engravingcompat.h

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

    ${CMAKE_CURRENT_LIST_DIR}/rendering/iscorerenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/isinglerenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/ieditmoderenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/layoutoptions.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/paddingtable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/paddingtable.h

    ${RENDERING_SCORE_SRC}
    ${RENDERING_SINGLE_SRC}
    ${RENDERING_EDITMODE_SRC}

    ${PLAYBACK_SRC}
    ${DEVTOOLS_SRC}
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
    set(MODULE_DEF ${MODULE_DEF}
        -DENGRAVING_NO_INTERNAL
        -DENGRAVING_NO_INTERACTIVE
        -DENGRAVING_NO_API
    )
else()
    include(${CMAKE_CURRENT_LIST_DIR}/api/v1/api_v1.cmake)
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingconfiguration.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingconfiguration.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qmimedataadapter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qmimedataadapter.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingfontsprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingfontsprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingfont.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingfont.h
        ${API_V1_SRC}
        )
endif()

set_source_files_properties( # For these files, Unity Build does not work
    ${CMAKE_CURRENT_LIST_DIR}/dom/chordlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/chordlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/read114.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/read206.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
    )

set(MODULE_INCLUDE
    # Many other modules include files from subfolders of this module;
    # when included from another module, these files can't find their
    # own includes, unless `engraving` is an include directory.
    # Therefore, in the case of the `engraving` module, we include its
    # root directory publicly, rather than privately as with most modules.
    ${CMAKE_CURRENT_LIST_DIR}
)

set(MODULE_INCLUDE_PRIVATE
    ${PROJECT_SOURCE_DIR}/thirdparty/dtl
    ${CMAKE_CURRENT_LIST_DIR}/infrastructure
)

set(MODULE_LINK
    muse::draw
)

if (MUE_ENABLE_ENGRAVING_RENDER_DEBUG)
    set(MODULE_DEF ${MODULE_DEF} -DMUE_ENABLE_ENGRAVING_RENDER_DEBUG)
endif()

if (MUE_ENABLE_ENGRAVING_LD_ACCESS)
    set(MODULE_DEF ${MODULE_DEF} -DMUE_ENABLE_ENGRAVING_LD_ACCESS)
endif()

if (MUE_ENABLE_ENGRAVING_LD_PASSES)
    set(MODULE_DEF ${MODULE_DEF} -DMUE_ENABLE_ENGRAVING_LD_PASSES)
endif()

if (MUE_BUILD_ENGRAVING_DEVTOOLS)
    set(MODULE_DEF ${MODULE_DEF} -DMUE_BUILD_ENGRAVING_DEVTOOLS)
endif()

if (QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Quick)
endif()

set(MODULE_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_USE_UNITY OFF)
setup_module()

set_property(TARGET ${MODULE} APPEND PROPERTY AUTOMOC_MACRO_NAMES "BEGIN_QT_REGISTERED_ENUM")

#target_compile_options(${MODULE} PUBLIC -Wconversion)

if (MUE_BUILD_ENGRAVING_TESTS)
    add_subdirectory(tests)
endif()
