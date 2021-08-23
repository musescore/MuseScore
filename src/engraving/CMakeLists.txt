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

    ${LIBMSCORE_SRC}

    ${CMAKE_CURRENT_LIST_DIR}/draw/color.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/color.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/geometry.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/transform.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/transform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/matrix.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/painterpath.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/painterpath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/bezier.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/bezier.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/drawtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/pen.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/brush.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/pixmap.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/painter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/painter.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/ipaintprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/buffereddrawtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/bufferedpaintprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/bufferedpaintprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/svgrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/svgrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/ifontprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/iimageprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/font.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/font.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/fontmetrics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/fontmetrics.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/rgba.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawlogger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawlogger.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawjson.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawjson.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawcomp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawcomp.h

    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessiblescore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessiblescore.h
    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessibleelement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessibleelement.h
    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessiblenote.cpp
    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessiblenote.h

    ${CMAKE_CURRENT_LIST_DIR}/interactive/messagebox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/interactive/messagebox.h

    ${CMAKE_CURRENT_LIST_DIR}/io/mscio.h
    ${CMAKE_CURRENT_LIST_DIR}/io/mscreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/mscreader.h
    ${CMAKE_CURRENT_LIST_DIR}/io/mscwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/mscwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/io/htmlparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/htmlparser.h
    ${CMAKE_CURRENT_LIST_DIR}/io/xml.h
    ${CMAKE_CURRENT_LIST_DIR}/io/xmlreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/xmlwriter.cpp

    ${CMAKE_CURRENT_LIST_DIR}/compat/scoreaccess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/scoreaccess.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/mscxcompat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/mscxcompat.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/pageformat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/pageformat.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/chordlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/chordlist.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/writescorehook.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/writescorehook.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/readstyle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/readstyle.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/read114.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/read114.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/read206.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/read206.h
    ${CMAKE_CURRENT_LIST_DIR}/compat/read302.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compat/read302.h

    ${COMPAT_MIDI_SRC}

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

    ${CMAKE_CURRENT_LIST_DIR}/layout/layout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutoptions.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutcontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutcontext.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutlyrics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutlyrics.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutmeasure.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutmeasure.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutbeams.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutbeams.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layouttuplets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layouttuplets.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutchords.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutchords.h
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
    )

if (NO_ENGRAVING_INTERNAL)
    set(MODULE_DEF ${MODULE_DEF} -DNO_ENGRAVING_INTERNAL)
else()
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingconfiguration.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingconfiguration.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qpainterprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qpainterprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimageprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimageprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qfontprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qfontprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontengineft.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontengineft.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimagepainterprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimagepainterprovider.h
        )

    set(FREETYPE_LIB )
    if (USE_SYSTEM_FREETYPE)
         set(MODULE_LINK freetype)
    else (USE_SYSTEM_FREETYPE)
         set(MODULE_LINK mscore_freetype)
    endif (USE_SYSTEM_FREETYPE)

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
    )

set(MODULE_LINK ${MODULE_LINK} qzip)

set(MODULE_NOT_LINK_GLOBAL ON)
include(SetupModule)

set_property(TARGET ${MODULE} APPEND PROPERTY AUTOMOC_MACRO_NAMES "BEGIN_QT_REGISTERED_ENUM")
