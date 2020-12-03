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

using namespace mu::midi;

void SynthesizersRegister::registerSynthesizer(const SynthName& name, std::shared_ptr<ISynthesizer> s)
{
    m_synths[name] = s;
}

std::shared_ptr<ISynthesizer> SynthesizersRegister::synthesizer(const SynthName& name) const
{
    auto it = m_synths.find(name);
    if (it != m_synths.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<ISynthesizer> > SynthesizersRegister::synthesizers() const
{
    std::vector<std::shared_ptr<ISynthesizer> > synths;
    for (auto it = m_synths.begin(); it != m_synths.end(); ++it) {
        synths.push_back(it->second);
    }
    return synths;
}

void SynthesizersRegister::setDefaultSynthesizer(const SynthName& name)
{
    m_defaultName = name;
}

std::shared_ptr<ISynthesizer> SynthesizersRegister::defaultSynthesizer() const
{
    std::shared_ptr<ISynthesizer> s = synthesizer(m_defaultName);
    if (s) {
        return s;
    }

    if (!m_synths.empty()) {
        return m_synths.begin()->second;
    }

    return nullptr;
}
