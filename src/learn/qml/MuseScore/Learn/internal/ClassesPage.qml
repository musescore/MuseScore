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
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

FocusScope {
    id: root

    property string authorName: ""
    property string authorRole: ""
    property string authorPosition: ""
    property string authorDescription: ""
    property string authorOrganizationName: ""
    property string authorAvatarUrl: ""

    property alias navigation: navPanel
    property int sideMargin: 46

    signal requestOpenVideo(string videoId)
    signal requestActiveFocus()

    signal requestOpenOrganizationUrl()

    NavigationPanel {
        id: navPanel
        direction: NavigationPanel.Both
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: root.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: root.sideMargin

        height: authorInfo.height

        radius: 12

        color: ui.theme.backgroundPrimaryColor

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.requestActiveFocus()
                root.forceActiveFocus()
            }
        }

        Column {
            id: authorInfo

            anchors.left: parent.left
            anchors.leftMargin: 36
            anchors.right: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 36

            height: childrenRect.height + 72

            spacing: 30

            Row {
                width: parent.width
                height: childrenRect.height

                spacing: 30

                Image {
                    id: avatar

                    height: implicitHeight

                    source: root.authorAvatarUrl
                    sourceSize: Qt.size(120, 120)

                    fillMode: Image.PreserveAspectCrop

                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Rectangle {
                            width: avatar.width
                            height: avatar.height
                            radius: width / 2
                            visible: false
                        }
                    }
                }

                Column {
                    height: childrenRect.height
                    anchors.verticalCenter: avatar.verticalCenter

                    spacing: 8

                    StyledTextLabel {
                        height: implicitHeight

                        text: root.authorRole
                        horizontalAlignment: Text.AlignLeft
                        font {
                            family: ui.theme.bodyFont.family
                            pixelSize: ui.theme.bodyFont.pixelSize
                            capitalization: Font.AllUppercase
                        }
                    }

                    StyledTextLabel {
                        height: implicitHeight

                        text: root.authorName
                        horizontalAlignment: Text.AlignLeft
                        font: ui.theme.headerBoldFont
                    }

                    StyledTextLabel {
                        height: implicitHeight

                        text: root.authorPosition
                        horizontalAlignment: Text.AlignLeft
                        font: ui.theme.largeBodyFont
                    }
                }
            }

            StyledTextLabel {
                height: implicitHeight
                width: parent.width

                text: root.authorDescription
                horizontalAlignment: Text.AlignLeft
                font: ui.theme.largeBodyFont
                wrapMode: Text.Wrap
            }

            FlatButton {
                orientation: Qt.Horizontal
                icon: IconCode.OPEN_LINK
                text: qsTrc("learn", "Open") + " " + root.authorOrganizationName
                accentButton: true

                onClicked: {
                    root.requestOpenOrganizationUrl()
                }
            }
        }
    }
}
