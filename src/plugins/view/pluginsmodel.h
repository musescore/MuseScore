/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_PLUGINS_PLUGINSMODEL_H
#define MU_PLUGINS_PLUGINSMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "ipluginsservice.h"
#include "ipluginsconfiguration.h"

namespace mu::plugins {
class PluginsModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(framework::IInteractive, interactive)
    INJECT(IPluginsService, service)
    INJECT(IPluginsConfiguration, configuration)

public:
    explicit PluginsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void setEnable(const QString& codeKey, bool enable);
    Q_INVOKABLE void editShortcut(QString codeKey);
    Q_INVOKABLE void reloadPlugins();

    Q_INVOKABLE QVariantList categories() const;

signals:
    void finished();

private:
    enum Roles {
        rCode = Qt::UserRole + 1,
        rName,
        rDescription,
        rThumbnailUrl,
        rEnabled,
        rCategory,
        rVersion,
        rShortcuts
    };

    void updatePlugin(const PluginInfo& plugin);
    int itemIndexByCodeKey(const QString& codeKey) const;

    QHash<int, QByteArray> m_roles;
    QList<PluginInfo> m_plugins;
};
}

#endif // MU_PLUGINS_PLUGINSMODEL_H
