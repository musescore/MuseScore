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
#ifndef MU_NOTATION_INOTATIONPLAYBACK_H
#define MU_NOTATION_INOTATIONPLAYBACK_H

#include <QRect>
#include "retval.h"
#include "midi/miditypes.h"
#include "notationtypes.h"

#include "notationtypes.h"

namespace mu::notation {
class INotationPlayback
{
public:
    virtual ~INotationPlayback() = default;

    virtual std::shared_ptr<midi::MidiStream> midiStream() const = 0;

    virtual QTime totalPlayTime() const = 0;

    virtual float tickToSec(int tick) const = 0;
    virtual int secToTick(float sec) const = 0;

    virtual QRect playbackCursorRectByTick(int tick) const = 0;

    virtual RetVal<int> playPositionTick() const = 0;
    virtual void setPlayPositionTick(int tick) = 0;
    virtual bool setPlayPositionByElement(const Element* element) = 0;
    virtual async::Channel<int> playPositionTickChanged() const = 0;

    virtual midi::MidiData playElementMidiData(const Element* element) const = 0;

    enum BoundaryTick : int {
        FirstScoreTick = -3,
        SelectedNoteTick,
        LastScoreTick
    };

    virtual void addLoopBoundary(LoopBoundaryType boundaryType, int tick) = 0;
    virtual void setLoopBoundariesVisible(bool visible) = 0;
    virtual ValCh<LoopBoundaries> loopBoundaries() const = 0;

    virtual Tempo tempo(int tick) const = 0;
    virtual MeasureBeat beat(int tick) const = 0;
    virtual int beatToTick(int measureIndex, int beatIndex) const = 0;
};

using INotationPlaybackPtr = std::shared_ptr<INotationPlayback>;
}

#endif // MU_NOTATION_INOTATIONPLAYBACK_H
