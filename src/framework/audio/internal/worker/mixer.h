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
#ifndef MU_AUDIO_MIXER_H
#define MU_AUDIO_MIXER_H

#include <memory>
#include <map>
#include "imixer.h"
#include "abstractaudiosource.h"
#include "mixerchannel.h"
#include "clock.h"

namespace mu::audio {
class Mixer : public IMixer, public AbstractAudioSource, public std::enable_shared_from_this<Mixer>
{
public:
    Mixer();
    ~Mixer();

    // IMixer
    Mode mode() const override;
    void setMode(const Mode& mode) override;

    void setLevel(float level) override;

    std::shared_ptr<IAudioProcessor> processor(unsigned int number) const override;
    void setProcessor(unsigned int number, std::shared_ptr<IAudioProcessor> insert) override;

    IAudioSourcePtr mixedSource() override;

    ChannelID addChannel(std::shared_ptr<IAudioSource> source) override;
    void removeChannel(ChannelID channelId) override;
    std::shared_ptr<IMixerChannel> channel(unsigned int number) const override;

    void setActive(ChannelID channelId, bool active) override;
    void setLevel(ChannelID channelId, unsigned int streamId, float level) override;
    void setBalance(ChannelID channelId, unsigned int streamId, std::complex<float> balance) override;

    // IAudioSource (AbstractAudioSource)
    void setSampleRate(unsigned int sampleRate) override;

    unsigned int streamCount() const override;

    void forward(float* buffer, unsigned int sampleCount) override;

    // Self

    void setClock(std::shared_ptr<Clock> clock);

private:
    //! mix the channel in to the buffer
    void mixinChannel(float *buffer, std::shared_ptr<MixerChannel> channel, unsigned int samplesCount);
    void mixinChannelStream(float *buffer, std::shared_ptr<MixerChannel> channel, unsigned int streamId, unsigned int samplesCount);

    Mode m_mode = STEREO;
    float m_masterLevel = 1.f;
    std::map<ChannelID, std::shared_ptr<MixerChannel> > m_inputList = {};
    std::map<unsigned int, std::shared_ptr<IAudioProcessor> > m_insertList = {};
    std::shared_ptr<Clock> m_clock;
};
}

#endif // MU_AUDIO_MIXER_H
