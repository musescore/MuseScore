//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Note Names Plugin
//
//  Copyright (C) 2012 Werner Schweer
//  Copyright (C) 2013 - 2021 Joachim Schmitz
//  Copyright (C) 2014 Jörn Eichler
//  Copyright (C) 2020 Johan Temmerman
//  Copyright (C) 2025 XiaoMigros
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

import QtQuick 2.2
import MuseScore 3.0

MuseScore {
   version: "4.7"
   description: "This plugin names notes as per your language setting"
   title: "Note Names"
   categoryCode: "composing-arranging-tools"
   thumbnailName: "note_names.png"

   function nameNote(note) {
        switch (note.tpc) {
        case -8: return qsTranslate("global", "F♭♭♭")
        case -7: return qsTranslate("global", "C♭♭♭")
        case -6: return qsTranslate("global", "G♭♭♭")
        case -5: return qsTranslate("global", "D♭♭♭")
        case -4: return qsTranslate("global", "A♭♭♭")
        case -3: return qsTranslate("global", "E♭♭♭")
        case -2: return qsTranslate("global", "B♭♭♭")

        case -1: return qsTranslate("global", "F♭♭")
        case  0: return qsTranslate("global", "C♭♭")
        case  1: return qsTranslate("global", "G♭♭")
        case  2: return qsTranslate("global", "D♭♭")
        case  3: return qsTranslate("global", "A♭♭")
        case  4: return qsTranslate("global", "E♭♭")
        case  5: return qsTranslate("global", "B♭♭")

        case  6: return qsTranslate("global", "F♭")
        case  7: return qsTranslate("global", "C♭")
        case  8: return qsTranslate("global", "G♭")
        case  9: return qsTranslate("global", "D♭")
        case 10: return qsTranslate("global", "A♭")
        case 11: return qsTranslate("global", "E♭")
        case 12: return qsTranslate("global", "B♭")

        case 13: return qsTranslate("global", "F")
        case 14: return qsTranslate("global", "C")
        case 15: return qsTranslate("global", "G")
        case 16: return qsTranslate("global", "D")
        case 17: return qsTranslate("global", "A")
        case 18: return qsTranslate("global", "E")
        case 19: return qsTranslate("global", "B")

        case 20: return qsTranslate("global", "F♯")
        case 21: return qsTranslate("global", "C♯")
        case 22: return qsTranslate("global", "G♯")
        case 23: return qsTranslate("global", "D♯")
        case 24: return qsTranslate("global", "A♯")
        case 25: return qsTranslate("global", "E♯")
        case 26: return qsTranslate("global", "B♯")

        case 27: return qsTranslate("global", "F♯♯")
        case 28: return qsTranslate("global", "C♯♯")
        case 29: return qsTranslate("global", "G♯♯")
        case 30: return qsTranslate("global", "D♯♯")
        case 31: return qsTranslate("global", "A♯♯")
        case 32: return qsTranslate("global", "E♯♯")
        case 33: return qsTranslate("global", "B♯♯")

        case 34: return qsTranslate("global", "F♯♯")
        case 35: return qsTranslate("global", "C♯♯♯")
        case 36: return qsTranslate("global", "G♯♯♯")
        case 37: return qsTranslate("global", "D♯♯♯")
        case 38: return qsTranslate("global", "A♯♯♯")
        case 39: return qsTranslate("global", "E♯♯♯")
        case 40: return qsTranslate("global", "B♯♯♯")

        default: return qsTr("?")
        }
    }

    onRun: {
        if (curScore.selection.elements.length == 0) {
            curScore.startCmd("Add note names to score")
            cmd("select-all")
        } else {
            curScore.startCmd("Add note names selection")
        }

        // Get list of chords (saves calculations)
        var chordList = []
        for (var el of curScore.selection.elements) {
            if (el.type == Element.NOTE && el.parent.type == Element.CHORD) {
                var add = true
                for (var chord of chordList) {
                    if (chord.is(el.parent)) {
                        add = false
                        break
                    }
                }
                if (add) {
                    chordList.push(el.parent)
                }
            }
        }

        for (var chord of chordList) {
            var vstaff = curScore.staves[chord.vStaffIdx]
            var lines = vstaff.lines(chord.fraction)
            var above = (chord.voice % 2 == 0)
            var lineScale = vstaff.lineDistance(chord.fraction) * vstaff.staffMag(chord.fraction)
            var topline = lineScale * Math.min((chord.bbox.y / lineScale) - 1, -1.5)
            var bottomline = lineScale * (Math.max((chord.bbox.y + chord.bbox.height) / lineScale + 1, lines + 0.5) - 4)
            var tabStaff = vstaff.isTabStaff(chord.fraction)

            for (var i in chord.notes) {
                var note = chord.notes[i]
                // Attempt to remove old texts
                for (var element of note.elements) {
                    if (element.type == Element.TEXT && /^[A-H](♭|♯)*$/.test(element.text)) {
                        note.remove(element)
                    }
                }
                // list selections / selection filter could be individual notes
                if (!note.selected || note.tieBack) {
                    continue
                }
                // Then apply new names
                var text = newElement(Element.TEXT) // This adds the text to the note: Better for grace notes and easier to remove
                text.text = nameNote(note)
                text.autoplace = false
                text.visible = note.visble
                if (note.noteType != NoteType.NORMAL) {
                    text.fontSize *= curScore.style.value("graceNoteMag")
                }
                if (above) {
                    text.placement = Placement.ABOVE
                    text.align = Align.BASELINE
                    text.offset.y = (topline + i * -2)
                } else {
                    text.placement = Placement.BELOW
                    text.align = Align.TOP
                    text.offset.y = (bottomline + (chord.notes.length - 1 - i) * 2)
                }
                text.offset.y -= (tabStaff ? note.string : 0.5 * note.line) * lineScale
                note.add(text)
            }
        }
        curScore.endCmd()
        quit()
    }
}
