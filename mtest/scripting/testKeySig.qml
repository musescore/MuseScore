import QtQuick 2.0
import MuseScore 1.0

MuseScore {
      onRun: {
            var seg = curScore.firstSegment();
            while (seg) {
                  if (seg.segmentType === Segment.KeySig) {
                        var track = curScore.ntracks;
                        while (track-- > 0) {
                              var el = seg.elementAt(track);
                              if (el && (el.type === Element.KEYSIG)) {
console.log('Found keysig at seg ' + seg + ' track ' + track + ': ' + el.key);
                                    if (el.key === Key.A) {
                                          el.key = Key.D;//C_B;
                                    }
                                    else if (el.key === Key.E_B) {
                                          el.key = Key.B;
                                    }
                              }
                        }
                  }
                  seg = seg.next;
            }
            Qt.quit();
            }
      }
