import QtQuick 1.0
import MuseScore 1.0


MuseScore {
      menuPath: "Plugins.helloQml"
      width: 150
      height: 75
      Rectangle {
            id: simplebutton
            color: "grey"
            width: 150
            height: 75

            Text {
                  id: buttonLabel
                  anchors.centerIn: parent
                  text: "Hello Qml"
                  }
            }
      }

