import QtQuick

import Muse.UiComponents

Rectangle {

    id: root

    property string text: ""

    color: ui.theme.backgroundPrimaryColor

    StyledTextLabel {
        anchors.centerIn: parent
        text: root.text
    }
}


