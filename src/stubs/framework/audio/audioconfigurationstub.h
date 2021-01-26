//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_AUDIO_AUDIOCONFIGURATIONSTUB_H
#define MU_AUDIO_AUDIOCONFIGURATIONSTUB_H

#include "audio/iaudioconfiguration.h"

namespace mu::audio {
class AudioConfigurationStub : public IAudioConfiguration
{
public:
    unsigned int driverBufferSize() const override;

    std::vector<io::path> soundFontPaths() const override;
    const synth::SynthesizerState& synthesizerState() const override;
    Ret saveSynthesizerState(const synth::SynthesizerState& state) override;
    async::Notification synthesizerStateChanged() const override;
    async::Notification synthesizerStateGroupChanged(const std::string& groupName) const override;
};
}

#endif // MU_AUDIO_AUDIOCONFIGURATIONSTUB_H
