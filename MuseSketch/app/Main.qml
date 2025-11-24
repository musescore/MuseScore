import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("MuseSketch")

    Rectangle {
        anchors.fill: parent
        color: "#2E2E2E"

        Text {
            anchors.centerIn: parent
            text: "MuseSketch"
            color: "white"
            font.pixelSize: 32
            font.bold: true
        }
        
        Text {
            anchors.top: parent.verticalCenter
            anchors.topMargin: 40
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Sketch motifs. Shape ideas. Build counterpoint."
            color: "#AAAAAA"
            font.pixelSize: 16
        }

        Image {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            width: 400
            height: 300
            source: "image://score/demo"
            fillMode: Image.PreserveAspectFit
            
            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: "white"
                border.width: 1
                opacity: 0.5
            }
        }
    }
}
