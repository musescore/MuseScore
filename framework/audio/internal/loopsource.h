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

#ifndef MU_AUDIO_LOOPSTREAM_H
#define MU_AUDIO_LOOPSTREAM_H

#include <memory>

#include "iaudiosource.h"
#include "audiotypes.h"

namespace mu {
namespace audio {
class LoopSource : public IAudioSource
{
public:
    LoopSource(std::shared_ptr<IAudioSource> origin, const std::string& name);
    ~LoopSource() override;

    void setSampleRate(float samplerate) override;
    SoLoud::AudioSource* source() override;

    void setLoopRegion(const LoopRegion& loop);

private:

    struct SL;
    struct SLInstance;
    std::shared_ptr<SL> m_sl = nullptr;
    std::shared_ptr<IAudioSource> m_origin = nullptr;
    std::string m_name;

    LoopRegion m_loopRegion;
};
}
}

#endif //MU_AUDIO_LOOPSTREAM_H
