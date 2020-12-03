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
#ifndef MU_FRAMEWORK_IINTERACTIVEURIREGISTER_H
#define MU_FRAMEWORK_IINTERACTIVEURIREGISTER_H

#include <QVariantMap>
#include <QDialog>

#include "modularity/imoduleexport.h"
#include "global/uri.h"
#include "uitypes.h"

namespace mu {
namespace framework {
class IInteractiveUriRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IInteractiveUriRegister)

public:
    virtual ~IInteractiveUriRegister() = default;

    virtual void registerUri(const Uri& uri, const ContainerMeta& meta) = 0;
    virtual ContainerMeta meta(const Uri& uri) const = 0;
};
}
}

#endif // MU_FRAMEWORK_IINTERACTIVEURIREGISTER_H
