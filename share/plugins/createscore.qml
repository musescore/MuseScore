import QtQuick 1.0
import MuseScore 1.0

MuseScore {
      menuPath: "Plugins.createscore"
      onRun: {
            console.log("hello createscore");
            var score = newScore("Test-Score", "Piano", 5);

            score.addText("title", "==Test-Score==");
            score.addText("subtitle", "subtitle");

            var cursor = score.newCursor();
            cursor.track = 0;

            cursor.rewind(0);
            cursor.setDuration(1, 4);
            var note = cursor.addNote(60);
            note.color = "#5040a1";

            cursor.next();
            cursor.setDuration(3, 8);
            cursor.addNote(64);

            cursor.next();
            cursor.setDuration(1, 4);
            cursor.addNote(68);
            cursor.next();
            cursor.addNote(72);
            }
      }

