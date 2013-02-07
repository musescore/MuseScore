//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Note Names Plugin
//
//  Copyright (C) 2012 Werner Schweer
//  Copyright (C) 2013 Joachim Schmitz
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

import QtQuick 1.1
import MuseScore 1.0

MuseScore {
   version: "2.0"
   description: qsTr("This plugin names notes as per your language settings")
   menuPath: "Plugins.Notes." + qsTr("Note Names") // this does not work, why?

   onRun: {
      if (typeof curScore === 'undefined')
         Qt.quit();

      var cursor = curScore.newCursor();

      for (var staff = 0; staff < curScore.nstaves; ++staff) {
         cursor.staffIdx = staff;
         cursor.voice = 0;
         cursor.rewind(0);  // set cursor to first chord/rest

         while (cursor.segment) {
            if (cursor.element && cursor.element.type == Element.CHORD) {
               var text  = newElement(Element.STAFF_TEXT);

               switch (cursor.element.notes[0].tpc) {
                  case -1: text.text = qsTr("Fbb"); break;
                  case 0:  text.text = qsTr("Cbb"); break;
                  case 1:  text.text = qsTr("Gbb"); break;
                  case 2:  text.text = qsTr("Dbb"); break;
                  case 3:  text.text = qsTr("Abb"); break;
                  case 4:  text.text = qsTr("Ebb"); break;
                  case 5:  text.text = qsTr("Bbb"); break;
                  case 6:  text.text = qsTr("Fb");  break;
                  case 7:  text.text = qsTr("Cb");  break;

                  case 8:  text.text = qsTr("Gb");  break;
                  case 9:  text.text = qsTr("Db");  break;
                  case 10: text.text = qsTr("Ab");  break;
                  case 11: text.text = qsTr("Eb");  break;
                  case 12: text.text = qsTr("Bb");  break;
                  case 13: text.text = qsTr("F");   break;
                  case 14: text.text = qsTr("C");   break;
                  case 15: text.text = qsTr("G");   break;
                  case 16: text.text = qsTr("D");   break;
                  case 17: text.text = qsTr("A");   break;
                  case 18: text.text = qsTr("E");   break;
                  case 19: text.text = qsTr("B");   break;

                  case 20: text.text = qsTr("F#");  break;
                  case 21: text.text = qsTr("C#");  break;
                  case 22: text.text = qsTr("G#");  break;
                  case 23: text.text = qsTr("D#");  break;
                  case 24: text.text = qsTr("A#");  break;
                  case 25: text.text = qsTr("E#");  break;
                  case 26: text.text = qsTr("B#");  break;
                  case 27: text.text = qsTr("F##"); break;
                  case 28: text.text = qsTr("C##"); break;
                  case 29: text.text = qsTr("G##"); break;
                  case 30: text.text = qsTr("D##"); break;
                  case 31: text.text = qsTr("A##"); break;
                  case 32: text.text = qsTr("E##"); break;
                  case 33: text.text = qsTr("B##"); break;
                  default: text.text = qsTr("?");   break;
               } // end switch tpc

// change below false to true for courtesy- and microtonal accidentals
// you might need to come up with suitable translations 
// only #, b, natural and possibly also ## seem to be available in UNICODE
               if (false) {
                  switch (cursor.element.notes[0].userAccidental) {
                     case 0:                                            break;
                     case 1:  text.text += qsTr("#");                   break;
                     case 2:  text.text += qsTr("b");                   break;
                     case 3:  text.text += qsTr("##");                  break;
                     case 4:  text.text += qsTr("bb");                  break;
                     case 5:  text.text += qsTr("natural");             break;
                     case 6:  text.text += qsTr("flat-slash");          break;
                     case 7:  text.text += qsTr("flat-slash2");         break;
                     case 8:  text.text += qsTr("mirrored-flat2");      break;
                     case 9:  text.text += qsTr("mirrored-flat");       break;
                     case 10: text.text += qsTr("mirrored-flat-slash"); break;
                     case 11: text.text += qsTr("flat-flat-slash");     break;
                     case 12: text.text += qsTr("sharp-slash");         break;
                     case 13: text.text += qsTr("sharp-slash2");        break;
                     case 14: text.text += qsTr("sharp-slash3");        break;
                     case 15: text.text += qsTr("sharp-slash4");        break;
                     case 16: text.text += qsTr("sharp arrow up");      break;
                     case 17: text.text += qsTr("sharp arrow down");    break;
                     case 18: text.text += qsTr("sharp arrow both");    break;
                     case 19: text.text += qsTr("flat arrow up");       break;
                     case 20: text.text += qsTr("flat arrow down");     break;
                     case 21: text.text += qsTr("flat arrow both");     break;
                     case 22: text.text += qsTr("natural arrow down");  break;
                     case 23: text.text += qsTr("natural arrow up");    break;
                     case 24: text.text += qsTr("natural arrow both");  break;
                     case 25: text.text += qsTr("sori");                break;
                     case 26: text.text += qsTr("koron");               break;
                     default: text.text += qsTr("?");                   break;
                  } // end switch userAccidental
               } // end if courtesy- and microtonal accidentals

               text.pos.y = 1;
               if (cursor.element.notes[0].pitch > 83)
                  text.pos.x = 1;
               cursor.add(text);
            } // end if CHORD
            cursor.next();
         } // end while segment
      } // end for staff
      Qt.quit();
   } // end onRun
}
