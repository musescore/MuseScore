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

#ifndef MU_AUDIO_ISYNTHESIZER_H
#define MU_AUDIO_ISYNTHESIZER_H

#include "io/path.h"
#include "ret.h"
#include "synthtypes.h"
#include "midi/miditypes.h"
#include "iaudiosource.h"

namespace mu::audio::synth {
class ISynthesizer : public IAudioSource
{
public:
    virtual ~ISynthesizer() = default;

    virtual bool isValid() const = 0;

    virtual std::string name() const = 0;
    virtual SoundFontFormats soundFontFormats() const = 0;

    virtual Ret init() = 0;
    virtual Ret addSoundFonts(const std::vector<io::path>& sfonts) = 0;
    virtual Ret removeSoundFonts() = 0;

    virtual Ret setupMidiChannels(const std::vector<midi::Event>& events) = 0;
    virtual bool handleEvent(const midi::Event& e) = 0;
    virtual void writeBuf(float* stream, unsigned int samples) = 0;

    virtual void allSoundsOff() = 0; // all channels
    virtual void flushSound() = 0;
    virtual void midiChannelSoundsOff(midi::channel_t chan) = 0;
    virtual bool midiChannelVolume(midi::channel_t chan, float val) = 0;  // 0. - 1.
    virtual bool midiChannelBalance(midi::channel_t chan, float val) = 0; // -1. - 1.
    virtual bool midiChannelPitch(midi::channel_t chan, int16_t val) = 0; // -12 - 12
};

using ISynthesizerPtr = std::shared_ptr<ISynthesizer>;
}

#endif // MU_AUDIO_ISYNTHESIZER_H
