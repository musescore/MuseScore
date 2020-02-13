//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Note Names Plugin
//
//  Copyright (C) 2012 Werner Schweer
//  Copyright (C) 2013 - 2020 Joachim Schmitz
//  Copyright (C) 2014 Jörn Eichler
//  Copyright (C) 2020 Johan Temmerman
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

import QtQuick 2.2
import MuseScore 3.0

MuseScore {
   version: "3.4.2.1"
   description: qsTr("This plugin names notes as per your language setting")
   menuPath: "Plugins.Notes." + qsTr("Note Names")

   // Small note name size is fraction of the full font size.
   property var fontSizeMini: 0.7;

   function nameChord (notes, text, small) {
      for (var i = 0; i < notes.length; i++) {
         var sep = "\n";   // change to "," if you want them horizontally (anybody?)
         if ( i > 0 )
            text.text = sep + text.text; // any but top note
         if (small)
             text.fontSize *= fontSizeMini
         if (typeof notes[i].tpc === "undefined") // like for grace notes ?!?
            return
         switch (notes[i].tpc) {
            case -1: text.text = qsTranslate("InspectorAmbitus", "F♭♭") + text.text; break;
            case  0: text.text = qsTranslate("InspectorAmbitus", "C♭♭") + text.text; break;
            case  1: text.text = qsTranslate("InspectorAmbitus", "G♭♭") + text.text; break;
            case  2: text.text = qsTranslate("InspectorAmbitus", "D♭♭") + text.text; break;
            case  3: text.text = qsTranslate("InspectorAmbitus", "A♭♭") + text.text; break;
            case  4: text.text = qsTranslate("InspectorAmbitus", "E♭♭") + text.text; break;
            case  5: text.text = qsTranslate("InspectorAmbitus", "B♭♭") + text.text; break;
            case  6: text.text = qsTranslate("InspectorAmbitus", "F♭")  + text.text; break;
            case  7: text.text = qsTranslate("InspectorAmbitus", "C♭")  + text.text; break;

            case  8: text.text = qsTranslate("InspectorAmbitus", "G♭")  + text.text; break;
            case  9: text.text = qsTranslate("InspectorAmbitus", "D♭")  + text.text; break;
            case 10: text.text = qsTranslate("InspectorAmbitus", "A♭")  + text.text; break;
            case 11: text.text = qsTranslate("InspectorAmbitus", "E♭")  + text.text; break;
            case 12: text.text = qsTranslate("InspectorAmbitus", "B♭")  + text.text; break;
            case 13: text.text = qsTranslate("InspectorAmbitus", "F")   + text.text; break;
            case 14: text.text = qsTranslate("InspectorAmbitus", "C")   + text.text; break;
            case 15: text.text = qsTranslate("InspectorAmbitus", "G")   + text.text; break;
            case 16: text.text = qsTranslate("InspectorAmbitus", "D")   + text.text; break;
            case 17: text.text = qsTranslate("InspectorAmbitus", "A")   + text.text; break;
            case 18: text.text = qsTranslate("InspectorAmbitus", "E")   + text.text; break;
            case 19: text.text = qsTranslate("InspectorAmbitus", "B")   + text.text; break;

            case 20: text.text = qsTranslate("InspectorAmbitus", "F♯")  + text.text; break;
            case 21: text.text = qsTranslate("InspectorAmbitus", "C♯")  + text.text; break;
            case 22: text.text = qsTranslate("InspectorAmbitus", "G♯")  + text.text; break;
            case 23: text.text = qsTranslate("InspectorAmbitus", "D♯")  + text.text; break;
            case 24: text.text = qsTranslate("InspectorAmbitus", "A♯")  + text.text; break;
            case 25: text.text = qsTranslate("InspectorAmbitus", "E♯")  + text.text; break;
            case 26: text.text = qsTranslate("InspectorAmbitus", "B♯")  + text.text; break;
            case 27: text.text = qsTranslate("InspectorAmbitus", "F♯♯") + text.text; break;
            case 28: text.text = qsTranslate("InspectorAmbitus", "C♯♯") + text.text; break;
            case 29: text.text = qsTranslate("InspectorAmbitus", "G♯♯") + text.text; break;
            case 30: text.text = qsTranslate("InspectorAmbitus", "D♯♯") + text.text; break;
            case 31: text.text = qsTranslate("InspectorAmbitus", "A♯♯") + text.text; break;
            case 32: text.text = qsTranslate("InspectorAmbitus", "E♯♯") + text.text; break;
            case 33: text.text = qsTranslate("InspectorAmbitus", "B♯♯") + text.text; break;
            default: text.text = qsTr("?")   + text.text; break;
         } // end switch tpc

         // octave, middle C being C4
         //text.text += (Math.floor(notes[i].pitch / 12) - 1)
         // or
         //text.text += (Math.floor(notes[i].ppitch / 12) - 1)
         // or even this, similar to the Helmholtz system but one octave up
         //var octaveTextPostfix = [",,,,,", ",,,,", ",,,", ",,", ",", "", "'", "''", "'''", "''''", "'''''"];
         //text.text += octaveTextPostfix[Math.floor(notes[i].pitch / 12)];

// change below false to true for courtesy- and microtonal accidentals
// you might need to come up with suitable translations
// only #, b, natural and possibly also ## seem to be available in UNICODE
         if (false) {
            switch (notes[i].userAccidental) {
               case  0: break;
               case  1: text.text = qsTranslate("accidental", "Sharp") + text.text; break;
               case  2: text.text = qsTranslate("accidental", "Flat") + text.text; break;
               case  3: text.text = qsTranslate("accidental", "Double sharp") + text.text; break;
               case  4: text.text = qsTranslate("accidental", "Double flat") + text.text; break;
               case  5: text.text = qsTranslate("accidental", "Natural") + text.text; break;
               case  6: text.text = qsTranslate("accidental", "Flat-slash") + text.text; break;
               case  7: text.text = qsTranslate("accidental", "Flat-slash2") + text.text; break;
               case  8: text.text = qsTranslate("accidental", "Mirrored-flat2") + text.text; break;
               case  9: text.text = qsTranslate("accidental", "Mirrored-flat") + text.text; break;
               case 10: text.text = qsTranslate("accidental", "Mirrored-flat-slash") + text.text; break;
               case 11: text.text = qsTranslate("accidental", "Flat-flat-slash") + text.text; break;
               case 12: text.text = qsTranslate("accidental", "Sharp-slash") + text.text; break;
               case 13: text.text = qsTranslate("accidental", "Sharp-slash2") + text.text; break;
               case 14: text.text = qsTranslate("accidental", "Sharp-slash3") + text.text; break;
               case 15: text.text = qsTranslate("accidental", "Sharp-slash4") + text.text; break;
               case 16: text.text = qsTranslate("accidental", "Sharp arrow up") + text.text; break;
               case 17: text.text = qsTranslate("accidental", "Sharp arrow down") + text.text; break;
               case 18: text.text = qsTranslate("accidental", "Sharp arrow both") + text.text; break;
               case 19: text.text = qsTranslate("accidental", "Flat arrow up") + text.text; break;
               case 20: text.text = qsTranslate("accidental", "Flat arrow down") + text.text; break;
               case 21: text.text = qsTranslate("accidental", "Flat arrow both") + text.text; break;
               case 22: text.text = qsTranslate("accidental", "Natural arrow down") + text.text; break;
               case 23: text.text = qsTranslate("accidental", "Natural arrow up") + text.text; break;
               case 24: text.text = qsTranslate("accidental", "Natural arrow both") + text.text; break;
               case 25: text.text = qsTranslate("accidental", "Sori") + text.text; break;
               case 26: text.text = qsTranslate("accidental", "Koron") + text.text; break;
               default: text.text = qsTr("?") + text.text; break;
            }  // end switch userAccidental
         }  // end if courtesy- and microtonal accidentals
      }  // end for note
   }

   function renderGraceNoteNames (cursor, list, text, small) {
      if (list.length > 0) {     // Check for existence.
         // Now render grace note's names...
         for (var chordNum = 0; chordNum < list.length; chordNum++) {
            // iterate through all grace chords
            var chord = list[chordNum];
            // Set note text, grace notes are shown a bit smaller
            nameChord(chord.notes, text, small)
            cursor.add(text)
            // X position the note name over the grace chord
            text.offsetX = chord.posX
            switch (cursor.voice) {
               case 1: case 3: text.placement = Placement.BELOW; break;
            }

            // If we consume a STAFF_TEXT we must manufacture a new one.
            text = newElement(Element.STAFF_TEXT);    // Make another STAFF_TEXT
         }
      }
      return text
   }

   onRun: {
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
         if (cursor.tick === 0) {
            // this happens when the selection includes
            // the last measure of the score.
            // rewind(2) goes behind the last segment (where
            // there's none) and sets tick=0
            endTick = curScore.lastSegment.tick + 1;
         } else {
            endTick = cursor.tick;
         }
         endStaff = cursor.staffIdx;
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
               if (cursor.element && cursor.element.type === Element.CHORD) {
                  var text = newElement(Element.STAFF_TEXT);      // Make a STAFF_TEXT

                  // First...we need to scan grace notes for existence and break them
                  // into their appropriate lists with the correct ordering of notes.
                  var leadingLifo = new Array();   // List for leading grace notes
                  var trailingFifo = new Array();  // List for trailing grace notes
                  var graceChords = cursor.element.graceNotes;
                  // Build separate lists of leading and trailing grace note chords.
                  if (graceChords.length > 0) {
                     for (var chordNum = 0; chordNum < graceChords.length; chordNum++) {
                        var noteType = graceChords[chordNum].notes[0].noteType
                        if (noteType === NoteType.GRACE8_AFTER || noteType === NoteType.GRACE16_AFTER ||
                              noteType === NoteType.GRACE32_AFTER) {
                           trailingFifo.unshift(graceChords[chordNum])
                        } else {
                           leadingLifo.push(graceChords[chordNum])
                        }
                     }
                  }

                  // Next process the leading grace notes, should they exist...
                  text = renderGraceNoteNames(cursor, leadingLifo, text, true)

                  // Now handle the note names on the main chord...
                  var notes = cursor.element.notes;
                  nameChord(notes, text, false);
                  cursor.add(text);

                  switch (cursor.voice) {
                     case 1: case 3: text.placement = Placement.BELOW; break;
                  }

                  text = newElement(Element.STAFF_TEXT) // Make another STAFF_TEXT object

                  // Finally process trailing grace notes if they exist...
                  text = renderGraceNoteNames(cursor, trailingFifo, text, true)
               } // end if CHORD
               cursor.next();
            } // end while segment
         } // end for voice
      } // end for staff
      Qt.quit();
   } // end onRun
}
