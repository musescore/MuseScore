//===========================================================================
// Retrograde
//
// Copyright (C) 2025 XiaoMigros
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 3
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE
//===========================================================================

import QtQuick 2.0
import MuseScore 3.0

MuseScore {
    title: qsTr("Retrograde")
    description: qsTr("Takes a selection of notes and reverses them.")
    version: "1.0"
    categoryCode: "composing-arranging-tools"
    thumbnailName: "retrograde.png"

    property var selection: false
    property var allTies: []
    property var globalStartTick: fraction(0, 1)
    property var globalEndTick: fraction(0, 1)

    function retrogradeSort(a, b) {
        return b.track == a.track ? b.startTick.ticks - a.startTick.ticks : b.track - a.track;
    }

    function retrogradedTick(fraction) {
        return globalStartTick.plus(globalEndTick.minus(fraction))
    }

    function retrogradeSelection() {
        globalStartTick = curScore.lastMeasure.tick.plus(curScore.lastMeasure.ticks)
        var readableElements = []
        var parsedElements = []
        for (var i in curScore.selection.elements) {
            var el = getRetrogradeElement(curScore.selection.elements[i], parsedElements)
            if (!el) {
                continue
            }

            var durationObject = el.type == Element.TUPLET ? getTupletObj(el) : getChordRestObj(el)
            parsedElements.push(el)
            readableElements.push(durationObject)
            const objEndTick = durationObject.startTick.plus(durationObject.actualDuration)
            if (objEndTick.greaterThan(globalEndTick)) {
                globalEndTick = objEndTick
            }
            if (durationObject.startTick.lessThan(globalStartTick)) {
                globalStartTick = durationObject.startTick
            }
        }
        var cursor = curScore.newCursor()

        // Remove existing elements (as they may not be overwritten depending on the voice situation)
        for (var i in parsedElements) {
            if (!parsedElements[i]) {
                continue
            }
            const range = [parsedElements[i].track, parsedElements[i].fraction, parsedElements[i].actualDuration]
            removeElement(parsedElements[i])
            cursor.track = range[0]
            cursor.rewindToFraction(range[1])
            if (cursor.element) {
                do {
                    removeElement(cursor.element)
                } while (cursor.next() && cursor.fraction.lessThan(range[1].plus(range[2])))
            }
        }

        // We need to add CRs inthe correct order, so we always have valid cursor positions
        readableElements.sort(retrogradeSort)

        for (var i in readableElements) {
            var el = readableElements[i]
            cursor.track = el.track
            cursor.rewindToFraction(retrogradedTick(el.startTick.plus(el.actualDuration)))
            if (cursor.element && cursor.element.tuplet) {
                removeElement(cursor.element.topTuplet)
            }
            if (el.type == Element.TUPLET) {
                addTupletObj(el, cursor)
            } else {
                addChordRestObj(el, cursor)
            }
        }
        addTies(allTies)
    }

    // Find usable element (non-grace chord/rest or outermost tuplet)
    function getRetrogradeElement(element, parsedElements) {
        var el = element
        switch (el.type) {
            case Element.NOTE:
                el = el.parent
                // fall through
            case Element.CHORD:
                el = el.noteType == NoteType.NORMAL ? el : el.parent
                // fall through
            case Element.REST:
            case Element.TUPLET:
                el = el.topTuplet ? el.topTuplet : el
                for (var i in parsedElements) {
                    if (parsedElements[i].is(el)) {
                        return false
                    }
                }
                return el
            default: return false
        }
    }

    // Creates a readable object from a chord/rest
    function getChordRestObj(element) {
        getTies(element)
        return {
            duration: element.duration,
            actualDuration: element.actualDuration,
            notes: getNotes(element),
            startTick: element.fraction,
            track: element.track,
            type: element.type,
            annotations: getAnnotations(element),
            articulations: getArticulations(element),
            graceNotes: getGraceNotes(element),
            beamMode: element.beamMode,
            offsetY: element.type == Element.REST ? element.offsetY : false,
            visible: element.type == Element.REST ? element.visible : false,
            gap: element.type == Element.REST ? element.gap : false
        }
    }

    // Creates a copy of notes used within a chord
    function getNotes(element) {
        var notes = []
        if (element.type == Element.REST) return notes
        for (var i in element.notes) {
            notes[i] = element.notes[i].clone()
        }
        return notes
    }

    // retrieves the annotations (dynamics, tempo text, etc) of a non-grace chord/rest
    function getAnnotations(element) {
        var annoList = []
        var removeList = []
        for (var i in element.parent.annotations) {
            var el = element.parent.annotations[i]
            if (el.track == element.track) {
                annoList.push(el.clone())
                removeList.push(el)
            }
        }
        for (var i in removeList) {
            removeElement(removeList[i])
        }
        return annoList
    }

    // Retrieves a chord's articulations
    function getArticulations(element) {
        var artiList = []
        if (element.type == Element.REST) return artiList
        for (var i in element.articulations) {
            artiList.push(element.articulations[i].clone())
        }
        return artiList
    }

    // Retrieves a chord's grace notes
    function getGraceNotes(element) {
        if (element.type == Element.REST || !element.graceNotes.length) {
            return []
        }
        var graceList = []
        for (var i in element.graceNotes) {
            var graceChord = element.graceNotes[0]
            graceList.push({
                duration: graceChord.duration,
                notes: getNotes(graceChord),
                type: getGraceNoteType(graceChord)
            })
            removeElement(graceChord)
        }
        return graceList
    }

    // Retrieves the type of grace note, formatted for the later add command
    // doesn't work with a switch statement
    function getGraceNoteType(graceChord) {
        var type = graceChord.notes[0].noteType
        if (type == NoteType.ACCIACCATURA)  return "acciaccatura"
        if (type == NoteType.APPOGGIATURA)  return "appoggiatura"
        if (type == NoteType.GRACE4)        return "grace4"
        if (type == NoteType.GRACE16)       return "grace16"
        if (type == NoteType.GRACE32)       return "grace32"
        if (type == NoteType.GRACE8_AFTER)  return "grace8after"
        if (type == NoteType.GRACE16_AFTER) return "grace16after"
        if (type == NoteType.GRACE32_AFTER) return "grace32after"
        return "invalid"
    }

    // Retrieves a list of notes with ties in a chordrest
    function getTies(element) {
        if (element.type == Element.REST) return
        for (var i in element.notes) {
            if (element.notes[i].tieBack) {
                allTies.push({
                    startTick: element.fraction,
                    track: element.track,
                    note: i
                })
            }
        }
    }

    // Creates a readable object from a tuplet
    function getTupletObj(tuplet) {
        return {
            duration: tuplet.duration,
            actualDuration: tuplet.actualDuration,
            type: tuplet.type,
            startTick: tuplet.fraction,
            track: tuplet.track,
            elements: getTupletElements(tuplet),
            ratio: fraction(tuplet.actualNotes, tuplet.normalNotes),
            bracketType: tuplet.bracketType,
            numberType: tuplet.numberType,
            visible: tuplet.visible
        }
    }

    // Returns the chords, rests and child tuplets within a tuplet
    function getTupletElements(tuplet) {
        var elementsArray = []
        for (var i in tuplet.elements) {
            if (tuplet.elements[i].type == Element.TUPLET) {
                elementsArray.push(getTupletObj(tuplet.elements[i]))
            } else {
                elementsArray.push(getChordRestObj(tuplet.elements[i]))
            }
        }
        elementsArray.sort(retrogradeSort)
        return elementsArray
    }

    function addChordRestObj(cr, c) {
        var t = c.fraction
        if (c.element) {
            // Not necessarily invalid position, could be v2
            c.setDuration(c.element.duration.numerator, c.element.duration.denominator)
            c.addRest()
            c.rewindToFraction(t)
        }
        c.setDuration(cr.duration.numerator, cr.duration.denominator)
        if (cr.type == Element.REST) {
            c.addRest()
            c.rewindToFraction(t)
            // Check for full measure rest
            if (c.element.duration.equals(c.measure.timesigActual)) {
                curScore.selection.select(c.element, false)
                cmd("full-measure-rest")
            }
            c.rewindToFraction(t)
            c.element.offsetY = cr.offsetY
            c.element.visible = cr.visible
            c.element.gap = cr.gap
        } else {
            c.addNote(cr.notes[0].pitch)
            c.rewindToFraction(t)
            var n = c.element.notes[0]
            for (var i in cr.notes) {
                // Remove trailing spanners, then add
                if (cr.notes[i].tieBack) {
                    removeElement(cr.notes[i].tieBack)
                }
                if (cr.notes[i].tieForward) {
                    removeElement(cr.notes[i].tieForward)
                }
                for (var j in cr.notes[i].spannerForward) {
                    removeElement(cr.notes[i].spannerForward[j])
                }
                for (var j in cr.notes[i].spannerBack) {
                    removeElement(cr.notes[i].spannerBack[j])
                }
                c.element.add(cr.notes[i])
            }
            removeElement(n)
            // If note newly crosses measure, we can't rely on duration set by cursor.
            if (!c.element.duration.equals(cr.duration)) {
                c.element.duration = cr.duration
            }

            c.rewindToFraction(t)
            addArticulations(c, cr.articulations)
            // To do: separate front and back grace notes
            addGraceNotes(c.element.notes[0], cr.graceNotes)
        }
        c.element.beamMode = cr.beamMode
        addAnnotations(c, cr.annotations)
    }

    function addTupletObj(tuplet, c) {
        var t = c.fraction
        if (c.element) {
            // Not necessarily invalid position, could be v2
            c.setDuration(c.element.duration.numerator, c.element.duration.denominator)
            c.addRest()
            c.rewindToFraction(t)
        }
        c.addTuplet(tuplet.ratio, tuplet.duration)
        c.rewindToFraction(t)
        if (!c.element.tuplet) {
            throw new Error(qsTr("Unable to add tuplet, possibly overlaps measure boundaries"))
        }
        c.element.tuplet.bracketType = tuplet.bracketType
        c.element.tuplet.numberType = tuplet.numberType
        c.element.tuplet.visible = tuplet.visible
        for (var i in tuplet.elements) {
            c.rewindToFraction(retrogradedTick(tuplet.elements[i].startTick.plus(tuplet.elements[i].actualDuration)))
            if (tuplet.elements[i].type == Element.TUPLET) {
                addTupletObj(tuplet.elements[i], c)
            } else {
                addChordRestObj(tuplet.elements[i], c)
            }
        }
    }

    function addAnnotations(cursor, annotations) {
        for (var i in cursor.segment.annotations) {
            var el = cursor.segment.annotations[i]
            if (el.track == cursor.track) {
                removeElement(el)
            }
        }
        for (var i in annotations) {
            var el = annotations[i]
            cursor.add(el)
        }
    }

    function addArticulations(cursor, artiList) {
        for (var i in artiList) {
            cursor.add(artiList[i])
        }
    }

    function addGraceNotes(note, graceList) {
        if (graceList.length == 0) {
            return
        }
        for (var i = graceList.length - 1; i >= 0; i--) {
            curScore.selection.select(note, false)
            cmd(graceList[i].type)
        }
        var graceNotes = note.parent.graceNotes
        for (var i in graceList) {
            var toRemove = graceNotes[i].notes[0]
            for (var j in graceList[i].notes) {
                graceNotes[i].add(graceList[i].notes[j])
            }
            removeElement(toRemove)
            graceNotes[i].duration = graceList[i].duration
        }
    }

    function addTies(tieList) {
        var c = curScore.newCursor()
        for (var i in tieList) {
            c.track = tieList[i].track
            c.rewindToFraction(retrogradedTick(tieList[i].startTick)) // Since we want the end position here, don't add actualDuration
            c.prev()
            if (!c.element || c.element.type == Element.REST || !c.element.notes[tieList[i].note]) {
                return console.log("Unable to add tie, notes missing")
            }
            curScore.selection.select(c.element.notes[tieList[i].note], false)
            cmd("tie")
        }
    }

    function readSelection() {
        if (!curScore.selection.elements.length) return false
        if (curScore.selection.isRange) {
            return {
                isRange: true,
                startSegment: curScore.selection.startSegment.tick,
                endSegment: curScore.selection.endSegment ? curScore.selection.endSegment.tick : curScore.lastSegment.tick + 1,
                startStaff: curScore.selection.startStaff,
                endStaff: curScore.selection.endStaff
            }
        }
        var selectObj = {
            isRange: false,
            elements: []
        }
        for (var i in curScore.selection.elements) {
            selectObj.elements.push(curScore.selection.elements[i])
        }
        return selectObj
    }

    function writeSelection(selectObj) {
        if (selectObj == false) return
        if (selectObj.isRange) {
            curScore.selection.selectRange(
                selectObj.startSegment,
                selectObj.endSegment,
                selectObj.startStaff,
                selectObj.endStaff
            )
            return
        }
        for (var i in selectObj.elements) {
            curScore.selection.select(selectObj.elements[i], true)
        }
    }

    onRun: {
        if (!curScore.selection.elements.length) {
            curScore.startCmd("Retrograde score")
            cmd("select-all")
        } else {
            curScore.startCmd("Retrograde selection")
        }
        try {
            selection = readSelection()
            retrogradeSelection()
            curScore.selection.clear()
            writeSelection(selection)
            curScore.endCmd()
        } catch (e) {
            // If we encounter an error, rollback all changes
            curScore.endCmd(true)
            curScore.startCmd("Retrograde: " + e.toString())
            var text = newElement(Element.STAFF_TEXT)
            text.text = e.toString()
            var c = curScore.newCursor()
            c.track = 0
            c.rewindToFraction(globalStartTick)
            c.add(text)
            curScore.endCmd()
        }
        quit()
    }
}
