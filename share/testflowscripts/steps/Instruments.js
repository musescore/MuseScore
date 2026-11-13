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

var Navigation = require("Navigation.js")

function isLayoutPanelOpened()
{
    var panels = api.navigation.panels("NavigationLeftPanel")
    for (var i = 0; i < panels.length; ++i) {
        if (panels[i].name === "InstrumentsHeader") {
            return panels[i].enabled
        }
    }
    return false
}

//! NOTE Triggering the tab again while it is being opened hides the panel, so trigger only if needed
function doOpenInstrumentsPanel()
{
    if (isLayoutPanelOpened()) {
        return
    }

    Navigation.triggerControl("NavigationLeftPanel", "PanelTabs", "Layout")

    for (var i = 0; i < 50 && !isLayoutPanelOpened(); ++i) {
        api.testflow.sleep(100)
    }

    if (!isLayoutPanelOpened()) {
        api.testflow.error("instruments: the Layout panel is not opened")
    }
}

function doGoToInstrument(name)
{
    Navigation.goToControl("NavigationLeftPanel", "LayoutPanelTree", name)
}

module.exports = {
    openInstrumentsPanel: doOpenInstrumentsPanel,

    openAddInstrumentsDialog: function()
    {
        doOpenInstrumentsPanel()
        //! NOTE "Add" opens a menu: new instrument or system markings
        Navigation.triggerControl("NavigationLeftPanel", "InstrumentsHeader", "Add")
        Navigation.goToControlInPopup("StyledMenu", "ADD_INSTRUMENT")
        //! NOTE The dialog is modal, so open it without waiting
        api.testflow.async(function() {
            api.navigation.trigger()
        })
    },

    chooseInstrument: function(family, instrument)
    {
        //! NOTE Only a triggered family switches the instruments list
        Navigation.triggerControl("DialogView", "FamilyView", family)
        //! NOTE The instruments list is rebuilt asynchronously, an instrument triggered meanwhile is added twice
        api.testflow.sleep(500)
        Navigation.goToControl("DialogView", "InstrumentsView", instrument)
        Navigation.trigger()
        //! NOTE The instrument is added to the selected list asynchronously
        if (!api.navigation.waitControl("DialogView", "ListView", instrument, 5000)) {
            api.testflow.error("instruments: not added to the selected list: " + instrument)
        }
    },

    doneAddInstruments: function()
    {
        Navigation.triggerControl("DialogView", "ButtonBox", "OK")
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
        Navigation.popupOpened()
    },

    closeSettingsInstrument: function()
    {
        Navigation.closePopupIfOpened()
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
