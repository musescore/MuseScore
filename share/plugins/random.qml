/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.1
import MuseScore 3.0

MuseScore {
      version:  "3.0"
      description: "Create random score."
      menuPath: "Plugins.random"
      requiresScore: false

      function addNote(key, cursor) {
            var cdur = [ 0, 2, 4, 5, 7, 9, 11 ];
            //           c  g  d  e
            var keyo = [ 0, 7, 2, 4 ];

            var idx    = Math.random() * 6;
            var octave = Math.floor(Math.random() * 2);
            var pitch  = cdur[Math.floor(idx)] + octave * 12 + 60  + keyo[key];
            cursor.addNote(pitch);
            }

      onRun: {
            var measures    = 18; //in 4/4 default time signature
            var numerator   = 3;
            var denominator = 4;
            var octaves     = 2;
            var key         = 3;

            var score = newScore("Random.mscz", "piano", measures);

            score.addText("title", "==Random==");
            score.addText("subtitle", "subtitle");

            var cursor = score.newCursor();
            cursor.track = 0;

            cursor.rewind(0);

            var ts = newElement(Element.TIMESIG);
            ts.timesig = fraction(numerator, denominator);
            cursor.add(ts);

            cursor.rewind(0);

            var realMeasures = Math.ceil(measures * denominator / numerator);
            console.log(realMeasures);
            var notes = realMeasures * 4; //number of 1/4th notes

            for (var i = 0; i < notes; ++i) {

                if (Math.random() < 0.5) {
                    cursor.setDuration(1, 8);
                    addNote(key, cursor);
                    addNote(key, cursor);
                    }
                else {
                    cursor.setDuration(1, 4);
                    addNote(key, cursor);
                    }

                }
            Qt.quit();
            }
      }
