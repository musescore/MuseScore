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

Item {
    id: root

    //! NOTE: list of { title: string, items: array<string> }
    property var fileRequirements: []

    property alias navigation: navCtrl

    visible: root.fileRequirements.length > 0

    implicitWidth: contentRow.implicitWidth
    implicitHeight: contentRow.implicitHeight

    function toggleOpened() {
        if (popup.isOpened) {
            popup.close()
        } else {
            popup.open()
        }
    }

    NavigationControl {
        id: navCtrl

        name: root.objectName !== "" ? root.objectName : "FileRequirements"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Button
        accessible.name: qsTrc("project/convert", "File requirements")
        accessible.visualItem: root
        accessible.enabled: navCtrl.enabled

        onTriggered: {
            root.toggleOpened()
        }
    }

    NavigationFocusBorder {
        navigationCtrl: navCtrl
    }

    Row {
        id: contentRow

        anchors.centerIn: parent
        spacing: 4

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter

            text: qsTrc("project/convert", "File requirements")
            font.family: ui.theme.bodyFont.family
            font.pixelSize: ui.theme.bodyFont.pixelSize
            font.underline: true
            color: ui.theme.linkColor
        }

        StyledIconLabel {
            anchors.verticalCenter: parent.verticalCenter

            iconCode: IconCode.INFO_FILLED
            font.pixelSize: 10
            color: ui.theme.linkColor
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor

        onClicked: {
            root.toggleOpened()
        }
    }

    FileRequirementsPopup {
        id: popup

        sections: root.fileRequirements
    }
}
