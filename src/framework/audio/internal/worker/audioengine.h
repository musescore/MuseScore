//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUDIO_AUDIOENGINE_H
#define MU_AUDIO_AUDIOENGINE_H

#include <memory>
#include <map>
#include <set>

#include "iaudioengine.h"
#include "modularity/ioc.h"

#include "ret.h"
#include "retval.h"

#include "mixer.h"
#include "internal/audiobuffer.h"
#include "sequencer.h"
#include "async/asyncable.h"
#include "isoundfontsprovider.h"
#include "isynthesizersregister.h"
#include "internal/synthesizers/synthesizercontroller.h"

namespace mu::audio {
class AudioEngine : public IAudioEngine, public async::Asyncable
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

    bool isInited() const override;
    async::Channel<bool> initChanged() const override;
    unsigned int sampleRate() const override;
    std::shared_ptr<IMixer> mixer() const override;
    std::shared_ptr<ISequencer> sequencer() const override;
    IAudioBufferPtr buffer() const override;
    void setAudioBuffer(IAudioBufferPtr buffer) override;

private:

    AudioEngine();

    bool m_inited = false;
    mu::async::Channel<bool> m_initChanged;
    unsigned int m_sampleRate = 0;
    std::shared_ptr<Sequencer> m_sequencer = nullptr;
    std::shared_ptr<IAudioDriver> m_driver = nullptr;
    std::shared_ptr<Mixer> m_mixer = nullptr;
    std::shared_ptr<IAudioBuffer> m_buffer = nullptr;

    // synthesizers

    std::shared_ptr<synth::SynthesizerController> m_synthesizerController = nullptr;
};
}

#endif // MU_AUDIO_AUDIOENGINE_H
