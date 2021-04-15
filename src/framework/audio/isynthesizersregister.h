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
#ifndef MU_AUDIO_ISYNTHESIZERSREGISTER_H
#define MU_AUDIO_ISYNTHESIZERSREGISTER_H

#include <string>
#include <memory>

#include "async/channel.h"
#include "modularity/imoduleexport.h"
#include "isynthesizer.h"

namespace mu::audio::synth {
class ISynthesizersRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISynthesizersRegister)
public:
    virtual ~ISynthesizersRegister() = default;

    virtual void registerSynthesizer(const SynthName& name, ISynthesizerPtr synthesizer) = 0;
    virtual ISynthesizerPtr synthesizer(const SynthName& name) const = 0;
    virtual std::vector<ISynthesizerPtr> synthesizers() const = 0;
    virtual async::Channel<ISynthesizerPtr> synthesizerAdded() const = 0;

    virtual void setDefaultSynthesizer(const SynthName& name) = 0;
    virtual ISynthesizerPtr defaultSynthesizer() const = 0;
};

using ISynthesizersRegisterPtr = std::shared_ptr<ISynthesizersRegister>;
}

#endif // MU_AUDIO_ISYNTHESIZERSREGISTER_H
