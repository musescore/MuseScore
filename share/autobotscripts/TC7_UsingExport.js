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
            NewScore.сhooseInstrument("Woodwinds", "Flute")
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
        {name: "Export PDF: Open Export Dialog", func: function() {
            api.autobot.async(function() {
                api.dispatcher.dispatch("file-export")
            })
        }},
        {name: "Export PDF: Setup and Export", func: function() {
            Navigation.triggerControl("DialogView", "ExportOptions", "ExportType_All parts combined in one file")
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export PDF: Check file and Open", func: function() {
            var filePath = api.autobot.selectedFilePath()
            checkFileSize(filePath)
            api.interactive.openUrl(filePath)
            seeChangesLong()
        }},
        {name: "Export PNG: Open Export Dialog", func: function() {
            api.autobot.showMainWindowOnFront()
            api.autobot.async(function() {
                api.dispatcher.dispatch("file-export")
            })
        }},
        {name: "Export PNG: Setup and Export", func: function() {
            Navigation.triggerControl("DialogView", "ExportOptions", "ExportTypeDropdown")
            api.autobot.waitPopup()
            Navigation.down() // to PNG
            seeChanges()
            Navigation.trigger()
            Navigation.triggerControl("DialogView", "ExportBottom", "Export")
        }},
        {name: "Export PNG: Check file and Open", func: function() {
            var filePath = api.autobot.selectedFilePath()
            checkFileSize(filePath)
            api.interactive.openUrl(filePath)
            seeChangesLong()
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
