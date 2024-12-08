// Apply a choice of tempraments and tunings.
// Copyright (C) 2018-2019  Bill Hails
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Muse.UiComponents 
import MuseScore 3.0
import FileIO 

MuseScore {
    id: window
    version: "4"
    title: "Temperaments"
    description: "Apply various temperaments and tunings"
    pluginType: "dialog"
    categoryCode: "playback"
    thumbnailName: "modal_tuning.png"
    
    width: 780
    height: 640

    property var offsetTextWidth: 40;
    property var offsetLabelAlignment: 0x02 | 0x80;

    property var history: 0;

    // set true if customisations are made to the tuning
    property var modified: false;

    /**
     * See http://leware.net/temper/temper.htm and specifically http://leware.net/temper/cents.htm
     *
     * I've taken the liberty of adding the Bach/Lehman temperament http://www.larips.com which was
     * my original motivation for doing this.
     *
     * These values are in cents. One cent is defined as 100th of an equal tempered semitone.
     * Each row is ordered in the cycle of fifths, so C, G, D, A, E, B, F#, C#, G#/Ab, Eb, Bb, F;
     * and the values are offsets from the equal tempered value.
     *
     * However for tunings who's default root note is not C, the values are pre-rotated so that applying the
     * root note rotation will put the first value of the sequence at the root note.
     */
    
    property var westernTemperaments: [
        {
            'name': "Equal",
            'offsets': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Pythagorean",
            'offsets': [-6.0, -4.0, -2.0, 0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0],
            'root': 9,
            'pure': 3
        },
        {
            'name': "Aaron",
            'offsets': [10.5, 7.0, 3.5, 0.0, -3.5, -7.0, -10.5, -14.0, -17.5, -21.0, -24.5, -28.0],
            'root': 9,
            'pure': 3
        },
        {
            'name': "Silberman",
            'offsets': [5.0, 3.3, 1.7, 0.0, -1.7, -3.3, -5.0, -6.7, -8.3, -10.0, -11.7, -13.3],
            'root': 9,
            'pure': 3
        },
        {
            'name': "Salinas",
            'offsets': [16.0, 10.7, 5.3, 0.0, -5.3, -10.7, -16.0, -21.3, -26.7, -32.0, -37.3, -42.7],
            'root': 9,
            'pure': 3
        },
        {
            'name': "Kirnberger",
            'offsets': [0.0, -3.5, -7.0, -10.5, -14.0, -12.0, -10.0, -10.0, -8.0, -6.0, -4.0, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Vallotti",
            'offsets': [0.0, -2.0, -4.0, -6.0, -8.0, -10.0, -8.0, -6.0, -4.0, -2.0, 0.0, 2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Werkmeister",
            'offsets': [0.0, -4.0, -8.0, -12.0, -10.0, -8.0, -12.0, -10.0, -8.0, -6.0, -4.0, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Marpurg",
            'offsets': [0.0, 2.0, 4.0, 6.0, 0.0, 2.0, 4.0, 6.0, 0.0, 2.0, 4.0, 6.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Just",
            'offsets': [0.0, 2.0, 4.0, -16.0, -14.0, -12.0, -10.0, -30.0, -28.0, 16.0, 18.0, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "MeanSemitone",
            'offsets': [0.0, -3.5, -7.0, -10.5, -14.0, 3.5, 0.0, -3.5, -7.0, -10.5, -14.0, -17.5],
            'root': 6,
            'pure': 6
        },
        {
            'name': "Grammateus",
            'offsets': [-2.0, 0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 0.0, 2.0, 4.0, 6.0, 8.0],
            'root': 11,
            'pure': 1
        },
        {
            'name': "French",
            'offsets': [0.0, -2.5, -5.0, -7.5, -10.0, -12.5, -13.0, -13.0, -11.0, -6.0, -1.5, 2.5],
            'root': 0,
            'pure': 0
        },
        {
            'name': "French2",
            'offsets': [0.0, -3.5, -7.0, -10.5, -14.0, -17.5, -18.2, -19.0, -17.0, -10.5, -3.5, 3.5],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Rameau",
            'offsets': [0.0, -3.5, -7.0, -10.5, -14.0, -17.5, -15.5, -13.5, -11.5, -2.0, 7.0, 3.5],
            'root': 0,
            'pure': 0
        },
        {
            'name': "IrrFr17e",
            'offsets': [-8.0, -2.0, 3.0, 0.0, -3.0, -6.0, -9.0, -12.0, -15.0, -18.0, -21.0, -24.0],
            'root': 9,
            'pure': 3
        },
        {
            'name': "BachLehman",
            'offsets': [0.0, -2.0, -3.9, -5.9, -7.8, -5.9, -3.9, -2.0, -2.0, -2.0, -2.0, 2.0],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Meantone (1/4) 5 flats",
            'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -6.8, 30.8, 27.4, 24.0, 20.5, 17.1, 13.7],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Meantone 1/4-comma",
            'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -6.8, -10.3, -13.7, -17.1, 20.5, 17.1, 13.7],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Meantone (1/4) 5 sharps",
            'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -6.8, -10.3, -13.7, -17.1, -20.5, -24.0, 13.7],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Meantone 1/5-comma",
            'offsets': [7.0, 4.7, 2.3, 0.0, -2.3, -4.7, -7.0, -9.4, -11.7, 14.1, 11.7, 9.4],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Meantone 1/6-comma",
            'offsets': [4.9, 3.3, 1.6, 0.0, -1.6, -3.3, -4.9, -6.5, -8.1, 9.8, 8.1, 6.5],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Werckmeister III",
            'offsets': [11.7, 7.8, 3.9, 0.0, 2.0, 3.9, 0.0, 2.0, 3.9, 5.9, 7.8, 9.8],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Kirnberger III",
            'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -1.5, 0.5, 0.5, 2.4, 4.4, 6.4, 8.3],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Vallotti",
            'offsets': [5.9, 3.9, 2.0, 0.0, -2.0, -3.9, -2.0, 0.0, 2.0, 3.9, 5.9, 7.8],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Young I",
            'offsets': [6.2, 4.2, 2.1, 0.0, -2.1, -1.9, -1.8, 0.1, 2.1, 4.1, -0.3, -0.1],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Kellner",
            'offsets': [8.2, 5.5, 2.7, 0.0, -2.7, -0.8, -3.5, -1.6, 0.4, 2.3, 4.3, 6.3],
            'root': 0,
            'pure': 3
        },
        {
            'name': "Fernando A. Martin 1/45-comma",
            'offsets': [6.6, 3.9, 1.7, 0.0, -1.2, -1.8, -2.0, -1.6, -0.7, 0.8, 2.7, 4.7],
            'root': 0,
            'pure': 0
        },
        {
            'name': "C Cm Db Dm Eb Ebm Em F Fm Ab Am Bb",
            'offsets': [0.0, 2.0, -17.6, -15.6, -13.7, -11.7, 31.3, 11.7, 13.7, 15.6, 17.6, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "C Cm C#m D Dm E Em F F#m A Am Bb",
            'offsets': [0.0, 2.0, -17.6, -15.6, -13.7, -11.7, -31.3, -29.3, -27.4, 15.6, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "C Cm Db D Dm Em F Fm Ab Am Bb Bbm",
            'offsets': [0.0, 2.0, -17.6, -15.6, -13.7, -11.7, -31.3, 11.7, 13.7, 15.6, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "C Cm Db Eb Em F Fm G Gm Ab Am Bm",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -11.7, -9.8, 11.7, 13.7, 15.6, 17.6, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "C Cm D Dm Eb Em F#m G Gm Ab Bb Bm",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, 13.7, 15.6, 17.6, 19.6],
            'root': 0,
            'pure': 0
        },
        {
            'name': "C D Ebm E Em F# F#m G G#m Bbm B Bm",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, -27.4, -25.4, -23.5, -21.5],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Simple Ratios",
            'offsets': [0.0, 2.0, 31.2, -15.6, -13.7, -28.3, -17.5, 38.6, 13.7, 15.6, -31.2, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Alternate Ratios",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -11.7, 17.5, 28.3, 13.7, 15.6, 17.6, -2.0],
            'root': 0,
            'pure': 0
        }
    ]

    property var middleEasternTemperaments: [
        {
            'name': "Melodic 1# 2b",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, 9.8, -9.8, -7.8, -7.8, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Harmonic 1# 2b",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -11.7, -9.8, 11.7, 13.7, 15.6, 17.6, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Rast, Sikah",
            'offsets': [0.0, 2.0, 3.9, 5.9, -35.2, -33.2, -9.8, -7.8, -7.8, -25.4, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Suznak, Huzam",
            'offsets': [0.0, 2.0, 3.9, 5.9, -35.2, -11.7, -9.8, -7.8, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Nayruz",
            'offsets': [0.0, 2.0, 3.9, -37.1, -35.2, 9.8, -9.8, -7.8, -7.8, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Bayati, Kurd, Huseyni",
            'offsets': [0.0, 2.0, 3.9, 5.9, -56.7, -54.7, -9.8, -7.8, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Qarjighar",
            'offsets': [0.0, 2.0, 3.9, 5.9, -56.7, -11.7, -9.8, -7.8, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Saba, Basta Nikar, Zanjaran",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -33.2, 9.8, 11.7, 13.7, 43.3, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Hijaz, Nikriz",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -33.2, -9.8, -7.8, 13.7, 15.6, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Nawa'athar, Shad Araban",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, 11.7, 13.7, 15.6, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Shehnaz",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, 13.7, 15.6, 17.6, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Nahawand, Hijaz Kar",
            'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, 11.7, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "hawand, Hijaz Kar Kurd",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, 9.8, -9.8, -7.8, -7.8, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Iraq, Yekah, Nawa",
            'offsets': [0.0, 2.0, 3.9, 5.9, -56.7, -33.2, -31.3, -7.8, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Farahnak, Yekah, Nawa",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, -33.2, -31.3, -29.3, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Jiharkah",
            'offsets': [0.0, 2.0, 3.9, -15.6, -35.2, -11.7, 9.8, 11.7, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Ajam Ashyran, Shawq Afza",
            'offsets': [0.0, 2.0, -17.6, -15.6, 7.8, 9.8, 9.8, 11.7, -7.8, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Hisar",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, 9.8, -9.8, -7.8, -7.8, 43.3, 17.6, 19.6],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Nishaburek (Rast in D & A)",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, 9.8, -31.3, -29.3, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Nishaburek (Rast in D, Bayati in A)",
            'offsets': [0.0, 2.0, 3.9, 5.9, 7.8, -54.7, -31.3, -29.3, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Saba Zamzam",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -33.2, 9.8, 11.7, 13.7, -5.9, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Rakb",
            'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -33.2, 58.9, 11.7, 13.7, 43.3, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Ikah Baladi",
            'offsets': [0.0, 2.0, 3.9, 5.9, -35.2, -33.2, -9.8, 39.4, 41.4, -25.4, -3.9, -2.0],
            'root': 0,
            'pure': 0
        },
        {
            'name': "Iraq (Cadence)",
            'offsets': [0.0, 2.0, 3.9, 5.9, -56.7, -33.2, -31.3, -7.8, 13.7, -5.9, -23.5, -2.0],
            'root': 0,
            'pure': 0
        }
    ]

    property var westernTemperamentsLength: 36
    property var middleEasternTemperamentsLength: 24

    property var currentTemperament: westernTemperaments[0];
    property var currentTab: 0 ;
    property var currentRoot: 0;
    property var currentPureTone: 0;
    property var currentTweak: 0.0;

    onRun: {
        if (!curScore) {
            error("No score open.\nThis plugin requires an open score to run.\n")
            quit()
        }
    }

    function getHistory() {
        if (history == 0) {
            history = new commandHistory()
        }
        return history
    }

    function applyTemperament() {
        var selection = new scoreSelection()
        curScore.startCmd()
        selection.map(filterNotes, reTune(getFinalTuning()))
        if (annotateValue.checked == true) {
            selection.map(filterNotes, annotate)
        }
        curScore.endCmd()
        return true
    }

    function filterNotes(element) {
        return element.type == Element.CHORD
    }

    function annotate(chord, cursor) {
        function addText(noteIndex, placement) {
            var note = chord.notes[noteIndex]
            var text = newElement(Element.STAFF_TEXT);
            text.text = '' + note.tuning
            text.autoplace = true
            text.fontSize = 7 // smaller
            text.placement = placement
            cursor.add(text)
        }

        if (cursor.voice == 0 || cursor.voice == 2) {
            for (var index = 0; index < chord.notes.length; index++) {
                addText(index, Placement.ABOVE)
            }
        } else {
            for (var index = chord.notes.length - 1; index >= 0; index--) {
                addText(index, Placement.BELOW)
            }
        }
    }

    function reTune(tuning) {
        return function(chord, cursor) {
            for (var i = 0; i < chord.notes.length; i++) {
                var note = chord.notes[i]
                note.tuning = tuning(note.pitch)
            }
        }
    }

    function scoreSelection() {
        const SCORE_START = 0
        const SELECTION_START = 1
        const SELECTION_END = 2
        var fullScore
        var startStaff
        var endStaff
        var endTick
        var inRange
        var rewind
        var cursor = curScore.newCursor()
        cursor.rewind(SELECTION_START)
        if (cursor.segment) {
            startStaff = cursor.staffIdx
            cursor.rewind(SELECTION_END)
            endStaff = cursor.staffIdx;
            endTick = 0 // unused
            if (cursor.tick === 0) {
               endTick = curScore.lastSegment.tick + 1;
            } else {
               endTick = cursor.tick;
            }
            inRange = function() {
                return cursor.segment && cursor.tick < endTick
            }
            rewind = function (voice, staff) {
                // no idea why, but if there is a selection then
                // we need to rewind the cursor *before* setting
                // the voice and staff index.
                cursor.rewind(SELECTION_START)
                cursor.voice = voice
                cursor.staffIdx = staff
            }
        } else {
            startStaff = 0
            endStaff  = curScore.nstaves - 1
            inRange = function () {
                return cursor.segment
            }
            rewind = function (voice, staff) {
                // no idea why, but if there's no selection then
                // we need to rewind the cursor *after* setting
                // the voice and staff index.
                cursor.voice = voice
                cursor.staffIdx = staff
                cursor.rewind(SCORE_START)
            }
        }

        this.map = function(filter, process) {
            for (var staff = startStaff; staff <= endStaff; staff++) {
                for (var voice = 0; voice < 4; voice++) {
                    rewind(voice, staff)
                    while (inRange()) {
                        if (cursor.element && filter(cursor.element)) {
                            process(cursor.element, cursor)
                        }
                        cursor.next()
                    }
                }
            }
        }
    }

    function error(errorMessage) {
        errorDialog.text = qsTr(errorMessage)
        errorDialog.open()
    }

    /**
     * map a note (pitch modulo 12) to a value in one of the above tables
     * then adjust for the choice of pure note and tweak.
     */
    function lookUp(note, table) {
        var i = ((note * 7) - currentRoot + 12) % 12;
        var offset = table.offsets[i];
        var j = (currentPureTone - currentRoot + 12) % 12;
        var pureNoteAdjustment = table.offsets[j];
        var finalOffset = offset - pureNoteAdjustment;
        var tweakFinalOffset = finalOffset + parseFloat(tweakValue.text);
        return tweakFinalOffset
    }

    /**
     * returns a function for use by recalculate()
     *
     * We use an abstract function here because recalculate can be passed
     * a different function, i.e. when restoring from a save file.
     */
    function getTuning() {
        return function(pitch) {
            return lookUp(pitch, currentTemperament);
        }
    }

    function getFinalTuning() {
        return function(pitch) {
            pitch = pitch % 12
            switch (pitch) {
                case 0:
                    return getFinalOffset(final_c)
                case 1:
                    return getFinalOffset(final_c_sharp)
                case 2:
                    return getFinalOffset(final_d)
                case 3:
                    return getFinalOffset(final_e_flat)
                case 4:
                    return getFinalOffset(final_e)
                case 5:
                    return getFinalOffset(final_f)
                case 6:
                    return getFinalOffset(final_f_sharp)
                case 7:
                    return getFinalOffset(final_g)
                case 8:
                    return getFinalOffset(final_g_sharp)
                case 9:
                    return getFinalOffset(final_a)
                case 10:
                    return getFinalOffset(final_b_flat)
                case 11:
                    return getFinalOffset(final_b)
                default:
                    error("unrecognised pitch: " + pitch)
            }
        }
    }

    function getFinalOffset(textField) {
        return parseFloat(textField.text)
    }

    function recalculate(tuning) {
        var old_final_c       = final_c.text
        var old_final_c_sharp = final_c_sharp.text
        var old_final_d       = final_d.text
        var old_final_e_flat  = final_e_flat.text
        var old_final_e       = final_e.text
        var old_final_f       = final_f.text
        var old_final_f_sharp = final_f_sharp.text
        var old_final_g       = final_g.text
        var old_final_g_sharp = final_g_sharp.text
        var old_final_a       = final_a.text
        var old_final_b_flat  = final_b_flat.text
        var old_final_b       = final_b.text
        getHistory().add(
            function () {
                final_c.text               = old_final_c
                final_c.previousText       = old_final_c
                final_c_sharp.text         = old_final_c_sharp
                final_c_sharp.previousText = old_final_c_sharp
                final_d.text               = old_final_d
                final_d.previousText       = old_final_d
                final_e_flat.text          = old_final_e_flat
                final_e_flat.previousText  = old_final_e_flat
                final_e.text               = old_final_e
                final_e.previousText       = old_final_e
                final_f.text               = old_final_f
                final_f.previousText       = old_final_f
                final_f_sharp.text         = old_final_f_sharp
                final_f_sharp.previousText = old_final_f_sharp
                final_g.text               = old_final_g
                final_g.previousText       = old_final_g
                final_g_sharp.text         = old_final_g_sharp
                final_g_sharp.previousText = old_final_g_sharp
                final_a.text               = old_final_a
                final_a.previousText       = old_final_a
                final_b_flat.text          = old_final_b_flat
                final_b_flat.previousText  = old_final_b_flat
                final_b.text               = old_final_b
                final_b.previousText       = old_final_b
            },
            function() {
                final_c.text               = tuning(0).toFixed(1)
                final_c.previousText       = final_c.text
                final_c_sharp.text         = tuning(1).toFixed(1)
                final_c_sharp.previousText = final_c_sharp.text
                final_d.text               = tuning(2).toFixed(1)
                final_d.previousText       = final_d.text
                final_e_flat.text          = tuning(3).toFixed(1)
                final_e_flat.previousText  = final_e_flat.text
                final_e.text               = tuning(4).toFixed(1)
                final_e.previousText       = final_e.text
                final_f.text               = tuning(5).toFixed(1)
                final_f.previousText       = final_f.text
                final_f_sharp.text         = tuning(6).toFixed(1)
                final_f_sharp.previousText = final_f_sharp.text
                final_g.text               = tuning(7).toFixed(1)
                final_g.previousText       = final_g.text
                final_g_sharp.text         = tuning(8).toFixed(1)
                final_g_sharp.previousText = final_g_sharp.text
                final_a.text               = tuning(9).toFixed(1)
                final_a.previousText       = final_a.text
                final_b_flat.text          = tuning(10).toFixed(1)
                final_b_flat.previousText  = final_b_flat.text
                final_b.text               = tuning(11).toFixed(1)
                final_b.previousText       = final_b.text
            },
            "final offsets"
        )
    }

    function setCurrentTemperament(temperament) {
        var oldTemperament = currentTemperament
        getHistory().add(
            function() {
                currentTemperament = oldTemperament
                checkCurrentTemperament()
            },
            function() {
                currentTemperament = temperament
                checkCurrentTemperament()
            },
            "current temperament"
        )
    }

    function checkCurrentTemperament() {
        var idx;
        switch (currentTab) {
        case 0:
            idx = westernTemperaments.indexOf(currentTemperament);
            westernListView.itemAtIndex(idx).checked = true;
            break;
        case 1:
            idx = middleEasternTemperaments.indexOf(currentTemperament);
            middleEasternListView.itemAtIndex(idx).checked = true;
            break;
        }        
    }

    function lookupTemperament(temperamentName) {
        switch (currentTab) {
        case 0:
            westernTemperaments.find(function (x) {
                return x.name === temperamentName;
            });
            break;
        case 1:
            middleEasternTemperaments.find(function (x) {
                return x.name === temperamentName;
            });
            break;
        }      
    }

    function setCurrentTab(tab) {
        var oldTab = currentTab
        getHistory().add(
            function(){
                currentTab = oldTab
                checkCurrentTab()
            },
            function(){
                currentTab = tab
                checkCurrentTab()
            },
            "current tab"            
        )
    }
    
    function checkCurrentTab() {
        switch (currentTab){
            case 0:
                westernTab.checked = true
                break
            case 1:
                middleEasternTab.checked = true
                break
        } 
    }
    
    function setCurrentRoot(root) {
        var oldRoot = currentRoot
        getHistory().add(
            function () {
                currentRoot = oldRoot
                checkCurrentRoot()
            },
            function() {
                currentRoot = root
                checkCurrentRoot()
            },
            "current root"
        )
    }

    function checkCurrentRoot() {
        switch (currentRoot) {
            case 0:
                root_c.checked = true
                break
            case 1:
                root_g.checked = true
                break
            case 2:
                root_d.checked = true
                break
            case 3:
                root_a.checked = true
                break
            case 4:
                root_e.checked = true
                break
            case 5:
                root_b.checked = true
                break
            case 6:
                root_f_sharp.checked = true
                break
            case 7:
                root_c_sharp.checked = true
                break
            case 8:
                root_g_sharp.checked = true
                break
            case 9:
                root_e_flat.checked = true
                break
            case 10:
                root_b_flat.checked = true
                break
            case 11:
                root_f.checked = true
                break
        }
    }

    function setCurrentPureTone(pureTone) {
        var oldPureTone = currentPureTone
        getHistory().add(
            function () {
                currentPureTone = oldPureTone
                checkCurrentPureTone()
            },
            function() {
                currentPureTone = pureTone
                checkCurrentPureTone()
            },
            "current pure tone"
        )
    }

    function setCurrentTweak(tweak) {
        var oldTweak = currentTweak
        getHistory().add(
            function () {
                currentTweak = oldTweak
                checkCurrentTweak()
            },
            function () {
                currentTweak = tweak
                checkCurrentTweak()
            },
            "current tweak"
        )
    }

    function checkCurrentTweak() {
        tweakValue.text = currentTweak.toFixed(1)
    }

    function checkCurrentPureTone() {
        pureTones.itemAt(currentPureTone).checked=true         
    }

    function setModified(state) {
        var oldModified = modified
        getHistory().add(
            function () {
                modified = oldModified
            },
            function () {
                modified = state
            },
            "modified"
        )
    }

    function temperamentClicked(temperament) {
        getHistory().begin()
        setCurrentTab(tabBar.currentIndex)
        setCurrentTemperament(temperament)
        setCurrentRoot(currentTemperament.root)
        setCurrentPureTone(currentTemperament.pure)
        setCurrentTweak(0.0)
        recalculate(getTuning())
        getHistory().end()
    }

    function rootNoteClicked(note) {
        getHistory().begin()
        setModified(true)
        setCurrentRoot(note)
        setCurrentPureTone(note)
        setCurrentTweak(0.0)
        recalculate(getTuning())
        getHistory().end()
    }

    function pureToneClicked(note) {
        getHistory().begin()
        setModified(true)
        setCurrentPureTone(note)
        setCurrentTweak(0.0)
        recalculate(getTuning())
        getHistory().end()
    }

    function tweaked() {
        getHistory().begin()
        setModified(true)
        setCurrentTweak(parseFloat(tweakValue.text))
        recalculate(getTuning())
        getHistory().end()
    }

    function editingFinishedFor(textField) {
        var oldText = textField.previousText
        var newText = textField.text
        getHistory().begin()
        setModified(true)
        getHistory().add(
            function () {
                textField.text = oldText
            },
            function () {
                textField.text = newText
            },
            "edit ".concat(textField.name)
        )
        getHistory().end()
        textField.previousText = newText
    }

    StyledTabBar {
        id:tabBar
        width: 400               
        height: 40
        x: 30
        y: 10
        
        currentIndex: 0 

        StyledTabButton {
            id: westernTab 
            text: "Western Temperaments" 
        }
        StyledTabButton {
            id: middleEasternTab
            text: "Middle Eastern Temperaments" 
        }
    }    

    Column {
        anchors.left: window.left
        anchors.top: window.top
        anchors.leftMargin:20
        anchors.topMargin: 60
        GroupBox {                 
            width:280
            height: 530
            ButtonGroup { id: tempGroup }   

            StyledListView {
                id: westernListView
                width: parent.width
                height: parent.height-30                      
                visible: westernTab.checked  
                model: westernTemperaments

                delegate: PageTabButton {
                    width: 280
                    height: 30
                    ButtonGroup.group: tempGroup
                    orientation: Qt.Horizontal
                    leftPadding: 5
                    checked: index == 0 ? true : false
                    title: modelData.name
                    onClicked: {
                        temperamentClicked(westernTemperaments[index]);
                    }
                }
            }

            StyledListView {
                id: middleEasternListView
                width: parent.width
                height: parent.height-30                    
                visible: middleEasternTab.checked 
                model: middleEasternTemperaments

                delegate: PageTabButton {
                    width: 280
                    height: 30
                    ButtonGroup.group: tempGroup
                    orientation: Qt.Horizontal
                    leftPadding: 5
                    title: modelData.name
                    onClicked: {
                        temperamentClicked(middleEasternTemperaments[index]);
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.top: parent.top
        anchors.topMargin: 60
        anchors.margins: 20
        x: 330
        GroupBox {
            //title: "Advanced"
            ColumnLayout {
                spacing: 10
                GroupBox {
                    title: "Root Note"
                    GridLayout {
                        columns: 6
                        anchors.margins: 10
                        ButtonGroup { id: rootNoteGroup }
                        FlatRadioButton {
                            text: "C"
                            checked: true
                            ButtonGroup.group: rootNoteGroup
                            id: root_c
                            onClicked: { rootNoteClicked(0) }
                        }
                        FlatRadioButton {
                            text: "G"
                            ButtonGroup.group: rootNoteGroup
                            id: root_g
                            onClicked: { rootNoteClicked(1) }
                        }
                        FlatRadioButton {
                            text: "D"
                            ButtonGroup.group: rootNoteGroup
                            id: root_d
                            onClicked: { rootNoteClicked(2) }
                        }
                        FlatRadioButton {
                            text: "A"
                            ButtonGroup.group: rootNoteGroup
                            id: root_a
                            onClicked: { rootNoteClicked(3) }
                        }
                        FlatRadioButton {
                            text: "E"
                            ButtonGroup.group: rootNoteGroup
                            id: root_e
                            onClicked: { rootNoteClicked(4) }
                        }
                        FlatRadioButton {
                            text: "B"
                            ButtonGroup.group: rootNoteGroup
                            id: root_b
                            onClicked: { rootNoteClicked(5) }
                        }
                        FlatRadioButton {
                            text: "F#"
                            ButtonGroup.group: rootNoteGroup
                            id: root_f_sharp
                            onClicked: { rootNoteClicked(6) }
                        }
                        FlatRadioButton {
                            text: "C#"
                            ButtonGroup.group: rootNoteGroup
                            id: root_c_sharp
                            onClicked: { rootNoteClicked(7) }
                        }
                        FlatRadioButton {
                            text: "G#"
                            ButtonGroup.group: rootNoteGroup
                            id: root_g_sharp
                            onClicked: { rootNoteClicked(8) }
                        }
                        FlatRadioButton {
                            text: "Eb"
                            ButtonGroup.group: rootNoteGroup
                            id: root_e_flat
                            onClicked: { rootNoteClicked(9) }
                        }
                        FlatRadioButton {
                            text: "Bb"
                            ButtonGroup.group: rootNoteGroup
                            id: root_b_flat
                            onClicked: { rootNoteClicked(10) }
                        }
                        FlatRadioButton {
                            text: "F"
                            ButtonGroup.group: rootNoteGroup
                            id: root_f
                            onClicked: { rootNoteClicked(11) }
                        }
                    }
                }

                GroupBox {
                    title: "Pure Tone"
                    GridLayout {
                        columns: 6
                        anchors.margins: 10
                        ButtonGroup { id: pureToneGroup }
                        Repeater {
                            id: pureTones
                            model: ["C", "G", "D", "A", "E", "B", "F#", "C#", "G#", "Eb", "Bb", "F"]
                            delegate: FlatRadioButton{
                                ButtonGroup.group: pureToneGroup
                                text: modelData
                                checked: index==0? true : false
                                onClicked: { pureToneClicked(index) }
                            }
                        }
                    }
                }

                GroupBox {
                    title: "Pure note offset"
                    RowLayout {
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: tweakValue
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "tweak"
                            onEditingFinished: { tweaked() }
                        }
                    }
                }

                GroupBox {
                    title: "Final Offsets"
                    GridLayout {
                        columns: 12
                        anchors.margins: 0

                        StyledTextLabel {
                            text: "C"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_c
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final C"
                            onEditingFinished: { editingFinishedFor(final_c) }
                        }

                        StyledTextLabel {
                            text: "G"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_g
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final G"
                            onEditingFinished: { editingFinishedFor(final_g) }
                        }

                        StyledTextLabel {
                            text: "D"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_d
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final D"
                            onEditingFinished: { editingFinishedFor(final_d) }
                        }

                        StyledTextLabel {
                            text: "A"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_a
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final A"
                            onEditingFinished: { editingFinishedFor(final_a) }
                        }

                        StyledTextLabel {
                            text: "E"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_e
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final E"
                            onEditingFinished: { editingFinishedFor(final_e) }
                        }

                        StyledTextLabel {
                            text: "B"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_b
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final B"
                            onEditingFinished: { editingFinishedFor(final_b) }
                        }

                        StyledTextLabel {
                            text: "F#"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_f_sharp
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final F#"
                            onEditingFinished: { editingFinishedFor(final_f_sharp) }
                        }

                        StyledTextLabel {
                            text: "C#"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_c_sharp
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final C#"
                            onEditingFinished: { editingFinishedFor(final_c_sharp) }
                        }

                        StyledTextLabel {
                            text: "G#"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_g_sharp
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final G#"
                            onEditingFinished: { editingFinishedFor(final_g_sharp) }
                        }

                        StyledTextLabel {
                            text: "Eb"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_e_flat
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final Eb"
                            onEditingFinished: { editingFinishedFor(final_e_flat) }
                        }

                        StyledTextLabel {
                            text: "Bb"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_b_flat
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final Bb"
                            onEditingFinished: { editingFinishedFor(final_b_flat) }
                        }

                        StyledTextLabel {
                            text: "F"
                            Layout.alignment: offsetLabelAlignment
                        }
                        TextField {
                            Layout.maximumWidth: offsetTextWidth
                            id: final_f
                            text: "0.0"
                            readOnly: false
                            validator: DoubleValidator { bottom: -99.9; decimals: 1; notation: DoubleValidator.StandardNotation; top: 99.9 }
                            property var previousText: "0.0"
                            property var name: "final F"
                            onEditingFinished: { editingFinishedFor(final_f) }
                        }
                    }
                }
                RowLayout {
                    FlatButton {
                        id: addButton
                        text: "Add"                                
                        isNarrow: true
                        onClicked: addDialog.open() 
                    }
                    FlatButton {
                        id: removeButton
                        text: "Remove"
                        isNarrow: true                                
                        onClicked: {
                            removeTemperament()
                            saveCustomTemperaments()
                        }
                    }
                    FlatButton {
                        id: undoButton
                        text: qsTranslate("PrefsDialogBase", "Undo")
                        isNarrow: true
                        onClicked: {
                            getHistory().undo()
                        }
                    }
                    FlatButton {
                        id: redoButton
                        text: qsTranslate("PrefsDialogBase", "Redo")
                        isNarrow: true
                        onClicked: {
                            getHistory().redo()
                        }
                    }
                }
            }
        }

        CheckBox {
            id: annotateValue
            text: qsTr("Annotate")
            checked: false
            onClicked: checked = !checked 
        }        
    }

    ButtonBox {
        id: buttonBox                
        anchors.bottom: window.bottom
        anchors.right: window.right
        anchors.margins: 20
        buttons: [ ButtonBoxModel.Cancel, ButtonBoxModel.Ok ]                   

        onStandardButtonClicked: function(buttonId) {
            if (buttonId === ButtonBoxModel.Cancel) {
                if (modified) quitDialog.open()
                else quit()                            
            } 
            else if (buttonId === ButtonBoxModel.Ok) {
                if (applyTemperament()) {
                    if (modified)  quitDialog.open()
                    else quit()                                
                }
            }
        }
    }

    MessageDialog {
        id: errorDialog
        title: "Error"
        text: ""
        onAccepted: {
            errorDialog.close()
        }
    }

    MessageDialog {
        id: quitDialog
        title: "Quit?"
        text: "Do you want to quit the plugin?"
        detailedText: "It looks like you have made customisations to this tuning, you could save them to a file before quitting if you like."
        standardButtons: [StandardButton.Ok, StandardButton.Cancel]
        onAccepted: {
            quit()
        }
        onRejected: {
            quitDialog.close()
        }
    }

    Dialog {        
        id: addDialog 
        title: "Add Temperament"
        anchors.centerIn: parent
            
        contentItem: Column {  
            spacing: 30              
            Row {
                spacing: 10
                StyledTextLabel {
                    text: "Temperament name:"
                }
                TextField {                
                    id: customTempName
                    //focus: true
                }
            }
            FlatButton {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Ok"
                accentButton: true
                onClicked: {                   
                    addTemperament()
                    addDialog.close()
                    saveCustomTemperaments()
                    customTempName.text = ""                                       
                }   
            } 
        }  
    }

    FileIO {
        id: fileIO
        source: Qt.resolvedUrl("tuningPluginData.json").toString().replace("file:///", ""); 
    }   

    function addTemperament() {
        var entry= {
                        "name": customTempName.text,
                        "offsets": [final_c.text, final_g.text, final_d.text, final_a.text, final_e.text, final_b.text, final_f_sharp.text, final_c_sharp.text, final_g_sharp.text, final_e_flat.text, final_b_flat.text, final_f.text ],
                        "root": currentRoot,
                        "pure": currentPureTone                        
                    } 

        switch (tabBar.currentIndex) {
            case 0:
                westernTemperaments = westernTemperaments.concat(entry) //adds new entry and updates buttons 
                westernListView.positionViewAtEnd()
                westernListView.itemAtIndex(westernTemperaments.length-1).checked = true   
                break
            case 1:
                middleEasternTemperaments = middleEasternTemperaments.concat(entry) //adds new entry and updates buttons 
                middleEasternListView.positionViewAtEnd()
                middleEasternListView.itemAtIndex(middleEasternTemperaments.length-1).checked = true
                break
        }                        
    }

    function removeTemperament() {
        switch (tabBar.currentIndex) {
            case 0:
                if (westernTemperaments.indexOf(currentTemperament) < westernTemperamentsLength) {
                    error("Cannot remove Built-in temperaments")
                } 
                else {
                    westernTemperaments = westernTemperaments.filter(x => x !== currentTemperament)
                    westernListView.positionViewAtEnd()
                }
                break

            case 1:
                if (middleEasternTemperaments.indexOf(currentTemperament) < middleEasternTemperamentsLength) {
                    error("Cannot remove Built-in temperaments")
                } 
                else {
                    middleEasternTemperaments = middleEasternTemperaments.filter(x => x !== currentTemperament)
                    middleEasternListView.positionViewAtEnd()
                }
                break
        }
    }

    function saveCustomTemperaments() {
        fileIO.write(
            JSON.stringify(
                [
                    westernTemperaments.slice(westernTemperamentsLength), 
                    middleEasternTemperaments.slice(middleEasternTemperamentsLength)
                ]
            )
        ) 
    }

    // Command pattern for undo/redo
    function commandHistory() {
        function Command(undo_fn, redo_fn, label) {
            this.undo = undo_fn
            this.redo = redo_fn
            this.label = label // for debugging
        }

        var history = []
        var index = -1
        var transaction = 0
        var maxHistory = 30

        function newHistory(commands) {
            if (index < maxHistory) {
                index++
                history = history.slice(0, index)
            } else {
                history = history.slice(1, index)
            }
            history.push(commands)
        }

        this.add = function(undo, redo, label) {
            var command = new Command(undo, redo, label)
            command.redo()
            if (transaction) {
                history[index].push(command)
            } else {
                newHistory([command])
            }
        }

        this.undo = function() {
            if (index != -1) {
                history[index].slice().forEach(
                    function(command) {
                        command.undo()
                    }
                )
                index--
            }
        }

        this.redo = function() {
            if ((index + 1) < history.length) {
                index++
                history[index].forEach(
                    function(command) {
                        command.redo()
                    }
                )
            }
        }

        this.begin = function() {
            if (transaction) {
                throw new Error("already in transaction")
            }
            newHistory([])
            transaction = 1
        }

        this.end = function() {
            if (!transaction) {
                throw new Error("not in transaction")
            }
            transaction = 0
        }
    }
}
// vim: ft=javascript
