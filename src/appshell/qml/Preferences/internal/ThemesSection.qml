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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import "../../shared"

BaseSection {
    id: root

    title: highContrastEnabled ? qsTrc("appshell", "High Contrast Themes") : qsTrc("appshell", "Themes")
    navigation.direction: NavigationPanel.Both

    property bool highContrastEnabled: false

    property alias themes: themeSamplesList.themes
    property alias currentThemeCode: themeSamplesList.currentThemeCode

    property alias accentColors: accentColorsSection.colors
    property alias currentAccentColorIndex: accentColorsSection.currentColorIndex

    signal themeChangeRequested(var newThemeCode)
    signal highContrastChangeRequested(bool enabled)
    signal accentColorChangeRequested(var newColorIndex)

    signal ensureContentVisibleRequested(var contentRect)

    CheckBox {
        width: parent.height

        text: qsTrc("appshell", "Enable high-contrast")

        checked: root.highContrastEnabled

        navigation.name: "EnableHighContrastBox"
        navigation.panel: root.navigation
        navigation.row: 0
        navigation.column: 0

        onClicked: {
            root.highContrastChangeRequested(!checked)
        }
    }

    ThemeSamplesList {
        id: themeSamplesList
        width: parent.width
        spacing: root.columnWidth + root.columnSpacing - sampleWidth

        navigationPanel: root.navigation
        navigationRow: 1

        onThemeChangeRequested: function(newThemeCode) {
            root.themeChangeRequested(newThemeCode)
        }
    }

    AccentColorsSection {
        id: accentColorsSection

        columnWidth: root.columnWidth
        spacing: root.columnSpacing

        visible: !root.highContrastEnabled

        navigation.section: root.navigation.section
        navigation.order: root.navigation.order + 1

        onAccentColorChangeRequested: function(newColorIndex) {
            root.accentColorChangeRequested(newColorIndex)
        }

        onFocusChanged: {
            if (activeFocus) {
                root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
            }
        }
    }
}
