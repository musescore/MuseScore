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
#ifndef MU_ZERBERUS_CONTROLLERS_H
#define MU_ZERBERUS_CONTROLLERS_H

namespace mu::zerberus {
enum {
    CTRL_VOLUME             = 0x07,
    CTRL_PANPOT             = 0x0a,
    CTRL_EXPRESSION         = 0x0b,
    CTRL_SUSTAIN            = 0x40,

    CTRL_ALL_NOTES_OFF      = 0x7b,

    // special midi events are mapped to internal
    // controller
    //
    CTRL_PROGRAM   = 0x81,
};
}

#endif // MU_ZERBERUS_CONTROLLERS_H
