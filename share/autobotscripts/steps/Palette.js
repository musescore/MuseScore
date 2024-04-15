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

function doGoToPalette(name, addPalette)
{
    api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", name)
    var ok = api.navigation.activeControl() === name
    if (!ok && addPalette) {
        api.navigation.triggerControl("NavigationLeftPanel", "PalettesHeader", "AddPalettesBtn")
        api.autobot.waitPopup()
        api.navigation.triggerControl("NavigationLeftPanel", "AddPalettesPopup", "Add"+name+"Palette")
        api.autobot.waitPopup() // wait close

        api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", name)
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
