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
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import MuseScore.AppShell 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Playback 1.0

Item {
    id: root

    NotationStatusBarModel {
        id: model
    }

    property NavigationSection navigationSection: NavigationSection {
        id: navSec
        name: "NotationStatusBar"
        enabled: root.enabled && root.visible
        order: 8
    }

    NavigationPanel {
        id: navPanel
        name: "NotationStatusBar"
        enabled: root.enabled && root.visible
        order: 0
        direction: NavigationPanel.Horizontal
        section: navSec
    }

    RowLayout {
        id: statusBarRow

        //! TODO: hiding of controls is disabled because there is a bug
        // Determination of the size of the window content works incorrectly
        readonly property int eps: 100
        //property int remainingSpace: Window.window ? Window.window.width - (viewModeControl.width + zoomControl.width + eps) : 0
        property int remainingSpace: 999999

        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 4

        height: parent.height

        spacing: 4

        PlaybackLoadingInfo {
            id: playbackLoadingInfo
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: 28
            Layout.preferredWidth: 312

            onStarted: {
                visible = true
            }

            onFinished: {
                visible = false
            }
        }

        SeparatorLine { orientation: Qt.Vertical; visible: playbackLoadingInfo.visible }

        StyledTextLabel {
            id: accessibiityInfo
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

            text: model.accessibilityInfo
            horizontalAlignment: Text.AlignLeft

            visible: !hiddenControlsMenuButton.visible
        }

        OnlineSoundsStatusView {
            id: onlineSoundsStatusView

            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: 28

            navigationPanel: navPanel
            navigationOrder: 1
        }

        SeparatorLine { orientation: Qt.Vertical; visible: workspaceControl.visible }

        FlatButton {
            id: workspaceControl
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: 28

            text: model.currentWorkspaceItem.title
            icon: IconCode.WORKSPACE
            orientation: Qt.Horizontal

            transparent: true
            visible: statusBarRow.remainingSpace > width + concertPitchControl.width

            navigation.panel: navPanel
            navigation.order: 2

            onClicked: {
                menuLoader.toggleOpened(model.currentWorkspaceItem.subitems)
            }

            StyledMenuLoader {
                id: menuLoader

                menuAnchorItem: ui.rootItem

                onHandleMenuItem: function(itemId) {
                    Qt.callLater(model.handleWorkspacesMenuItem, itemId)
                }
            }
        }

        SeparatorLine { orientation: Qt.Vertical; visible: concertPitchControl.visible }

        ConcertPitchControl {
            id: concertPitchControl
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: 28

            text: model.concertPitchItem.title
            icon: model.concertPitchItem.icon
            checked: model.concertPitchItem.checked
            enabled: model.concertPitchItem.enabled
            visible: statusBarRow.remainingSpace > width

            navigation.panel: navPanel
            navigation.order: 3

            onToggleConcertPitchRequested: {
                model.toggleConcertPitch()
            }
        }

        SeparatorLine { orientation: Qt.Vertical }

        ViewModeControl {
            id: viewModeControl
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: 28

            currentViewMode: model.currentViewMode
            availableViewModeList: model.availableViewModeList

            navigation.panel: navPanel
            navigation.order: 4

            onChangeCurrentViewModeRequested: function(newViewMode) {
                model.setCurrentViewMode(newViewMode)
            }
        }

        ZoomControl {
            id: zoomControl
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: 28

            enabled: model.zoomEnabled
            currentZoomPercentage: model.currentZoomPercentage
            minZoomPercentage: model.minZoomPercentage()
            maxZoomPercentage: model.maxZoomPercentage()
            availableZoomList: model.availableZoomList

            navigationPanel: navPanel
            navigationOrderMin: 5

            onChangeZoomPercentageRequested: function(newZoomPercentage) {
                model.currentZoomPercentage = newZoomPercentage
            }

            onChangeZoomRequested: function(zoomId) {
                model.setCurrentZoom(zoomId)
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

            Layout.alignment: Qt.AlignVCenter

            visible: !concertPitchControl.visible ||
                     !workspaceControl.visible

            navigation.panel: navPanel
            navigation.order: zoomControl.navigationOrderMax + 1

            menuModel: {
                var result = []

                if (!concertPitchControl.visible) {
                    result.push(model.concertPitchItem)
                }

                if (!workspaceControl.visible) {
                    result.push(model.currentWorkspaceItem)
                }

                return result
            }

            onHandleMenuItem: function(itemId) {
                switch (itemId) {
                case model.concertPitchItem.id:
                    model.handleAction(model.concertPitchItem.code)
                    break
                case model.currentWorkspaceItem.id:
                    model.handleAction(model.currentWorkspaceItem.code)
                    break
                }
            }
        }
    }
}
