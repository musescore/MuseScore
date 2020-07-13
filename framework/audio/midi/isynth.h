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

#ifndef MU_AUDIO_ISYNTH_H
#define MU_AUDIO_ISYNTH_H

#include <vector>

#include "modularity/imoduleexport.h"

#include "miditypes.h"

namespace mu {
namespace audio {
namespace midi {
class ISynth : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISynth)

public:
    virtual ~ISynth() = default;

    using OnLoadingChanged = std::function<void (uint16_t percent)>;
    using OnInited = std::function<void (bool success)>;

    virtual void loadSF(const Programs& programs,const std::string& overridden_sf,
                        const OnLoadingChanged& onloading) = 0;

    virtual void init(float samplerate, float gain, const OnInited& oninited) = 0;

    virtual void setGain(float gain) = 0;

    virtual bool handle_event(uint16_t chan, const Event& e) = 0;

    virtual void all_sounds_off() = 0; // all channels
    virtual void flush_sound() = 0;
    virtual void channel_sounds_off(uint16_t chan) = 0;
    virtual bool channel_volume(uint16_t chan, float val) = 0;  // 0. - 1.
    virtual bool channel_balance(uint16_t chan, float val) = 0; // -1. - 1.
    virtual bool channel_pitch(uint16_t chan, int16_t val) = 0; // -12 - 12

    virtual void write_buf(float* stream, unsigned int len) = 0;
};
}
}
}

#endif // MU_AUDIO_ISYNTH_H
