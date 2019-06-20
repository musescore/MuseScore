import QtQuick 2.0
import MuseScore 3.0


MuseScore {
    menuPath: "Plugins.scorelist"
    version:  "3.0"
    description: "This test plugin iterates through the score list."
    pluginType: "dialog"

    width:  150
    height: 75
    Component.onCompleted: {
        console.log("hello scorelist");
        for (var i = 0; i < scores.length; i++) {
            console.log(scores[i].name);
            scoreList.text = scoreList.text + scores[i].name + '\n';
            }
        }

    Rectangle {
        color: "grey"
        anchors.fill: parent

        Text {
            id: scoreList
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: ""
            }

        MouseArea {
            anchors.fill: parent
            onClicked: Qt.quit()
            }
        }
    }

