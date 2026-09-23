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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

Item {
    id: root

    property NavigationPanel navigationPanel: null

    readonly property string accessibleName: qsTrc("project/convert", "How does your score look?")
    readonly property string accessibleDescription: qsTrc("project/convert", "We’re always improving our score conversion accuracy. Let us know how we did with this one.")

    signal goodRequested()
    signal badRequested()

    implicitWidth: 400
    implicitHeight: Math.max(content.implicitHeight + content.anchors.margins * 2, 124)

    ColumnLayout {
        id: content

        anchors.fill: parent
        anchors.margins: 16

        spacing: 12

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            StyledTextLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter

                font.pixelSize: 14
                font.bold: true

                text: qsTrc("project/convert", "How does your score look?")
            }

            StyledTextLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap

                text: qsTrc("project/convert", "We’re always improving our score conversion accuracy. Let us know how we did with this one.")
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8

            FlatButton {
                Layout.minimumWidth: 132

                //: Button to rate the quality of a converted score as good
                text: qsTrc("project/convert", "Good")
                orientation: Qt.Horizontal
                icon: IconCode.LIKE
                accentButton: true

                navigation.panel: root.navigationPanel
                navigation.order: 1

                onClicked: root.goodRequested()
            }

            FlatButton {
                Layout.minimumWidth: 132

                //: Button to rate the quality of a converted score as bad
                text: qsTrc("project/convert", "Bad")
                orientation: Qt.Horizontal
                icon: IconCode.DISLIKE

                navigation.panel: root.navigationPanel
                navigation.order: 2

                onClicked: root.badRequested()
            }
        }
    }
}
