/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MU_NOTATION_NOTATIONSOLOMUTESTATE_H
#define MU_NOTATION_NOTATIONSOLOMUTESTATE_H

#include "../inotationsolomutestate.h"
#include "async/asyncable.h"

namespace mu::notation {
class NotationSoloMuteState : public INotationSoloMuteState, public async::Asyncable
{
public:
    Ret read(const engraving::MscReader& reader, const io::path_t& pathPrefix = "") override;
    Ret write(io::IODevice* out) override;

    bool trackSoloMuteStateExists(const engraving::InstrumentTrackId& trackId) const override;
    SoloMuteState trackSoloMuteState(const engraving::InstrumentTrackId& trackId) const override;
    void setTrackSoloMuteState(const engraving::InstrumentTrackId& trackId, const SoloMuteState& state) override;
    void removeTrackSoloMuteState(const engraving::InstrumentTrackId& trackId) override;
    async::Channel<engraving::InstrumentTrackId, SoloMuteState> trackSoloMuteStateChanged() const override;

private:
    std::unordered_map<engraving::InstrumentTrackId, SoloMuteState> m_trackSoloMuteStatesMap;
    async::Channel<engraving::InstrumentTrackId, SoloMuteState> m_trackSoloMuteStateChanged;
};
}

#endif // MU_NOTATION_NOTATIONSOLOMUTESTATE_H
