import QtQuick 1.0
import MuseScore 1.0


MuseScore {
      menuPath:   "Plugins.panel"
      pluginType: "panel-right"
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

