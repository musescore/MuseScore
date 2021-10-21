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

function openNewScoreDialog()
{
    api.navigation.triggerControl("AppTitleBar", "AppMenuBar", "&File")
    // wait popup open
    api.autobot.waitPopup()
    // New become current automatically, so just trigger
    api.navigation.trigger()
}

function сhooseFluteAndPiano()
{
    // Flute
    api.navigation.goToControl("NewScoreDialog", "FamilyView", "Woodwinds")
    api.navigation.goToControl("NewScoreDialog", "InstrumentsView", "Flute")
    api.navigation.triggerControl("NewScoreDialog", "SelectPanel", "Select")

    // just for see changes
    api.autobot.sleep(500)

    // Piano
    api.navigation.goToControl("NewScoreDialog", "FamilyView", "Keyboards")
    api.navigation.goToControl("NewScoreDialog", "InstrumentsView", "Piano")
    api.navigation.triggerControl("NewScoreDialog", "SelectPanel", "Select")

    // just for see changes
    api.autobot.sleep(500)

    // Done
    api.navigation.triggerControl("NewScoreDialog", "BottomPanel", "Done")
}

function chooseRandomInstruments(count, see_msec)
{
    see_msec = see_msec || 50
    api.log.debug("chooseRandomInstruments count: " + count)
    for (var i = 0; i < count; i++) {

        api.log.debug("chooseRandomInstruments i: " + i)
        // Go to first family
        api.navigation.goToControl("NewScoreDialog", "FamilyView", "Woodwinds")

        // Choose family
        var familyCount = api.autobot.randomInt(0, 20);
        api.log.debug("chooseRandomInstruments familyCount: " + familyCount)
        for (var f = 0; f < familyCount; f++) {
            api.navigation.down()
            api.autobot.seeChanges(see_msec)
        }

        if (api.navigation.activeControl() === "genreBox") {
            api.navigation.down()
        }

        // Got to Instruments
        api.navigation.nextPanel()
        api.autobot.seeChanges(see_msec)

        // Choose instrument
        var instrCount = api.autobot.randomInt(0, 20);
        api.log.debug("chooseRandomInstruments instrCount: " + instrCount)
        for (var j = 0; j < instrCount; j++) {
            api.navigation.down()
            api.autobot.seeChanges(see_msec)
        }

        if (api.navigation.activeControl() === "SearchInstruments") {
            api.navigation.down()
        }

        // Select
        api.navigation.triggerControl("NewScoreDialog", "SelectPanel", "Select")
    }

    // Done
    api.navigation.triggerControl("NewScoreDialog", "BottomPanel", "Done")
}

module.exports = {
    openNewScoreDialog: openNewScoreDialog,
    сhooseFluteAndPiano: сhooseFluteAndPiano,
    chooseRandomInstruments: chooseRandomInstruments
}
