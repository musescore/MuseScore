/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
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
