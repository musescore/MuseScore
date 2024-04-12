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

var Navigation = require("Navigation.js")

function doOpenInstrumentsPanel()
{
    Navigation.triggerControl("NavigationLeftPanel", "PanelTabs", "Instruments")
}

function doGoToInstrument(name)
{
    Navigation.goToControl("NavigationLeftPanel", "InstrumentsTree", name)
}

module.exports = {
    openInstrumentsPanel: doOpenInstrumentsPanel,

    openAddInstrumentsDialog: function()
    {
        doOpenInstrumentsPanel()
        Navigation.goToControl("NavigationLeftPanel", "InstrumentsHeader", "Add")
        api.autobot.seeChanges()
        api.autobot.async(function() {
            api.navigation.trigger()
        })
    },

    chooseInstrument: function(family, instrument)
    {
        Navigation.goToControl("DialogView", "FamilyView", family)
        Navigation.goToControl("DialogView", "InstrumentsView", instrument)
        Navigation.trigger()
    },

    doneAddInstruments: function()
    {
        Navigation.triggerControl("DialogView", "BottomPanel", "OK")
    },

    goToInstrument: doGoToInstrument,

    toggleVisibleInstrument: function(instrument)
    {
        doGoToInstrument(instrument)
        Navigation.right()
        Navigation.trigger()
    },

    openSettingsInstrument: function(instrument)
    {
        doGoToInstrument(instrument)
        Navigation.left()
        Navigation.trigger()
    },

    moveDownInstrument: function(instrument)
    {
        doGoToInstrument(instrument)
        Navigation.trigger() // select instrument
        Navigation.triggerControl("NavigationLeftPanel", "InstrumentsHeader", "Down")

    },

    moveUpInstrument: function(instrument)
    {
        doGoToInstrument(instrument)
        Navigation.trigger() // select instrument
        Navigation.triggerControl("NavigationLeftPanel", "InstrumentsHeader", "Up")

    },

    removeInstrument: function(instrument)
    {
        doGoToInstrument(instrument)
        Navigation.trigger() // select instrument
        Navigation.triggerControl("NavigationLeftPanel", "InstrumentsHeader", "Remove")
    }
}
