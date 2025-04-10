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

function doFocusIn()
{
    api.autobot.showMainWindowOnFront()
    api.autobot.sleep(100)
    api.navigation.goToControl("NotationView", "ScoreView", "Score")
}

module.exports = {
    focusIn: doFocusIn,

    firstElement: function()
    {
        doFocusIn()
        api.shortcuts.activate("Ctrl+Home")
    },

    nextChord: function()
    {
        doFocusIn()
        api.shortcuts.activate("Right")
    },

    prevChord: function()
    {
        doFocusIn()
        api.shortcuts.activate("Left")
    },

    nextMeasure: function()
    {
        doFocusIn()
        api.shortcuts.activate("Ctrl+Right")
    },

    prevMeasure: function()
    {
        doFocusIn()
        api.shortcuts.activate("Ctrl+Left")
    },

    appendMeasures: function(count)
    {
        doFocusIn()
        for (var i = 0; i < count; i++) {
            api.shortcuts.activate("Ctrl+B")
            if (i%10 == 0) {
                api.autobot.sleep(1)
            }
        }
    },

    pitchDown: function(count)
    {
        count = count ?? 1;
        doFocusIn()
        for (var i = 0; i < count; ++i) {
            api.shortcuts.activate("Down")
        }
    },

    pitchUp: function(count)
    {
        count = count ?? 1;
        doFocusIn()
        for (var i = 0; i < count; ++i) {
            api.shortcuts.activate("Up")
        }
    },
}
