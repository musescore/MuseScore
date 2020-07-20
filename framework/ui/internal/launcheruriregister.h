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
#ifndef MU_FRAMEWORK_LAUNCHERURIREGISTER_H
#define MU_FRAMEWORK_LAUNCHERURIREGISTER_H

#include "ilauncheruriregister.h"

namespace mu {
namespace framework {
class LauncherUriRegister : public ILauncherUriRegister
{
public:

    void registerUri(const QString& uri, const QString& qmlPath) override;
    void registerUri(const QString& uri, int dialogMetaTypeId) override;

    UriType uriType(const QString& uri) const override;

    QVariantMap qmlPage(const QString& uri, const QVariantMap& params) const override;
    int widgetDialogMetaTypeId(const QString& uri) const override;

private:
    QHash<QString, QString> m_qmlUriHash;
    QHash<QString, int> m_widgetUriHash;
};
}
}

#endif // MU_FRAMEWORK_LAUNCHERURIREGISTER_H
