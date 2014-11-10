//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Note Names Plugin
//
//  Copyright (C) 2012 Werner Schweer
//  Copyright (C) 2013, 2014 Joachim Schmitz
//  Copyright (C) 2014 Jörn Eichler
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

import QtQuick 2.0
import MuseScore 1.0

MuseScore {
   version: "2.0"
   description: qsTr("This plugin names notes")
   menuPath: "Plugins.Notes." + qsTr("Note Names") // this does not work, why?

   function nameChord (notes, text) {
      for (var i = 0; i < notes.length; i++) {
         var sep = ","; // change to "\n" if you want them vertically
         if ( i > 0 )
            text.text = sep + text.text;

         if (typeof notes[i].tpc === "undefined") // just in case
            return
         switch (notes[i].tpc) {
            case -1: text.text = qsTr("Fbb") + text.text; break;
            case  0: text.text = qsTr("Cbb") + text.text; break;
            case  1: text.text = qsTr("Gbb") + text.text; break;
            case  2: text.text = qsTr("Dbb") + text.text; break;
            case  3: text.text = qsTr("Abb") + text.text; break;
            case  4: text.text = qsTr("Ebb") + text.text; break;
            case  5: text.text = qsTr("Bbb") + text.text; break;
            case  6: text.text = qsTr("Fb")  + text.text; break;
            case  7: text.text = qsTr("Cb")  + text.text; break;

            case  8: text.text = qsTr("Gb")  + text.text; break;
            case  9: text.text = qsTr("Db")  + text.text; break;
            case 10: text.text = qsTr("Ab")  + text.text; break;
            case 11: text.text = qsTr("Eb")  + text.text; break;
            case 12: text.text = qsTr("Bb")  + text.text; break;
            case 13: text.text = qsTr("F")   + text.text; break;
            case 14: text.text = qsTr("C")   + text.text; break;
            case 15: text.text = qsTr("G")   + text.text; break;
            case 16: text.text = qsTr("D")   + text.text; break;
            case 17: text.text = qsTr("A")   + text.text; break;
            case 18: text.text = qsTr("E")   + text.text; break;
            case 19: text.text = qsTr("B")   + text.text; break;

            case 20: text.text = qsTr("F#")  + text.text; break;
            case 21: text.text = qsTr("C#")  + text.text; break;
            case 22: text.text = qsTr("G#")  + text.text; break;
            case 23: text.text = qsTr("D#")  + text.text; break;
            case 24: text.text = qsTr("A#")  + text.text; break;
            case 25: text.text = qsTr("E#")  + text.text; break;
            case 26: text.text = qsTr("B#")  + text.text; break;
            case 27: text.text = qsTr("F##") + text.text; break;
            case 28: text.text = qsTr("C##") + text.text; break;
            case 29: text.text = qsTr("G##") + text.text; break;
            case 30: text.text = qsTr("D##") + text.text; break;
            case 31: text.text = qsTr("A##") + text.text; break;
            case 32: text.text = qsTr("E##") + text.text; break;
            case 33: text.text = qsTr("B##") + text.text; break;
            default: text.text = qsTr("?")   + text.text; break;
         } // end switch tpc

         // octave, middle C being C4
         //text.text += (Math.floor(notes[i].pitch / 12) - 1)
         // or
         //text.text += (Math.floor(notes[i].ppitch / 12) - 1)

// change below false to true for courtesy- and microtonal accidentals
// you might need to come up with suitable translations
// only #, b, natural and possibly also ## seem to be available in UNICODE
         if (false) {
            switch (notes[i].userAccidental) {
               case  0: break;
               case  1: text.text = qsTr("#") + text.text; break;
               case  2: text.text = qsTr("b") + text.text; break;
               case  3: text.text = qsTr("##") + text.text; break;
               case  4: text.text = qsTr("bb") + text.text; break;
               case  5: text.text = qsTr("natural") + text.text; break;
               case  6: text.text = qsTr("flat-slash") + text.text; break;
               case  7: text.text = qsTr("flat-slash2") + text.text; break;
               case  8: text.text = qsTr("mirrored-flat2") + text.text; break;
               case  9: text.text = qsTr("mirrored-flat") + text.text; break;
               case 10: text.text = qsTr("mirrored-flat-slash") + text.text; break;
               case 11: text.text = qsTr("flat-flat-slash") + text.text; break;
               case 12: text.text = qsTr("sharp-slash") + text.text; break;
               case 13: text.text = qsTr("sharp-slash2") + text.text; break;
               case 14: text.text = qsTr("sharp-slash3") + text.text; break;
               case 15: text.text = qsTr("sharp-slash4") + text.text; break;
               case 16: text.text = qsTr("sharp arrow up") + text.text; break;
               case 17: text.text = qsTr("sharp arrow down") + text.text; break;
               case 18: text.text = qsTr("sharp arrow both") + text.text; break;
               case 19: text.text = qsTr("flat arrow up") + text.text; break;
               case 20: text.text = qsTr("flat arrow down") + text.text; break;
               case 21: text.text = qsTr("flat arrow both") + text.text; break;
               case 22: text.text = qsTr("natural arrow down") + text.text; break;
               case 23: text.text = qsTr("natural arrow up") + text.text; break;
               case 24: text.text = qsTr("natural arrow both") + text.text; break;
               case 25: text.text = qsTr("sori") + text.text; break;
               case 26: text.text = qsTr("koron") + text.text; break;
               default: text.text = qsTr("?") + text.text; break;
            } // end switch userAccidental
         } // end if courtesy- and microtonal accidentals
      } // end for note
   }

   onRun: {
      if (typeof curScore === 'undefined')
        Qt.quit();
      var cursor = curScore.newCursor();
      var startStaff;
      var endStaff;
      var endTick;
      var fullScore = false;
      cursor.rewind(1);
      if (!cursor.segment) { // no selection
        fullScore = true;
        startStaff = 0; // start with 1st staff
        endStaff  = curScore.nstaves - 1; // and end with last
      } else {
        startStaff = cursor.staffIdx;
        cursor.rewind(2);
        if (cursor.tick == 0) {
          // this happens when the selection includes
          // the last measure of the score.
          // rewind(2) goes behind the last segment (where
          // there's none) and sets tick=0
          endTick = curScore.lastSegment.tick + 1;
        } else {
          endTick = cursor.tick;
        }
        endStaff   = cursor.staffIdx;
      }
      console.log(startStaff + " - " + endStaff + " - " + endTick)

      for (var staff = startStaff; staff <= endStaff; staff++) {
        for (var voice = 0; voice < 4; voice++) {
          cursor.rewind(1); // beginning of selection
          cursor.voice    = voice;
          cursor.staffIdx = staff;

          if (fullScore)  // no selection
            cursor.rewind(0); // beginning of score

          while (cursor.segment && (fullScore || cursor.tick < endTick)) {
            if (cursor.element && cursor.element.type == Element.CHORD) {
              var text = newElement(Element.STAFF_TEXT);

              var graceChords = cursor.element.graceNotes;
              for (var i = 0; i < graceChords.length; i++) {
                // iterate through all grace chords
                var notes = graceChords[i].notes;
                nameChord(notes, text);
                // there seems to be no way of knowing the exact horizontal pos.
                // of a grace note, so we have to guess:
                text.pos.x = -2.5 * (graceChords.length - i);
                switch (voice) {
                  case 0: text.pos.y =  1; break;
                  case 1: text.pos.y = 10; break;
                  case 2: text.pos.y = -1; break;
                  case 3: text.pos.y = 12; break;
                }

                cursor.add(text);
                // new text for next element
                text  = newElement(Element.STAFF_TEXT);
              }

              var notes = cursor.element.notes;
              nameChord(notes, text);

              switch (voice) {
                case 0: text.pos.y =  1; break;
                case 1: text.pos.y = 10; break;
                case 2: text.pos.y = -1; break;
                case 3: text.pos.y = 12; break;
              }
              if ((voice == 0) && (notes[0].pitch > 83))
                text.pos.x = 1;
              cursor.add(text);
            } // end if CHORD
            cursor.next();
          } // end while segment
        } // end for voice
      } // end for staff
      Qt.quit();
   } // end onRun
}
