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
#ifndef MU_AUDIO_ISOUNDFONTSPROVIDER_H
#define MU_AUDIO_ISOUNDFONTSPROVIDER_H

#include <vector>
#include <memory>

#include "modularity/imoduleexport.h"
#include "async/promise.h"

#include "synthtypes.h"
#include "audiotypes.h"

namespace mu::audio::synth {
class ISoundFontsProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISoundFontsProvider)
public:
    virtual ~ISoundFontsProvider() = default;

    virtual void refreshPaths() = 0;
    virtual async::Promise<SoundFontPaths> soundFontPaths(SoundFontFormats formats) const = 0;
};

using ISoundFontsProviderPtr = std::shared_ptr<ISoundFontsProvider>;
}

#endif // MU_AUDIO_ISOUNDFONTSPROVIDER_H
