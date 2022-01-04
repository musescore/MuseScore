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
#ifndef MU_AUDIO_AUDIOENGINE_H
#define MU_AUDIO_AUDIOENGINE_H

#include <memory>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "retval.h"

#include "iaudiodriver.h"
#include "internal/worker/mixer.h"
#include "internal/iaudiobuffer.h"

namespace mu::audio {
class AudioEngine : public async::Asyncable
{
public:
    ~AudioEngine();

    static AudioEngine* instance();

    Ret init(IAudioBufferPtr bufferPtr);
    void deinit();

    void setSampleRate(unsigned int sampleRate);
    void setReadBufferSize(uint16_t readBufferSize);
    void setAudioChannelsCount(const audioch_t count);

    MixerPtr mixer() const;

private:
    AudioEngine();

    bool m_inited = false;

    MixerPtr m_mixer = nullptr;
    IAudioBufferPtr m_buffer = nullptr;
};
}

#endif // MU_AUDIO_AUDIOENGINE_H
