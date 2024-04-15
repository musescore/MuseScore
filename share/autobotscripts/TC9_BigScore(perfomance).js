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

var Navigation = require("steps/Navigation.js")
var Score = require("steps/Score.js")
var Palette = require("steps/Palette.js")

var testCase = {
    name: "TC9: Big Score (perfomance)",
    description: "Let's check perfomance",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            api.dispatcher.dispatch("file-close")
            api.navigation.triggerControl("TopTool", "MainToolBar", "Home")
        }},
        {name: "Open Big Score", func: function() {
            api.autobot.openProject("Big_Score.mscz")
        }},
        {name: "Select All", func: function() {
            Score.focusIn()
            api.shortcuts.activate("Ctrl+A")
        }},
        {name: "Add Slur", func: function() {
            api.shortcuts.activate("S")
        }}
    ]
};

function main()
{
    api.autobot.setInterval(500)
    api.autobot.runTestCase(testCase)
}
