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

chooseDefaultMode: function()
{
    api.navigation.triggerControl("NoteInputSection", "NoteInputBar", "note-input-steptime")
    api.autobot.waitPopup()
    // First item become automatically current, so just trigger
    api.navigation.trigger()
},

// "note-longa", "note-breve",
// "pad-note-1", "pad-note-2", "pad-note-4", "pad-note-8", "pad-note-16", "pad-note-32", "pad-note-64", "pad-note-128", "pad-note-256", "pad-note-512", "pad-note-1024"
// "pad-dot", "pad-dotdot", "pad-dot3", "pad-dot4", "pad-rest"
chooseNoteDuration: function(mode)
{
    api.navigation.triggerControl("NoteInputSection", "NoteInputBar", mode)
},

// "note-c", "note-d", "note-e", "note-f", "note-g", "note-a", "note-b"
putNote: function(note)
{
    api.dispatcher.dispatch(note)
}

}
