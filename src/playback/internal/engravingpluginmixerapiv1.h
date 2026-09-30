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

#pragma once

#include <optional>

#include "async/asyncable.h"
#include "engraving/iengravingpluginmixerapiv1.h"
#include "modularity/ioc.h"
#include "audio/main/iplayback.h"
#include "context/iglobalcontext.h"
#include "iplaybackcontroller.h"

namespace mu::playback {
class EngravingPluginMixerApi : public engraving::IEngravingPluginMixerApi, public muse::Contextable, public muse::async::Asyncable
{
    muse::ContextInject<muse::audio::IPlayback> playback = { this };
    muse::ContextInject<IPlaybackController> controller = { this };
    muse::ContextInject<context::IGlobalContext> context = { this };

public:
    explicit EngravingPluginMixerApi(const muse::modularity::ContextPtr& ctx)
        : muse::Contextable(ctx) {}

    float volume(const engraving::InstrumentTrackId& trackId) const override;
    void setVolume(const engraving::InstrumentTrackId& trackId, float volume) override;
    float balance(const engraving::InstrumentTrackId& trackId) const override;
    void setBalance(const engraving::InstrumentTrackId& trackId, float balance) override;
    bool muted(const engraving::InstrumentTrackId& trackId) const override;
    void setMuted(const engraving::InstrumentTrackId& trackId, bool muted) override;
    bool solo(const engraving::InstrumentTrackId& trackId) const override;
    void setSolo(const engraving::InstrumentTrackId& trackId, bool solo) override;

    muse::async::Promise<QVariantList> availableSounds(const engraving::InstrumentTrackId& trackId) const override;
    muse::async::Promise<bool> setSound(const engraving::InstrumentTrackId& trackId, const QString& soundId) override;

private:
    std::optional<muse::audio::TrackId> audioTrackId(const engraving::InstrumentTrackId& trackId) const;
};
}
