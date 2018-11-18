import QtQuick 2.0
import MuseScore 3.0


MuseScore {
    menuPath:   "Plugins.panel"
    version:  "3.0"
    description: "This demo plugin creates a GUI panel."
    requiresScore: false

    pluginType: "dock"
    dockArea:   "left"

    width:  150
    height: 75
    onRun:  console.log("hello panel");

    Rectangle {
        color: "grey"
        anchors.fill: parent

        Text {
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "Hello Panel"
            }

        MouseArea {
            anchors.fill: parent
            onClicked: Qt.quit()
            }
        }
    }

