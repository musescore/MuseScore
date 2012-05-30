//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Color notehead plugin
//	Noteheads are colored according to pitch. User can change to color by
//  modifying the colors array. First element is C, second C# etc...
//
//  Copyright (C)2008 Werner Schweer and others
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
      // width:  150
      // height: 75
      onRun: {
            console.log("hello colornotes");
            var score = curScore;

//            var colors = [new color(226,28,72),new color(242,102,34),new color(249,157,28),
//            new color(255,204,51),new color(255,243,43),new color(188,216,95),
//            new color(98,188,71),new color(0,156,149),new color(0,113,187),
//            new color(94,80,161),new color(141,91,166),new color(207,62,150)];

            var colors = [
               "#ff0000", "#00ff00", "#0000ff",
               "#ff0000", "#00ff00", "#0000ff",
               "#ff0000", "#00ff00", "#0000ff",
               "#ff0000", "#00ff00", "#0000ff"
               ];

            if (typeof curScore === 'undefined')
                  return;
            var cursor = newCursor();
            for (var staff = 0; staff < curScore.nstaves(); ++staff) {
                  cursor.staffIdx = staff;
                  for (var v = 0; v < 4; v++) {
                        cursor.voice = v;
                        cursor.rewind(0);  // set cursor to first chord/rest

                        while (cursor.element()) {
                              if (cursor.element().type == MScore.CHORD) {
                                    var notes = cursor.element().notes;
                                    for (var i = 0; i < notes.length; i++) {
                                          var note = notes[i];
                                          note.color = colors[note.pitch % 12];
                                          }
                                    }
                              cursor.next();
                              }
                        }
                  }
            Qt.quit()
            }
      }

