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

#ifndef MU_INSPECTOR_INSPECTORSETUP_H
#define MU_INSPECTOR_INSPECTORSETUP_H

#include "modularity/imodulesetup.h"

namespace mu::inspector {
class InspectorModule : public mu::framework::IModuleSetup
{
public:
    InspectorModule() = default;

    std::string moduleName() const override;
    void registerExports() override;
    void registerResources() override;
    void registerUiTypes() override;
};
}

#endif // MU_INSPECTOR_INSPECTORSETUP_H
