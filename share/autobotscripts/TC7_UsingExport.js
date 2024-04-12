/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

var NewScore = require("steps/NewScore.js")
var NoteInput = require("steps/NoteInput.js")
var Navigation = require("steps/Navigation.js")

var testCase = {
    name: "TC7: Using Export",
    description: "Let's check the functionality of the export",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            api.dispatcher.dispatch("file-close")
            api.navigation.triggerControl("TopTool", "MainToolBar", "Home")
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Create score with Flute", func: function() {
            NewScore.selectTab("instruments")
            NewScore.chooseInstrument("Woodwinds", "Flute")
            NewScore.done()
        }},
        {name: "Turn on note input", func: function() {
            NoteInput.chooseDefaultMode()
            NoteInput.chooseNoteDuration("pad-note-8")
        }},
        {name: "Put notes", func: function() {
            NoteInput.putNote("note-c")
            NoteInput.putNote("note-d")
            NoteInput.putNote("note-e")
            NoteInput.putNote("note-f")
            NoteInput.putNote("note-g")
            NoteInput.putNote("note-a")
            NoteInput.putNote("note-b")
        }},
        {name: "Save", func: function() {
            api.autobot.saveProject("TC7_UsingExport.mscz")
        }},
        {name: "Export PDF: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export PDF: Setup and Export", skip: false, func: function() {
            Navigation.triggerControl("DialogView", "ExportOptions", "ExportType_All parts combined in one file")
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export PDF: Check file and Open", skip: false, func: function() {
            checkAndOpen()
        }},
        {name: "Export PNG: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export PNG: Setup and Export", skip: false, func: function() {
            selectExportType("PNG")
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export PNG: Check file and Open", skip: false, func: function() {
            checkAndOpen()
        }},
        {name: "Export SVG: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export SVG: Setup and Export", skip: false, func: function() {
            selectExportType("SVG")
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export SVG: Check file and Open", skip: false, func: function() {
            checkAndOpen()
        }},
        {name: "Export MusicXML: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export MusicXML: Setup and Export", skip: false, func: function() {
            selectExportType("MusicXML")
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export MusicXML: Check file and Open", skip: false, func: function() {
            checkAndOpen()
        }},
        {name: "Export MP3: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export MP3: Setup and Export", skip: false, func: function() {
            selectExportType("MP3")
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export MP3: Check file and Open", skip: false, func: function() {
            checkAndOpen()
        }},
        {name: "Export WAV: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export WAV: Setup and Export", skip: false, func: function() {
            selectExportType("WAV")
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export WAV: Check file and Open", skip: false, func: function() {
            checkAndOpen()
        }},
        {name: "Export OGG: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export OGG: Setup and Export", skip: false, func: function() {
            selectExportType("OGG")
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export OGG: Check file and Open", skip: false, func: function() {
            checkAndOpen()
        }},
        {name: "Export FLAC: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export FLAC: Setup and Export", skip: false, func: function() {
            selectExportType("FLAC")
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export FLAC: Check file and Open", skip: false, func: function() {
            checkAndOpen()
        }},
        {name: "Export MIDI: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export MIDI: Setup and Export", skip: false, func: function() {
            selectExportType("MIDI")
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export MIDI: Check file and Open", skip: false, func: function() {
            checkAndOpen()
        }},
    ]
};

function main()
{
    api.autobot.setInterval(1000)
    api.autobot.runTestCase(testCase)
}

function seeChanges()
{
    api.autobot.seeChanges()
}

function seeChangesLong()
{
    api.autobot.sleep(1000)
}

function checkFileSize(path)
{
    var size = api.autobot.fileSize(path)
    if (size === 0) {
        api.autobot.error("file not exists or zero, path: " + path)
    }
}

function openExportDialog()
{
    api.autobot.showMainWindowOnFront()
    api.autobot.async(function() {
        api.dispatcher.dispatch("file-export")
    })
}

const TYPES = ["PDF", "PNG", "SVG", "MP3", "WAV", "OGG", "FLAC", "MIDI", "MusicXML"]

function selectExportType(type)
{
    var idx = TYPES.indexOf(type)
    if (idx === 0) {
        //! NOTE first is default
        return
    }

    Navigation.triggerControl("DialogView", "ExportOptions", "ExportTypeDropdown")
    api.autobot.waitPopup()
    for (var s = 0; s < idx; ++s) {
        Navigation.down()
    }
    seeChanges()
    Navigation.trigger()
}

function checkAndOpen()
{
    var filePath = api.autobot.selectedFilePath()
    checkFileSize(filePath)
    api.interactive.openUrl(filePath)
    seeChangesLong()
}
