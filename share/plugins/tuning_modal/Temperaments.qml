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

import MuseScore 3.0
import FileIO 3.0

MuseScore {
    version: "3.3.2"
    title: "Modal Temperaments"
    description: "Apply various temperaments and tunings"
    pluginType: "dialog"
    categoryCode: "playback"
    thumbnailName: "modal_tuning.png"

    width: 900
    height: 740

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
    property var equal: {
        'offsets': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        'root': 0,
        'pure': 0,
        'name': "equal"
    }
        property var tuning01: {
        'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -6.8, 30.8, 27.4, 24.0, 20.5, 17.1, 13.7],
        'root': 0,
        'pure': 3,
        'name': "tuning01"
    }
        property var tuning02: {
        'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -6.8, -10.3, -13.7, -17.1, 20.5, 17.1, 13.7],
        'root': 0,
        'pure': 3,
        'name': "tuning02"
    }
        property var tuning03: {
        'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -6.8, -10.3, -13.7, -17.1, -20.5, -24.0, 13.7],
        'root': 0,
        'pure': 3,
        'name': "tuning03"
    }
        property var tuning04: {
        'offsets': [7.0, 4.7, 2.3, 0.0, -2.3, -4.7, -7.0, -9.4, -11.7, 14.1, 11.7, 9.4],
        'root': 0,
        'pure': 3,
        'name': "tuning04"
    }
        property var tuning05: {
        'offsets': [4.9, 3.3, 1.6, 0.0, -1.6, -3.3, -4.9, -6.5, -8.1, 9.8, 8.1, 6.5],
        'root': 0,
        'pure': 3,
        'name': "tuning05"
    }
        property var tuning06: {
        'offsets': [11.7, 7.8, 3.9, 0.0, 2.0, 3.9, 0.0, 2.0, 3.9, 5.9, 7.8, 9.8],
        'root': 0,
        'pure': 3,
        'name': "tuning06"
    }
        property var tuning07: {
        'offsets': [10.3, 6.8, 3.4, 0.0, -3.4, -1.5, 0.5, 0.5, 2.4, 4.4, 6.4, 8.3],
        'root': 0,
        'pure': 3,
        'name': "tuning07"
    }
        property var tuning08: {
        'offsets': [5.9, 3.9, 2.0, 0.0, -2.0, -3.9, -2.0, 0.0, 2.0, 3.9, 5.9, 7.8],
        'root': 0,
        'pure': 3,
        'name': "tuning08"
    }
        property var tuning09: {
        'offsets': [6.2, 4.2, 2.1, 0.0, -2.1, -1.9, -1.8, 0.1, 2.1, 4.1, -0.3, -0.1],
        'root': 0,
        'pure': 3,
        'name': "tuning09"
    }
        property var tuning10: {
        'offsets': [8.2, 5.5, 2.7, 0.0, -2.7, -0.8, -3.5, -1.6, 0.4, 2.3, 4.3, 6.3],
        'root': 0,
        'pure': 3,
        'name': "tuning10"
    }
    property var tuning11: {
        'offsets': [6.6, 3.9, 1.7, 0.0, -1.2, -1.8, -2.0, -1.6, -0.7, 0.8, 2.7, 4.7],
        'root': 0,
        'pure': 0,
        'name': "tuning11"
    }
    property var tuning12: {
        'offsets': [0.0, 2.0, -17.6, -15.6, -13.7, -11.7, 31.3, 11.7, 13.7, 15.6, 17.6, -2.0],
        'root': 0,
        'pure': 0,
        'name': "tuning12"
    }
    property var tuning13: {
        'offsets': [0.0, 2.0, -17.6, -15.6, -13.7, -11.7, -31.3, -29.3, -27.4, 15.6, -3.9, -2.0],
        'root': 0,
        'pure': 0,
        'name': "tuning13"
    }
    property var tuning14: {
        'offsets': [0.0, 2.0, -17.6, -15.6, -13.7, -11.7, -31.3, 11.7, 13.7, 15.6, -3.9, -2.0],
        'root': 0,
        'pure': 0,
        'name': "tuning14"
    }
    property var tuning15: {
        'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -11.7, -9.8, 11.7, 13.7, 15.6, 17.6, -2.0],
        'root': 0,
        'pure': 0,
        'name': "tuning15"
    }
    property var tuning16: {
        'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, 13.7, 15.6, 17.6, 19.6],
        'root': 0,
        'pure': 0,
        'name': "tuning16"
    }
    property var tuning17: {
        'offsets': [0.0, 2.0, 3.9, 5.9, -13.7, -11.7, -9.8, -7.8, -27.4, -25.4, -23.5, -21.5],
        'root': 0,
        'pure': 0,
        'name': "tuning17"
    }
    property var tuning18: {
        'offsets': [0.0, 2.0, 31.2, -15.6, -13.7, -28.3, -17.5, 38.6, 13.7, 15.6, -31.2, -2.0],
        'root': 0,
        'pure': 0,
        'name': "tuning18"
    }
    property var tuning19: {
        'offsets': [0.0, 2.0, 3.9, -15.6, -13.7, -11.7, 17.5, 28.3, 13.7, 15.6, 17.6, -2.0],
        'root': 0,
        'pure': 0,
        'name': "tuning19"
    }
    property var tuning20: {
        'offsets': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        'root': 0,
        'pure': 0,
        'name': "tuning20"
    }
    property var tuning21: {
        'offsets': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        'root': 0,
        'pure': 0,
        'name': "tuning21"
    }
    property var tuning22: {
        'offsets': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        'root': 0,
        'pure': 0,
        'name': "tuning22"
    }
    property var tuning23: {
        'offsets': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        'root': 0,
        'pure': 0,
        'name': "tuning23"
    }
    property var tuning24: {
        'offsets': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        'root': 0,
        'pure': 0,
        'name': "tuning24"
    }

    property var currentTemperament: equal;
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

    function applyTemperament()
    {
        var selection = new scoreSelection()
        curScore.startCmd()
        selection.map(filterNotes, reTune(getFinalTuning()))
        if (annotateValue.checkedState == Qt.Checked) {
            selection.map(filterNotes, annotate)
        }
        curScore.endCmd()
        return true
    }

    function filterNotes(element)
    {
        return element.type == Element.CHORD
    }

    function annotate(chord, cursor)
    {
        for (var i = 0; i < chord.notes.length; i++) {
            var note = chord.notes[i]
            var text = newElement(Element.STAFF_TEXT);
            text.text = '' + note.tuning
            text.autoplace = true
            text.fontSize = 7 // smaller
            if (cursor.voice == 0 || cursor.voice == 2) {
                text.placement = Placement.ABOVE
            } else {
                text.placement = Placement.BELOW
            }
            cursor.add(text)
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
        switch (currentTemperament.name) {
            case "equal":
                equal_button.checked = true
                return
            case "tuning01":
                tuning01_button.checked = true
                return
            case "tuning02":
                tuning02_button.checked = true
                return
            case "tuning03":
                tuning03_button.checked = true
                return
            case "tuning04":
                tuning04_button.checked = true
                return
            case "tuning05":
                tuning05_button.checked = true
                return
            case "tuning06":
                tuning06_button.checked = true
                return
            case "tuning07":
                tuning07_button.checked = true
                return
            case "tuning08":
                tuning08_button.checked = true
                return
            case "tuning09":
                tuning09_button.checked = true
                return
            case "tuning10":
                tuning10_button.checked = true
                return
            case "tuning11":
                tuning11_button.checked = true
                return
            case "tuning12":
                tuning12_button.checked = true
                return
            case "tuning13":
                tuning13_button.checked = true
                return
            case "tuning14":
                tuning14_button.checked = true
                return
            case "tuning15":
                tuning15_button.checked = true
                return
            case "tuning16":
                tuning16_button.checked = true
                return
            case "tuning17":
                tuning17_button.checked = true
                return
            case "tuning18":
                tuning18_button.checked = true
                return
            case "tuning19":
                tuning19_button.checked = true
                return
            case "tuning20":
                tuning20_button.checked = true
                return
            case "tuning21":
                tuning21_button.checked = true
                return
            case "tuning22":
                tuning22_button.checked = true
                return
            case "tuning23":
                tuning23_button.checked = true
                return
            case "tuning24":
                tuning24_button.checked = true
                return

        }
    }

    function lookupTemperament(temperamentName) {
        switch (temperamentName) {
            case "equal":
                return equal
            case "tuning01":
                return tuning01
            case "tuning02":
                return tuning02
            case "tuning03":
                return tuning03
            case "tuning04":
                return tuning04
            case "tuning05":
                return tuning05
            case "tuning06":
                return tuning06
            case "tuning07":
                return tuning07
            case "tuning08":
                return tuning08
            case "tuning09":
                return tuning09
            case "tuning10":
                return tuning10
            case "tuning11":
                return tuning11
            case "tuning12":
                return tuning12
            case "tuning13":
                return tuning13
            case "tuning14":
                return tuning14
            case "tuning15":
                return tuning15
            case "tuning16":
                return tuning16
            case "tuning17":
                return tuning17
            case "tuning18":
                return tuning18
            case "tuning19":
                return tuning19
            case "tuning20":
                return tuning20
            case "tuning21":
                return tuning21
            case "tuning22":
                return tuning22
            case "tuning23":
                return tuning23
            case "tuning24":
                return tuning24
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
        switch (currentPureTone) {
            case 0:
                pure_c.checked = true
                break
            case 1:
                pure_g.checked = true
                break
            case 2:
                pure_d.checked = true
                break
            case 3:
                pure_a.checked = true
                break
            case 4:
                pure_e.checked = true
                break
            case 5:
                pure_b.checked = true
                break
            case 6:
                pure_f_sharp.checked = true
                break
            case 7:
                pure_c_sharp.checked = true
                break
            case 8:
                pure_g_sharp.checked = true
                break
            case 9:
                pure_e_flat.checked = true
                break
            case 10:
                pure_b_flat.checked = true
                break
            case 11:
                pure_f.checked = true
                break
        }
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

    Item {
        anchors.fill: parent

        ButtonGroup { id: temperamentTypeGroup }

        component TuningItem: RadioButton {
            padding: 2
            ButtonGroup.group: temperamentTypeGroup
        }

        GridLayout {
            columns: 2
            anchors.fill: parent
            anchors.margins: 10
            GroupBox {
                title: "Temperament"
                ColumnLayout {
                    TuningItem {
                        id: equal_button
                        text: "Equal"
                        checked: true
                        onClicked: { temperamentClicked(equal) }
                    }
                    TuningItem {
                        id: tuning01_button
                        text: "Meantone (1/4) 5 flats"
                        onClicked: { temperamentClicked(tuning01) }
                    }
                    TuningItem {
                        id: tuning02_button
                        text: "Meantone 1/4-comma"
                        onClicked: { temperamentClicked(tuning02) }
                    }
                    TuningItem {
                        id: tuning03_button
                        text: "Meantone (1/4) 5 sharps"
                        onClicked: { temperamentClicked(tuning03) }
                    }
                    TuningItem {
                        id: tuning04_button
                        text: "Meantone 1/5-comma"
                        onClicked: { temperamentClicked(tuning04) }
                    }
                    TuningItem {
                        id: tuning05_button
                        text: "Meantone 1/6-comma"
                        onClicked: { temperamentClicked(tuning05) }
                    }
                    TuningItem {
                        id: tuning06_button
                        text: "Werckmeister III"
                        onClicked: { temperamentClicked(tuning06) }
                    }
                    TuningItem {
                        id: tuning07_button
                        text: "Kirnberger III"
                        onClicked: { temperamentClicked(tuning07) }
                    }
                    TuningItem {
                        id: tuning08_button
                        text: "Vallotti"
                        onClicked: { temperamentClicked(tuning08) }
                    }
                    TuningItem {
                        id: tuning09_button
                        text: "Young I"
                        onClicked: { temperamentClicked(tuning09) }
                    }
                    TuningItem {
                        id: tuning10_button
                        text: "Kellner"
                        onClicked: { temperamentClicked(tuning10) }
                    }
                    TuningItem {
                        id: tuning11_button
                        text: "Fernando A. Martin 1/45-comma"
                        onClicked: { temperamentClicked(tuning11) }
                    }
                    TuningItem {
                        id: tuning12_button
                        text: "C Cm Db Dm Eb Ebm Em F Fm Ab Am Bb"
                        onClicked: { temperamentClicked(tuning12) }
                    }
                    TuningItem {
                        id: tuning13_button
                        text: "C Cm C#m D Dm E Em F F#m A Am Bb"
                        onClicked: { temperamentClicked(tuning13) }
                    }
                    TuningItem {
                        id: tuning14_button
                        text: "C Cm Db D Dm Em F Fm Ab Am Bb Bbm"
                        onClicked: { temperamentClicked(tuning14) }
                    }
                    TuningItem {
                        id: tuning15_button
                        text: "C Cm Db Eb Em F Fm G Gm Ab Am Bm"
                        onClicked: { temperamentClicked(tuning15) }
                    }
                    TuningItem {
                        id: tuning16_button
                        text: "C Cm D Dm Eb Em F#m G Gm Ab Bb Bm"
                        onClicked: { temperamentClicked(tuning16) }
                    }
                    TuningItem {
                        id: tuning17_button
                        text: "C D Ebm E Em F# F#m G G#m Bbm B Bm"
                        onClicked: { temperamentClicked(tuning17) }
                    }
                    TuningItem {
                        id: tuning18_button
                        text: "Simple Ratios"
                        onClicked: { temperamentClicked(tuning18) }
                    }
                    TuningItem {
                        id: tuning19_button
                        text: "Alternate Ratios"
                        onClicked: { temperamentClicked(tuning19) }
                    }
                    TuningItem {
                        id: tuning20_button
                        text: "Empty"
                        onClicked: { temperamentClicked(tuning20) }
                    }
                    TuningItem {
                        id: tuning21_button
                        text: "Empty"
                        onClicked: { temperamentClicked(tuning21) }
                    }
                    TuningItem {
                        id: tuning22_button
                        text: "Empty"
                        onClicked: { temperamentClicked(tuning22) }
                    }
                    TuningItem {
                        id: tuning23_button
                        text: "Empty"
                        onClicked: { temperamentClicked(tuning23) }
                    }
                    TuningItem {
                        id: tuning24_button
                        text: "Empty"
                        onClicked: { temperamentClicked(tuning24) }

                    }
                }
            }

            ColumnLayout {
                GroupBox {
                    title: "Advanced"
                    ColumnLayout {
                        GroupBox {
                            title: "Root Note"
                            GridLayout {
                                columns: 4
                                anchors.margins: 10
                                ButtonGroup { id: rootNoteGroup }
                                RadioButton {
                                    text: "C"
                                    checked: true
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_c
                                    onClicked: { rootNoteClicked(0) }
                                }
                                RadioButton {
                                    text: "G"
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_g
                                    onClicked: { rootNoteClicked(1) }
                                }
                                RadioButton {
                                    text: "D"
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_d
                                    onClicked: { rootNoteClicked(2) }
                                }
                                RadioButton {
                                    text: "A"
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_a
                                    onClicked: { rootNoteClicked(3) }
                                }
                                RadioButton {
                                    text: "E"
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_e
                                    onClicked: { rootNoteClicked(4) }
                                }
                                RadioButton {
                                    text: "B"
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_b
                                    onClicked: { rootNoteClicked(5) }
                                }
                                RadioButton {
                                    text: "F#"
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_f_sharp
                                    onClicked: { rootNoteClicked(6) }
                                }
                                RadioButton {
                                    text: "C#"
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_c_sharp
                                    onClicked: { rootNoteClicked(7) }
                                }
                                RadioButton {
                                    text: "G#"
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_g_sharp
                                    onClicked: { rootNoteClicked(8) }
                                }
                                RadioButton {
                                    text: "Eb"
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_e_flat
                                    onClicked: { rootNoteClicked(9) }
                                }
                                RadioButton {
                                    text: "Bb"
                                    ButtonGroup.group: rootNoteGroup
                                    id: root_b_flat
                                    onClicked: { rootNoteClicked(10) }
                                }
                                RadioButton {
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
                                columns: 4
                                anchors.margins: 10
                                ButtonGroup { id: pureToneGroup }
                                RadioButton {
                                    text: "C"
                                    checked: true
                                    id: pure_c
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(0) }
                                }
                                RadioButton {
                                    text: "G"
                                    id: pure_g
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(1) }
                                }
                                RadioButton {
                                    text: "D"
                                    id: pure_d
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(2) }
                                }
                                RadioButton {
                                    text: "A"
                                    id: pure_a
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(3) }
                                }
                                RadioButton {
                                    text: "E"
                                    id: pure_e
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(4) }
                                }
                                RadioButton {
                                    text: "B"
                                    id: pure_b
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(5) }
                                }
                                RadioButton {
                                    text: "F#"
                                    id: pure_f_sharp
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(6) }
                                }
                                RadioButton {
                                    text: "C#"
                                    id: pure_c_sharp
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(7) }
                                }
                                RadioButton {
                                    text: "G#"
                                    id: pure_g_sharp
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(8) }
                                }
                                RadioButton {
                                    text: "Eb"
                                    id: pure_e_flat
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(9) }
                                }
                                RadioButton {
                                    text: "Bb"
                                    id: pure_b_flat
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(10) }
                                }
                                RadioButton {
                                    text: "F"
                                    id: pure_f
                                    ButtonGroup.group: pureToneGroup
                                    onClicked: { pureToneClicked(11) }
                                }
                            }
                        }

                        GroupBox {
                            title: "Tweak"
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
                                columns: 8
                                anchors.margins: 0

                                Label {
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

                                Label {
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

                                Label {
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

                                Label {
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

                                Label {
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

                                Label {
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

                                Label {
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

                                Label {
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

                                Label {
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

                                Label {
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

                                Label {
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

                                Label {
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
                            Button {
                                id: saveButton
                                text: qsTranslate("PrefsDialogBase", "Save")
                                onClicked: {
                                    // declaring this directly in the saveDialog's properties doesn't seem to work
                                    saveDialog.folder = filePath
                                    saveDialog.visible = true
                                }
                            }
                            Button {
                                id: loadButton
                                text: qsTranslate("PrefsDialogBase", "Load")
                                onClicked: {
                                    loadDialog.folder = filePath
                                    loadDialog.visible = true
                                }
                            }
                            Button {
                                id: undoButton
                                text: qsTranslate("PrefsDialogBase", "Undo")
                                onClicked: {
                                    getHistory().undo()
                                }
                            }
                            Button {
                                id: redoButton
                                text: qsTranslate("PrefsDialogBase", "Redo")
                                onClicked: {
                                    getHistory().redo()
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Button {
                        id: applyButton
                        text: qsTranslate("PrefsDialogBase", "Apply")
                        onClicked: {
                            if (applyTemperament()) {
                                if (modified) {
                                    quitDialog.open()
                                } else {
                                    quit()
                                }
                            }
                        }
                    }
                    Button {
                        id: cancelButton
                        text: qsTranslate("PrefsDialogBase", "Cancel")
                        onClicked: {
                            if (modified) {
                                quitDialog.open()
                            } else {
                                quit()
                            }
                        }
                    }
                    CheckBox {
                        id: annotateValue
                        text: qsTr("Annotate")
                        checked: false
                    }
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

    FileIO {
        id: saveFile
        source: ""
    }

    FileIO {
        id: loadFile
        source: ""
    }

    function getFile(dialog) {
        var source = dialog.filePath
        return source
    }

    function formatCurrentValues() {
        var data = {
            offsets: [
                parseFloat(final_c.text),
                parseFloat(final_c_sharp.text),
                parseFloat(final_d.text),
                parseFloat(final_e_flat.text),
                parseFloat(final_e.text),
                parseFloat(final_f.text),
                parseFloat(final_f_sharp.text),
                parseFloat(final_g.text),
                parseFloat(final_g_sharp.text),
                parseFloat(final_a.text),
                parseFloat(final_b_flat.text),
                parseFloat(final_b.text)
            ],
            temperament: currentTemperament.name,
            root: currentRoot,
            pure: currentPureTone,
            tweak: currentTweak
        };
        return(JSON.stringify(data))
    }

    function restoreSavedValues(data) {
        getHistory().begin()
        setCurrentTemperament(lookupTemperament(data.temperament))
        setCurrentRoot(data.root)
        setCurrentPureTone(data.pure)
        // support older save files
        if (data.hasOwnProperty('tweak')) {
            setCurrentTweak(data.tweak)
        } else {
            setCurrentTweak(0.0)
        }
        recalculate(
            function(pitch) {
                return data.offsets[pitch % 12]
            }
        )
        getHistory().end()
    }

    FileDialog {
        id: loadDialog
        type: FileDialog.Load
        title: "Please choose a file"
        //sidebarVisible: true
        onAccepted: {
            loadFile.source = getFile(loadDialog)
            var data = JSON.parse(loadFile.read())
            restoreSavedValues(data)
            loadDialog.visible = false
        }
        onRejected: {
            loadDialog.visible = false
        }
        visible: false
    }

    FileDialog {
        id: saveDialog
        type: FileDialog.Save
        title: "Please name a file"
        //sidebarVisible: true
        //selectExisting: false
        onAccepted: {
            saveFile.source = getFile(saveDialog)
            saveFile.write(formatCurrentValues())
            saveDialog.visible = false
        }
        onRejected: {
            saveDialog.visible = false
        }
        visible: false
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
                history[index].slice().reverse().forEach(
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
