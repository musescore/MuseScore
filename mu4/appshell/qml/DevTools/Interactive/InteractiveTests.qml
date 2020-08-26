import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {

    color: "#71C2EF"

    InteractiveTestsModel {
        id: testModel
    }

    Text {
        id: header
        width: parent.width
        height: 40
        verticalAlignment: Text.AlignVCenter
        text: testModel.currentUri
    }

    Grid {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 16
        spacing: 16
        columns: 2


        FlatButton {
            width: 200
            text: "[cpp] Sample dialog"
            onClicked: testModel.openSampleDialog()
        }

        FlatButton {
            width: 200
            text: "[qml] Sample dialog"
            onClicked: {
                console.log("qml: before open")
                api.launcher.open("musescore://devtools/interactive/sample?color=#0F9D58")
                console.log("qml: after open")
            }
        }

        FlatButton {
            width: 200
            text: "[cpp] Sample dialog async"
            onClicked: testModel.openSampleDialogAsync()
        }

        FlatButton {
            width: 200
            text: "[qml] Sample dialog sync"
            onClicked: {
                console.log("qml: before open")
                api.launcher.open("musescore://devtools/interactive/sample?sync=true&color=#EF8605")
                console.log("qml: after open")
            }
        }

        FlatButton {
            width: 200
            text: "[qml] Sample dialog modal"
            onClicked: {
                console.log("qml: before open")
                api.launcher.open("musescore://devtools/interactive/sample?modal=true&color=#D13F31")
                console.log("qml: after open")
            }
        }

        FlatButton {
            width: 200
            text: "Open musescore.com"
            onClicked: {
                api.launcher.openUrl("https://musescore.com/")
            }
        }

        FlatButton {
            width: 200
            text: "Question"
            onClicked: testModel.question()
        }

        FlatButton {
            width: 200
            text: "Custom question"
            onClicked: testModel.customQuestion()
        }

        FlatButton {
            width: 200
            text: "Information"
            onClicked: testModel.information()
        }

        FlatButton {
            width: 200
            text: "Warning"
            onClicked: testModel.warning()
        }

        FlatButton {
            width: 200
            text: "Critical"
            onClicked: testModel.critical()
        }

        FlatButton {
            width: 200
            text: "Require"
            onClicked: testModel.require()
        }

        FlatButton {
            width: 200
            text: "Widget dialog"
            onClicked: testModel.openWidgetDialog()
        }
        FlatButton {
            width: 200
            text: "Widget dialog async"
            onClicked: testModel.openWidgetDialogAsync()
        }

        FlatButton {
            width: 200
            text: "Edit style"
            onClicked: api.launcher.open("musescore://notation/style")
        }
    }
}
