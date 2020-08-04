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
#ifndef MU_AUDIO_SINESOURCE_H
#define MU_AUDIO_SINESOURCE_H

#include <memory>
#include <vector>

#include "iaudiosource.h"

namespace mu {
namespace audio {
class SineSource : public IAudioSource
{
public:
    SineSource();
    ~SineSource() = default;

    void setSampleRate(float samplerate) override;
    SoLoud::AudioSource* source() override;

private:

    struct SL;
    struct SLInstance;

    using Samples = std::vector<float>;

    void generateSine(Samples& samples, float samplerate, float freq, int seconds) const;

    std::shared_ptr<SL> m_sl;
    std::shared_ptr<Samples> m_samples;
};
}
}

#endif // MU_AUDIO_SINESOURCE_H
