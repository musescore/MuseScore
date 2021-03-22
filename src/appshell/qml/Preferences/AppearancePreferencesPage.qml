import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

import "internal"

Flickable {
    id: root

    contentWidth: width
    contentHeight: content.height

    clip: true
    boundsBehavior: Flickable.StopAtBounds
    interactive: height < contentHeight

    ScrollBar.vertical: StyledScrollBar {}

    AppearancePreferencesModel {
        id: appearanceModel
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

            colors: appearanceModel.accentColorSamples()
            currentColorIndex: appearanceModel.currentAccentColorIndex
            firstColumnWidth: parent.firstColumnWidth

            onAccentColorChangeRequested: {
                appearanceModel.currentAccentColorIndex = newColorIndex
            }
        }

        SeparatorLine {}

        UiFontSection {
            width: parent.width

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
                appearanceModel.backgroundWallpaperPath = wallpaperPath
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

        CheckBox {
            text: qsTrc("appshell", "Use the same colour in palettes")

            checked: appearanceModel.useSameColorInPalettes

            onClicked: {
                appearanceModel.useSameColorInPalettes = !checked
            }
        }
    }
}
