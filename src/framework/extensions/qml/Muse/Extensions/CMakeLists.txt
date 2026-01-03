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

muse_create_qml_module(muse_extensions_qml ALIAS muse::extensions_qml FOR muse_extensions)

qt_add_qml_module(muse_extensions_qml
    URI Muse.Extensions
    VERSION 1.0
    SOURCES
        devtools/devextensionslistmodel.cpp
        devtools/devextensionslistmodel.h
        devtools/apidumpmodel.cpp
        devtools/apidumpmodel.h
        extensionbuilder.cpp
        extensionbuilder.h
        extensionslistmodel.cpp
        extensionslistmodel.h
        extensionstoolbarmodel.cpp
        extensionstoolbarmodel.h
    QML_FILES
        ExtensionErrorMessage.qml
        ExtensionsApiDump.qml
        ExtensionsApiDumpDialog.qml
        ExtensionsListPanel.qml
        ExtensionsToolBar.qml
        ExtensionViewer.qml
        ExtensionViewerDialog.qml
        internal/EnablePanel.qml
        internal/ExtensionItem.qml
        internal/ExtensionsListView.qml
    RESOURCES
        internal/resources/placeholder.png
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_extensions_qml)

# Necessary for the auto-generated sources
target_include_directories(muse_extensions_qml PRIVATE devtools)
