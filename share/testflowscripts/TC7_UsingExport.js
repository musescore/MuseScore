/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
var Project = require("steps/Project.js")
var Commands = require("steps/Commands.js")

var testCase = {
    name: "TC7: Using Export",
    description: "Let's check the functionality of the export",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            Project.closeIfOpened()
            Navigation.triggerControl("TopTool", "MainToolBar", "Home")
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
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_EIGHTH_COMMAND)
        }},
        {name: "Put notes", func: function() {
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_D_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_E_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_F_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_G_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_A_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_B_COMMAND)
        }},
        {name: "Save", func: function() {
            api.testflow.saveProject("TC7_UsingExport.mscz")
        }},
        {name: "Export PDF: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export PDF: Setup and Export", skip: false, func: function() {
            Navigation.triggerControl("DialogView", "ExportOptions", "ExportType_All parts combined in one file")
            Navigation.triggerControl("DialogView", "ButtonBox", "Export")
        }},
        {name: "Export PDF: Check file", skip: false, func: function() {
            checkExportedFile()
        }},
        {name: "Export PNG: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export PNG: Setup and Export", skip: false, func: function() {
            selectExportType("PNG")
            Navigation.triggerControl("DialogView", "ButtonBox", "Export")
        }},
        {name: "Export PNG: Check file", skip: false, func: function() {
            checkExportedFile()
        }},
        {name: "Export SVG: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export SVG: Setup and Export", skip: false, func: function() {
            selectExportType("SVG")
            Navigation.triggerControl("DialogView", "ButtonBox", "Export")
        }},
        {name: "Export SVG: Check file", skip: false, func: function() {
            checkExportedFile()
        }},
        {name: "Export MusicXML: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export MusicXML: Setup and Export", skip: false, func: function() {
            selectExportType("MusicXML")
            Navigation.triggerControl("DialogView", "ButtonBox", "Export")
        }},
        {name: "Export MusicXML: Check file", skip: false, func: function() {
            checkExportedFile()
        }},
        {name: "Export MP3: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export MP3: Setup and Export", skip: false, func: function() {
            selectExportType("MP3")
            Navigation.triggerControl("DialogView", "ButtonBox", "Export")
        }},
        {name: "Export MP3: Check file", skip: false, func: function() {
            checkExportedFile()
        }},
        {name: "Export WAV: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export WAV: Setup and Export", skip: false, func: function() {
            selectExportType("WAV")
            Navigation.triggerControl("DialogView", "ButtonBox", "Export")
        }},
        {name: "Export WAV: Check file", skip: false, func: function() {
            checkExportedFile()
        }},
        {name: "Export OGG: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export OGG: Setup and Export", skip: false, func: function() {
            selectExportType("OGG")
            Navigation.triggerControl("DialogView", "ButtonBox", "Export")
        }},
        {name: "Export OGG: Check file", skip: false, func: function() {
            checkExportedFile()
        }},
        {name: "Export FLAC: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export FLAC: Setup and Export", skip: false, func: function() {
            selectExportType("FLAC")
            Navigation.triggerControl("DialogView", "ButtonBox", "Export")
        }},
        {name: "Export FLAC: Check file", skip: false, func: function() {
            checkExportedFile()
        }},
        {name: "Export MIDI: Open Export Dialog", skip: false, func: function() {
            openExportDialog()
        }},
        {name: "Export MIDI: Setup and Export", skip: false, func: function() {
            selectExportType("MIDI")
            Navigation.triggerControl("DialogView", "ButtonBox", "Export")
        }},
        {name: "Export MIDI: Check file", skip: false, func: function() {
            checkExportedFile()
        }},
    ]
};

function main()
{
    api.testflow.setInterval(1000)
    api.testflow.runTestCase(testCase)
}

function seeChanges()
{
    api.testflow.seeChanges()
}

function checkFileSize(path)
{
    var size = api.testflow.fileSize(path)
    if (size === 0) {
        api.testflow.error("file not exists or zero, path: " + path)
    }
}

function openExportDialog()
{
    api.testflow.showMainWindowOnFront()
    api.testflow.async(function() {
        api.dispatcher.dispatch(Commands.PROJECT_EXPORT_COMMAND)
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
    api.testflow.waitPopup()
    for (var s = 0; s < idx; ++s) {
        Navigation.down()
    }
    seeChanges()
    Navigation.trigger()
}

//! NOTE Only check the file, opening it in an external application is not needed (and breaks unattended runs)
function checkExportedFile()
{
    var filePath = api.testflow.selectedFilePath()
    checkFileSize(filePath)
    //! NOTE Export is finished asynchronously, the export dialog can't be opened again until then
    api.testflow.sleep(1000)
}
