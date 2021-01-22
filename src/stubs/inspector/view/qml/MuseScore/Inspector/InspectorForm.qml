import QtQuick 2.15

import MuseScore.UiComponents 1.0

Rectangle {
    property var model: null

    color: ui.theme.backgroundPrimaryColor

    StyledTextLabel {
        anchors.centerIn: parent
        text: "Inspector Form Stub"
    }
}
