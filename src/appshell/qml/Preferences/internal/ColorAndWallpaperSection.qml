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
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0

Column {
    id: root

    property alias title: titleLabel.text
    property alias wallpaperDialogTitle: wallpaperPicker.dialogTitle

    property bool useColor: true
    property alias color: colorPicker.color

    property alias wallpaperPath: wallpaperPicker.path
    property alias wallpapersDir: wallpaperPicker.dir
    property alias wallpaperFilter: wallpaperPicker.filter

    property int firstColumnWidth: 0

    signal useColorChangeRequested(var newValue)
    signal colorChangeRequested(var newColor)
    signal wallpaperPathChangeRequested(var newWallpaperPath)

    spacing: 18

    StyledTextLabel {
        id: titleLabel

        font: ui.theme.bodyBoldFont
    }

    GridLayout {
        rows: 2
        columns: 2

        rowSpacing: 8
        columnSpacing: 0

        RoundedRadioButton {
            implicitWidth: root.firstColumnWidth

            checked: root.useColor
            text: qsTrc("appshell", "Colour:")

            onClicked: {
                root.useColorChangeRequested(true)
            }
        }

        ColorPicker {
            id: colorPicker

            width: 112

            enabled: root.useColor

            onNewColorSelected: {
                root.colorChangeRequested(newColor)
            }
        }

        RoundedRadioButton {
            implicitWidth: root.firstColumnWidth

            checked: !root.useColor
            text: qsTrc("appshell", "Wallpaper:")

            onClicked: {
                root.useColorChangeRequested(false)
            }
        }

        FilePicker {
            id: wallpaperPicker

            width: 208

            enabled: !root.useColor

            onPathEdited: {
                root.wallpaperPathChangeRequested(newPath)
            }
        }
    }
}
