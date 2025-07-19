/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Audio 1.0

Item {
    id: root

    property QtObject resourceItemModel: null

    property var menuAnchorItem: undefined

    readonly property string title: root.resourceItemModel ? root.resourceItemModel.title : ""
    readonly property bool isActive: root.resourceItemModel ? root.resourceItemModel.isActive : false
    readonly property bool isBlank: root.resourceItemModel ? root.resourceItemModel.isBlank : true

    property bool supportsByPassing: !isBlank
    readonly property bool supportsTitle: !isBlank || !showAdditionalButtons
    readonly property bool supportsMenu: true

    readonly property color separatorColor: ui.theme.borderWidth > 0 ? ui.theme.strokeColor : Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.3)

    property bool resourcePickingActive: false

    readonly property bool showAdditionalButtons: rootMouseArea.containsMouse || (navigationPanel ? navigationPanel.highlight : false) || resourcePickingActive

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0
    readonly property int navigationRowEnd: navigationRowStart + 2
    property string navigationName: ""
    property string accessibleName: ""

    signal turnedOn()
    signal turnedOff()

    signal titleClicked()

    signal navigateControlIndexChanged(var index)

    height: 24
    width: 96

    QtObject {
        id: prv

        // We can't just check the containsMouse property of the mouseAreas of the buttons themselves,
        // because that property will always be false because this MouseArea is (and must be) on top
        readonly property bool isActivityButtonHovered: rootMouseArea.containsMouse && activityLoader.visible && rootMouseArea.mouseX < root.height // activityLoader.visible && activityLoader.contains(rootMouseArea.mapToItem(activityLoader, rootMouseArea.mouseX, rootMouseArea.mouseY))
        readonly property bool isTitleButtonHovered: rootMouseArea.containsMouse && titleLoader.visible && titleLoader.contains(rootMouseArea.mapToItem(titleLoader, rootMouseArea.mouseX, rootMouseArea.mouseY))
        readonly property bool isSelectorButtonHovered: rootMouseArea.containsMouse && selectorLoader.visible && selectorLoader.contains(rootMouseArea.mapToItem(selectorLoader, rootMouseArea.mouseX, rootMouseArea.mouseY))
    }

    RowLayout {
        anchors.fill: parent

        spacing: 0

        Loader {
            id: activityLoader

            Layout.fillHeight: true
            Layout.preferredWidth: root.height
            Layout.alignment: Qt.AlignLeft

            visible: root.supportsByPassing && root.showAdditionalButtons
            active: visible

            sourceComponent: FlatButton {
                id: activityButton

                icon: IconCode.BYPASS

                navigation.panel: root.navigationPanel
                navigation.name: root.navigationName + "ActivityButton"
                navigation.row: root.navigationRowStart + 1
                navigation.accessible.name: root.accessibleName + " " + root.title + " " + qsTrc("playback", "Bypass")
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                    }
                }

                backgroundItem: RoundedRectangle {
                    id: activityButtonBackground

                    property real backgroundOpacity: ui.theme.buttonOpacityNormal
                    color: Utils.colorWithAlpha(root.isActive ? ui.theme.accentColor : ui.theme.buttonColor, backgroundOpacity)

                    topLeftRadius: 3
                    topRightRadius: 0
                    bottomLeftRadius: 3
                    bottomRightRadius: 0

                    NavigationFocusBorder {
                        navigationCtrl: activityButton.navigation
                    }

                    states: [
                        State {
                            name: "PRESSED"
                            when: activityButton.mouseArea.pressed

                            PropertyChanges {
                                target: activityButtonBackground
                                backgroundOpacity: ui.theme.buttonOpacityHit
                            }
                        },

                        State {
                            name: "HOVERED"
                            when: !activityButton.mouseArea.pressed && prv.isActivityButtonHovered

                            PropertyChanges {
                                target: activityButtonBackground
                                backgroundOpacity: ui.theme.buttonOpacityHover
                            }
                        }
                    ]

                    SeparatorLine {
                        anchors.right: parent.right
                        orientation: Qt.Vertical
                        color: root.separatorColor
                    }
                }

                onClicked: {
                    if (root.isActive) {
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
            Layout.fillHeight: true

            visible: root.supportsTitle
            active: visible

            sourceComponent: FlatButton {
                id: titleButton

                anchors.fill: parent

                enabled: root.showAdditionalButtons
                         ? (root.resourceItemModel ? root.resourceItemModel.hasNativeEditorSupport : false)
                         : true

                navigation.panel: root.navigationPanel
                navigation.name: root.navigationName + "TitleButton"
                navigation.row: root.navigationRowStart + 2
                navigation.accessible.name: root.accessibleName + " " + root.title
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                    }
                }

                backgroundItem: RoundedRectangle {
                    id: titleButtonBackground

                    property real backgroundOpacity: ui.theme.buttonOpacityNormal
                    color: Utils.colorWithAlpha(root.isActive ? ui.theme.accentColor : ui.theme.buttonColor, backgroundOpacity)

                    topLeftRadius: activityLoader.visible ? 0 : 3
                    topRightRadius: selectorLoader.visible ? 0 : 3
                    bottomLeftRadius: topLeftRadius
                    bottomRightRadius: topRightRadius

                    NavigationFocusBorder {
                        navigationCtrl: titleButton.navigation
                    }

                    states: [
                        State {
                            name: "DISABLED"
                            when: !titleButton.enabled
                        },

                        State {
                            name: "PRESSED"
                            when: titleButton.mouseArea.pressed

                            PropertyChanges {
                                target: titleButtonBackground
                                backgroundOpacity: ui.theme.buttonOpacityHit
                            }
                        },

                        State {
                            name: "HOVERED"
                            when: !titleButton.mouseArea.pressed && prv.isTitleButtonHovered

                            PropertyChanges {
                                target: titleButtonBackground
                                backgroundOpacity: ui.theme.buttonOpacityHover
                            }
                        }
                    ]
                }

                contentItem: StyledTextLabel {
                    width: titleButton.width - 8 // 4px padding on each side
                    height: titleButton.height

                    text: root.title
                }

                onClicked: {
                    root.titleClicked()
                }
            }
        }

        Loader {
            id: selectorLoader

            Layout.fillWidth: !titleLoader.visible
            Layout.fillHeight: true
            Layout.preferredWidth: root.height
            Layout.alignment: Qt.AlignRight

            visible: root.showAdditionalButtons && root.supportsMenu
            active: visible

            sourceComponent: FlatButton {
                id: menuButton

                navigation.panel: root.navigationPanel
                navigation.name: root.navigationName + "MenuButton"
                navigation.row: root.navigationRowStart + 3
                navigation.accessible.name: root.accessibleName + " " + qsTrc("playback", "Menu")
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                    }
                }

                contentItem: Item {
                    width: menuButton.width
                    height: menuButton.height

                    StyledIconLabel {
                        anchors.right: parent.right
                        width: titleLoader.visible ? parent.width : parent.height
                        height: parent.height
                        iconCode: IconCode.SMALL_ARROW_DOWN
                    }
                }

                backgroundItem: RoundedRectangle {
                    id: menuButtonBackground

                    property real backgroundOpacity: ui.theme.buttonOpacityNormal
                    color: Utils.colorWithAlpha(root.isActive ? ui.theme.accentColor : ui.theme.buttonColor, backgroundOpacity)

                    topLeftRadius: titleLoader.visible ? 0 : 3
                    bottomLeftRadius: topLeftRadius
                    topRightRadius: 3
                    bottomRightRadius: topRightRadius

                    NavigationFocusBorder {
                        navigationCtrl: menuButton.navigation
                    }

                    states: [
                        State {
                            name: "PRESSED"
                            when: menuButton.mouseArea.pressed || menuLoader.isMenuOpened

                            PropertyChanges {
                                target: menuButtonBackground
                                backgroundOpacity: ui.theme.buttonOpacityHit
                            }
                        },

                        State {
                            name: "HOVERED"
                            when: !menuButton.mouseArea.pressed && prv.isSelectorButtonHovered

                            PropertyChanges {
                                target: menuButtonBackground
                                backgroundOpacity: ui.theme.buttonOpacityHover
                            }
                        }
                    ]

                    SeparatorLine {
                        visible: titleLoader.visible
                        anchors.left: parent.left
                        orientation: Qt.Vertical
                        color: root.separatorColor
                    }
                }

                StyledMenuLoader {
                    id: menuLoader

                    onHandleMenuItem: function(itemId) {
                        if (root.resourceItemModel) {
                            Qt.callLater(root.resourceItemModel.handleMenuItem, itemId)
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

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: ui.theme.strokeColor
        border.width: root.isBlank ? 1 : ui.theme.borderWidth
        radius: 3
    }

    MouseArea {
        id: rootMouseArea
        anchors.fill: parent

        enabled: parent.enabled
        acceptedButtons: Qt.NoButton
        hoverEnabled: true
    }
}
