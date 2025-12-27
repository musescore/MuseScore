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

muse_create_module(muse_extensions ALIAS muse::extensions)

include(api/v1/apiv1.cmake)

target_sources(muse_extensions PRIVATE
    extensionsmodule.cpp
    extensionsmodule.h
    extensionstypes.h
    extensionserrors.h
    iextensionsprovider.h
    iextensionsconfiguration.h
    iextensionsuiengine.h
    iextensioninstaller.h
    iextensionsexecpointsregister.h

    api/extapi.cpp
    api/extapi.h
    ${API_V1_SRC}

    internal/extensionsprovider.cpp
    internal/extensionsprovider.h
    internal/extensionsconfiguration.cpp
    internal/extensionsconfiguration.h
    internal/extensionsloader.cpp
    internal/extensionsloader.h
    internal/extensionrunner.cpp
    internal/extensionrunner.h
    internal/extensionsuiengine.cpp
    internal/extensionsuiengine.h
    internal/scriptengine.cpp
    internal/scriptengine.h
    internal/jsmoduleloader.cpp
    internal/jsmoduleloader.h
    internal/extensionsuiactions.cpp
    internal/extensionsuiactions.h
    internal/extensionsactioncontroller.cpp
    internal/extensionsactioncontroller.h
    internal/extensioninstaller.cpp
    internal/extensioninstaller.h
    internal/extensionsexecpointsregister.cpp
    internal/extensionsexecpointsregister.h

    internal/legacy/extpluginsloader.cpp
    internal/legacy/extpluginsloader.h
    internal/legacy/extpluginrunner.cpp
    internal/legacy/extpluginrunner.h
)

if (MUSE_QT_SUPPORT)
    target_link_libraries(muse_extensions PRIVATE Qt::Quick)
endif()

if (MUSE_MODULE_EXTENSIONS_TESTS)
#    add_subdirectory(tests)
endif()

if (MUSE_MODULE_EXTENSIONS_QML)
    add_subdirectory(qml/Muse/Extensions)
endif()
