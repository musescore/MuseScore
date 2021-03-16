import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Flickable {
    id: root

    ScrollBar.vertical: StyledScrollBar {}

    AppearancePreferencesModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }
}
