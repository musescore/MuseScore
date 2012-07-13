import QtQuick 1.0
import MuseScore 1.0


MuseScore {
      menuPath: "Plugins.ScoreView"
      width:  400
      height: 400
      onRun: {
            if (typeof curScore === 'undefined')
                  Qt.quit();

            scoreview.setScore(curScore);
            }

      ScoreView {
            id: scoreview
            anchors.fill: parent
            color: "white"
            MouseArea {
                  anchors.fill: parent
                  onClicked: Qt.quit()
                  }
            }
      }

