/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_MIDI_MIDISTUBMODULE_H
#define MU_MIDI_MIDISTUBMODULE_H

#include "modularity/imodulesetup.h"
#include "async/asyncable.h"
#include "async/channel.h"
#include "framework/midi/miditypes.h"

namespace mu::midi {
class MidiModule : public modularity::IModuleSetup
{
public:
    MidiModule() = default;

    std::string moduleName() const override;
    void registerExports() override;
    mu::async::Channel<tick_t, mu::midi::Event>* getMidiInputQueue();
};
}

#endif // MU_MIDI_MIDISTUBMODULE_H
