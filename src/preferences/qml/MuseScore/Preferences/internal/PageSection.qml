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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

BaseSection {
    id: root

    title: qsTrc("preferences", "Page")

    navigation.direction: NavigationPanel.Both

    property alias scoreInversionEnabled: scoreInversionEnable.checked
    property alias isOnlyInvertInDarkTheme: isOnlyInvertInDarkTheme.checked
    property bool isCurrentThemeDark
    property alias engravingColor: engravingColorPicker.color
    property alias colorAndWallpaper: colorAndWallpaper

    signal scoreInversionEnableChangeRequested(bool enable)
    signal isOnlyInvertInDarkThemeChangeRequested(bool enable)
    signal engravingColorChangeRequested(var newColor)

    GridLayout {
        id: gridSection

        rows: 3
        columns: 2

        rowSpacing: root.rowSpacing
        columnSpacing: root.columnSpacing

        ToggleButton {
            id: scoreInversionEnable
            implicitWidth: root.columnWidth

            text: qsTrc("preferences", "Invert score colors")

            navigation.name: "ScoreInversionBox"
            navigation.panel: root.navigation
            navigation.row: 0
            navigation.column: 0

            onToggled: {
                root.scoreInversionEnableChangeRequested(!checked)
            }
        }

        CheckBox {
            id: isOnlyInvertInDarkTheme
            width: root.columnWidth

            enabled: root.scoreInversionEnabled

            text: qsTrc("preferences", "Only invert colors in dark theme")

            navigation.name: "IsOnlyInvertInDarkThemeBox"
            navigation.panel: root.navigation
            navigation.row: 0
            navigation.column: 1

            onClicked: {
                root.isOnlyInvertInDarkThemeChangeRequested(!checked)
            }
        }

        StyledTextLabel {
            id: engravingColorLabel
            text: qsTrc("preferences", "Engraving color")
            width: root.columnWidth
            horizontalAlignment: Qt.AlignLeft
        }

        ColorPicker {
            id: engravingColorPicker

            width: 112

            navigation.name: "ColorBox"
            navigation.panel: root.navigation
            navigation.row: 1
            navigation.column: 1

            onNewColorSelected: function(newColor) {
                root.engravingColorChangeRequested(newColor)
            }
        }
    }

    ColorAndWallpaperSection {
        id: colorAndWallpaper

        enabled: !root.scoreInversionEnabled || (root.isOnlyInvertInDarkTheme && !root.isCurrentThemeDark)
        opacityOverride: enabled ? 1.0 : ui.theme.itemOpacityDisabled

        wallpaperDialogTitle: qsTrc("preferences", "Choose notepaper")

        navigation: root.navigation
        navigationRowStart: 2
    }
}
