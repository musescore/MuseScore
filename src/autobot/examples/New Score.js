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

function main()
{
    var testCase = {
        name: "New Score",
        steps: [
            {name: "Open Dialog", func: function() {
                api.autobot.async(function() {
                    api.dispatcher.dispatch("file-new")
                })
            }},
            {name: "Select Flute", func: function() {
                api.navigation.goToControl("NewScoreDialog", "FamilyView", "Woodwinds")
                api.navigation.goToControl("NewScoreDialog", "InstrumentsView", "Flute")

            }},
            {name: "Select Flute (apply)", func: function() {
                api.navigation.goToControl("NewScoreDialog", "SelectPanel", "Select")
                api.navigation.trigger()

            }},
            {name: "Select Piano", func: function() {
                api.navigation.goToControl("NewScoreDialog", "FamilyView", "Keyboards")
                api.navigation.goToControl("NewScoreDialog", "InstrumentsView", "Piano")
            }},
            {name: "Select Piano (apply)", func: function() {
                api.navigation.goToControl("NewScoreDialog", "SelectPanel", "Select")
                api.navigation.trigger()
                api.autobot.pause()

            }},
            {name: "Done", func: function() {
                api.navigation.goToControl("NewScoreDialog", "BottomPanel", "Done")
                api.navigation.trigger()
            }},
        ]
    };

    api.autobot.setInterval(500)
    api.autobot.runTestCase(testCase)
    api.log.info("----------- end script ---------------")
}
