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
#ifndef MU_MIDI_ZERBERUSSYNTH_H
#define MU_MIDI_ZERBERUSSYNTH_H

#include "../isynthesizer.h"

namespace mu {
namespace zerberus {
class Zerberus;
}

namespace midi {
class ZerberusSynth : public ISynthesizer
{
public:

    ZerberusSynth();

    std::string name() const override;
    SoundFontFormats soundFontFormats() const override;

    Ret init(float samplerate) override;
    Ret addSoundFonts(std::vector<io::path> sfonts) override;
    Ret removeSoundFonts() override;

    bool isActive() const override;
    void setIsActive(bool arg) override;

    Ret setupChannels(const std::vector<Event>& events) override;
    bool handleEvent(const Event& e) override;
    void writeBuf(float* stream, unsigned int samples) override;

    void allSoundsOff() override; // all channels
    void flushSound() override;

    void channelSoundsOff(channel_t chan) override;
    bool channelVolume(channel_t chan, float val) override;  // 0. - 1.
    bool channelBalance(channel_t chan, float val) override; // -1. - 1.
    bool channelPitch(channel_t chan, int16_t pitch) override; // -12 - 12

private:

    zerberus::Zerberus* m_zerb = nullptr;
    std::vector<float> m_preallocated;     // used to flush a sound
    bool m_isLoggingSynthEvents = false;
    bool m_isActive = false;
};
}
}

#endif // MU_MIDI_ZERBERUSSYNTH_H
