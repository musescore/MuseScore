import QtQuick 2.7
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

StyledTextLabel {
    NotationAccessibilityModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    text: model.accessibilityInfo
}
