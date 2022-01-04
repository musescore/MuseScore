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

#ifndef MU_AUDIO_FLUIDSYNTHCREATOR_H
#define MU_AUDIO_FLUIDSYNTHCREATOR_H

#include <unordered_map>

#include "async/asyncable.h"
#include "async/channel.h"
#include "modularity/ioc.h"
#include "system/ifilesystem.h"

#include "isynthresolver.h"
#include "fluidsynth.h"

namespace mu::audio::synth {
class FluidResolver : public ISynthResolver::IResolver, public async::Asyncable
{
    INJECT(audio, system::IFileSystem, fileSystem)
public:
    explicit FluidResolver(const io::paths& soundFontDirs, async::Channel<io::paths> sfDirsChanges);

    ISynthesizerPtr resolveSynth(const audio::TrackId trackId, const audio::AudioInputParams& params) const override;
    audio::AudioResourceMetaList resolveResources() const override;

    void refresh() override;

private:
    FluidSynthPtr createSynth(const audio::AudioResourceId& resourceId) const;
    void updateCaches(const std::string& fileExtension);

    io::paths m_soundFontDirs;
    std::unordered_map<AudioResourceId, io::path> m_resourcesCache;
};
}

#endif // MU_AUDIO_FLUIDSYNTHCREATOR_H
