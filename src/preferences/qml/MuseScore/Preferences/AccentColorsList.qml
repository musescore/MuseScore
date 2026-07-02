/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import Muse.Ui
import Muse.UiComponents

Grid {
    id: root

    property var colors: []
    property int currentColorIndex: -1

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "AccentColorsList"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Both

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive && root.currentColorIndex >= 0) {
                event.setData("controlIndex", [root.navigationRowOf(root.currentColorIndex),
                                                root.navigationColumnOf(root.currentColorIndex)])
            }
        }
    }

    property int navigationRow: 0
    property int navigationColumnStart: 0

    property real sampleSize: 24
    readonly property real totalSampleSize: sampleSize + 6

    signal accentColorChangeRequested(var newColorIndex)

    columns: Math.floor(root.colors.length / 2)
    spacing: 6

    // Visual order of the color indices (first row is left-to-right, second row is right-to-left):
    readonly property var orderedColorIndices: {
        let indices = []

        for (let i = 0; i < root.columns && i < root.colors.length; ++i) {
            indices.push(i)
        }
        for (let j = root.colors.length - 1; j >= root.columns; --j) {
            indices.push(j)
        }
        return indices
    }

    function navigationRowOf(colorIndex) {
        return root.navigationRow + (colorIndex < root.columns ? 0 : 1)
    }

    function navigationColumnOf(colorIndex) {
        return root.navigationColumnStart + (colorIndex < root.columns
                                             ? colorIndex
                                             : root.colors.length - 1 - colorIndex)
    }

    Repeater {
        model: root.orderedColorIndices

        delegate: RoundedRadioButton {
            id: button

            required property var modelData
            readonly property int colorIndex: modelData
            readonly property color accentColor: root.colors[colorIndex]

            width: root.totalSampleSize
            height: width

            checked: root.currentColorIndex === colorIndex

            navigation.name: "AccentColourButton"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowOf(colorIndex)
            navigation.column: root.navigationColumnOf(colorIndex)
            navigation.accessible.name: Utils.accessibleColorDescription(accentColor)

            onToggled: {
                root.accentColorChangeRequested(colorIndex)
            }

            indicator: Rectangle {
                anchors.fill: parent

                color: "transparent"
                border.color: ui.theme.fontPrimaryColor
                border.width: parent.checked ? 1 : 0
                radius: width / 2

                NavigationFocusBorder { navigationCtrl: button.navigation }

                Rectangle {
                    anchors.centerIn: parent

                    width: root.sampleSize
                    height: width
                    radius: width / 2

                    border.color: ui.theme.strokeColor
                    border.width: 1

                    color: button.accentColor
                }
            }

            background: null
        }
    }
}
