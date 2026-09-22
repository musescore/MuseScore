/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

Rectangle {
    id: root

    enum Status {
        Processing,
        Failed
    }

    property int status: ScoreProcessingPlaceholder.Processing
    property bool compact: false // icon only, no text or buttons
    property int iconSize: 24

    property NavigationPanel navigationPanel: null
    property int navigationRow: 0
    property int navigationColumn: 0

    signal retryRequested()
    signal cancelRequested()

    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    radius: 3
    border.color: ui.theme.strokeColor
    border.width: 1

    Loader {
        anchors.fill: parent

        sourceComponent: root.status === ScoreProcessingPlaceholder.Failed ? failedComp : processingComp
    }

    Component {
        id: processingComp

        Item {
            anchors.fill: parent

            Column {
                anchors.centerIn: parent
                spacing: root.compact ? 0 : 8

                StyledBusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    indicatorSize: root.iconSize
                }

                StyledTextLabel {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTrc("global", "Processing…")
                    font: ui.theme.bodyBoldFont
                    visible: !root.compact
                }
            }

            FlatButton {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                anchors.bottomMargin: 20

                text: qsTrc("global", "Cancel")
                visible: !root.compact

                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow
                navigation.column: root.navigationColumn

                onClicked: root.cancelRequested()
            }
        }
    }

    Component {
        id: failedComp

        Item {
            anchors.fill: parent

            Column {
                anchors.centerIn: parent
                spacing: root.compact ? 0 : 8

                StyledIconLabel {
                    anchors.horizontalCenter: parent.horizontalCenter
                    iconCode: IconCode.WARNING_FILLED
                    font.pixelSize: root.iconSize
                }

                StyledTextLabel {
                    anchors.horizontalCenter: parent.horizontalCenter
                    //: Status label shown on a score's thumbnail when its conversion failed
                    text: qsTrc("project/convert", "Processing failed")
                    font: ui.theme.bodyBoldFont
                    visible: !root.compact
                }
            }

            FlatButton {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                anchors.bottomMargin: 20

                text: qsTrc("global", "Retry")
                visible: !root.compact

                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow
                navigation.column: root.navigationColumn

                onClicked: root.retryRequested()
            }
        }
    }
}
