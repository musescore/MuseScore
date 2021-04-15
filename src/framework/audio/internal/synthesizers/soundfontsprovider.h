/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_AUDIO_SOUNDFONTSPROVIDER_H
#define MU_AUDIO_SOUNDFONTSPROVIDER_H

#include <map>
#include <mutex>

#include "isoundfontsprovider.h"

#include "modularity/ioc.h"
#include "iaudioconfiguration.h"
#include "isynthesizersregister.h"
#include "iglobalconfiguration.h"
#include "system/ifilesystem.h"
#include "async/asyncable.h"

namespace mu::audio::synth {
class SoundFontsProvider : public ISoundFontsProvider, public async::Asyncable
{
    INJECT(audio, IAudioConfiguration, configuration)
    INJECT(audio, ISynthesizersRegister, synthRegister)
    INJECT(audio, system::IFileSystem, fileSystem)

public:

    std::vector<io::path> soundFontPathsForSynth(const SynthName& synthName) const override;
    async::Notification soundFontPathsForSynthChanged(const SynthName& synth) const override;

    std::vector<io::path> soundFontPaths(SoundFontFormats formats) const override;

private:
    mutable std::mutex m_mutex;
    mutable std::map<SynthName, async::Notification > m_soundFontPathsForSynthChangedMap;
};
}

#endif // MU_AUDIO_SOUNDFONTSPROVIDER_H
