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

#include "isoundfontsprovider.h"
#include "iaudiodriver.h"
#include "isynthesizersregister.h"
#include "imixer.h"

#include "internal/synthesizers/synthesizercontroller.h"
#include "internal/iaudiobuffer.h"

namespace mu::audio {
class AudioEngine : public async::Asyncable
{
    INJECT(audio, synth::ISoundFontsProvider, soundFontsProvider)
    INJECT(audio, synth::ISynthesizersRegister, synthesizersRegister)
public:
    ~AudioEngine();

    static AudioEngine* instance();

    Ret init();
    void deinit();
    void onDriverOpened(unsigned int sampleRate, uint16_t readBufferSize);

    void setSampleRate(unsigned int sampleRate);
    void setReadBufferSize(uint16_t readBufferSize);

    bool isInited() const;
    async::Channel<bool> initChanged() const;
    unsigned int sampleRate() const;
    IMixerPtr mixer() const;
    void setMixer(IMixerPtr mixerPtr);
    IAudioBufferPtr buffer() const;
    void setAudioBuffer(IAudioBufferPtr buffer);

private:

    AudioEngine();

    bool m_inited = false;
    mu::async::Channel<bool> m_initChanged;
    unsigned int m_sampleRate = 0;

    IMixerPtr m_mixer = nullptr;
    IAudioBufferPtr m_buffer = nullptr;

    // synthesizers
    std::shared_ptr<synth::SynthesizerController> m_synthesizerController = nullptr;
};
}

#endif // MU_AUDIO_AUDIOENGINE_H
