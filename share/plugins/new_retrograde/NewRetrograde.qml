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
    title: "Retrograde"
    description: "Takes a selection of notes and reverses them."
    version: "1.0"
    categoryCode: "composing-arranging-tools"
    thumbnailName: "retrograde.png"
    id: root

    property var selection: false
    property var allTies: []

    function retrogradeSort(a, b) {
        return b.track == a.track ? b.startTick - a.startTick : b.track - a.track;
    }

    function retrogradeSelection() {
        storeSelection(true)
        var sendObjs = []
        var parsedTuplets = []
        var parsedCRs = []
        for (var i in curScore.selection.elements) {
            var el = curScore.selection.elements[i]
            if (!el || getType(el) == "unknown") {
                continue
            }
                
            var sendObj = {}
            var add = true
            if (el.type == Element.TUPLET || getChordRest(el).tuplet) {
                var tuplet = getMotherTuplet(getChordRest(el))
                for (var j in parsedTuplets) {
                    if (parsedTuplets[j].is(tuplet)) {
                        add = false
                        break
                    }
                }
                if (add) {
                    sendObj = getTupletObj(tuplet)
                    parsedTuplets.push(tuplet)
                } else {
                    continue
                }
            } else {
                var cr = getChordRest(el)
                if (cr.type == Element.CHORD && cr.notes[0].noteType != NoteType.NORMAL) {
                    continue
                }
                for (var j in parsedCRs) {
                    if (parsedCRs[j].is(cr)) {
                        add = false
                        break
                    }
                }
                if (add) {
                    sendObj = getChordRestObj(el)
                    parsedCRs.push(cr)
                } else {
                    continue
                }
            }
            console.log("logging " + sendObj.type + ": Tick " + sendObj.startTick + ", voice " + (sendObj.track % 4 + 1) + ", staff "
                + Math.ceil((sendObj.track + 1) / 4) + ", duration: " + sendObj.duration.numerator + "/" + sendObj.duration.denominator)
            sendObjs.push(sendObj)
        }

        sendObjs.sort(retrogradeSort)
        
        var curTrack = -1
        var cursor = curScore.newCursor()
        for (var i in sendObjs) {
            var el = sendObjs[i]
            console.log("Received " + el.type + ": Tick " + el.startTick
                + ", voice " + (el.track % 4 + 1) + ", staff " + Math.ceil((el.track + 1) / 4))
            if (el.track != curTrack) {
                curTrack = el.track
                cursor.track = curTrack
                cursor.rewindToTick(sendObjs[sendObjs.length-1].startTick)
                if (sendObjs[sendObjs.length-1].startTick != cursor.measure.firstSegment.tick) cursor.rewindToTick(sendObjs[sendObjs.length-1].startTick)
            }
            if (cursor.element && cursor.element.tuplet) {
                removeElement(getMotherTuplet(cursor.element))
            }
            if (el.type == "tuplet") {
                addTupletObj(el, cursor)
            } else {
                addChordRestObj(el, cursor)
            }
        }
        addTies(allTies)
        retrieveSelection()
    }

    //creates a sendObj from a note/chord/rest
    function getChordRestObj(element) {
        return {
            duration: getDuration(element),
            notes: getNotes(element),
            startTick: getTick(element),
            track: element.track,
            type: getType(element),
            annotations: getAnnotations(element),
            articulations: getArticulations(element),
            graceNotes: getGraceNotes(element),
            ties: getTies(element),
            beamMode: getBeamMode(element),
            offsetY: element.type == Element.REST ? element.offsetY : false,
            visible: element.type == Element.REST ? element.visible : false
        }
    }
    //returns readable duration values from a note/chord/rest/tuplet
    function getDuration(element) {
        return {numerator: getChordRest(element).duration.numerator, denominator: getChordRest(element).duration.denominator}
    }
    //returns the notes in a note/chord/rest
    function getNotesArray(element) {
        switch (element.type) {
            case Element.REST: return [] //null
            default: return getChordRest(element).notes
        }
    }
    //creates readable information from the notes in a note/chord/rest
    function getNotes(element) {
        var notes = []
        var chordNotes = getNotesArray(element)
        for (var i in chordNotes) {
            notes[i] = {
                pitch: chordNotes[i].pitch,
                tpc: chordNotes[i].tpc,
                tpc1: chordNotes[i].tpc1,
                tpc2: chordNotes[i].tpc2
            }
        }
        return notes
    }
    //allows identical treatment/parenthood of notes/chords/rests
    function getChordRest(element) {
        switch (element.type) {
            case Element.NOTE: return element.parent
            default: return element
        }
    }
    //returns the segment of a given note/chord/rest
    function getSegment(element) {
        return getChordRest(element).parent
    }
    //returns the tick of a given note/chord/rest/tuplet
    function getTick(element) {
        return element.type == Element.TUPLET ? getTick(element.elements[0]) : getSegment(element).tick
    }
    //retrieves the annotations of a chordrest in a readable form
    function getAnnotations(element) {
        var annotations = getSegment(element).annotations
        var annoList = []
        for (var i in annotations) {
            var el = annotations[i]
            var obj = {}
            switch (el.type) {
                case Element.TEMPO_TEXT: {
                    console.log("logging tempo marking", 1)
                    obj = {
                        text: el.text,
                        tempo: el.tempo,
                        tempoFollowText: el.tempoFollowText
                    }
                    break
                }
                case Element.STAFF_TEXT:
                case Element.SYSTEM_TEXT: {
                    console.log("logging staff/system text", 1)
                    obj = {
                        text: el.text,
                        fontStyle: el.fontStyle
                    }
                    break
                }
                case Element.DYNAMIC: {
                    console.log("logging dynamic", 1)
                    obj = {
                        text: el.text,
                        velocity: el.velocity,
                        veloChange: el.veloChange
                    }
                    break
                }
                case Element.FERMATA: {
                    console.log("logging fermata", 1)
                    obj = {
                        symbol: el.symbol,
                        timeStretch: el.timeStretch
                    }
                    break
                }
                default: {
                    console.log("Unknown annotation element", 2)
                    logMessage("Unknown annotation element")
                    break
                }
            }
            obj.type = el.type
            obj.visible = el.visible
            obj.placement = el.placement
            annoList.push(obj)
        }
        for (var i in annotations) {
            removeElement(annotations[i])
        }
        return annoList
    }
    //retrieves a chordrests articulations
    function getArticulations(element) {
        var artiList = []
        if (element.type == Element.REST) return artiList
        var storedSelection = false
        if (!curScore.selection.isRange) {
            storeSelection()
            storedSelection = true
            cmd("select-all")
        }
        for (var i in curScore.selection.elements) {
            if (curScore.selection.elements[i].type == Element.ARTICULATION && getTick(curScore.selection.elements[i].parent) == getTick(element)
                && curScore.selection.elements[i].track == element.track) {
                artiList.push({
                    placement: curScore.selection.elements[i].placement,
                    play: curScore.selection.elements[i].play,
                    symbol: curScore.selection.elements[i].symbol,
                    visible: curScore.selection.elements[i].visible
                })
            }
        }
        if (storedSelection) retrieveSelection()
        return artiList
    }
    //retrieves a chordrests grace notes
    function getGraceNotes(element) {
        if (element.type == Element.REST) return []
        var graceNotes = getChordRest(element).graceNotes
        if (graceNotes) {
            var graceList = []
            for (var i in graceNotes) {
                var graceChord = graceNotes[i]
                graceList.push({
                    duration: getDuration(graceChord),
                    notes: getNotes(graceChord),
                    type: getGraceNoteType(graceChord)
                })
            }
            return graceList
        }
        return []
    }
    //retrieves the type of grace note, formatted for sendObj
    //doesnt work with switch for some reason
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
    //retrieves a list of notes with ties in a chordrest
    // modified for retrograde
    function getTies(element) {
        if (element.type == Element.REST) return []
        var tieList = []
        for (var i in getChordRest(element).notes) {
            if (getChordRest(element).notes[i].tieBack) {
                tieList.push({
                    startTick: 0,
                    track: element.track,
                    note: i
                })
                console.log("Logging tie at tick " +  getTick(element))
            }
        }
        return tieList
    }
    function getBeamMode(element) {
        return getChordRest(element).beamMode
    }
    //returns the mother tuplet (unnested tuplet)
    function getMotherTuplet(element) {
        var tuplet = element
        while (tuplet.tuplet) {
            tuplet = tuplet.tuplet
        }
        return tuplet
    }
    //creates a sendObj from a tuplet
    function getTupletObj(tuplet) {
        return {
            duration: getDuration(tuplet),
            type: getType(tuplet),
            startTick: getTick(tuplet),
            track: tuplet.track,
            elements: getTupletElements(tuplet),
            ratio: getTupletRatio(tuplet),
            bracketType: tuplet.bracketType,
            numberType: tuplet.numberType,
            visible: tuplet.visible
        }
    }
    //returns the ratio of a given tuplet
    function getTupletRatio(tuplet) {
        return {numerator: tuplet.actualNotes, denominator: tuplet.normalNotes}
    }
    //returns the chordrests within a tuplet
    function getTupletElements(tuplet) {
        var elementsArray = []
        for (var i in tuplet.elements) {
            if (getType(tuplet.elements[i]) == "tuplet") {
                elementsArray.push(getTupletObj(tuplet.elements[i]))
            } else {
                elementsArray.push(getChordRestObj(tuplet.elements[i]))
            }
        }
        elementsArray.sort(retrogradeSort)
        return elementsArray
    }
    //returns a readable 'type' property for elements
    function getType(element) {
        switch (element.type) {
            case Element.NOTE:
            case Element.CHORD:   return "note"
            case Element.REST:    return "rest"
            case Element.TUPLET:  return "tuplet"
            default:              return "unknown"
        }
    }
    //adds chordrests to the score
    function addChordRestObj(element, c) {
        var t = c.tick
        if (c.element) {
            c.setDuration(c.element.duration.numerator, c.element.duration.denominator)
            c.addRest()
            c.rewindToTick(t)
        }
        c.setDuration(element.duration.numerator, element.duration.denominator)
        if (element.type == "rest") {
            c.addRest()
            c.rewindToTick(t)
            //check for full measure rest
            if (c.element.duration.numerator / c.element.duration.denominator == c.measure.timesigActual.numerator / c.measure.timesigActual.denominator) {
                storeSelection()
                curScore.selection.select(c.element, false)
                cmd("full-measure-rest")
                retrieveSelection()
            }
            c.rewindToTick(t)
            c.element.offsetY = element.offsetY
            c.element.visible = element.visible
        } else {
            c.rewindToTick(t)
            for (var i in element.notes) {c.addNote(element.notes[i].pitch, i != 0)}
            c.rewindToTick(t)
            for (var i in c.element.notes) {
                c.element.notes[i].tpc  = element.notes[i].tpc
                c.element.notes[i].tpc1 = element.notes[i].tpc1
                c.element.notes[i].tpc2 = element.notes[i].tpc2
            }
            addArticulations(c, element.articulations)
            addGraceNotes(c.element.notes[0], element.graceNotes)
            for (var i in element.ties) {
                element.ties[i].startTick = t
                allTies.push(element.ties[i])
            }
        }
        c.element.beamMode = element.beamMode
        addAnnotations(c, element.annotations)
        c.next()
    }
    //adds tuplets and their contents to the score
    function addTupletObj(element, c) {
        var t = c.tick
        c.setDuration(element.duration.numerator, element.duration.denominator)
        c.addRest()
        c.rewindToTick(t)
        c.addTuplet(fraction(element.ratio.numerator, element.ratio.denominator), fraction(element.duration.numerator, element.duration.denominator))
        c.rewindToTick(t)
        console.log("Adding tuplet at tick " + t)
        if (!c.element.tuplet) {
            console.log("could not add tuplet. Adding as regular notes instead.")
        } else {
            c.element.tuplet.bracketType = element.bracketType
            c.element.tuplet.numberType = element.numberType
            c.element.tuplet.visible = element.visible
        }
        for (var i in element.elements) {
            if (element.elements[i].type == "tuplet") addTupletObj(element.elements[i], c)
            else addChordRestObj(element.elements[i], c)
        }
    }
    //adds annotations (dynamics, tempo text, etc) to the score
    function addAnnotations(cursor, annotations) {
        for (var i in getSegment(cursor.element).annotations) {
            removeElement(getSegment(cursor.element).annotations[i])
        }
        if (annotations.length < 1) return
        for (var i in annotations) {
            var el = annotations[i]
            var obj = newElement(el.type)
            obj.visible = el.visible
            obj.placement = el.placement
            switch (el.type) {
                case Element.TEMPO_TEXT: {
                    console.log("adding tempo marking", 1)
                    obj.text = el.text
                    break
                }
                case Element.STAFF_TEXT:
                case Element.SYSTEM_TEXT: {
                    console.log("adding staff/system text", 1)
                    obj.text = el.text
                    obj.fontStyle = el.fontStyle
                    break
                }
                case Element.DYNAMIC: {
                    console.log("adding dynamic", 1)
                    obj.text = el.text
                    obj.velocity = el.velocity
                    obj.dynamicRange = el.dynamicRange
                    obj.veloChange = el.veloChange
                    break
                }
                case Element.FERMATA: {
                    console.log("adding fermata", 1)
                    obj.symbol = el.symbol
                    obj.timeStretch = el.timeStretch
                    break
                }
            }
            cursor.add(obj)
            if (el.type == Element.TEMPO_TEXT) {
                obj.tempo = el.tempo
                obj.tempoFollowText = el.tempoFollowText
                //crash if applying before adding
            }
            delete obj
        }
    }
    //adds articulations from sendObj
    function addArticulations(cursor, artiList) {
        //if (artiList.length < 1) return
        for (var i in artiList) {
            var obj = newElement(Element.ARTICULATION)
            obj.placement = artiList[i].placement
            obj.play = artiList[i].play
            obj.symbol = artiList[i].symbol
            obj.visible = artiList[i].visible
            cursor.add(obj)
            delete obj
        }
    }
    //adds grace notes from sendObj
    function addGraceNotes(note, graceList) {
        var graceNotes = note.parent.graceNotes
        if (graceList.length > 0) {
            storeSelection()
            for (var i in graceNotes) {
                removeElement(graceNotes[0])
            }
            for (var i = (graceList.length-1); i >= 0; i--) {
                curScore.selection.select(note, false)
                cmd(graceList[i].type)
            }
            for (var i in graceList) {
                curScore.selection.select(graceNotes[i].notes[0], false)
                applyGraceNoteDuration(graceNotes[i].notes[0], graceList[i].duration)
                
                for (var j in graceList[i].notes) {
                    curScore.selection.select(graceNotes[i].notes[j], false)
                    if (j != 0) {
                        cmd("chord-e")
                        cmd("note-input") //cancel chord-e effect
                    }
                    graceNotes[i].notes[j].pitch = graceList[i].notes[j].pitch
                    graceNotes[i].notes[j].tpc = graceList[i].notes[j].tpc
                    graceNotes[i].notes[j].tpc1 = graceList[i].notes[j].tpc1
                    graceNotes[i].notes[j].tpc2 = graceList[i].notes[j].tpc2
                }
            }
            retrieveSelection()
        }
    }
    function applyGraceNoteDuration(note, targetDuration) {
        console.log(qsTr("Calculating grace note duration..."))
        var startN = note.parent.duration.numerator
        var startD = note.parent.duration.denominator
        var endN = targetDuration.numerator
        var endD = targetDuration.denominator
        
        console.log(qsTr("Removing dots..."))
        switch (note.dots.length) {
            case 4:
            case 3: {
                cmd("pad-dot" + note.dots.length)
                break
            }
            case 2: {
                cmd("pad-dotdot")
                break
            }
            case 1: {
                cmd("pad-dot")
                break
            }
            default: console.log(qsTr("No dots detected"))
        }
        console.log(qsTr("Calculating base duration..."))
        var i = -1
        while (Math.pow(2, i) < (endD / endN)) i++
        switch (Math.pow(2, i)) {
            case 0.25: {
                cmd("note-longa")
                break
            }
            case 0.5: {
                cmd("note-breve")
                break
            }
            default: cmd("pad-note-" + Math.pow(2, i))
        }
        console.log(qsTr("Adding dots..."))
        switch(endN) {
            case 31:
            case 15: {
                cmd("pad-dot" + note.dots.length)
                break
            }
            case 7: {
                cmd("pad-dotdot")
                break
            }
            case 3: {
                cmd("pad-dot")
                break
            }
            default: console.log(qsTr("No dots added"))
        }
    }
    //adds ties
    function addTies(tieList) {
        storeSelection()
        for (var i in tieList) {
            var c = curScore.newCursor()
            c.rewindToTick(tieList[i].startTick)
            c.track = tieList[i].track
            if (tieList[i].startTick != c.measure.firstSegment.tick) c.rewindToTick(tieList[i].startTick)
            if (!c.element || c.element.type == Element.REST) return console.log("Unable to add tie, notes missing", 2)
            var n = c.element.notes[tieList[i].note]
            while (n.tieForward) {
                n = n.tieForward.endNote
            }
            curScore.selection.select(n, false)
            cmd("tie")
            delete c
        }
        retrieveSelection()
    }
    function storeSelection(keep = false) {
        root.selection = readSelection()
        if (keep) return
        curScore.selection.clear()
    }
    function retrieveSelection() {
        curScore.selection.clear()
        writeSelection(root.selection)
        root.selection = false
    }
    function readSelection() {
        var selectObj
        if (!curScore.selection.elements.length) return false
            if (curScore.selection.isRange) {
                selectObj = {
                    isRange: true,
                    startSegment: curScore.selection.startSegment.tick,
                    endSegment: curScore.selection.endSegment ? curScore.selection.endSegment.tick : curScore.lastSegment.tick + 1,
                    startStaff: curScore.selection.startStaff,
                    endStaff: curScore.selection.endStaff
                }
            } else {
                selectObj = {
                    isRange: false,
                    elements: []
                }
                for (var i in curScore.selection.elements) {
                    selectObj.elements.push(curScore.selection.elements[i])
                }
            }
        return selectObj
    }
    function writeSelection(selectObj) {
        if (!selectObj) return
        if (selectObj.isRange) {
            curScore.selection.selectRange(
                selectObj.startSegment,
                selectObj.endSegment,
                selectObj.startStaff,
                selectObj.endStaff
            )
        } else {
            for (var i in selectObj.elements) {
                curScore.selection.select(selectObj.elements[i], true)
            }
        }
    }

    onRun: {
        if (curScore.selection.elements.length == 0) {
            curScore.startCmd("Retrograde score")
            cmd("select-all")
        } else {
            curScore.startCmd("Retrograde selection")
        }
        retrogradeSelection();
        curScore.endCmd()
        quit()
    }
}
