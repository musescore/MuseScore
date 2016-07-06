import QtQuick 2.0
import MuseScore 1.0

MuseScore {
      menuPath: "Plugins.p1"
      onRun: {
            openLog("p1.log");
            logn("test script p1")

            // check Direction enum
            log2("     Direction.AUTO: ", Direction.AUTO);
            log2("     Direction.UP:   ", Direction.UP);
            log2("     Direction.DOWN: ", Direction.DOWN);

            var cursor = curScore.newCursor();
            cursor.voice = 0;
            cursor.staffIdx = 0;
            log2("filter:", cursor.filter);
            cursor.filter = -1;
            log2("filter:", cursor.filter);
            cursor.rewind(0);

            while (cursor.segment) {
                  if (cursor.element) {
                        var type = cursor.element.type;
                        var e    = cursor.element;
                        logn(e._name());
                        log2("type is:", type);
                        if (type == Element.CHORD) {
                            log2("  durationType:", e.durationType);
                            log2("  beamMode:", e.beamMode);
                            log2("  small:",    e.small);
                            log2("  stemDirection:", e.stemDirection);

                            logn("  duration:");
                            log2("    numerator:",   e.duration.numerator);
                            log2("    denominator:", e.duration.denominator);
                            log2("    ticks:",       e.duration.ticks);
//                            var notes = e.notes;
//                            for (var i = 0; i < notes.length; i++) {
//                                var note = notes[i];
//                                log2("  ", note._name());
//                                log2("    subchannel:", note.subchannel);
//                                log2("    line:", note.line);
//                                log2("    fret:", note.fret);
//                                log2("    string:", note.string);
//                                log2("    tpc:", note.tpc);
//                                log2("    tpc1:", note.tpc1);
//                                log2("    tpc2:", note.tpc2);
//                                log2("    pitch:", note.pitch);
//                                log2("    ppitch:", note.ppitch);
//                                log2("    ghost:", note.ghost);
//                                log2("    hidden:", note.hidden);
//                                log2("    mirror:", note.mirror);
//                                log2("    small:", note.small);
//                                log2("    play:", note.play);
//                                log2("    tuning:", note.tuning);
//                                log2("    veloType:", note.veloType);
//                                log2("    veloOffset:", note.veloOffset);
//                                log2("    userMirror:", note.userMirror);
//                                log2("    userDotPosition:", note.userDotPosition);
//                                log2("    headGroup:", note.headGroup);
//                                log2("    headType:", note.headType);
//                                log2("    accidentalType:", note.accidentalType);
//                                log2("    dotsCount:", note.dotsCount);
//                                if (note.accidental) {
//                                      var acc = note.accidental;
//                                      log2("      ", acc._name());
//                                      log2("        hasBracket:", acc.hasBracket);
//                                      log2("        small:", acc.small);
//                                      log2("        accType:", acc.accType);
//                                      log2("        role:", acc.role);
//                                      }
//                                }
                            }
                        if (type == Element.REST) {
                            logn("  duration:");
                            log2("    numerator:",   e.duration.numerator);
                            log2("    denominator:", e.duration.denominator);
                            log2("    ticks:",       e.duration.ticks);
                            log2("  beamMode:", e.beamMode);
                            log2("  small:", e.small);
                            }
                        }
                  cursor.next();
                  }
            closeLog();
            Qt.quit()
            }
      }
