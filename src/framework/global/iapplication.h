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
#ifndef MU_FRAMEWORK_IAPPLICATION_H
#define MU_FRAMEWORK_IAPPLICATION_H

#include "modularity/imoduleexport.h"

namespace mu::framework {
class IApplication : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IApplication)
public:
    virtual ~IApplication() = default;

    enum class RunMode {
        Editor,
        Converter
    };

    virtual void setRunMode(const RunMode& mode) = 0;
    virtual RunMode runMode() const = 0;
    virtual bool noGui() const = 0;
};
}

#endif // MU_FRAMEWORK_IAPPLICATION_H
