/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import MuseScore.NotationScene 1.0

Item {
    id: root

    property alias navigationSection: navPanel.section
    property int navigationOrderStart: 0

    ImageCapturePanelModel {
        id: panelModel
    }

    NavigationPanel {
        id: navPanel
        name: "ImageCapturePanel"
        direction: NavigationPanel.Horizontal
        order: root.navigationOrderStart
        enabled: root.enabled && root.visible
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // Header
        StyledTextLabel {
            Layout.fillWidth: true
            text: qsTrc("notation", "Image Capture")
            font: ui.theme.largeBodyFont
            horizontalAlignment: Text.AlignLeft
        }

        SeparatorLine { Layout.fillWidth: true }

        // Capture mode toggle
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            StyledTextLabel {
                text: qsTrc("notation", "Capture mode:")
                horizontalAlignment: Text.AlignLeft
            }

            Item { Layout.fillWidth: true }

            ToggleButton {
                id: captureModeToggle

                checked: panelModel.captureModeEnabled

                navigation.panel: navPanel
                navigation.order: 1

                onToggled: {
                    panelModel.captureModeEnabled = !panelModel.captureModeEnabled
                }
            }
        }

        // Info text
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            Layout.margins: 4

            color: ui.theme.backgroundSecondaryColor
            border.color: ui.theme.strokeColor
            border.width: 1
            radius: 4

            StyledTextLabel {
                anchors.fill: parent
                anchors.margins: 8

                text: panelModel.captureInfo
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                font: ui.theme.bodyFont
            }
        }

        // Buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            FlatButton {
                id: exportButton

                Layout.fillWidth: true

                text: qsTrc("notation", "Export as PNG...")
                enabled: panelModel.hasCapture

                navigation.panel: navPanel
                navigation.order: 2

                onClicked: {
                    panelModel.exportCapture()
                }
            }

            FlatButton {
                id: clearButton

                Layout.preferredWidth: 80

                text: qsTrc("notation", "Clear")
                enabled: panelModel.hasCapture

                navigation.panel: navPanel
                navigation.order: 3

                onClicked: {
                    panelModel.clearCapture()
                }
            }
        }

        // Spacer
        Item {
            Layout.fillHeight: true
        }
    }
}
