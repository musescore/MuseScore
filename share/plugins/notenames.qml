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
      menuPath: "Plugins.notenames"
      version:  "2.0"
      description: "This demo plugin names notes."

      onRun: {
            if (typeof curScore === 'undefined')
                  Qt.quit();

            var cursor = curScore.newCursor();
            cursor.rewind(0);  // set cursor to first chord/rest

            var names = ["C", "Cis", "D", "Dis", "E", "F", "Fis", "G", "Gis", "A", "Ais", "B" ];

            while (cursor.segment) {
                  if (cursor.element.type == Element.CHORD) {
                        var text  = newElement(Element.STAFF_TEXT);
                        text.text = names[cursor.element.notes[0].pitch % 12];
                        cursor.add(text);
                        }
                  cursor.next();
                  }

            Qt.quit();
            }
      }

