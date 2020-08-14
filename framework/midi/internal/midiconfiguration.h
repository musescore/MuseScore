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
#ifndef MU_MIDI_MIDICONFIGURATION_H
#define MU_MIDI_MIDICONFIGURATION_H

#include <map>

#include "../imidiconfiguration.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

namespace mu {
namespace midi {
class MidiConfiguration : public IMidiConfiguration
{
    INJECT(midi, framework::IGlobalConfiguration, globalConfiguration)

public:

    std::vector<io::path> soundFontPaths() const override;

    const SynthesizerState& defaultSynthesizerState() const;
    const SynthesizerState& synthesizerState() const override;
    Ret saveSynthesizerState(const SynthesizerState& state) override;
    async::Notification synthesizerStateChanged() const override;
    async::Notification synthesizerStateGroupChanged(const std::string& gname) const override;

private:

    io::path stateFilePath() const;
    bool readState(const io::path& path, SynthesizerState& state) const;
    bool writeState(const io::path& path, const SynthesizerState& state);

    mutable SynthesizerState m_state;
    async::Notification m_synthesizerStateChanged;
    mutable std::map<std::string, async::Notification> m_synthesizerStateGroupChanged;
};
}
}

#endif // MU_MIDI_MIDICONFIGURATION_H
