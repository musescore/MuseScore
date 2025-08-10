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
#ifndef MUSE_AUDIO_SOUNDFONTREPOSITORY_H
#define MUSE_AUDIO_SOUNDFONTREPOSITORY_H

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "audio/common/rpc/irpcchannel.h"

#include "../../isoundfontrepository.h"

namespace muse::audio::synth {
class SoundFontRepository : public ISoundFontRepository, public Injectable, public async::Asyncable
{
    Inject<rpc::IRpcChannel> channel;

public:
    SoundFontRepository(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    const SoundFontPaths& soundFontPaths() const override;
    const SoundFontsMap& soundFonts() const override;
    async::Notification soundFontsChanged() const override;

private:

    void addSoundFont(const SoundFontPath& path);
    void loadSoundFonts(const SoundFontPaths& paths);

    SoundFontPaths m_soundFontPaths;
    SoundFontsMap m_soundFonts;
    async::Notification m_soundFontsChanged;
};
}

#endif // MUSE_AUDIO_SOUNDFONTREPOSITORY_H
