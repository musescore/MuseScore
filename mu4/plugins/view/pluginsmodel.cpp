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

#include "pluginsmodel.h"

#include "log.h"

using namespace mu::plugins;
using namespace mu::framework;
using namespace mu::async;

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

    QList<QString> thumbnailUrlExamples {
        "https://i.pinimg.com/originals/94/d8/fc/94d8fcd12fa17160b3adea1805693339.jpg",
        "https://i.pinimg.com/originals/20/50/3d/20503de4f96ace6f1ff06e61a4540c13.png",
        "https://i.pinimg.com/originals/50/b2/13/50b213e13c2f88e74cc7b75efdb09c3c.png",
        "https://i.pinimg.com/originals/ee/b4/ab/eeb4abe7b60f47bfa10ead658e7f95c1.jpg",
        "https://i.pinimg.com/originals/49/2b/c3/492bc31ccc6988cb86f2f9b6356abdc1.png"
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
        plugins.val[i].thumbnailUrl = thumbnailUrlExamples[i % 5];
        plugins.val[i].category = categoriesExamples[i % 5];
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
    if (index == -1) {
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

    return result.toList();
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

    return 0;
}
