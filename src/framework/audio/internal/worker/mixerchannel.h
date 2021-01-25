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
#ifndef MU_AUDIO_MIXERCHANNEL_H
#define MU_AUDIO_MIXERCHANNEL_H

#include <complex>
#include <memory>
#include <map>
#include "async/asyncable.h"
#include "iaudiosource.h"
#include "iaudioinsert.h"
#include "imixerchannel.h"
#include "abstractaudiosource.h"

namespace mu::audio {
class MixerChannel : public IMixerChannel, public AbstractAudioSource, public async::Asyncable
{
public:
    MixerChannel();

    unsigned int streamCount() const override;
    void checkStreams();
    void forward(unsigned int sampleCount) override;
    void setBufferSize(unsigned int samples) override;
    void setSampleRate(unsigned int sampleRate) override;

    void setSource(std::shared_ptr<IAudioSource> source) override;

    bool active() const override;
    void setActive(bool active) override;

    float level(unsigned int streamId) const override;
    void setLevel(float level) override;
    void setLevel(unsigned int streamId, float level) override;

    std::complex<float> balance(unsigned int streamId) const override;
    void setBalance(std::complex<float> value) override;
    void setBalance(unsigned int streamId, std::complex<float> value) override;

    std::shared_ptr<IAudioInsert> insert(unsigned int number) const override;
    void setInsert(unsigned int number, std::shared_ptr<IAudioInsert> insert) override;

protected:
    void updateBalanceLevelMaps();

    bool m_active = false;
    std::shared_ptr<IAudioSource> m_source = nullptr;
    std::map<unsigned int, std::complex<float> > m_balance = {};
    std::map<unsigned int, float> m_level = {};
    std::map<unsigned int, std::shared_ptr<IAudioInsert> > m_insertList = {};
};
}

#endif // MU_AUDIO_MIXERCHANNEL_H
