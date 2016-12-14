import QtQuick 2.0
import MuseScore 1.0

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
//                        var type = e.type;
                        if (e.type == Element.FRET_DIAGRAM) {
                              log2("userMag:",    e.userMag)
                              log2("strings:",    e.strings)
                              log2("frets:",      e.frets)
                              log2("barre:",      e.barre)
                              log2("fretOffset:", e.fretOffset)
                              e.userMag = 2.2
                              e.strings = 8
                              e.frets   = 7
                              e.barre   = 2
                              e.fretOffset = 4
                              log2("set userMag:",     e.userMag)
                              log2("set strings:",     e.strings)
                              log2("set frets:",       e.frets)
                              log2("set barre:",       e.barre)
                              log2("set fretOffset:",  e.fretOffset)
                              }
                        }
                  cursor.next();
                  }
            closeLog();
            Qt.quit()
            }
      }
