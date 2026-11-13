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

//! NOTE The active control changes asynchronously after a navigation move
function waitActiveChanged(prev)
{
    for (var i = 0; i < 10; ++i) {
        if (api.navigation.activeControl() !== prev) {
            return true
        }
        api.testflow.sleep(30)
    }
    return false
}

//! NOTE Moves through the tree while the active control changes, stops at the end or on the name
function walkTree(move, name)
{
    for (var i = 0; i < MAX_TREE_STEPS; ++i) {
        if (name && api.navigation.activeControl() === name) {
            return true
        }

        var prev = api.navigation.activeControl()
        move()
        if (!waitActiveChanged(prev)) {
            return false // the end of the tree
        }
    }

    return false
}

//! NOTE The palettes tree creates items lazily (only visible ones),
//! so a palette scrolled out of view has no navigation control.
//! Walk the tree as a user would do with the keyboard, it scrolls the tree.
function findInTree(name)
{
    //! NOTE A miss is expected here, the palette may not be added
    if (api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", name)) {
        return true
    }

    //! NOTE Start from any created palette header (column 0, the cells are in the next columns)
    var controls = api.navigation.controls("NavigationLeftPanel", "PalettesTree")
    var start = null
    for (var c = 0; c < controls.length; ++c) {
        if (controls[c].index.column === 0 && controls[c].enabled) {
            start = controls[c].index
            break
        }
    }

    if (!start || !api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", [start.row, start.column])) {
        return false
    }

    //! NOTE Go up to the top of the tree, then down to its end
    if (walkTree(api.navigation.up, name)) {
        return true
    }

    return walkTree(api.navigation.down, name)
}

//! NOTE The add palettes popup lists only the palettes that are not in the tree,
//! so it is a quick check instead of walking the whole tree. Returns false if the palette is in the tree
function addPaletteIfAbsent(name)
{
    Navigation.triggerControl("NavigationLeftPanel", "PalettesHeader", "AddPalettesBtn")
    var section = Navigation.popupOpened("AddPalettesPopup")

    var control = "Add" + name + "Palette"
    if (!Navigation.hasControl(section, "AddPalettesPopup", control)) {
        Navigation.closePopupIfOpened()
        return false
    }

    Navigation.goToControl(section, "AddPalettesPopup", control)
    api.navigation.trigger()
    Navigation.closePopupIfOpened()
    return true
}

function doGoToPalette(name, addPalette)
{
    //! NOTE A miss is expected here, the palette may not be added or may be scrolled out of view
    if (api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", name)) {
        return
    }

    if (addPalette && addPaletteIfAbsent(name)) {
        //! NOTE The added palette is scrolled into view
        if (api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", name, 2000)) {
            return
        }
    }

    if (!findInTree(name)) {
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
