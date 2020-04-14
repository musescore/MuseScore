import QtQuick 2.0
import MuseScore 3.0

MuseScore {
      onRun: {
            var seg = curScore.firstSegment();
            while (seg) {
                  for (var i = seg.annotations.length; i--; ) {
                        if (seg.annotations[i].type === Element.STAFF_TEXT) {
                              if (seg.annotations[i].subStyle === Tid.SYSTEM) {
                                    seg.annotations[i].subStyle = Tid.FIGURED_BASS;
                              }
                              else if (seg.annotations[i].subStyle === Tid.EXPRESSION) {
                                    seg.annotations[i].subStyle = Tid.STRING_NUMBER;
                              }
                        }
                  }
                  seg = seg.next;
            }
            Qt.quit();
            }
      }
