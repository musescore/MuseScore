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

function doOpenInstrumentsPanel()
{
    api.navigation.triggerControl("NavigationLeftPanel", "PanelTabs", "Instruments")
}

function doGoToInstrument(name)
{
    api.navigation.goToControl("NavigationLeftPanel", "InstrumentsTree", name)
}

module.exports = {
    openInstrumentsPanel: doOpenInstrumentsPanel,

    openAddInstrumentsDialog: function()
    {
        doOpenInstrumentsPanel()
        api.navigation.goToControl("NavigationLeftPanel", "InstrumentsHeader", "Add")
        api.autobot.seeChanges()
        api.autobot.async(function() {
            api.navigation.trigger()
        })
    },

    сhooseInstrument: function(family, instrument)
    {
        api.navigation.goToControl("DialogView", "FamilyView", family)
        api.navigation.goToControl("DialogView", "InstrumentsView", instrument)
        api.navigation.trigger()
    },

    doneAddInstruments: function()
    {
        api.navigation.triggerControl("DialogView", "BottomPanel", "OK")
    },

    goToInstrument: doGoToInstrument,

    toggleVisibleInstrument: function(instrument)
    {
        doGoToInstrument(instrument)
        api.navigation.right()
        api.navigation.trigger()
    },

    openSettingsInstrument: function(instrument)
    {
        doGoToInstrument(instrument)
        api.navigation.left()
        api.navigation.trigger()
    },

    moveDownInstrument: function(instrument)
    {
        doGoToInstrument(instrument)
        api.navigation.trigger() // select instrument
        api.navigation.triggerControl("NavigationLeftPanel", "InstrumentsHeader", "Down")

    },

    moveUpInstrument: function(instrument)
    {
        doGoToInstrument(instrument)
        api.navigation.trigger() // select instrument
        api.navigation.triggerControl("NavigationLeftPanel", "InstrumentsHeader", "Up")

    },

    removeInstrument: function(instrument)
    {
        doGoToInstrument(instrument)
        api.navigation.trigger() // select instrument
        api.navigation.triggerControl("NavigationLeftPanel", "InstrumentsHeader", "Remove")
    }
}
