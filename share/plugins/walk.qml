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

            for (var track = 0; track < curScore.ntracks; ++track) {
                  var segment = curScore.firstSegment();
                  while (segment) {
                        console.log("segment: " + segment + "  type " + segment.segmentType);
                        var element = segment.elementAt(track);
                        if (element) {
                              var type    = element.type;
	                        console.log(type);
                              }
                        segment = segment.next;
                        }
                  }
            Qt.quit();
            }
      }
