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

#ifndef MU_MIDI_ISYNTHESIZER_H
#define MU_MIDI_ISYNTHESIZER_H

#include "miditypes.h"
#include "io/path.h"
#include "ret.h"

namespace mu {
namespace midi {
class ISynthesizer
{
public:
    virtual ~ISynthesizer() = default;

    virtual std::string name() const = 0;
    virtual SoundFontFormats soundFontFormats() const = 0;

    virtual Ret init(float samplerate) = 0;
    virtual Ret addSoundFonts(std::vector<io::path> sfonts) = 0;
    virtual Ret removeSoundFonts() = 0;

    virtual bool isActive() const = 0;
    virtual void setIsActive(bool arg) = 0;

    virtual Ret setupChannels(const std::vector<Event>& events) = 0;
    virtual bool handleEvent(const Event& e) = 0;
    virtual void writeBuf(float* stream, unsigned int samples) = 0;

    virtual void allSoundsOff() = 0; // all channels
    virtual void flushSound() = 0;
    virtual void channelSoundsOff(channel_t chan) = 0;
    virtual bool channelVolume(channel_t chan, float val) = 0;  // 0. - 1.
    virtual bool channelBalance(channel_t chan, float val) = 0; // -1. - 1.
    virtual bool channelPitch(channel_t chan, int16_t val) = 0; // -12 - 12
};
}
}

#endif // MU_MIDI_ISYNTHESIZER_H
