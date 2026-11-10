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
var engraving = require("MuseApi.Engraving")

function doFocusIn()
{
    api.testflow.showMainWindowOnFront()
    api.testflow.sleep(100)
    Navigation.goToControl("NotationView", "ScoreView", "Score")
}

//! NOTE The single selected note or null
function selectedNote()
{
    var score = engraving.curScore
    if (!score) {
        return null
    }

    var elements = score.selection.elements
    if (elements.length !== 1 || elements[0].type !== engraving.Element.NOTE) {
        return null
    }

    return elements[0]
}

//! NOTE Moves the selection by the shortcut until a note (with the pitch, if given) is selected
function isNoteSelected(pitch)
{
    var note = selectedNote()
    return note !== null && (pitch === undefined || note.pitch === pitch)
}

function findNote(shortcut, pitch, maxSteps)
{
    for (var i = 0; i < maxSteps; ++i) {
        if (isNoteSelected(pitch)) {
            return true
        }

        api.shortcuts.activate(shortcut)
        api.testflow.sleep(50)
    }

    return isNoteSelected(pitch)
}

module.exports = {
    focusIn: doFocusIn,
    selectedNote: selectedNote,
    findNote: findNote,

    firstElement: function()
    {
        doFocusIn()
        api.shortcuts.activate("Ctrl+Home")
    },

    lastElement: function()
    {
        doFocusIn()
        api.shortcuts.activate("Ctrl+End")
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
                api.testflow.sleep(1)
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
