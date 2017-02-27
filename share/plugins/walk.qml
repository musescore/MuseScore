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

import QtQuick 2.0
import MuseScore 1.0

MuseScore {
      version:  "1.0"
      description: "This test plugin walks through all elements in a score"
      menuPath: "Plugins.Walk"

      onRun: {
            console.log("Hello Walker");

            if (typeof curScore === 'undefined')
                  Qt.quit();

            var cursor = curScore.newCursor();
            cursor.rewind(0);
            cursor.voice    = 0;
            cursor.staffIdx = 0;

            while (cursor.segment()) {
                var element = cursor.element();
                if (element) {
                    var type = element.type;
	              console.log("type: " + element.type + " tick: " + element.tick() + " color " + element.get("color"));
                    if (type == "Rest") {
                        var d = element.get("duration");
                        console.log(d);
                        console.log("   duration " + d.numerator + "/" + d.denominator);
                        }
                    }
                cursor.next();
                }
            Qt.quit();
            }
      }
