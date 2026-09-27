import QtQuick

import MuseApi.Log
import MuseApi.Controls
import MuseApi.Http

ExtensionBlank {
    id: root

    implicitHeight: 400
    implicitWidth: 400

    color: api.theme.backgroundPrimaryColor

    property string url: "https://raw.githubusercontent.com/musescore/muse_framework/refs/heads/main/docs/adr/README.md"

    Component.onCompleted: {
        Log.info("Component.onCompleted from fetch 1")
    }

    TextInputField {
        id: urlField
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        anchors.top: parent.top
        anchors.topMargin: 8

        hint: "URL"
        currentText: root.url

        onTextChanged: function(newTextValue) {
            root.url = newTextValue
        }

        onAccepted: btn1.clicked()
    }

    FlatButton {
        id: btn1
        anchors.left: urlField.left
        anchors.top: urlField.bottom
        anchors.topMargin: 8

        text: "Fetch"

        onClicked: {
            label1.text = "Loading..."
            Http.fetch(root.url)
            .then((response) => {
                if (!response.ok) {
                    throw new Error("HTTP " + response.status + " " + response.statusText)
                }
                return response.text()
            })
            .then((text) => {
                console.log(text)
                label1.text = text
            })
            .catch((error) => {
                console.log(error)
                label1.text = String(error)
            })
        }
    }

    StyledTextLabel {
        id: label1
        anchors.left: urlField.left
        anchors.right: urlField.right
        anchors.top: btn1.bottom
        anchors.topMargin: 8
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignTop
        wrapMode: Text.Wrap
        text: "Fetch 1"
    }
}
