//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
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
      description: "This demo plugin colors notes depending on pitch"
      menuPath: "Plugins.colornotes"

      onRun: {
            console.log("hello colornotes");

            var colors = [
               "#e21c48", "#f26622", "#f99d1c",
               "#ffcc33", "#fff32b", "#bcd85f",
               "#62bc47", "#009c95", "#0071bb",
               "#5e50a1", "#8d5ba6", "#cf3e96"
               ];

            if (typeof curScore === 'undefined')
                  Qt.quit();

            var cursor = curScore.newCursor();
            for (var track = 0; track < curScore.ntracks; ++track) {
                  cursor.track = track;
                  cursor.rewind(0);  // set cursor to first chord/rest

                  while (cursor.segment) {
                        if (cursor.element && cursor.element.type == MScore.CHORD) {
                              var notes = cursor.element.notes;
                              for (var i = 0; i < notes.length; i++) {
                                    var note = notes[i];
                                    note.color = colors[note.pitch % 12];
                                    }
                              }
                        cursor.next();
                        }
                  }
            Qt.quit();
            }
      }

