import QtQuick

import MuseApi.Controls
import MuseApi.Interactive

ExtensionBlank {

    id: root

    implicitHeight: 400
    implicitWidth: 400

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
            Interactive.info("Quick start", "Clicked on button")
        }
    }
}
