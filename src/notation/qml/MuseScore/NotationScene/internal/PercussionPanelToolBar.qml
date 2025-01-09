/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.GraphicalEffects 1.0
import MuseScore.NotationScene 1.0

Item {
    id: root

    required property var model
    required property int panelWidth

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    QtObject {
        id: prv
        // The central buttons are placed centrally relative to the panel, not the toolbar area
        readonly property int tabAreaLength: root.panelWidth - root.width
        readonly property int centerX: (root.panelWidth / 2) - prv.tabAreaLength

        readonly property int spacing: 4
    }

    NavigationPanel {
        id: centralNavPanel
        name: "PercussionPanelToolBarCentral"
        section: root.navigationSection
        order: root.navigationOrderStart
    }

    Row {
        id: centralButtonsRow

        readonly property int buttonWidth: Math.max(writeButton.implicitWidth, previewButton.implicitWidth)

        anchors.verticalCenter: parent.verticalCenter
        x: prv.centerX - (centralButtonsRow.width / 2)

        visible: root.model.currentPanelMode !== PanelMode.EDIT_LAYOUT

        FlatButton {
            id: writeButton

            width: centralButtonsRow.buttonWidth

            enabled: root.model.enabled

            icon: IconCode.EDIT
            text: qsTrc("notation/percussion", "Write")
            orientation: Qt.Horizontal
            accentButton: root.model.currentPanelMode === PanelMode.WRITE
            backgroundRadius: 0

            navigation.panel: centralNavPanel
            navigation.row: 0

            backgroundItem: RoundedRectangle {
                id: writeButtonBackground

                property real backgroundOpacity: ui.theme.buttonOpacityNormal
                color: Utils.colorWithAlpha(writeButton.accentButton ? ui.theme.accentColor : ui.theme.buttonColor, backgroundOpacity)

                topLeftRadius: 3
                topRightRadius: 0
                bottomLeftRadius: 3
                bottomRightRadius: 0

                NavigationFocusBorder {
                    drawOutsideParent: false
                    navigationCtrl: writeButton.navigation
                }

                states: [
                    State {
                        name: "PRESSED"
                        when: writeButton.mouseArea.pressed

                        PropertyChanges {
                            target: writeButtonBackground
                            backgroundOpacity: ui.theme.buttonOpacityHit
                        }
                    },

                    State {
                        name: "HOVERED"
                        when: !writeButton.mouseArea.pressed && writeButton.mouseArea.containsMouse

                        PropertyChanges {
                            target: writeButtonBackground
                            backgroundOpacity: ui.theme.buttonOpacityHover
                        }
                    }
                ]
            }

            onClicked: {
                root.model.currentPanelMode = PanelMode.WRITE
            }
        }

        Rectangle {
            id: separator
            height: parent.height
            width: 1
            color: ui.theme.strokeColor
        }

        FlatButton {
            id: previewButton

            width: centralButtonsRow.buttonWidth

            enabled: root.model.enabled

            icon: IconCode.PLAY
            text: qsTrc("notation/percussion", "Preview")
            orientation: Qt.Horizontal
            accentButton: root.model.currentPanelMode === PanelMode.SOUND_PREVIEW
            backgroundRadius: 0

            navigation.panel: centralNavPanel
            navigation.row: 1

            backgroundItem: RoundedRectangle {
                id: previewButtonBackground

                property real backgroundOpacity: ui.theme.buttonOpacityNormal
                color: Utils.colorWithAlpha(previewButton.accentButton ? ui.theme.accentColor : ui.theme.buttonColor, backgroundOpacity)

                topLeftRadius: 0
                topRightRadius: 3
                bottomLeftRadius: 0
                bottomRightRadius: 3

                NavigationFocusBorder {
                    drawOutsideParent: false
                    navigationCtrl: previewButton.navigation
                }

                states: [
                    State {
                        name: "PRESSED"
                        when: previewButton.mouseArea.pressed

                        PropertyChanges {
                            target: previewButtonBackground
                            backgroundOpacity: ui.theme.buttonOpacityHit
                        }
                    },

                    State {
                        name: "HOVERED"
                        when: !previewButton.mouseArea.pressed && previewButton.mouseArea.containsMouse

                        PropertyChanges {
                            target: previewButtonBackground
                            backgroundOpacity: ui.theme.buttonOpacityHover
                        }
                    }
                ]
            }
            onClicked: {
                root.model.currentPanelMode = PanelMode.SOUND_PREVIEW
            }
        }
    }

    FlatButton {
        id: finishEditingButton

        anchors.verticalCenter: parent.verticalCenter
        x: prv.centerX - (finishEditingButton.width / 2)

        enabled: root.model.enabled

        visible: root.model.currentPanelMode === PanelMode.EDIT_LAYOUT
        text: qsTrc("notation/percussion", "Finish editing")
        orientation: Qt.Horizontal
        accentButton: true

        navigation.panel: centralNavPanel
        navigation.row: 0

        onClicked: {
            root.model.finishEditing()
        }
    }

    NavigationPanel {
        id: rightSideNavPanel
        name: "PercussionPanelToolBarRightSide"
        section: root.navigationSection
        order: centralNavPanel.order + 1
    }

    Row {
        id: rightSideButtonsRow

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: prv.spacing

        spacing: prv.spacing

        FlatButton {
            enabled: root.model.enabled

            icon: IconCode.SPLIT_VIEW_HORIZONTAL
            text: qsTrc("notation/percussion", "Layout")
            orientation: Qt.Horizontal

            navigation.panel: rightSideNavPanel
            navigation.row: 0

            onClicked: {
                menuLoader.toggleOpened(root.model.layoutMenuItems)
            }

            StyledMenuLoader {
                id: menuLoader

                onHandleMenuItem: function(itemId) {
                    root.model.handleMenuItem(itemId)
                }
            }
        }

        FlatButton {
            enabled: root.model.enabled && root.model.currentPanelMode !== PanelMode.EDIT_LAYOUT
            text: qsTrc("notation/percussion", "Customize kit")
            orientation: Qt.Horizontal

            navigation.panel: rightSideNavPanel
            navigation.row: 1

            onClicked: {
                root.model.customizeKit()
            }
        }
    }
}
