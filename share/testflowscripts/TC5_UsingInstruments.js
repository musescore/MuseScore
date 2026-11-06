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

var Navigation = require("steps/Navigation.js")
var NewScore = require("steps/NewScore.js")
var Instruments = require("steps/Instruments.js")
var Project = require("steps/Project.js")
var Commands = require("steps/Commands.js")

var testCase = {
    name: "TC5: Using instruments",
    description: "Let's check the functionality of the instruments",
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
        {name: "Open Instruments panel", func: function() {
            Instruments.openInstrumentsPanel()
        }},
        {name: "Open Add Instruments Dialog", func: function() {
            Instruments.openAddInstrumentsDialog()
        }},
        {name: "Add Instruments Piano and Flute(2)", func: function() {
            Instruments.chooseInstrument("Keyboards", "Piano")
            seeChanges()
            Instruments.chooseInstrument("Woodwinds", "Flute")
            seeChanges()
            Instruments.doneAddInstruments()
        }},
        {name: "Hide Flute 2", func: function() {
            Instruments.toggleVisibleInstrument("Flute 2")
            seeChangesLong()
        }},
        {name: "Show Flute 2", func: function() {
            Instruments.toggleVisibleInstrument("Flute 2")
        }},
        {name: "Open Settings Dialog Flute 2", func: function() {
            Instruments.openSettingsInstrument("Flute 2")
        }},
        {name: "Manually Edit Setting", func: function() {
            api.testflow.pause()
        }},
        {name: "Move Flute Down", func: function() {
            Instruments.moveDownInstrument("Flute")
            seeChangesLong()
        }},
        {name: "Move Flute Down Again", func: function() {
            Instruments.moveDownInstrument("Flute")
            seeChangesLong()
        }},
        {name: "Move Flute Up", func: function() {
            Instruments.moveUpInstrument("Flute")
            seeChangesLong()
        }},
        {name: "Remove Piano", func: function() {
            Instruments.removeInstrument("Piano")
            seeChangesLong()
        }},
        {name: "Save", func: function() {
            api.testflow.saveProject("TC5_UsingInstruments.mscz")
        }},
        {name: "Close", func: function() {
            api.dispatcher.dispatch(Commands.PROJECT_CLOSE_COMMAND)
        }},
        {name: "Home", func: function() {
            Navigation.triggerControl("TopTool", "MainToolBar", "Home")
        }},
        {name: "Open last", func: function() {
            Navigation.goToControl("RecentScores", "RecentScores", "New score")
            api.navigation.right()
            api.navigation.trigger()
        }},
        {name: "Open Instruments panel", func: function() {
            Instruments.openInstrumentsPanel()
        }}
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

function seeChangesLong()
{
    api.testflow.sleep(1000)
}
