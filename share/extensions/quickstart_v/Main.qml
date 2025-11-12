import QtQuick

import MuseApi.Extensions
//import MuseApi.Controls
import Muse.UiComponents

ExtensionBlank {

    id: root

    implicitHeight: 400
    implicitWidth: 400

    color: api.theme.backgroundPrimaryColor

    StyledTextLabel {
        id: label1
        anchors.centerIn: parent
        text: "Quick start"
    }

    FlatButton {
        id: btn1
        anchors.top: label1.bottom
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter

        text: "Click me"

        onClicked: {
            api.interactive.info("Quick start", "Clicked on button")
        }
    }
}
