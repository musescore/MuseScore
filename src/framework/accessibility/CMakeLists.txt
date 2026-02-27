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

muse_create_module(muse_accessibility ALIAS muse::accessibility)

target_sources(muse_accessibility PRIVATE
    accessibilitymodule.cpp
    accessibilitymodule.h
    iaccessible.h
    iaccessibilitycontroller.h
    iaccessibilityconfiguration.h
    iqaccessibleinterfaceregister.h

    api/accessibilityapi.cpp
    api/accessibilityapi.h

    internal/accessibilitycontroller.cpp
    internal/accessibilitycontroller.h
    internal/accessibleobject.cpp
    internal/accessibleobject.h
    internal/accessiblestub.cpp
    internal/accessiblestub.h
    internal/accessibleiteminterface.cpp
    internal/accessibleiteminterface.h
    internal/accessiblewindowinterface.cpp
    internal/accessiblewindowinterface.h
    internal/accessibilityconfiguration.cpp
    internal/accessibilityconfiguration.h
    internal/qaccessibleinterfaceregister.cpp
    internal/qaccessibleinterfaceregister.h
)

if (OS_IS_LIN)
    # it needed to send a spontaneous event by activating navigation
    find_package(Qt6 REQUIRED COMPONENTS CorePrivate)
    target_link_libraries(muse_accessibility PRIVATE Qt6::CorePrivate)
endif()

if (MUSE_MODULE_ACCESSIBILITY_TRACE)
    target_compile_definitions(muse_accessibility PRIVATE MUSE_MODULE_ACCESSIBILITY_TRACE)
endif()

if (MUSE_QT_SUPPORT)
    target_link_libraries(muse_accessibility PRIVATE Qt::Quick)
endif()

if (MUSE_MODULE_ACCESSIBILITY_TESTS)
    add_subdirectory(tests)
endif()
