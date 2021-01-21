import QtQuick 2.15

import MuseScore.UiComponents 1.0

Rectangle {
    property string search: ""
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    color: backgroundColor

    StyledTextLabel {
        anchors.centerIn: parent
        text: "Extensions Page Stub"
    }
}
