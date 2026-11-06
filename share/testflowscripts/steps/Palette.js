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

function doGoToPalette(name, addPalette)
{
    //! NOTE Probe whether the palette is already added, a miss is expected here
    api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", name)
    var ok = api.navigation.activeControl() === name
    if (!ok && addPalette) {
        Navigation.triggerControl("NavigationLeftPanel", "PalettesHeader", "AddPalettesBtn")
        api.testflow.waitPopup()
        Navigation.triggerControl("NavigationLeftPanel", "AddPalettesPopup", "Add"+name+"Palette")
        api.testflow.waitPopup() // wait close

        Navigation.goToControl("NavigationLeftPanel", "PalettesTree", name)
    }
}

module.exports = {

    goToPalette: function(name)
    {
        doGoToPalette(name, true)
    },

    togglePalette: function(name) {
        doGoToPalette(name, true)
        api.navigation.right()
        api.navigation.trigger()
    }
}
