import QtQuick 2.1
import MuseScore 1.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

MuseScore {
      version:  "2.1"
      description: "Create random score."
      menuPath: "Plugins.random2"
      requiresScore: false
      pluginType: "dock"
      dockArea:   "left"
      width:  150
      height: 75

      onRun: { }

      function addNote(key, cursor) {
            var cdur = [ 0, 2, 4, 5, 7, 9, 11 ];
            //           c  g  d  e
            var keyo = [ 0, 7, 2, 4 ];

            var idx    = Math.random() * 6;
            var octave = Math.floor(Math.random() * octaves.value);
            var pitch  = cdur[Math.floor(idx)] + octave * 12 + 60 + keyo[key];
            console.log("Add note pitch "+pitch);
            cursor.addNote(pitch);
            }

      function createScore() {
            var measures    = 18; //in 4/4 default time signature
            var numerator   = 3;
            var denominator = 4;
            var key         = 2; //index in keyo from addNote function above

            var score = newScore("Random2.mscz", "piano", measures);

            score.startCmd();
            score.addText("title", "==Random2==");
            score.addText("subtitle", "Another subtitle");

            var cursor = score.newCursor();
            cursor.track = 0;

            cursor.rewind(0);

            var ts = newElement(Element.TIMESIG);
            ts.setSig(numerator, denominator);
            cursor.add(ts);

            var realMeasures = Math.ceil(measures * denominator / numerator);
            console.log(realMeasures);
            var notes = realMeasures * 4; //number of 1/4th notes

            for (var staff = 0; staff < 2; ++staff) { //piano has two staves to fill
                  cursor.track = staff * 4; //4 voice tracks per staff
                  cursor.rewind(0); //go to the start of the score
                  //add notes
                  for (var i = 0; i < notes; ++i) {
                        if (Math.random() < 0.4) {
                              console.log("Adding two notes at ", i);
                              cursor.setDuration(1, 8);
                              addNote(key, cursor);
                              addNote(key, cursor);
                              }
                        else {
                              console.log("Adding note at ", i);
                              cursor.setDuration(1, 4);
                              addNote(key, cursor);
                              }
                        } //done adding notes to this staff
                  }
            score.endCmd();
            Qt.quit();
            }

    GridLayout {
        anchors.fill: parent
        columns: 2
        rowSpacing: 5


        Text {
            text: "Octaves"
            color: "white"
            }

        SpinBox {
            id: octaves
            minimumValue: 1
            maximumValue: 3
            stepSize:     1
            Layout.fillWidth: true
            Layout.preferredHeight: 25
            value: 1
            }

        Button {
            text: "create"
            Layout.columnSpan: 2
            Layout.fillWidth: true
            onClicked: {
                createScore()
                }
            }
        }
    }
