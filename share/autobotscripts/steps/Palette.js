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

var palettes = ["Clefs", "Key signatures", "Time signatures"]

//! NOTE Currently, position-based navigation
function doGoToPalette(name)
{
    api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", [10000,0]) // first index
    var idx = palettes.indexOf(name)
    for (var i = 0; i < idx; i++) {
        api.navigation.down()
    }
}

module.exports = {

    goToPalette: function(name)
    {
        doGoToPalette(name)
    },

    togglePalette: function(name) {
        doGoToPalette(name)
        api.navigation.right()
        api.navigation.trigger()
    }
}
