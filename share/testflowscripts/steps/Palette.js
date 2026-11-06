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

const MAX_TREE_STEPS = 300

//! NOTE The palettes tree creates items lazily (only visible ones),
//! so a palette scrolled out of view has no navigation control.
//! Walk the tree from the top as a user would do with the keyboard, it scrolls the tree.
function findInTree(name)
{
    //! NOTE A miss is expected here, the palette may not be added
    if (api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", name)) {
        return true
    }

    //! NOTE Start from the topmost created item
    var controls = api.navigation.controls("NavigationLeftPanel", "PalettesTree")
    var top = null
    for (var c = 0; c < controls.length; ++c) {
        var idx = controls[c].index
        if (!top || idx.row < top.row || (idx.row === top.row && idx.column < top.column)) {
            top = idx
        }
    }

    if (!top || !api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", [top.row, top.column])) {
        return false
    }

    for (var i = 0; i < MAX_TREE_STEPS; ++i) {
        if (api.navigation.activeControl() === name) {
            return true
        }

        var prev = api.navigation.activeControl()
        api.navigation.down()
        if (api.navigation.activeControl() === prev) {
            return false // the end of the tree
        }
    }

    return false
}

function doGoToPalette(name, addPalette)
{
    var ok = findInTree(name)
    if (!ok && addPalette) {
        Navigation.triggerControl("NavigationLeftPanel", "PalettesHeader", "AddPalettesBtn")
        Navigation.triggerControlInPopup("AddPalettesPopup", "Add"+name+"Palette")

        ok = findInTree(name)
    }

    if (!ok) {
        api.testflow.error("palette error: not found palette: " + name)
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
