/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

import MuseScore.UiComponents 1.0

Column {
    id: root

    property alias canRemove: deleteButton.enabled

    spacing: 24

    signal createNewWorkspaceRequested()
    signal removeSelectedWorkspaceRequested()

    RowLayout {
        width: parent.width

        spacing: 8

        StyledTextLabel {
            Layout.alignment: Qt.AlignLeft

            text: qsTrc("workspace", "Workspaces")
            font: ui.theme.headerBoldFont
        }

        Item {
            Layout.fillWidth: true
        }

        FlatButton {
            Layout.alignment: Qt.AlignRight

            text: qsTrc("workspace", "Create new workspace")

            onClicked: {
                root.createNewWorkspaceRequested()
            }
        }

        FlatButton {
            id: deleteButton

            Layout.alignment: Qt.AlignRight

            icon: IconCode.DELETE_TANK

            onClicked: {
                root.removeSelectedWorkspaceRequested()
            }
        }
    }

    StyledTextLabel {
        text: qsTrc("workspace", "Use workspaces to save different arrangements of the MuseScore interface")

        horizontalAlignment: Qt.AlignLeft
    }
}
