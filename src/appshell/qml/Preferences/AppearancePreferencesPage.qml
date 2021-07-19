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

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    contentHeight: content.height

    AppearancePreferencesModel {
        id: appearanceModel
    }

    Component.onCompleted: {
        appearanceModel.init()
    }

    Column {
        id: content

        width: parent.width
        height: childrenRect.height

        spacing: 24

        readonly property int firstColumnWidth: 212

        ThemesSection {
            width: parent.width

            themes: appearanceModel.themes
            currentThemeIndex: appearanceModel.currentThemeIndex

            onThemeChangeRequested: {
                appearanceModel.currentThemeIndex = newThemeIndex
            }
        }

        AccentColorsSection {
            width: parent.width

            colors: appearanceModel.accentColors
            currentColorIndex: appearanceModel.currentAccentColorIndex
            firstColumnWidth: parent.firstColumnWidth

            onAccentColorChangeRequested: {
                appearanceModel.currentAccentColorIndex = newColorIndex
            }
        }

        SeparatorLine {}

        UiFontSection {
            allFonts: appearanceModel.allFonts()
            currentFontIndex: appearanceModel.currentFontIndex
            bodyTextSize: appearanceModel.bodyTextSize
            firstColumnWidth: parent.firstColumnWidth

            onFontChangeRequested: {
                appearanceModel.currentFontIndex = newFontIndex
            }

            onBodyTextSizeChangeRequested: {
                appearanceModel.bodyTextSize = newBodyTextSize
            }
        }

        SeparatorLine {}

        ColorAndWallpaperSection {
            width: parent.width

            title: qsTrc("appshell", "Background")
            wallpaperDialogTitle: qsTrc("appshell", "Choose Background Wallpaper")
            useColor: appearanceModel.backgroundUseColor
            color: appearanceModel.backgroundColor
            wallpaperPath: appearanceModel.backgroundWallpaperPath
            wallpapersDir: appearanceModel.wallpapersDir()
            wallpaperFilter: appearanceModel.wallpaperPathFilter()
            firstColumnWidth: parent.firstColumnWidth

            onUseColorChangeRequested: {
                appearanceModel.backgroundUseColor = newValue
            }

            onColorChangeRequested: {
                appearanceModel.backgroundColor = newColor
            }

            onWallpaperPathChangeRequested: {
                appearanceModel.backgroundWallpaperPath = newWallpaperPath
            }
        }

        SeparatorLine {}

        ColorAndWallpaperSection {
            width: parent.width

            title: qsTrc("appshell", "Paper")
            wallpaperDialogTitle: qsTrc("appshell", "Choose Notepaper")
            useColor: appearanceModel.foregroundUseColor
            color: appearanceModel.foregroundColor
            wallpaperPath: appearanceModel.foregroundWallpaperPath
            wallpapersDir: appearanceModel.wallpapersDir()
            wallpaperFilter: appearanceModel.wallpaperPathFilter()
            firstColumnWidth: parent.firstColumnWidth

            onUseColorChangeRequested: {
                appearanceModel.foregroundUseColor = newValue
            }

            onColorChangeRequested: {
                appearanceModel.foregroundColor = newColor
            }

            onWallpaperPathChangeRequested: {
                appearanceModel.foregroundWallpaperPath = newWallpaperPath
            }
        }
    }
}
