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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.GraphicalEffects 1.0

Item {
    id: root

    property string authorName: ""
    property string authorRole: ""
    property string authorPosition: ""
    property string authorDescription: ""
    property string authorOrganizationName: ""
    property string authorAvatarUrl: ""
    property string authorOrganizationUrl: ""

    property color backgroundColor: ui.theme.backgroundSecondaryColor
    property int sideMargin: 46

    property alias navigation: navPanel

    NavigationPanel {
        id: navPanel
        name: "Classes"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Horizontal

        onActiveChanged: function(active) {
            if (active) {
                openMoreInfoButton.navigation.requestActive()
                accessibleInfo.ignored = false
                accessibleInfo.focused = true
            } else {
                accessibleInfo.ignored = true
                accessibleInfo.focused = false
                openMoreInfoButton.accessible.ignored = true
            }
        }
    }

    GradientRectangle {
        id: topGradient

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: flickable.top

        startColor: root.backgroundColor
        endColor: "transparent"
    }

    StyledFlickable {
        id: flickable

        anchors.fill: parent

        contentWidth: parent.width
        contentHeight: authorInfo.height + sideMargin

        topMargin: topGradient.height

        Rectangle {
            anchors.left: parent.left
            anchors.leftMargin: root.sideMargin

            height: authorInfo.height
            width: Math.min(1000, parent.width - 2 * root.sideMargin)

            radius: 12

            color: ui.theme.backgroundPrimaryColor

            Column {
                id: authorInfo

                anchors.left: parent.left
                anchors.leftMargin: 36
                anchors.right: parent.right
                anchors.rightMargin: 36
                anchors.top: parent.top
                anchors.topMargin: 36

                height: childrenRect.height + 72

                spacing: 30

                AccessibleItem {
                    id: accessibleInfo
                    accessibleParent: root.navigation.accessible
                    visualItem: authorInfo
                    role: MUAccessible.Button
                    name: {
                        var template = "%1 %2. %3. %4. %5"

                        return template.arg(root.authorRole)
                        .arg(root.authorName)
                        .arg(root.authorPosition)
                        .arg(root.authorDescription)
                        .arg(openMoreInfoButton.text)
                    }
                }

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

                        layer.enabled: ui.isEffectsAllowed
                        layer.effect: EffectOpacityMask {
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
                    id: openMoreInfoButton
                    orientation: Qt.Horizontal
                    icon: IconCode.OPEN_LINK
                    text: qsTrc("learn", "Open %1").arg(root.authorOrganizationName)
                    accentButton: true

                    navigation.panel: root.navigation
                    navigation.name: "OpenMoreInfoButton"
                    navigation.column: 1
                    navigation.accessible.ignored: true
                    navigation.onActiveChanged: {
                        if (!navigation.active) {
                            accessible.ignored = false
                            accessibleInfo.ignored = true
                        }
                    }

                    onClicked: {
                        api.launcher.openUrl(root.authorOrganizationUrl)
                    }
                }
            }
        }
    }
}
