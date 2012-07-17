import QtQuick 1.0
import MuseScore 1.0


MuseScore {
      menuPath:   "Plugins.panel"
      version:  "2.0"
      description: "This demo plugin creates a GUI panel."

      pluginType: "dock"
      dockArea:   "left"

      width:  150
      height: 75
      onRun: {
            console.log("hello panel");
            }

      Rectangle {
            id: simplebutton
            color: "grey"
            anchors.fill: parent

            Text {
                  id: buttonLabel
                  anchors.centerIn: parent
                  text: "Hello Panel"
                  }

            MouseArea {
                  anchors.fill: parent
                  onClicked: Qt.quit()
                  }
            }
      }

