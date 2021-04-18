import QtQuick 2.0
import MuseScore 3.0

MuseScore {
      menuPath: "Plugins.p2"
      onRun: {
            openLog("p2.log");
            logn("test script p2")

            var cursor = curScore.newCursor();
            cursor.voice = 0;
            cursor.staffIdx = 0;
            cursor.filter = -1;
            cursor.rewind(0);

            while (cursor.segment) {
                  var a = cursor.segment.annotations;
                  var l = a.length;
                  for (var i = 0; i < l; i++) {
                        var e = a[i];
                        logn(e._name());
                        if (e.type == Element.FRET_DIAGRAM) {
                              log2("userMag:",    e.mag)
                              log2("strings:",    e.fretStrings)
                              log2("frets:",      e.fretFrets)
//                               log2("barre:",      e.fretBarre) // property does not exist anymore, probably needs to be replaced
                              log2("fretOffset:", e.fretOffset)
                              e.mag = 2.2
                              e.fretStrings = 8
                              e.fretFrets   = 7
//                               e.fretBarre   = 2
                              e.fretOffset = 4
                              log2("set userMag:",     e.mag)
                              log2("set strings:",     e.fretStrings)
                              log2("set frets:",       e.fretFrets)
//                               log2("set barre:",       e.fretBarre)
                              log2("set fretOffset:",  e.fretOffset)
                              }
                        }
                  cursor.next();
                  }
            closeLog();
            Qt.quit()
            }
      }
