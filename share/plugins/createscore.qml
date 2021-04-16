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
import QtQuick 2.0
import MuseScore 3.0

MuseScore {
      version:  "3.0"
      description: "This demo plugin creates a new score."
      menuPath: "Plugins.createscore"
      requiresScore: false

      onRun: {
            console.log("hello createscore");
            var score = newScore("Test-Score", "piano", 5);
            var numerator = 3;
            var denominator = 4;

            score.addText("title", "==Test-Score==");
            score.addText("subtitle", "subtitle");
            score.addText("composer", "Composer");
            score.addText("lyricist", "Lyricist");

            var cursor = score.newCursor();
            cursor.track = 0;

            cursor.rewind(0);
            var ts = newElement(Element.TIMESIG);
            ts.timesig = fraction(numerator, denominator);
            cursor.add(ts);

            cursor.rewind(0);
            cursor.setDuration(1, 4);
            cursor.addNote(60);

            cursor.next();
            cursor.setDuration(3, 8);
            cursor.addNote(64);

            cursor.next();
            cursor.setDuration(1, 4);
            cursor.addNote(68);

            cursor.next();
            cursor.addNote(72);

            Qt.quit();
            }
      }
