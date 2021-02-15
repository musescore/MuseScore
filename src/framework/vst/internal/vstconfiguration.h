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
#ifndef MU_VST_VSTCONFIGURATION_H
#define MU_VST_VSTCONFIGURATION_H

#include <string>
#include "settings.h"

namespace mu {
namespace vst {
class VSTConfiguration
{
public:
    void init();

    std::string searchPaths() const;

    //! default paths in system for plugin scanning
    static const std::string DEFAULT_PATHS;
    static const mu::framework::Settings::Key SEARCH_PATHS;
};
} // namespace vst
} // namespace mu

#endif // MU_VST_VSTCONFIGURATION_H
