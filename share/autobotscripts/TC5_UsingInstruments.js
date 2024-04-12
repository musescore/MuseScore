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
var Instruments = require("steps/Instruments.js")

var testCase = {
    name: "TC5: Using instruments",
    description: "Let's check the functionality of the instruments",
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
            api.autobot.pause()
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
            api.autobot.saveProject("TC5_UsingInstruments.mscz")
        }},
        {name: "Close", func: function() {
            api.dispatcher.dispatch("file-close")
        }},
        {name: "Home", func: function() {
            api.navigation.triggerControl("TopTool", "MainToolBar", "Home")
        }},
        {name: "Open last", func: function() {
            api.navigation.goToControl("RecentScores", "RecentScores", "New score")
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
