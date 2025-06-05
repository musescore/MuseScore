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

#include "global/modularity/ioc.h"
#include "global/iinteractive.h"
#include "global/io/ifilesystem.h"
#include "global/async/asyncable.h"

#include "isoundfontrepository.h"
#include "iaudioconfiguration.h"

namespace muse::audio {
class SoundFontRepository : public ISoundFontRepository, public Injectable, public async::Asyncable
{
    Inject<IInteractive> interactive = { this };
    Inject<IAudioConfiguration> configuration = { this };
    Inject<io::IFileSystem> fileSystem = { this };

public:
    SoundFontRepository(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    const synth::SoundFontPaths& soundFontPaths() const override;
    const synth::SoundFontsMap& soundFonts() const override;
    async::Notification soundFontsChanged() const override;

    void addSoundFont(const synth::SoundFontPath& path) override;

private:

    Ret doAddSoundFont(const synth::SoundFontPath& src, const synth::SoundFontPath& dst);

    void loadSoundFonts();
    void loadSoundFont(const synth::SoundFontPath& path, const synth::SoundFontsMap& oldSoundFonts = {});

    RetVal<synth::SoundFontPath> resolveInstallationPath(const synth::SoundFontPath& path) const;

    synth::SoundFontPaths m_soundFontPaths;
    synth::SoundFontsMap m_soundFonts;
    async::Notification m_soundFontsChanged;
};
}

#endif // MUSE_AUDIO_SOUNDFONTREPOSITORY_H
