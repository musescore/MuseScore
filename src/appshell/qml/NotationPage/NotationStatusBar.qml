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
import QtQuick.Window 2.15

import MuseScore.AppShell 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    NotationStatusBarModel {
        id: model
    }

    NavigationSection {
        id: navSec
        name: "NotationStatusBar"
        enabled: root.visible
        order: 7
    }

    NavigationPanel {
        id: navPanel
        name: "NotationStatusBar"
        enabled: root.visible
        order: 0
        direction: NavigationPanel.Horizontal
        section: navSec
    }

    Component.onCompleted: {
        model.load()
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.forceActiveFocus()
    }

    StyledTextLabel {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: statusBarRow.left
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Text.AlignLeft

        text: model.accessibilityInfo

        visible: !hiddenControlsMenuButton.visible
    }

    Row {
        id: statusBarRow

        //! TODO: hiding of controls is disabled because there is a bug
        // Determination of the size of the window content works incorrectly
        readonly property int eps: 100
        //property int remainingSpace: Window.window ? Window.window.width - (viewModeControl.width + zoomControl.width + eps) : 0
        property int remainingSpace: 999999

        anchors.right: parent.right
        anchors.rightMargin: hiddenControlsMenuButton.visible ? 4 : 12

        height: parent.height

        spacing: 10

        SeparatorLine { orientation: Qt.Vertical; visible: workspaceControl.visible }

        FlatButton {
            id: workspaceControl
            anchors.verticalCenter: parent.verticalCenter

            text: model.currentWorkspaceAction.title
            normalStateColor: "transparent"
            visible: statusBarRow.remainingSpace > width + concertPitchControl.width

            navigation.panel: navPanel
            navigation.order: 1

            onClicked: {
                Qt.callLater(model.selectWorkspace)
            }
        }

        SeparatorLine { orientation: Qt.Vertical; visible: concertPitchControl.visible }

        ConcertPitchControl {
            id: concertPitchControl
            anchors.verticalCenter: parent.verticalCenter

            text: model.concertPitchAction.title
            icon: model.concertPitchAction.icon
            checked: model.concertPitchAction.checked
            enabled: model.concertPitchAction.enabled
            visible: statusBarRow.remainingSpace > width

            navigation.panel: navPanel
            navigation.order: 2

            onToggleConcertPitchRequested: {
                model.toggleConcertPitch()
            }
        }

        SeparatorLine { orientation: Qt.Vertical }

        ViewModeControl {
            id: viewModeControl
            anchors.verticalCenter: parent.verticalCenter

            currentViewMode: model.currentViewMode
            availableViewModeList: model.availableViewModeList

            navigation.panel: navPanel
            navigation.order: 3

            onChangeCurrentViewModeRequested: {
                model.setCurrentViewMode(newViewMode)
            }
        }

        ZoomControl {
            id: zoomControl

            anchors.verticalCenter: parent.verticalCenter

            enabled: model.zoomEnabled
            currentZoomPercentage: model.currentZoomPercentage
            minZoomPercentage: model.minZoomPercentage()
            maxZoomPercentage: model.maxZoomPercentage()
            availableZoomList: model.availableZoomList

            navigationPanel: navPanel
            navigationOrderMin: 4

            onChangeZoomPercentageRequested: {
                model.currentZoomPercentage = newZoomPercentage
            }

            onChangeZoomRequested: {
                model.setCurrentZoomIndex(newZoomIndex)
            }

            onZoomInRequested: {
                model.zoomIn()
            }

            onZoomOutRequested: {
                model.zoomOut()
            }
        }

        SeparatorLine { orientation: Qt.Vertical; visible: hiddenControlsMenuButton.visible }

        MenuButton {
            id: hiddenControlsMenuButton

            anchors.verticalCenter: parent.verticalCenter

            visible: !concertPitchControl.visible ||
                     !workspaceControl.visible

            navigation.panel: navPanel
            navigation.order: zoomControl.navigationOrderMax + 1

            menuModel: {
                var result = []

                if (!concertPitchControl.visible) {
                    result.push(model.concertPitchAction)
                }

                if (!workspaceControl.visible) {
                    result.push(model.currentWorkspaceAction)
                }

                return result
            }

            onHandleAction: {
                model.handleAction(actionCode)
            }
        }
    }
}
