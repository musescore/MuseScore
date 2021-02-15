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

#include "modularity/imoduleexport.h"

//! NOTE There used to be synthesizer settings here, now nothing
//! but we will keep the interface, maybe something will appear,
//! for example, midi port settings

namespace mu::midi {
class IMidiConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiConfiguration)
public:
    virtual ~IMidiConfiguration() = default;
};
}

#endif // MU_MIDI_IMIDICONFIGURATION_H
