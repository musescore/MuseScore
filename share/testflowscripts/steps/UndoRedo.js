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

var ScoreState = require("ScoreState.js")

var UNDO_COMMAND = "command://notation/undo"
var REDO_COMMAND = "command://notation/redo"

//! NOTE A safety limit, so a broken undo stack can't hang the test
var MAX_STEPS = 2000

//! NOTE How long to let the event loop run after each undo/redo
var STEP_WAIT_MS = 20

function runAll(command, what)
{
    var steps = 0
    while (api.testflow.isCommandEnabled(command)) {
        if (steps >= MAX_STEPS) {
            api.testflow.fatal(what + ": still enabled after " + MAX_STEPS + " steps")
            return steps
        }
        api.dispatcher.dispatch(command)
        //! NOTE dispatch() only queues the command; it runs on the next
        //! event loop pass. Let it run before checking the enabled state again.
        api.testflow.sleep(STEP_WAIT_MS)
        steps++
    }
    api.log.info("[testflow] " + what + ": " + steps + " steps")
    return steps
}

module.exports = {
    undoAll: function() { return runAll(UNDO_COMMAND, "undo all") },
    redoAll: function() { return runAll(REDO_COMMAND, "redo all") },

    //! NOTE Undoes everything and checks the score is back to `initial`,
    //! then redoes everything and checks the score is back to `final`.
    //! A crash during undo or redo fails the test case too.
    //! Must run before the score is closed: closing clears the undo history.
    fullCycle: function(initial, final)
    {
        runAll(UNDO_COMMAND, "undo all")
        api.testflow.seeChanges()
        ScoreState.expectEquals(initial, "after undo all")

        runAll(REDO_COMMAND, "redo all")
        api.testflow.seeChanges()
        ScoreState.expectEquals(final, "after redo all")
    }
}
