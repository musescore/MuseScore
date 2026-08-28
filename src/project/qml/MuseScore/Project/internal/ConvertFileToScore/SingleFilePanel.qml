/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick

import Muse.Ui
import Muse.UiComponents

Rectangle {
    id: root

    property alias fileName: fileItem.fileName
    property alias iconCode: fileItem.iconCode
    property alias fileRequirements: fileRequirementsItem.fileRequirements
    property NavigationPanel navigationPanel: null

    readonly property int sideMargin: 32

    signal removeRequested()

    function focusOnFileList() {
        fileItem.navigation.requestActive()
    }

    color: ui.theme.backgroundSecondaryColor
    border.width: 1
    border.color: ui.theme.strokeColor
    radius: 3

    Column {
        anchors.centerIn: parent
        width: parent.width - 2 * root.sideMargin
        spacing: 16

        FileItem {
            id: fileItem
            width: parent.width

            selectable: false

            navigation.panel: root.navigationPanel
            navigation.order: 1

            removeButtonNavigation.panel: root.navigationPanel
            removeButtonNavigation.order: 2

            onRemoveSelectionRequested: root.removeRequested()
        }

        FileRequirements {
            id: fileRequirementsItem

            anchors.horizontalCenter: parent.horizontalCenter

            navigation.panel: root.navigationPanel
            navigation.order: 3
        }
    }
}
