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

#ifndef MU_AUDIO_IAUDIOPLAYER_H
#define MU_AUDIO_IAUDIOPLAYER_H

#include <memory>
#include "iplayer.h"
#include "iaudiosource.h"
#include "iaudiostream.h"
#include "ret.h"

namespace mu::audio {
class IAudioPlayer : public IPlayer
{
public:
    ~IAudioPlayer() = default;

    virtual void unload() = 0;
    virtual Ret load(const std::shared_ptr<audio::IAudioStream>& stream) = 0;

    virtual IAudioSourcePtr audioSource() = 0;
};
}
#endif // MU_AUDIO_IAUDIOPLAYER_H
