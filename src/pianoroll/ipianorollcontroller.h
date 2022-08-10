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
#ifndef MU_PIANOROLL_IPIANOROLLCONTROLLER_H
#define MU_PIANOROLL_IPIANOROLLCONTROLLER_H

#include "modularity/imoduleexport.h"
#include "async/notification.h"
#include "async/channel.h"

#include "notation/notationtypes.h"
#include "audio/audiotypes.h"
#include "actions/actiontypes.h"
#include "types/fraction.h"

namespace mu::pianoroll {
class IPianorollController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPianorollController)

public:
    virtual ~IPianorollController() = default;

    virtual int getNotes() const = 0;

    virtual async::Notification noteLayoutChanged() const = 0;

    virtual double xZoom() const = 0;
    virtual void setXZoom(double value) = 0;
    virtual int noteHeight() const = 0;
    virtual void setNoteHeight(int value) = 0;

    virtual int widthInTicks() const = 0;
    virtual engraving::Fraction widthInBeats() = 0;

    virtual async::Notification pitchHighlightChanged() const = 0;
    virtual bool isPitchHighlight(int pitch) const = 0;
    virtual void setPitchHighlight(int pitch, bool value) = 0;
};
}

#endif // MU_PIANOROLL_IPIANOROLLCONTROLLER_H
