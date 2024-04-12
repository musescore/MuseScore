/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#ifndef MU_NOTATION_INOTATIONSOLOMUTESTATE_H
#define MU_NOTATION_INOTATIONSOLOMUTESTATE_H

#include "engraving/infrastructure/mscreader.h"
#include "io/iodevice.h"

#include "notationtypes.h"

namespace mu::notation {
class INotationSoloMuteState
{
public:
    struct SoloMuteState {
        bool mute = false;
        bool solo = false;

        bool operator ==(const SoloMuteState& other) const
        {
            return mute == other.mute
                   && solo == other.solo;
        }
    };

    virtual ~INotationSoloMuteState() = default;

    virtual muse::Ret read(const engraving::MscReader& reader, const muse::io::path_t& pathPrefix = "") = 0;
    virtual muse::Ret write(muse::io::IODevice* out) = 0;

    virtual bool trackSoloMuteStateExists(const engraving::InstrumentTrackId& trackId) const = 0;
    virtual SoloMuteState trackSoloMuteState(const engraving::InstrumentTrackId& trackId) const = 0;
    virtual void setTrackSoloMuteState(const engraving::InstrumentTrackId& trackId, const SoloMuteState& state) = 0;
    virtual void removeTrackSoloMuteState(const engraving::InstrumentTrackId& trackId) = 0;
    virtual muse::async::Channel<engraving::InstrumentTrackId, SoloMuteState> trackSoloMuteStateChanged() const = 0;
};

using INotationSoloMuteStatePtr = std::shared_ptr<INotationSoloMuteState>;
}

#endif // MU_NOTATION_INOTATIONSOLOMUTESTATE_H
