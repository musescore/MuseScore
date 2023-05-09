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
#ifndef MU_PIANOROLL_IPIANOROLLCONFIGURATION_H
#define MU_PIANOROLL_IPIANOROLLCONFIGURATION_H

#include "modularity/imoduleexport.h"

namespace mu::pianoroll {
class IPianorollConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPianorollConfiguration)

public:
    virtual ~IPianorollConfiguration() = default;

//    virtual bool playNotesWhenEditing() const = 0;
//    virtual void setPlayNotesWhenEditing(bool value) = 0;

//    virtual bool playChordWhenEditing() const = 0;
//    virtual void setPlayChordWhenEditing(bool value) = 0;

//    virtual bool playHarmonyWhenEditing() const = 0;
//    virtual void setPlayHarmonyWhenEditing(bool value) = 0;

//    virtual PlaybackCursorType cursorType() const = 0;
};
}

#endif // MU_PIANOROLL_IPIANOROLLCONFIGURATION_H
