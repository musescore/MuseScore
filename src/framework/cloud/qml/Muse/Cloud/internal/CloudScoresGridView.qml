/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
import MuseScore.Project 1.0

ScoresGridView {
    id: root

    navigation.name: "OnlineScoresGrid"
    navigation.accessible.name: qsTrc("project", "Online scores grid")

    Component.onCompleted: {
        prv.updateDesiredRowCount()
    }

    QtObject {
        id: prv

        readonly property int remainingFullRowsBelowViewport:
            Math.floor(root.model.rowCount / root.view.columns) - Math.ceil((root.view.contentY + root.view.height) / root.view.cellHeight)

        onRemainingFullRowsBelowViewportChanged: {
            updateDesiredRowCount()
        }

        property bool updateDesiredRowCountScheduled: false

        function updateDesiredRowCount() {
            if (updateDesiredRowCountScheduled) {
                return
            }

            updateDesiredRowCountScheduled = true

            Qt.callLater(function() {
                let newDesiredRowCount = root.model.rowCount + (3 - remainingFullRowsBelowViewport) * view.columns

                if (root.model.desiredRowCount < newDesiredRowCount) {
                    root.model.desiredRowCount = newDesiredRowCount
                }

                updateDesiredRowCountScheduled = false
            })
        }
    }

    view.footer: root.model.state === CloudScoresModel.Loading
                 ? busyIndicatorComp : null

    Component {
        id: busyIndicatorComp

        Item {
            width: GridView.view ? GridView.view.width : 0
            height: indicator.implicitHeight + indicator.anchors.topMargin + indicator.anchors.bottomMargin

            StyledBusyIndicator {
                id: indicator

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: root.view.spacingBetweenRows / 2
                anchors.bottomMargin: root.view.spacingBetweenRows / 2
            }
        }
    }
}
