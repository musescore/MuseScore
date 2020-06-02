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

#ifndef MU_FRAMEWORK_IMODULEEXPORT_H
#define MU_FRAMEWORK_IMODULEEXPORT_H

#include <memory>

#define INTERFACE_ID(cls)               \
public:                                 \
    static const char* interfaceId() {  \
        static const char* id = #cls;   \
        return id;                      \
    }                                   \

namespace mu {
namespace framework {
class IModuleExportInterface
{
public:
    virtual ~IModuleExportInterface() {}
};

struct IModuleExportCreator {
    virtual ~IModuleExportCreator() {}
    virtual std::shared_ptr<IModuleExportInterface> create() = 0;
};
}
}

#define MODULE_EXPORT_INTERFACE public mu::framework::IModuleExportInterface
#define MODULE_EXPORT_CREATOR public mu::framework::IModuleExportCreator

#endif // MU_FRAMEWORK_IMODULEEXPORT_H
