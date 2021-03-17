import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

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

    Component.onCompleted: {
        appearanceModel.load()
    }

    Column {
        id: content

        width: parent.width
        height: childrenRect.height

        spacing: 24

        readonly property int firstColumnWidth: 212

        ThemesView {
            width: parent.width

            themes: appearanceModel.themes
            currentThemeIndex: appearanceModel.currentThemeIndex

            onThemeChangeRequested: {
                appearanceModel.currentThemeIndex = newThemeIndex
            }
        }

        AccentColorsView {
            width: parent.width

            colors: appearanceModel.accentColorSamples()
            currentColorIndex: appearanceModel.currentAccentColorIndex
            firstColumnWidth: parent.firstColumnWidth
        }

        SeparatorLine {}

        UiFontView {
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

        ColorAndWallpaperView {
            width: parent.width

            title: qsTrc("preferences", "Background")
            color: appearanceModel.backgroundColor
            wallpaperPath: appearanceModel.backgroundWallpaperPath
            firstColumnWidth: parent.firstColumnWidth
        }

        SeparatorLine {}

        ColorAndWallpaperView {
            width: parent.width

            title: qsTrc("preferences", "Paper")
            color: appearanceModel.foregroundColor
            wallpaperPath: appearanceModel.foregroundWallpaperPath
            firstColumnWidth: parent.firstColumnWidth
        }

        CheckBox {
            text: qsTrc("preferences", "Use the same colour in palettes")

            checked: appearanceModel.useSameColorInPalettes

            onClicked: {
                appearanceModel.useSameColorInPalettes = !checked
            }
        }
    }
}
