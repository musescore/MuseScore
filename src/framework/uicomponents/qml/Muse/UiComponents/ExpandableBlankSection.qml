/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import Muse.Ui 1.0
import Muse.UiComponents

FocusScope {
    id: root

    property alias title: titleLabel.text
    property alias titleFont: titleLabel.font

    property alias accessory: accessoryLoader.sourceComponent

    property bool isExpanded: true

    property alias navigation: navCtrl

    anchors.left: parent.left
    anchors.leftMargin: -expandButtonIcon.width / 3
    anchors.right: parent.right

    implicitWidth: expandSectionRow.implicitWidth
    implicitHeight: expandSectionRow.implicitHeight

    NavigationControl {
        id: navCtrl
        name: root.title
        enabled: root.enabled && root.visible
        accessible.role: MUAccessible.ListItem
        accessible.name: root.title

        onTriggered: {
            root.isExpanded = !root.isExpanded
        }
    }

    NavigationFocusBorder {
        navigationCtrl: navCtrl
        drawOutsideParent: false
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        enabled: root.enabled
        hoverEnabled: true

        onClicked: {
            navigation.requestActiveByInteraction()

            root.isExpanded = !root.isExpanded
        }
    }

    RowLayout {
        id: expandSectionRow
        anchors.fill: parent

        spacing: 4

        Item {
            id: expandButton

            Layout.preferredWidth: expandButtonIcon.font.pixelSize
            Layout.preferredHeight: expandButtonIcon.font.pixelSize

            StyledIconLabel {
                id: expandButtonIcon

                anchors.centerIn: parent

                rotation: root.isExpanded ? 0 : -90

                iconCode: IconCode.SMALL_ARROW_DOWN

                Behavior on rotation {
                    NumberAnimation {
                        easing.type: Easing.OutQuad
                        duration: 50
                    }
                }
            }
        }

        StyledTextLabel {
            id: titleLabel
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignLeft
            font: ui.theme.bodyBoldFont
            wrapMode: Text.Wrap
        }

        Loader {
            id: accessoryLoader
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            visible: root.isExpanded
        }
    }

    states: [
        State {
            name: "DISABLED"
            when: !root.enabled

            PropertyChanges { target: root; isExpanded: false; opacity: ui.theme.itemOpacityDisabled }
        }
    ]
}
