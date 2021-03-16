import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Flickable {
    id: root

    ScrollBar.vertical: StyledScrollBar {}

    AppearancePreferencesModel {
        id: appearanceModel
    }

    Component.onCompleted: {
        appearanceModel.load()
    }

    ThemesView {
        anchors.left: parent.left
        anchors.right: parent.right

        themes: appearanceModel.themes
        currentThemeIndex: appearanceModel.currentThemeIndex
    }
}
