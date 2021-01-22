import QtQuick 2.15

import MuseScore.UiComponents 1.0

Rectangle {
    readonly property QtObject paletteWorkspace: null
    readonly property bool hasFocus: false

    color: ui.theme.backgroundPrimaryColor

    StyledTextLabel {
        anchors.centerIn: parent
        text: "Palette Panel Stub"
    }
}
