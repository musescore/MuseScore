//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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
      menuPath: "Plugins.notenames"

      onRun: {
            if (typeof curScore === 'undefined')
                  return;
            var cursor = newCursor();
            cursor.staffIdx = 0;
            cursor.voice = 0;
            cursor.rewind(0);  // set cursor to first chord/rest

            var names = ["C", "Cis", "D", "Dis", "E", "F", "Fis", "G", "Gis", "A", "Ais", "B" ];

            while (cursor.segment) {
                  if (cursor.element.type == MScore.CHORD) {
                        var text  = newElement(MScore.STAFF_TEXT);
                        text.text = names[cursor.element.notes[0].pitch % 12];
                        text.parent = cursor.segment;
                        cursor.add(text);
                        }
                  cursor.next();
                  }
            }
      }

