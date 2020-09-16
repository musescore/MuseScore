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
#ifndef MU_PLUGINS_PLUGINSTESTMODEL_H
#define MU_PLUGINS_PLUGINSTESTMODEL_H

#include "ipluginsservice.h"
#include "modularity/ioc.h"

#include <QObject>

namespace mu {
namespace plugins {
class PluginsTestModel : public QObject
{
    Q_OBJECT

    INJECT(plugins, IPluginsService, service)

    Q_PROPERTY(QStringList installedPluginsNames READ installedPluginsNames NOTIFY loaded)

public:
    QStringList installedPluginsNames() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void run(int index);

signals:
    void loaded();

private:
    PluginInfoList m_installedPlugins;
};
}
}

#endif // MU_PLUGINS_PLUGINSTESTMODEL_H
