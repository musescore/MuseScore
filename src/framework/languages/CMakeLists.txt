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

muse_create_module(muse_languages ALIAS muse::languages)

target_sources(muse_languages PRIVATE
    languagesmodule.cpp
    languagesmodule.h

    languagestypes.h
    languageserrors.h

    ilanguagesconfiguration.h
    ilanguagesservice.h

    internal/languagesconfiguration.cpp
    internal/languagesconfiguration.h
    internal/languagesservice.cpp
    internal/languagesservice.h
)

if (MUSE_QT_SUPPORT)
    target_link_libraries(muse_languages PRIVATE Qt::Concurrent Qt::Gui Qt::Network Qt::Qml)
endif()

if (MUSE_MODULE_LANGUAGES_TESTS)
    add_subdirectory(tests)
endif()
