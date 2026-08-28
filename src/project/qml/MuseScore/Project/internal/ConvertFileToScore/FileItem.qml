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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

ListItemBlank {
    id: root

    property alias fileName: fileNameLabel.text
    property alias iconCode: iconlabel.iconCode
    property bool selectable: false

    property alias removeButtonNavigation: removeButton.navigation

    signal removeButtonNavigationActivated()

    implicitHeight: 32

    radius: 3
    normalColor: ui.theme.buttonColor

    mouseArea.enabled: root.visible && root.enabled && root.selectable

    navigation.accessible.name: root.fileName

    RowLayout {
        id: contentRow

        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 4

        readonly property int iconSize: 24
        readonly property int iconFontSize: 14

        StyledIconLabel {
            Layout.preferredWidth: contentRow.iconSize
            Layout.preferredHeight: contentRow.iconSize

            iconCode: IconCode.TOOLBAR_GRIP
            font.pixelSize: contentRow.iconFontSize
            visible: root.selectable
        }

        StyledIconLabel {
            id: iconlabel

            Layout.preferredWidth: contentRow.iconSize
            Layout.preferredHeight: contentRow.iconSize

            font.pixelSize: contentRow.iconFontSize
        }

        StyledTextLabel {
            id: fileNameLabel
            Layout.fillWidth: true
            Layout.leftMargin: 2

            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideMiddle
        }

        FlatButton {
            id: removeButton

            icon: IconCode.DELETE_TANK
            toolTipTitle: qsTrc("global", "Remove")
            transparent: true

            navigation.accessible.name: removeButton.toolTipTitle + " " + root.fileName

            onClicked: root.removeSelectionRequested()

            Connections {
                target: removeButton.navigation

                function onActiveChanged(active) {
                    if (active) {
                        root.removeButtonNavigationActivated()
                    }
                }
            }
        }
    }
}
