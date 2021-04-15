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
#include "synthesizersregister.h"
#include "internal/audiosanitizer.h"
#include "sanitysynthesizer.h"

using namespace mu;
using namespace mu::audio::synth;

void SynthesizersRegister::registerSynthesizer(const SynthName& name, ISynthesizerPtr s)
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_synths[name] = std::make_shared<SanitySynthesizer>(s);

    m_synthAdded.send(m_synths[name]);
}

std::shared_ptr<ISynthesizer> SynthesizersRegister::synthesizer(const SynthName& name) const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_synths.find(name);
    if (it != m_synths.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<ISynthesizer> > SynthesizersRegister::synthesizers() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::shared_ptr<ISynthesizer> > synths;
    for (auto it = m_synths.begin(); it != m_synths.end(); ++it) {
        synths.push_back(it->second);
    }
    return synths;
}

async::Channel<ISynthesizerPtr> SynthesizersRegister::synthesizerAdded() const
{
    return m_synthAdded;
}

void SynthesizersRegister::setDefaultSynthesizer(const SynthName& name)
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_defaultName = name;
}

std::shared_ptr<ISynthesizer> SynthesizersRegister::defaultSynthesizer() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;
    std::shared_ptr<ISynthesizer> s = synthesizer(m_defaultName);
    if (s) {
        return s;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_synths.empty()) {
        return m_synths.begin()->second;
    }

    return nullptr;
}
