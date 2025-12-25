# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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

muse_create_module(iex_musicxml)

target_sources(iex_musicxml PRIVATE
    musicxmlmodule.cpp
    musicxmlmodule.h
    imusicxmlconfiguration.h

    internal/export/exportmusicxml.cpp
    internal/export/exportmusicxml.h
    internal/import/importmusicxml.cpp
    internal/import/importmusicxml.h
    internal/import/importmusicxmllogger.cpp
    internal/import/importmusicxmllogger.h
    internal/import/importmusicxmlnoteduration.cpp
    internal/import/importmusicxmlnoteduration.h
    internal/import/importmusicxmlnotepitch.cpp
    internal/import/importmusicxmlnotepitch.h
    internal/import/importmusicxmlpass1.cpp
    internal/import/importmusicxmlpass1.h
    internal/import/importmusicxmlpass2.cpp
    internal/import/importmusicxmlpass2.h
    internal/import/musicxmlpart.cpp
    internal/import/musicxmlpart.h
    internal/import/musicxmltupletstate.cpp
    internal/import/musicxmltupletstate.h
    internal/import/musicxmlvalidation.cpp
    internal/import/musicxmlvalidation.h
    internal/import/musicxmlvoicedesc.cpp
    internal/import/musicxmlvoicedesc.h
    internal/shared/musicxmlfonthandler.cpp
    internal/shared/musicxmlfonthandler.h
    internal/shared/musicxmlsupport.cpp
    internal/shared/musicxmlsupport.h
    internal/shared/musicxmltypes.cpp
    internal/shared/musicxmltypes.h  
)

if (MUSICXML_NO_INTERNAL)
    target_compile_definitions(iex_musicxml PRIVATE MUSICXML_NO_INTERNAL MUSICXML_NO_INTERACTIVE MUSICXML_NO_VALIDATION)
else()
    target_sources(iex_musicxml PRIVATE
        internal/musicxmlconfiguration.cpp
        internal/musicxmlconfiguration.h
        internal/musicxmlreader.cpp
        internal/musicxmlreader.h
        internal/musicxmlwriter.cpp
        internal/musicxmlwriter.h
        internal/mxlwriter.cpp
        internal/mxlwriter.h
    )

    target_compile_definitions(iex_musicxml PRIVATE MUSICXML_NO_VALIDATION) # TODO

    qt_add_resources(iex_musicxml iexmusicxml_resources
        PREFIX
            /
        FILES
            schema/musicxml.xsd
            schema/xlink.xsd
            schema/xml.xsd
    )
endif()

target_link_libraries(iex_musicxml PRIVATE engraving)

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
