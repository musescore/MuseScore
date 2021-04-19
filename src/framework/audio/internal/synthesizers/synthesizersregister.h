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
#ifndef MU_AUDIO_SYNTHESIZERSREGISTER_H
#define MU_AUDIO_SYNTHESIZERSREGISTER_H

#include <map>
#include <mutex>
#include "async/asyncable.h"
#include "isynthesizersregister.h"

namespace mu::audio::synth {
class SynthesizersRegister : public ISynthesizersRegister, public async::Asyncable
{
public:

    void registerSynthesizer(const SynthName& name, ISynthesizerPtr s) override;
    std::shared_ptr<ISynthesizer> synthesizer(const SynthName& name) const override;
    std::vector<std::shared_ptr<ISynthesizer> > synthesizers() const override;
    async::Channel<ISynthesizerPtr> synthesizerAdded() const override;

    void setDefaultSynthesizer(const SynthName& name) override;
    std::shared_ptr<ISynthesizer> defaultSynthesizer() const override;

private:

    mutable std::mutex m_mutex;
    std::map<std::string, std::shared_ptr<ISynthesizer> > m_synths;
    async::Channel<ISynthesizerPtr> m_synthAdded;
    SynthName m_defaultName;
};
}

#endif // MU_AUDIO_SYNTHESIZERSREGISTER_H
