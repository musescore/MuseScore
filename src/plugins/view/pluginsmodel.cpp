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

#include "pluginsmodel.h"

#include "log.h"

using namespace mu::plugins;
using namespace mu::framework;
using namespace mu::async;

static constexpr int INVALID_INDEX = -1;

PluginsModel::PluginsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles.insert(rCode, "codeKey");
    m_roles.insert(rName, "name");
    m_roles.insert(rDescription, "description");
    m_roles.insert(rThumbnailUrl, "thumbnailUrl");
    m_roles.insert(rInstalled, "installed");
    m_roles.insert(rCategory, "category");
    m_roles.insert(rHasUpdate, "hasUpdate");
}

void PluginsModel::load()
{
    beginResetModel();
    m_plugins.clear();

    // TODO: this is temporary solution and will be changed in future
    QList<QString> thumbnailUrlExamples {
        "qrc:/qml/MuseScore/Plugins/internal/placeholders/placeholder1.jpeg",
        "qrc:/qml/MuseScore/Plugins/internal/placeholders/placeholder2.jpeg",
        "qrc:/qml/MuseScore/Plugins/internal/placeholders/placeholder3.jpeg",
        "qrc:/qml/MuseScore/Plugins/internal/placeholders/placeholder4.jpeg",
        "qrc:/qml/MuseScore/Plugins/internal/placeholders/placeholder5.jpeg",
        "qrc:/qml/MuseScore/Plugins/internal/placeholders/placeholder6.jpeg",
        "qrc:/qml/MuseScore/Plugins/internal/placeholders/placeholder7.jpeg"
    };

    QList<QString> categoriesExamples {
        "Simplified notation",
        "Other",
        "Accidentals",
        "Notes & Rests",
        "Chord symbols"
    };

    RetVal<PluginInfoList> plugins = service()->plugins();
    if (!plugins.ret) {
        LOGE() << plugins.ret.toString();
    }

    for (int i = 0; i < plugins.val.size(); ++i) {
        plugins.val[i].thumbnailUrl = thumbnailUrlExamples[i % thumbnailUrlExamples.size()];
        plugins.val[i].category = categoriesExamples[i % categoriesExamples.size()];
        m_plugins << plugins.val[i];
    }

    Channel<PluginInfo> pluginChanged = service()->pluginChanged();
    pluginChanged.onReceive(this, [this](const PluginInfo& plugin) {
        updatePlugin(plugin);
    });

    endResetModel();
}

QVariant PluginsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    PluginInfo plugin = m_plugins[index.row()];

    switch (role) {
    case rCode:
        return plugin.codeKey;
    case rName:
        return plugin.name;
    case rDescription:
        return plugin.description;
    case rThumbnailUrl:
        return plugin.thumbnailUrl;
    case rInstalled:
        return plugin.installed;
    case rCategory:
        return plugin.category;
    case rHasUpdate:
        return plugin.hasUpdate;
    }

    return QVariant();
}

int PluginsModel::rowCount(const QModelIndex&) const
{
    return m_plugins.count();
}

QHash<int, QByteArray> PluginsModel::roleNames() const
{
    return m_roles;
}

void PluginsModel::install(QString codeKey)
{
    service()->install(codeKey);
    emit finished();
}

void PluginsModel::uninstall(QString codeKey)
{
    Ret ret = service()->uninstall(codeKey);

    if (!ret) {
        LOGE() << ret.toString();
        return;
    }

    emit finished();
}

void PluginsModel::update(QString codeKey)
{
    NOT_IMPLEMENTED;
    Q_UNUSED(codeKey)
}

void PluginsModel::restart(QString codeKey)
{
    Ret ret = service()->run(codeKey);

    if (!ret) {
        LOGE() << ret.toString();
    }
}

void PluginsModel::openFullDescription(QString codeKey)
{
    int index = itemIndexByCodeKey(codeKey);
    if (index == INVALID_INDEX) {
        return;
    }

    std::string url = m_plugins[index].detailsUrl.toString().toStdString();
    Ret ret = interactive()->openUrl(url);

    if (!ret) {
        LOGE() << ret.toString();
    }
}

QStringList PluginsModel::categories() const
{
    QSet<QString> result;

    for (const PluginInfo& plugin: m_plugins) {
        result << plugin.category;
    }

    return result.values();
}

void PluginsModel::updatePlugin(const PluginInfo& plugin)
{
    for (int i = 0; i < m_plugins.count(); ++i) {
        if (m_plugins[i].codeKey == plugin.codeKey) {
            PluginInfo tmp = m_plugins[i];
            m_plugins[i] = plugin;
            m_plugins[i].thumbnailUrl = tmp.thumbnailUrl;
            m_plugins[i].category = tmp.category;
            QModelIndex index = createIndex(i, 0);
            emit dataChanged(index, index);
            return;
        }
    }
}

int PluginsModel::itemIndexByCodeKey(const QString& codeKey) const
{
    for (int i = 0; i < m_plugins.count(); ++i) {
        if (m_plugins[i].codeKey == codeKey) {
            return i;
        }
    }

    return INVALID_INDEX;
}
