/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    AppearancePreferencesModel {
        id: appearanceModel
    }

    Component.onCompleted: {
        appearanceModel.init()
    }

    Column {
        width: parent.width
        spacing: root.sectionsSpacing

        ThemesSection {
            width: parent.width

            themes: appearanceModel.highContrastEnabled ? appearanceModel.highContrastThemes : appearanceModel.generalThemes
            currentThemeCode: appearanceModel.currentThemeCode
            highContrastEnabled: appearanceModel.highContrastEnabled
            isFollowSystemThemeAvailable: appearanceModel.isFollowSystemThemeAvailable
            isFollowSystemTheme: appearanceModel.isFollowSystemTheme
            accentColors: appearanceModel.accentColors
            currentAccentColorIndex: appearanceModel.currentAccentColorIndex

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            onThemeChangeRequested: function(newThemeCode) {
                appearanceModel.currentThemeCode = newThemeCode
            }

            onHighContrastChangeRequested: function(enabled) {
                appearanceModel.highContrastEnabled = enabled
            }

            onSetFollowSystemThemeRequested: function(enabled) {
                appearanceModel.isFollowSystemTheme = enabled
            }

            onAccentColorChangeRequested: function(newColorIndex) {
                appearanceModel.currentAccentColorIndex = newColorIndex
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }

            onEnsureContentVisibleRequested: function(contentRect) {
                root.ensureContentVisibleRequested(contentRect)
            }
        }

        SeparatorLine {
            visible: uiColorsSection.visible
        }

        UiColorsSection {
            id: uiColorsSection

            width: parent.width

            visible: appearanceModel.highContrastEnabled

            navigation.section: root.navigationSection
            //! NOTE: 3 because ThemesSection have two panels
            navigation.order: root.navigationOrderStart + 3

            onColorChangeRequested: function(newColor, propertyType) {
                appearanceModel.setNewColor(newColor, propertyType)
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine {}

        PageSection {
            id: paperSettings

            width: parent.width

            scoreInversionEnabled: appearanceModel.scoreInversionEnabled
            isOnlyInvertInDarkTheme: appearanceModel.isOnlyInvertInDarkTheme
            isCurrentThemeDark: appearanceModel.isCurrentThemeDark

            colorAndWallpaper.useColor: appearanceModel.foregroundUseColor
            colorAndWallpaper.color: appearanceModel.foregroundColor
            colorAndWallpaper.wallpaperPath: appearanceModel.foregroundWallpaperPath
            colorAndWallpaper.wallpapersDir: appearanceModel.wallpapersDir()
            colorAndWallpaper.wallpaperFilter: appearanceModel.wallpaperPathFilter()

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 4

            onScoreInversionEnableChangeRequested: function(enable) {
                appearanceModel.scoreInversionEnabled = enable
            }

            onIsOnlyInvertInDarkThemeChangeRequested: function(enable) {
                appearanceModel.isOnlyInvertInDarkTheme = enable
            }

            colorAndWallpaper.onUseColorChangeRequested: function(newValue) {
                appearanceModel.foregroundUseColor = newValue
            }

            colorAndWallpaper.onColorChangeRequested: function(newColor) {
                appearanceModel.foregroundColor = newColor
            }

            colorAndWallpaper.onWallpaperPathChangeRequested: function(newWallpaperPath) {
                appearanceModel.foregroundWallpaperPath = newWallpaperPath
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine {}

        ColorAndWallpaperSection {
            id: backgroundSettings

            width: parent.width

            title: qsTrc("appshell/preferences", "Background")
            wallpaperDialogTitle: qsTrc("appshell/preferences", "Choose background wallpaper")
            useColor: appearanceModel.backgroundUseColor
            color: appearanceModel.backgroundColor
            wallpaperPath: appearanceModel.backgroundWallpaperPath
            wallpapersDir: appearanceModel.wallpapersDir()
            wallpaperFilter: appearanceModel.wallpaperPathFilter()

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 5

            onUseColorChangeRequested: function(newValue) {
                appearanceModel.backgroundUseColor = newValue
            }

            onColorChangeRequested: function(newColor) {
                appearanceModel.backgroundColor = newColor
            }

            onWallpaperPathChangeRequested: function(newWallpaperPath) {
                appearanceModel.backgroundWallpaperPath = newWallpaperPath
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine {}

        UiFontSection {
            allFonts: appearanceModel.allFonts()
            currentFontIndex: appearanceModel.currentFontIndex
            bodyTextSize: appearanceModel.bodyTextSize

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 6

            onFontChangeRequested: function(newFontIndex) {
                appearanceModel.currentFontIndex = newFontIndex
            }

            onBodyTextSizeChangeRequested: function(newBodyTextSize) {
                appearanceModel.bodyTextSize = newBodyTextSize
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine {}

        ThemeAdditionalOptionsSection {

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 7

            onResetThemeToDefaultRequested: {
                appearanceModel.resetAppearancePreferencesToDefault()
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }
    }
}
