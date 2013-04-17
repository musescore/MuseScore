//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//  Copyright (C) 2013 Nicolas Froment, Joachim Schmitz
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

import QtQuick 1.0
import MuseScore 1.0

MuseScore {
      version:  "1.0"
      description: "This demo plugin colors notes in the selection depending on pitch"
      menuPath: "Plugins.Notes.Color Notes"
 
      property variant colors : [
               "#e21c48", "#f26622", "#f99d1c",
               "#ffcc33", "#fff32b", "#bcd85f",
               "#62bc47", "#009c95", "#0071bb",
               "#5e50a1", "#8d5ba6", "#cf3e96"
               ]
      property variant black : "#000000"
 
      // Apply the given function to all notes in selection
      // or, if nothing is selected, in the entire score
      function applyToNotesInSelection(func) {
            var cursor     = curScore.newCursor();
            cursor.rewind(1);
            var startStaff  = cursor.staffIdx;
            cursor.rewind(2);
            var endStaff   = cursor.staffIdx;
            var endTick    = cursor.tick // if no selection, end of score
            var fullScore = false;
            if (!cursor.segment) { // no selection
                  fullScore = true;
                  startStaff = 0; // start with 1st staff
                  endStaff = curScore.nstaves; // and end with last
            }
       console.log(startStaff + " - " + endStaff + " - " + endTick)
            for (var staff = startStaff; staff <= endStaff; staff++) {
                  for (var voice = 0; voice < 4; voice++) {          
                        cursor.rewind(1); // sets voice to 0
                        cursor.voice = voice; //voice has to be set after goTo
                        cursor.staffIdx = staff;

                        if (fullScore)
                              cursor.rewind(0) // if no selection, beginning of score

                        while (cursor.segment && (fullScore || cursor.tick < endTick)) {
                              if (cursor.element && cursor.element.type == Element.CHORD) {
                                    var notes = cursor.element.notes;
                                    for (var i = 0; i < notes.length; i++) {
                                          var note = notes[i];
                                          func(note);
                                    }
                              }
                              cursor.next();
                        }
                  }
            }
      }
 
      function colorNote(note) {
            if (note.color == black)
                  note.color = colors[note.pitch % 12];
            else
                  note.color = black;
      }
 
      onRun: {
            console.log("hello colornotes");
 
            if (typeof curScore === 'undefined')
                  Qt.quit();
 
            applyToNotesInSelection(colorNote)
 
            Qt.quit();
         }
}
