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
#ifndef MU_AUDIO_SYNTHESIZERSOURCE_H
#define MU_AUDIO_SYNTHESIZERSOURCE_H

#include "iaudiosource.h"
#include "framework/midi/isynthesizer.h"
#include <soloud.h>
#include "log.h"

namespace mu {
namespace audio {
class SynthesizerSource : public IAudioSource
{
public:
    SynthesizerSource(std::shared_ptr<midi::ISynthesizer> synthesizer);

    void setSampleRate(float sampleRate) override;
    SoLoud::AudioSource* source() override;

private:
    class SynthesizerSLASI;
    class SynthesizerSLAS;

    std::shared_ptr<midi::ISynthesizer> m_synthesizer;
    std::shared_ptr<SynthesizerSLAS> m_sosource;
};
}
}

#endif // MU_AUDIO_SYNTHESIZERSOURCE_H
