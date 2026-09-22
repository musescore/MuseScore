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

Rectangle {
    id: root

    property string hintText: ""
    property string audioComUrl: ""

    property NavigationPanel navigationPanel: null
    property int navigationOrder: 0

    signal convertFromLinkRequested()

    readonly property int contentPadding: 24

    visible: Boolean(root.hintText)

    implicitHeight: contentColumn.implicitHeight + 2 * root.contentPadding

    color: ui.theme.backgroundSecondaryColor
    border.width: 1
    border.color: ui.theme.strokeColor
    radius: 3

    ColumnLayout {
        id: contentColumn

        anchors.centerIn: parent

        width: parent.width - 2 * root.contentPadding
        spacing: 12

        StyledTextLabel {
            id: hintLabel

            Layout.fillWidth: true

            text: root.hintText
            font: ui.theme.bodyFont
            horizontalAlignment: Text.AlignHCenter

            NavigationControl {
                id: audioComNavCtrl

                name: "AudioComLink"
                panel: root.navigationPanel
                order: root.navigationOrder + 1
                enabled: Boolean(root.audioComUrl) && root.enabled && root.visible

                accessible.role: MUAccessible.Button
                accessible.name: "Audio.com"
                accessible.visualItem: hintLabel

                onTriggered: {
                    Qt.openUrlExternally(root.audioComUrl)
                }
            }

            NavigationFocusBorder {
                navigationCtrl: audioComNavCtrl
            }
        }

        FlatButton {
            id: convertFromLinkButton

            Layout.alignment: Qt.AlignHCenter

            text: qsTrc("project/convert", "Convert from link")
            accentButton: true

            navigation.panel: root.navigationPanel
            navigation.order: root.navigationOrder

            onClicked: {
                root.convertFromLinkRequested()
            }
        }
    }
}
