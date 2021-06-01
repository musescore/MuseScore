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

import MuseScore.AppShell 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    NotationStatusBarModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    StyledTextLabel {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: statusBarRow.left
        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Text.AlignLeft

        text: model.accessibilityInfo
    }

    Row {
        id: statusBarRow

        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        spacing: 12

        FlatButton {
            text: qsTrc("workspace", "Workspace: ") + model.currentWorkspaceName

            normalStateColor: "transparent"

            onClicked: {
                Qt.callLater(model.selectWorkspace)
            }
        }

        ConcertPitchControl {
            concertPitchEnabled: model.concertPitchEnabled

            onToggleConcertPitchRequested: {
                model.toggleConcertPitch()
            }
        }

        ViewModeControl {
            currentViewMode: model.currentViewMode
            availableViewModeList: model.availableViewModeList

            onChangeCurrentViewModeRequested: {
                model.setCurrentViewMode(newViewMode)
            }
        }

        ZoomControl {
            currentZoom: model.currentZoom

            onZoomInRequested: {
                model.zoomIn()
            }

            onZoomOutRequested: {
                model.zoomOut()
            }
        }
    }
}
