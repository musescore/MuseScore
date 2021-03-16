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

        ThemesView {
            anchors.left: parent.left
            anchors.right: parent.right

            themes: appearanceModel.themes
            currentThemeIndex: appearanceModel.currentThemeIndex
        }

        AccentColorsView {
            anchors.left: parent.left
            anchors.right: parent.right

            colors: appearanceModel.accentColorSamples()
            currentColorIndex: appearanceModel.currentAccentColorIndex
        }

        SeparatorLine {}

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            color: "red"
            height: 80
        }

        SeparatorLine {}

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            color: "red"
            height: 80
        }

        SeparatorLine {}

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            color: "red"
            height: 80
        }
    }
}
