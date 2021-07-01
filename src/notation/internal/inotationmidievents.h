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

#ifndef MU_NOTATION_INOTATIONMIDIDATA_H
#define MU_NOTATION_INOTATIONMIDIDATA_H

#include <memory>

#include "retval.h"
#include "midi/miditypes.h"

#include "notation/notationtypes.h"
#include "notation/internal/igetscore.h"

namespace mu::notation {
class INotationMidiEvents
{
public:
    virtual ~INotationMidiEvents() = default;

    virtual void init() = 0;
    virtual midi::Events retrieveEvents(const midi::channel_t midiChannel, const midi::tick_t fromTick,
                                        const midi::tick_t toTick) const = 0;
    virtual midi::Events retrieveEventsForElement(const Element* element, const midi::channel_t midiChannel) const = 0;
    virtual std::vector<midi::Event> retrieveSetupEvents(const std::list<InstrumentChannel*> instrChannel) const = 0;
};

using INotationMidiEventsPtr = std::shared_ptr<INotationMidiEvents>;
}

#endif // MU_NOTATION_INOTATIONMIDIDATA_H
