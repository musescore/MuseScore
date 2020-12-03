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

#ifndef MU_FRAMEWORK_IUIENGINE_H
#define MU_FRAMEWORK_IUIENGINE_H

#include <QString>
#include "framework/global/modularity/imoduleexport.h"

class QQmlEngine;

namespace mu {
namespace framework {
class IUiEngine : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(mu::framework::IUiEngine)

public:
    virtual ~IUiEngine() {}

    virtual void updateTheme() = 0;
    virtual QQmlEngine* qmlEngine() const = 0;
    virtual void clearComponentCache() = 0;

    virtual void addSourceImportPath(const QString& path) = 0;
};
}
}

#endif // MU_FRAMEWORK_UIENGINEMODULE_H
