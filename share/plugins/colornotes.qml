//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Color notehead plugin
//	Noteheads are colored according to pitch. User can change to color by
//  modifying the colors array. First element is C, second C# etc...
//
//  Copyright (C)2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

import QtQuick 1.0
import MuseScore 1.0

MuseScore {
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

