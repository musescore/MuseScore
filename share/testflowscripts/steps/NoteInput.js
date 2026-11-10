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
var Commands = require("Commands.js")

function triggerNoteInputBtn(btn)
{
    Navigation.triggerControl("NoteInputSection", "NoteInputBar", btn)
    //! NOTE NoteInputBar buttons handlers are asynchronous (Qt.callLater, which also merges calls
    //! made before the next event loop iteration), so wait until the action is done.
    api.testflow.sleep(100)
}

module.exports = {

    chooseDefaultMode: function()
    {
        triggerNoteInputBtn(Commands.TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND)
    },

    turnoffDefaultMode: function()
    {
        triggerNoteInputBtn(Commands.TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND)
    },

    // Commands.SET_DURATION_*_COMMAND, Commands.TOGGLE_REST_COMMAND
    chooseNoteDuration: function(mode)
    {
        triggerNoteInputBtn(mode)
    },

    // Commands.TOGGLE_DOT_COMMAND
    toggleDot: function(dot)
    {
        triggerNoteInputBtn(dot)
    },

    // Commands.TOGGLE_FLAT2_COMMAND, ..._FLAT_, ..._NAT_, ..._SHARP_, ..._SHARP2_
    toggleAccidental: function(acc)
    {
        triggerNoteInputBtn(acc)
    },

    putTie: function()
    {
        triggerNoteInputBtn(Commands.TOGGLE_TIE_COMMAND)
    },

    toggleSlur: function()
    {
        triggerNoteInputBtn(Commands.ADD_SLUR_COMMAND)
    },

    // Commands.TOGGLE_MARCATO_COMMAND, ..._SFORZATO_, ..._TENUTO_, ..._STACCATO_
    toggleArticulation: function(art)
    {
        triggerNoteInputBtn(art)
    },

    putTuplet: function(count)
    {
        Navigation.triggerControl("NoteInputSection", "NoteInputBar", "tuplets-subitems")
        api.testflow.waitPopup()
        var steps = count - 2
        for (var i = 0; i < steps; i++) {
            api.navigation.down()
        }
        api.navigation.trigger()
    },

    // Commands.ENTER_NOTE_C_COMMAND ... Commands.ENTER_NOTE_B_COMMAND
    putNote: function(note)
    {
        api.dispatcher.dispatch(note)
    },

    putNotes: function(note, count)
    {
        for (var i = 0; i < count; i++) {
            api.dispatcher.dispatch(note)
        }
    }
}
