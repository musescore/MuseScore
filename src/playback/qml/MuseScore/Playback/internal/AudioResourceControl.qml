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
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0

Item {
    id: root

    property QtObject resourceItemModel: null

    property bool supportsByPassing: root.resourceItemModel ? !root.resourceItemModel.isBlank : true
    property bool supportsTitle: true
    property bool supportsMenu: true

    property string title: root.resourceItemModel ? root.resourceItemModel.title : ""
    property bool active: root.resourceItemModel ? !root.resourceItemModel.isBlank : false

    property bool resourcePickingActive: false

    property bool hovered: rootMouseArea.containsMouse

    signal turnedOn()
    signal turnedOff()

    signal titleClicked()

    height: 22
    width: 96

    RowLayout {
        anchors.fill: parent

        spacing: 0

        Loader {
            id: activityLoader

            Layout.preferredWidth: activityLoader.active ? 24 : 0
            Layout.preferredHeight: root.height
            Layout.alignment: Qt.AlignLeft

            active: root.supportsByPassing && (root.hovered || root.resourcePickingActive)

            sourceComponent: FlatButton {
                id: activityButton

                icon: IconCode.BYPASS

                backgroundItem: RoundedRectangle {
                    id: activityButtonBackground

                    color: root.active ? ui.theme.accentColor : ui.theme.buttonColor
                    opacity: ui.theme.buttonOpacityNormal

                    topLeftRadius: 3
                    topRightRadius: 0
                    bottomLeftRadius: 3
                    bottomRightRadius: 0

                    border.width: ui.theme.borderWidth
                    border.color: ui.theme.strokeColor

                    states: [
                        State {
                            name: "PRESSED"
                            when: activityButton.mouseArea.pressed

                            PropertyChanges {
                                target: activityButtonBackground
                                opacity: ui.theme.buttonOpacityHit
                            }
                        },

                        State {
                            name: "HOVERED"
                            when: root.hovered

                            PropertyChanges {
                                target: activityButtonBackground
                                opacity: ui.theme.buttonOpacityHover
                            }
                        }
                    ]

                    Rectangle {
                        id: rightSeparator

                        anchors.right: parent.right

                        height: root.height
                        width: 1

                        color: ui.theme.fontPrimaryColor
                        opacity: 0.3
                    }
                }

                onClicked: {
                    if (root.active) {
                        root.turnedOff()
                    } else {
                        root.turnedOn()
                    }
                }
            }
        }

        Loader {
            id: titleLoader

            Layout.fillWidth: true
            Layout.maximumWidth: 96
            Layout.preferredWidth: 96
            Layout.minimumWidth: 48
            Layout.preferredHeight: root.height

            active: root.supportsTitle

            sourceComponent: FlatButton {
                id: titleButton

                height: root.height
                width: titleLoader.width

                backgroundItem: RoundedRectangle {
                    id: titleButtonBackground

                    color: root.active ? ui.theme.accentColor : ui.theme.buttonColor
                    opacity: ui.theme.buttonOpacityNormal
                    radius: 3

                    border.width: ui.theme.borderWidth
                    border.color: ui.theme.strokeColor

                    states: [
                        State {
                            name: "PRESSED"
                            when: titleButton.mouseArea.pressed

                            PropertyChanges {
                                target: titleButtonBackground
                                opacity: ui.theme.buttonOpacityHit
                            }
                        },

                        State {
                            name: "HOVERED"
                            when: root.hovered || root.resourcePickingActive

                            PropertyChanges {
                                target: titleButtonBackground
                                topRightRadius: 0
                                bottomRightRadius: topRightRadius
                                topLeftRadius: root.supportsByPassing ? 0 : 3
                                bottomLeftRadius: topLeftRadius
                                opacity: ui.theme.buttonOpacityHover
                            }

                            PropertyChanges {
                                target: titleButton
                                enabled: root.resourceItemModel ? root.resourceItemModel.hasNativeEditorSupport : true
                            }
                        }
                    ]
                }

                contentItem: StyledTextLabel {
                    height: root.height
                    width: titleLoader.width

                    text: root.title
                }

                onClicked: {
                    root.titleClicked()
                }
            }
        }

        Loader {
            id: selectorLoader

            Layout.preferredWidth: selectorLoader.active ? 24 : 0
            Layout.preferredHeight: root.height
            Layout.alignment: Qt.AlignRight

            active: (root.hovered || root.resourcePickingActive) && root.supportsMenu

            sourceComponent: FlatButton {
                id: menuButton

                icon: IconCode.SMALL_ARROW_DOWN

                backgroundItem: RoundedRectangle {
                    id: menuButtonBackground

                    color: root.active ? ui.theme.accentColor : ui.theme.buttonColor
                    opacity: ui.theme.buttonOpacityNormal

                    topLeftRadius: 0
                    topRightRadius: 3
                    bottomLeftRadius: 0
                    bottomRightRadius: 3

                    border.width: ui.theme.borderWidth
                    border.color: ui.theme.strokeColor

                    states: [
                        State {
                            name: "PRESSED"
                            when: root.resourcePickingActive

                            PropertyChanges {
                                target: menuButtonBackground
                                opacity: ui.theme.buttonOpacityHit
                            }
                        },

                        State {
                            name: "HOVERED"
                            when: root.hovered && !root.resourcePickingActive

                            PropertyChanges {
                                target: menuButtonBackground
                                opacity: ui.theme.buttonOpacityHover
                            }
                        }
                    ]

                    Rectangle {
                        id: leftSeparator

                        anchors.left: parent.left

                        height: root.height
                        width: 1

                        color: ui.theme.fontPrimaryColor
                        opacity: 0.3
                    }
                }

                StyledMenuLoader {
                    id: menuLoader

                    menuAnchorItem: ui.rootItem
                    navigation: menuButton.navigation

                    onHandleMenuItem: {
                        if (root.resourceItemModel) {
                            root.resourceItemModel.handleMenuItem(itemId)
                        }
                    }

                    onOpened: {
                        root.resourcePickingActive = true
                    }

                    onClosed: {
                        root.resourcePickingActive = false
                    }
                }

                Connections {
                    target: root.resourceItemModel
                    function onAvailableResourceListResolved(resources) {
                        menuLoader.toggleOpened(resources)
                    }
                }

                onClicked: {
                    if (root.resourceItemModel) {
                        root.resourceItemModel.requestAvailableResources()
                    }
                }
            }
        }
    }

    MouseArea {
        id: rootMouseArea

        anchors.fill: parent
        propagateComposedEvents: true
        hoverEnabled: true

        onClicked: mouse.accepted = false
        onPressed: mouse.accepted = false
        onReleased: mouse.accepted = false
        onDoubleClicked: mouse.accepted = false
        onPositionChanged: mouse.accepted = false
        onPressAndHold: mouse.accepted = false
    }
}
