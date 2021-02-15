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
#ifndef MU_AUDIO_IAUDIOCONFIGURATION_H
#define MU_AUDIO_IAUDIOCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "io/path.h"
#include "ret.h"
#include "async/notification.h"
#include "synthtypes.h"

namespace mu::audio {
class IAudioConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioConfiguration)
public:
    virtual ~IAudioConfiguration() = default;

    virtual unsigned int driverBufferSize() const = 0; // samples

    // synthesizers
    virtual std::vector<io::path> soundFontPaths() const = 0;
    virtual const synth::SynthesizerState& synthesizerState() const = 0;
    virtual Ret saveSynthesizerState(const synth::SynthesizerState& state) = 0;
    virtual async::Notification synthesizerStateChanged() const = 0;
    virtual async::Notification synthesizerStateGroupChanged(const std::string& groupName) const = 0;
};
}

#endif // MU_AUDIO_IAUDIOCONFIGURATION_H
