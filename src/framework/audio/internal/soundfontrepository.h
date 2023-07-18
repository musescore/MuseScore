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
#ifndef MU_AUDIO_SOUNDFONTREPOSITORY_H
#define MU_AUDIO_SOUNDFONTREPOSITORY_H

#include "audio/isoundfontrepository.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "audio/iaudioconfiguration.h"
#include "io/ifilesystem.h"
#include "async/asyncable.h"

namespace mu::audio {
class SoundFontRepository : public ISoundFontRepository, public async::Asyncable
{
    INJECT(framework::IInteractive, interactive)
    INJECT(IAudioConfiguration, configuration)
    INJECT(io::IFileSystem, fileSystem)

public:
    void init();

    synth::SoundFontPaths soundFontPaths() const override;
    async::Notification soundFontPathsChanged() const override;

    mu::Ret addSoundFont(const synth::SoundFontPath& path) override;

private:
    void loadSoundFontPaths();

    mu::RetVal<synth::SoundFontPath> resolveInstallationPath(const synth::SoundFontPath& path) const;

    synth::SoundFontPaths m_soundFontPaths;
    mu::async::Notification m_soundFontPathsChanged;
};
}

#endif // MU_AUDIO_SOUNDFONTREPOSITORY_H
