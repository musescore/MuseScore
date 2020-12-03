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
#ifndef MU_VST_VSTSYNTHESIZER_H
#define MU_VST_VSTSYNTHESIZER_H

#include "framework/midi/isynthesizer.h"
#include "modularity/ioc.h"
#include "framework/midi/isynthesizersregister.h"

namespace mu {
namespace vst {
class PluginInstance;
class VSTSynthesizer : public mu::midi::ISynthesizer
{
    INJECT_STATIC(midi, mu::midi::ISynthesizersRegister, synthesizersRegister)

public:
    static std::shared_ptr<VSTSynthesizer> create(std::shared_ptr<PluginInstance> instance);
    VSTSynthesizer(std::string name, std::shared_ptr<PluginInstance> instance);

    virtual std::string name() const;
    virtual midi::SoundFontFormats soundFontFormats() const;

    virtual Ret init(float samplerate);
    virtual Ret addSoundFonts(std::vector<io::path> sfonts);
    virtual Ret removeSoundFonts();

    virtual bool isActive() const;
    virtual void setIsActive(bool arg);

    virtual Ret setupChannels(const std::vector<mu::midi::Event>& events);
    virtual bool handleEvent(const mu::midi::Event& e);
    virtual void writeBuf(float* stream, unsigned int samples);

    virtual void allSoundsOff();
    virtual void flushSound();
    virtual void channelSoundsOff(mu::midi::channel_t chan);
    virtual bool channelVolume(mu::midi::channel_t chan, float val);  // 0. - 1.
    virtual bool channelBalance(mu::midi::channel_t chan, float val); // -1. - 1.
    virtual bool channelPitch(mu::midi::channel_t chan, int16_t val); // -12 - 12

private:
    std::string m_name;
    std::shared_ptr<PluginInstance> m_instance;
};
}
}
#endif // MU_VST_VSTSYNTHESIZER_H
