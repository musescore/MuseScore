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
#ifndef MU_PLUGINS_PLUGINSMODEL_H
#define MU_PLUGINS_PLUGINSMODEL_H

#include "ipluginsservice.h"
#include "iinteractive.h"

#include "modularity/ioc.h"

#include <QAbstractListModel>

namespace mu::plugins {
class PluginsModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(plugins, IPluginsService, service)
    INJECT(plugins, framework::IInteractive, interactive)

public:
    explicit PluginsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setCurrentPlugin(QString codeKey);

    Q_INVOKABLE void load();
    Q_INVOKABLE void install();
    Q_INVOKABLE void uninstall();
    Q_INVOKABLE void update();
    Q_INVOKABLE void restart();
    Q_INVOKABLE void openDetails();

private:
    enum Roles {
        rCode = Qt::UserRole + 1,
        rName,
        rDescription,
        rThumbnailUrl,
        rInstalled,
        rHasUpdate
    };

    int itemIndexByCodeKey(const QString& codeKey) const;

    QHash<int, QByteArray> m_roles;
    QList<Plugin> m_plugins;
    CodeKey m_currentPluginCodeKey;
};
}

#endif // MU_PLUGINS_PLUGINSMODEL_H
