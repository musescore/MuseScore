import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {

    color: "#71C2EF"

    LauncherTestsModel {
        id: testModel
    }

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 20

        Text {
            width: parent.width
            height: 40
            verticalAlignment: Text.AlignVCenter
            text: testModel.currentUri
        }

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
                api.launcher.open("musescore://devtools/launcher/sample?color=#0F9D58")
                console.log("qml: after open")
            }
        }

        FlatButton {
            width: 200
            text: "[cpp] Sample dialog sync"
            onClicked: testModel.openSampleDialogSync()
        }

        FlatButton {
            width: 200
            text: "[qml] Sample dialog sync"
            onClicked: {
                console.log("bqml: efore open")
                api.launcher.open("musescore://devtools/launcher/sample?sync=true&color=#EF8605")
                console.log("qml: after open")
            }
        }

        FlatButton {
            width: 200
            text: "[qml] Sample dialog modal"
            onClicked: {
                console.log("bqml: efore open")
                api.launcher.open("musescore://devtools/launcher/sample?modal=true&color=#D13F31")
                console.log("qml: after open")
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
    }
}
