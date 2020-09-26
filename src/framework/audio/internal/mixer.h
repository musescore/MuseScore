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
#ifndef MU_AUDIO_MIXER_H
#define MU_AUDIO_MIXER_H

#include <memory>
#include <map>
#include "imixer.h"
#include "abstractaudiosource.h"
#include "internal/mixerchannel.h"
#include "internal/clock.h"
#include "rpc/irpctarget.h"

namespace mu::audio {
class Mixer : public AbstractAudioSource, public IMixer, public rpc::IRPCTarget
{
public:
    Mixer();
    ~Mixer();

    Mode mode() const override;
    void setMode(const Mode& mode) override;

    void setClock(std::shared_ptr<Clock> clock);

    virtual void setSampleRate(unsigned int sampleRate) override;

    unsigned int streamCount() const override;

    void setLevel(float level) override;
    std::shared_ptr<IAudioInsert> insert(unsigned int number) const override;
    void setInsert(unsigned int number, std::shared_ptr<IAudioInsert> insert) override;

    std::shared_ptr<IMixerChannel> channel(unsigned int number) const override;
    unsigned int addChannel(std::shared_ptr<IAudioSource> source) override;
    void removeChannel(unsigned int channelId) override;

    void setActive(unsigned int channelId, bool active) override;
    void setLevel(unsigned int channelId, unsigned int streamId, float level) override;
    void setBalance(unsigned int channelId, unsigned int streamId, std::complex<float> balance) override;

    void setBufferSize(unsigned int samples) override;
    void forward(unsigned int sampleCount) override;

protected:
    void buildRpcReflection() override;

private:
    //! mix the channel in to the buffer
    void mixinChannel(std::shared_ptr<MixerChannel> channel, unsigned int samplesCount);
    void mixinChannelStream(std::shared_ptr<MixerChannel> channel, unsigned int streamId, unsigned int samplesCount);

    Mode m_mode = STEREO;
    float m_masterLevel = 1.f;
    std::map<unsigned int, std::shared_ptr<MixerChannel> > m_inputList = {};
    std::map<unsigned int, std::shared_ptr<IAudioInsert> > m_insertList = {};
    std::mutex m_process;
    std::shared_ptr<Clock> m_clock;
};
}

#endif // MU_AUDIO_MIXER_H
