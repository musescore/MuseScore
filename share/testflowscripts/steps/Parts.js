/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
var engraving = require("MuseApi.Engraving")

const OPEN_PARTS_COMMAND = "command://notation/open-parts"

module.exports = {
    //! NOTE Opens the Parts dialog and presses "Open all".
    //! This creates a part for every instrument and opens each in its own tab.
    openAll: function()
    {
        api.dispatcher.dispatch(OPEN_PARTS_COMMAND)
        Navigation.goToControlInPopup("ButtonBox", "Open all")
        api.navigation.trigger()
        api.testflow.sleep(1000)
    },

    //! NOTE Switches to the main score tab (the first tab), so that
    //! `curScore` is the main score, not a part
    goToMainScore: function()
    {
        Navigation.triggerControl("NotationView", "NotationViewTabs", "NotationTab0")
        api.testflow.sleep(300)
    },

    //! NOTE The parts of the main score: [{title, score}]
    list: function()
    {
        var result = []
        var score = engraving.curScore
        if (!score) {
            return result
        }
        for (var i = 0; i < score.excerpts.length; ++i) {
            result.push({ title: score.excerpts[i].title, score: score.excerpts[i].partScore })
        }
        return result
    }
}
