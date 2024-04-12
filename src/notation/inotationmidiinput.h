/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_NOTATION_INOTATIONMIDIINPUT_H
#define MU_NOTATION_INOTATIONMIDIINPUT_H

#include "async/notification.h"
#include "midi/miditypes.h"

#include "notationtypes.h"

namespace mu::notation {
class INotationMidiInput
{
public:
    virtual ~INotationMidiInput() = default;

    virtual void onMidiEventReceived(const muse::midi::Event& event) = 0;
    virtual muse::async::Channel<std::vector<const Note*> > notesReceived() const = 0;

    virtual void onRealtimeAdvance() = 0;
};

using INotationMidiInputPtr = std::shared_ptr<INotationMidiInput>;
}

#endif // MU_NOTATION_INOTATIONMIDIINPUT_H
