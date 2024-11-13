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
    readonly property int navigationOrderEnd: rightSideNavPanel.order

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
        order: root.navigationOrderStart + 1
    }

    Row {
        id: centralButtonsRow

        readonly property int buttonWidth: Math.max(writeButton.implicitWidth, previewButton.implicitWidth)

        anchors.verticalCenter: parent.verticalCenter
        x: prv.centerX - (centralButtonsRow.width / 2)

        // Workaround - you can't change the radius of specific corners in this version of Qt Quick (and we
        // only want to round the "outer corners" of the central buttons)
        layer.enabled: ui.isEffectsAllowed
        layer.effect: EffectOpacityMask {
            maskSource: Rectangle {
                width: centralButtonsRow.width
                height: centralButtonsRow.height
                radius: 3
            }
        }

        visible: model.currentPanelMode !== PanelMode.EDIT_LAYOUT

        FlatButton {
            id: writeButton

            width: centralButtonsRow.buttonWidth

            icon: IconCode.EDIT
            text: qsTrc("notation", "Write")
            orientation: Qt.Horizontal
            accentButton: model.currentPanelMode === PanelMode.WRITE
            backgroundRadius: 0

            navigation.panel: centralNavPanel
            navigation.row: 0

            disableFocusBorder: true

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

            icon: IconCode.PLAY
            text: qsTrc("notation", "Preview")
            orientation: Qt.Horizontal
            accentButton: model.currentPanelMode === PanelMode.SOUND_PREVIEW
            backgroundRadius: 0

            navigation.panel: centralNavPanel
            navigation.row: 1

            disableFocusBorder: true

            onClicked: {
                root.model.currentPanelMode = PanelMode.SOUND_PREVIEW
            }
        }
    }

    // These focus borders are necessary due to the above "rounded corners" workaround - the
    // opacity mask clips-out the buttons' built-in focus borders
    NavigationFocusBorder {
        id: writeFocusBorder

        anchors {
            fill: null

            left: centralButtonsRow.left
            right: centralButtonsRow.horizontalCenter
            top: centralButtonsRow.top
            bottom: centralButtonsRow.bottom

            rightMargin: separator.width
        }

        navigationCtrl: writeButton.navigation
    }

    NavigationFocusBorder {
        id: previewFocusBorder

        anchors {
            fill: null

            left: centralButtonsRow.horizontalCenter
            right: centralButtonsRow.right
            top: centralButtonsRow.top
            bottom: centralButtonsRow.bottom

            leftMargin: separator.width
        }

        navigationCtrl: previewButton.navigation
    }

    FlatButton {
        id: finishEditingButton

        anchors.verticalCenter: parent.verticalCenter
        x: prv.centerX - (finishEditingButton.width / 2)

        visible: model.currentPanelMode === PanelMode.EDIT_LAYOUT
        text: qsTrc("notation", "Finish editing")
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
        order: root.navigationOrderStart + 2
    }

    Row {
        id: rightSideButtonsRow

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: prv.spacing

        spacing: prv.spacing

        FlatButton {
            icon: IconCode.SPLIT_VIEW_HORIZONTAL
            text: qsTrc("notation", "Layout")
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
            enabled: model.currentPanelMode !== PanelMode.EDIT_LAYOUT
            text: qsTrc("notation", "Customize kit")
            orientation: Qt.Horizontal

            navigation.panel: rightSideNavPanel
            navigation.row: 1

            onClicked: {
                api.launcher.open("muse://devtools/interactive/sample")
            }
        }
    }
}
