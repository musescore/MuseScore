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
#ifndef MU_FRAMEWORK_IGLOBALCONFIGURATION_H
#define MU_FRAMEWORK_IGLOBALCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "io/path.h"

namespace mu {
namespace framework {
class IGlobalConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IGlobalConfiguration)
public:

    virtual ~IGlobalConfiguration() = default;

    virtual io::path sharePath() const = 0;
    virtual io::path dataPath() const = 0;
    virtual io::path backupPath() const = 0;

    virtual bool useFactorySettings() const = 0;
    virtual bool enableExperimental() const = 0;
};
}
}

#endif // MU_FRAMEWORK_IGLOBALCONFIGURATION_H
