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

var Commands = require("Commands.js")

// A command that is enabled only when a project is opened
const HAS_PROJECT_COMMAND = Commands.PROJECT_PROPERTIES_COMMAND

module.exports = {
    isOpened: function()
    {
        return api.testflow.isCommandEnabled(HAS_PROJECT_COMMAND)
    },

    //! NOTE Closing without an opened project quits the application,
    //! so close only if there is something to close
    closeIfOpened: function()
    {
        if (api.testflow.isCommandEnabled(HAS_PROJECT_COMMAND)) {
            api.dispatcher.dispatch(Commands.PROJECT_CLOSE_COMMAND)
        }
    }
}
