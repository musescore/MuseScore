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
#ifndef MU_FRAMEWORK_ILAUNCERURIREGISTER_H
#define MU_FRAMEWORK_ILAUNCERURIREGISTER_H

#include <QVariantMap>
#include <QDialog>
#include <QSharedPointer>

#include "modularity/imoduleexport.h"
#include "uitypes.h"

namespace mu {
namespace framework {
class ILauncherUriRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILauncherUriRegister)
public:
    virtual ~ILauncherUriRegister() = default;

    virtual void registerUri(const QString& uri, const QString& qmlPath) = 0;
    virtual void registerUri(const QString& uri, int dialogMetaTypeId) = 0;

    virtual UriType uriType(const QString& uri) const = 0;

    virtual QVariantMap qmlPage(const QString& uri, const QVariantMap& params = QVariantMap()) const = 0;
    virtual int widgetDialogMetaTypeId(const QString& uri) const = 0;
};
}
}

#endif // MU_FRAMEWORK_ILAUNCERURIREGISTER_H
