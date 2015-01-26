import QtQuick 2.0
import MuseScore 1.0

MuseScore {
      version:  "2.0"
      description: "This demo plugin creates a new score."
      menuPath: "Plugins.createscore"

      onRun: {
            console.log("hello createscore");
            var score = newScore("Test-Score", "piano", 5);
            var numerator = 3;
            var denominator = 4;

            score.addText("title", "==Test-Score==");
            score.addText("subtitle", "subtitle");

            var cursor = score.newCursor();
            cursor.track = 0;

            cursor.rewind(0);
            var ts = newElement(Element.TIMESIG);
            ts.setSig(numerator, denominator);
            cursor.add(ts);

            cursor.rewind(0);
            cursor.setDuration(1, 4);
            cursor.addNote(60);

            cursor.next();
            cursor.setDuration(3, 8);
            cursor.addNote(64);

            cursor.next();
            cursor.setDuration(1, 4);
            cursor.addNote(68);

            cursor.next();
            cursor.addNote(72);

            Qt.quit();
            }
      }