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
import QtQuick

import Muse.Ui
import Muse.UiComponents

import "internal"

ListView {
    id: root

    required property var themes // re-read on every color edit, used to paint the samples
    property alias themeCodes: root.model // does not change on color edits, so that the delegates aren't destroyed & re-created
    property string currentThemeCode

    currentIndex: themeCodes.indexOf(currentThemeCode)

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "ThemeSamplesList"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Horizontal

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive) {
                event.setData("controlIndex", [navigationRow, navigationColumnStart + root.currentIndex])
            }
        }
    }

    property int navigationRow: -1
    property int navigationColumnStart: 0
    readonly property int navigationColumnEnd: navigationColumnStart + count

    signal themeChangeRequested(var newThemeCode)

    readonly property int sampleWidth: 112
    readonly property int sampleHeight: 120

    implicitWidth: count * sampleWidth + (count - 1) * spacing
    height: contentHeight
    contentHeight: sampleHeight

    orientation: Qt.Horizontal
    interactive: false

    spacing: 64

    delegate: Column {
        width: sampleWidth
        height: sampleHeight

        spacing: 16

        readonly property var themeData: root.themes.find((theme) => theme.codeKey === modelData)

        ThemeSample {
            theme: parent.themeData ?? ui.theme

            onClicked: {
                root.themeChangeRequested(modelData)
            }
        }

        RoundedRadioButton {
            width: parent.width
            checked: root.currentThemeCode === modelData
            text: parent.themeData ? parent.themeData.title : ""

            navigation.name: text
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: root.navigationColumnStart + index

            onToggled: {
                root.themeChangeRequested(modelData)
            }
        }
    }
}
