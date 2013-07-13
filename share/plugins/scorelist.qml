import QtQuick 2.0
import MuseScore 1.0


MuseScore {
    menuPath: "Plugins.scorelist"
    version:  "2.0"
    description: "This test plugin interates through the score list."

    width:  150
    height: 75
    onRun: {
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
            id:scoreList
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

