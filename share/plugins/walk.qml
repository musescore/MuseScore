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
    version: "3.1"
    description: "This test plugin walks through all available plugin elements in a score"
    menuPath: "Plugins.Walk"
    requiresScore: true;

    onRun: {
        var SegmentTypes = Object.freeze({
            0: "Invalid",
            1: "BeginBarLine",
            2: "HeaderClef",
            4: "KeySig",
            8: "Ambitus",
            16: "TimeSig",
            32: "StartRepeatBarLine",
            64: "Clef",
            128: "BarLine",
            256: "Breath",
            512: "ChordRest",
            1024: "EndBarLine",
            2048: "KeySigAnnounce",
            4096: "TimeSigAnnounce"
        });

        var partListing = "";
        var parts = curScore.parts;
        for (var p in parts) {
            partListing += parts[p].partName + "\n";
            var instrumentListing = "";
            for (var i in parts[p].instruments) {
                instrumentListing += "\t" + parts[p].instruments[i].instrumentId + " (" + parts[p].instruments[i].channels.length + " Channel(s))\n";
            }
            if (parts[p].instruments.length <= 1) {
                partListing = partListing.slice(0, -1);
            }
            partListing += instrumentListing;
        }
        console.log("=== Part Listing ===\n" + partListing);

        var measure = curScore.firstMeasure;
        var measureCounter = 1;
        while (measure) {
            var measureInfo = "== Measure " + measureCounter + " ==\n";
            for (var e in measure.elements) {
                measureInfo += measure.elements[e].type + " " + measure.elements[e].name + "\n";
            }
            var segment = measure.firstSegment;
            while (segment) {
                var segmentInfo = segment.tick + "\t" + SegmentTypes[Number(segment.segmentType)];
                if (segment.annotations && segment.annotations.length) {
                    for (var a in segment.annotations) {
                        segmentInfo += "\n\tAN: " + segment.annotations[a].type + "\t" + segment.annotations[a].name;
                    }
                }

                for (var t = 0; t < curScore.ntracks; ++t) {
                    var el = segment.elementAt(t);
                    if (el) {
                        segmentInfo += "\n\tEL: " + el.type + "\t" + el.name + "\tTrack: " + t;
                        if (el.type == Element.CHORD) {
                            segmentInfo += "\n\t\t\tGrace notes: " + el.graceNotes.length;
                            segmentInfo += "\n\t\t\tNotes:";
                            for (var n in el.notes) {
                                segmentInfo += "\n\t\t\t\t" + el.notes[n].pitch + "\ttieBack: " + (el.notes[n].tieBack != null) + "\ttieForward: " + (el.notes[n].tieForward != null);
                                for (var notEl in el.notes[n].elements) {
                                    segmentInfo += "\n\t\t\t\t" + el.notes[n].elements[notEl].type + "\t" + el.notes[n].elements[notEl].name;
                                }
                            }
                        }
                    }
                }

                measureInfo += segmentInfo + "\n";
                segment = segment.nextInMeasure;
            }
            console.log(measureInfo);
            measure = measure.nextMeasure;
            ++measureCounter;
        }

        Qt.quit(); // WARNING: this kills off ALL active plugins!
    }
}
