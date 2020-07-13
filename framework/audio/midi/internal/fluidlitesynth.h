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

#ifndef MU_AUDIO_FLUIDLITESYNTH_H
#define MU_AUDIO_FLUIDLITESYNTH_H

#include <memory>
#include <vector>
#include <cstdint>
#include <functional>

#include "../isynth.h"
#include "../miditypes.h"

#include "modularity/ioc.h"
#include "isffileprovider.h"

//! NOTE Used for the test, the main synthesizer will not be this one.

namespace mu {
namespace audio {
namespace midi {
struct Fluid;
class FluidLiteSynth : public ISynth
{
    INJECT(midi, ISFFileProvider, sfprovider)

public:
    FluidLiteSynth();
    ~FluidLiteSynth() override;

    void loadSF(const Programs& programs,const std::string& overridden_sf,const OnLoadingChanged& onloading) override;

    void init(float samplerate, float gain, const OnInited& oninited) override;

    void setGain(float gain) override;

    bool handle_event(uint16_t chan, const Event& e) override;

    void all_sounds_off() override; // all channels
    void flush_sound() override;

    void channel_sounds_off(uint16_t chan) override;
    bool channel_volume(uint16_t chan, float val) override;  // 0. - 1.
    bool channel_balance(uint16_t chan, float val) override; // -1. - 1.
    bool channel_pitch(uint16_t chan, int16_t pitch) override; // -12 - 12

    void write_buf(float* stream, unsigned int len) override;

private:

    void doInit(float samplerate, float gain, const OnInited& oninited);
    bool init_synth(const std::string& sf_path, float samplerate, float gain);

    const Program& program(uint16_t chan) const;

    enum midi_control
    {
        BANK_SELECT_MSB = 0x00,
        VOLUME_MSB      = 0x07,
        BALANCE_MSB     = 0x08,
        PAN_MSB         = 0x0A
    };

    struct SF {
        bool loaded{ false };
        std::vector<Program> programs;
        std::string file_path;
        std::function<void()> onLoaded;
    };

    SF m_sf;
    std::shared_ptr<Fluid> m_fluid{ nullptr };
    float m_gain = 1.0f;
    int16_t m_generalPitch = 0;
    bool m_isLoggingSynthEvents = false;

    std::vector<float> m_preallocated; // used to flush a sound
    float m_sampleRate = 44100.0f;

    int m_sfontID = -1;
};
}
}
}

#endif //MU_AUDIO_FLUIDLITESYNTH_H
