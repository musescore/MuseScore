import QtQuick 1.0
import MuseScore 1.0


MuseScore {
      menuPath: "Plugins.helloQml"
      version:  "2.0"
      description: "This demo plugin shows some basic tasks."

      width:  150
      height: 75
      onRun: {
            console.log("hello world");
            if (typeof curScore === 'undefined')
                  Qt.quit();

            var score = curScore;
            console.log(curScore);
            console.log(score.name);
            var m;
            m = score.firstMeasure();
            while (m) {
                  console.log("measure");
                  var segment = m.first();
                  while (segment) {
                        console.log("  segment");
                        console.log(segment.type);
                        if (segment.type == Element.SEGMENT)
                              console.log(" ---hello segment");
                        else {
                              console.log(Element.SEGMENT);
                              }

                        var element;
                        element = segment.element(0);
                        if (element) {
                              console.log("    element");
                              console.log(element.type);
                              }
                        segment = segment.next();
                        }
                  m = m.nextMeasure();
                  }
            }

      Rectangle {
            id: simplebutton
            color: "grey"
            anchors.fill: parent

            Text {
                  id: buttonLabel
                  anchors.centerIn: parent
                  text: "Hello Qml"
                  }

            MouseArea {
                  anchors.fill: parent
                  onClicked: Qt.quit()
                  }
            }
      }

