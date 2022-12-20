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
#include "async/notification.h"
#include "types/retval.h"

#include "../../iaudiodriver.h"
#include "internal/worker/mixer.h"

namespace mu::audio {
class AudioBuffer;
class AudioEngine : public async::Asyncable
{
public:
    ~AudioEngine();

    static AudioEngine* instance();

    Ret init(std::shared_ptr<AudioBuffer> bufferPtr);
    void deinit();

    sample_rate_t sampleRate() const;

    void setSampleRate(unsigned int sampleRate);
    void setReadBufferSize(uint16_t readBufferSize);
    void setAudioChannelsCount(const audioch_t count);

    RenderMode mode() const;
    void setMode(const RenderMode newMode);
    async::Notification modeChanged() const;

    MixerPtr mixer() const;

private:
    AudioEngine();

    bool m_inited = false;

    sample_rate_t m_sampleRate = 0;

    MixerPtr m_mixer = nullptr;
    std::shared_ptr<AudioBuffer> m_buffer = nullptr;

    RenderMode m_currentMode = RenderMode::Undefined;
    async::Notification m_modeChanges;
};
}

#endif // MU_AUDIO_AUDIOENGINE_H
