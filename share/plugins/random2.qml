import QtQuick 2.1
import MuseScore 1.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

MuseScore {
      version:  "1.0"
      description: "Create random score."
      menuPath: "Plugins.random2"
      pluginType: "dock"
      dockArea:   "left"
      width:  150
      // height: 75

      property variant octaves : 2;

      onRun: { }

      function addNote(key, cursor) {
            var cdur = [ 0, 2, 4, 5, 7, 9, 11 ];
            //           c  g  d  e
            var keyo = [ 0, 7, 2, 4 ];

            var idx    = Math.random() * 6;
            var octave = Math.floor(Math.random() * 2);
            var pitch  = cdur[Math.floor(idx)] + octave * 12 + 60;
            var pitch  = pitch + keyo[key];
            cursor.addNote(pitch);
            }

      function createScore() {
            var measures    = 18;
            var numerator   = 3;
            var denominator = 4;
            var key         = 3;

            var score = newScore("Random.mscz", "Piano", measures);

            score.startCmd()
            score.addText("title", "==Random==");
            score.addText("subtitle", "subtitle");

            var cursor = score.newCursor();
            cursor.track = 0;

            cursor.rewind(0);

            var ts = newElement(Element.TIMESIG);
            ts.setSig(numerator, denominator);
            cursor.add(ts);

            if (key != 0) {
                var sig = newElement(Element.KEYSIG);
                sig.setSig(0, key);
                cursor.add(sig);
                }

            cursor.rewind(0);
            cursor.setDuration(1, denominator);

            var realMeasures = Math.floor((measures * 4 + numerator - 1) / numerator);
            console.log(realMeasures);
            var notes = realMeasures * numerator;

            for (var i = 0; i < notes; ++i) {
                if (Math.random() < 0.5) {
                    cursor.setDuration(1, 8);
                    addNote(key, cursor);
                    cursor.next();
                    addNote(key, cursor);
                    }
                else {
                    cursor.setDuration(1, 4);
                    addNote(key, cursor);
                    }
                if (i % 12 == 11) {
                    var lb = newElement(Element.LAYOUT_BREAK);
                    lb.layoutBreakType = LayoutBreak.LINE;
                    cursor.add(lb);
                    }
                cursor.next();
                }
            score.endCmd()
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
            minimumValue: 1
            maximumValue: 3
            stepSize:     1
            Layout.fillWidth: true
            Layout.preferredHeight: 25

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

