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

module.exports = {
    nextChord: function()
    {
        api.navigation.goToControl("NotationView", "ScoreView", "Score")
        api.shortcuts.activate("Right")
    },

    prevChord: function()
    {
        api.navigation.goToControl("NotationView", "ScoreView", "Score")
        api.shortcuts.activate("Left")
    },

    nextMeasure: function()
    {
        api.navigation.goToControl("NotationView", "ScoreView", "Score")
        api.shortcuts.activate("Ctrl+Right")
    },

    prevMeasure: function()
    {
        api.navigation.goToControl("NotationView", "ScoreView", "Score")
        api.shortcuts.activate("Ctrl+Left")
    },

    appendMeasures: function(count)
    {
        api.navigation.goToControl("NotationView", "ScoreView", "Score")
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
        api.navigation.goToControl("NotationView", "ScoreView", "Score")
        for (var i = 0; i < count; ++i) {
            api.shortcuts.activate("Down")
        }
    },

    pitchUp: function(count)
    {
        count = count ?? 1;
        api.navigation.goToControl("NotationView", "ScoreView", "Score")
        for (var i = 0; i < count; ++i) {
            api.shortcuts.activate("Up")
        }
    },
}
