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

#ifndef MU_FRAMEWORK_IMODULESETUP_H
#define MU_FRAMEWORK_IMODULESETUP_H

#include <string>

namespace mu {
namespace framework {
class IModuleSetup
{
public:

    virtual ~IModuleSetup() {}

    virtual std::string moduleName() const = 0;

    virtual void registerExports() {}
    virtual void resolveImports() {}

    virtual void registerResources() {}
    virtual void registerUiTypes() {}

    virtual void onInit() {}
    virtual void onDeinit() {}

    virtual void onStartApp() {}
};
}
}

#endif // MU_FRAMEWORK_IMODULESETUP_H
