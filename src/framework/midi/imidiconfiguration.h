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
#ifndef MU_MIDI_IMIDICONFIGURATION_H
#define MU_MIDI_IMIDICONFIGURATION_H

#include <vector>
#include "modularity/imoduleexport.h"
#include "ret.h"
#include "io/path.h"
#include "miditypes.h"
#include "async/notification.h"

namespace mu {
namespace midi {
class IMidiConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiConfiguration)
public:
    virtual ~IMidiConfiguration() = default;

    virtual std::vector<io::path> soundFontPaths() const = 0;

    virtual const SynthesizerState& synthesizerState() const = 0;
    virtual Ret saveSynthesizerState(const SynthesizerState& state) = 0;
    virtual async::Notification synthesizerStateChanged() const = 0;
    virtual async::Notification synthesizerStateGroupChanged(const std::string& gname) const = 0;
};
}
}

#endif // MU_MIDI_IMIDICONFIGURATION_H
