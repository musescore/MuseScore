/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
import QtQuick.Layouts 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.GraphicalEffects 1.0

Rectangle {
    id: root

    property alias title: titleLabel.text
    property alias subtitle: subtitleLabel.text
    property alias thumbnailUrl: thumbnailImage.source
    property bool selected: false

    signal getSoundLibraryRequested()

    color: ui.theme.backgroundPrimaryColor
    radius: 24

    property NavigationControl navigation: NavigationControl {
        name: root.title
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Button
        accessible.name: "%1. %2. %3".arg(root.title).arg(root.subtitle).arg(getItNowBtn.text)

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.getSoundLibraryRequested()
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16

        spacing: 16

        Item {
            id: thumbnail

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 118
            Layout.preferredHeight: 118

            property real radius: 12

            Image {
                id: thumbnailImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
            }
        }

        Item {
            id: container

            Layout.fillWidth: true
            Layout.fillHeight: true

            StyledTextLabel {
                id: titleLabel

                width: container.width

                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            StyledTextLabel {
                id: subtitleLabel

                anchors.top: titleLabel.bottom
                anchors.topMargin: 4
                anchors.bottom: getItNowBtn.top
                anchors.bottomMargin: 8

                width: container.width

                font: ui.theme.largeBodyFont
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                maximumLineCount: 3
                wrapMode: Text.WordWrap
            }

            FlatButton {
                id: getItNowBtn

                anchors.bottom: parent.bottom
                anchors.topMargin: 4

                width: container.width

                text: qsTrc("musesounds", "Get it now")
                icon: IconCode.OPEN_LINK
                orientation: Qt.Horizontal
                accentButton: true

                onClicked: {
                    root.getSoundLibraryRequested()
                }
            }
        }
    }

    Rectangle {
        id: selectionBorder

        readonly property real padding: 2 // add some padding between image and border, to make border better distinguishable

        anchors.fill: parent
        anchors.margins: -border.width - padding

        visible: root.selected

        color: "transparent"

        border.color: ui.theme.fontPrimaryColor
        border.width: 2
        radius: root.radius - anchors.margins
    }

    NavigationFocusBorder {
        navigationCtrl: root.navigation

        padding: 2
    }
}
