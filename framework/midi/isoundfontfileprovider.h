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

#ifndef MU_AUDIO_ISOUNDFONTFILEPROVIDER_H
#define MU_AUDIO_ISOUNDFONTFILEPROVIDER_H

#include <vector>
#include <functional>

#include "modularity/imoduleexport.h"
#include "miditypes.h"

namespace mu {
namespace midi {
class ISoundFontFileProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISoundFontFileProvider)

public:
    virtual ~ISoundFontFileProvider() = default;

    using OnLoading = std::function<void (uint16_t percent)>;
    using OnLoaded = std::function<void (bool, const std::string& sf_path, const std::vector<midi::Program>& progs)>;

    virtual void loadSF(const midi::Programs& programs, const OnLoading& onloading, const OnLoaded& onloaded) = 0;
};
}
}

#endif // MU_AUDIO_ISOUNDFONTFILEPROVIDER_H
