/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Column {
    id: root

    property string firstWorkspaceTitle: ""
    property alias canRemove: deleteButton.enabled

    spacing: 24

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "WorkspacesTopPanel"
        direction: NavigationPanel.Horizontal
        accessible.role: MUAccessible.Dialog
        accessible.name: titleLabel.text
    }

    signal createNewWorkspaceRequested()
    signal removeSelectedWorkspaceRequested()

    function readInfo() {
        accessibleInfo.ignored = false
        accessibleInfo.focused = true
    }

    AccessibleItem {
        id: accessibleInfo
        accessibleParent: root.navigationPanel.accessible
        visualItem: root
        role: MUAccessible.ListItem
        name: descriptionLabel.text + " " + root.firstWorkspaceTitle
    }

    RowLayout {
        width: parent.width

        spacing: 8

        StyledTextLabel {
            id: titleLabel
            Layout.alignment: Qt.AlignLeft

            text: qsTrc("workspace", "Workspaces")
            font: ui.theme.headerBoldFont
        }

        Item {
            Layout.fillWidth: true
        }

        FlatButton {
            Layout.alignment: Qt.AlignRight

            text: qsTrc("workspace", "Save layout as new workspace")
            icon: IconCode.SAVE
            orientation: Qt.Horizontal

            navigation.name: "CreateNewWorkspaceButton"
            navigation.panel: root.navigationPanel
            navigation.column: 1

            onClicked: {
                root.createNewWorkspaceRequested()
            }
        }

        FlatButton {
            id: deleteButton

            Layout.alignment: Qt.AlignRight

            icon: IconCode.DELETE_TANK

            navigation.name: "RemoveButton"
            navigation.panel: root.navigationPanel
            navigation.column: 2
            navigation.accessible.name: qsTrc("workspace", "Remove")

            onClicked: {
                root.removeSelectedWorkspaceRequested()
            }
        }
    }

    StyledTextLabel {
        id: descriptionLabel

        text: qsTrc("workspace", "Use workspaces to save different arrangements of the MuseScore Studio interface")

        horizontalAlignment: Qt.AlignLeft
    }
}
