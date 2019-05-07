//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

import QtQuick 2.0
import MuseScore 3.0

MuseScore {
      version:  "3.0"
      description: "This test plugin walks through all elements in a score"
      menuPath: "Plugins.Walk"

      onRun: {
            console.log("Hello Walker");

            if (!curScore)
                  Qt.quit();

            var cursor = curScore.newCursor();
            cursor.voice    = 0;
            cursor.staffIdx = 0;
            cursor.rewind(Cursor.SCORE_START);

            while (cursor.segment) {
                var e = cursor.element;
                if (e) {
                    console.log("type:", e.name, "at  tick:", e.tick, "color", e.color);
                    if (e.type == Element.REST) {
                        var d = e.duration;
                        console.log("   duration " + d.numerator + "/" + d.denominator);
                        }
                    }
                cursor.next();
                }
            Qt.quit();
            }
      }
