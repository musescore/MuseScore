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

PluginsModel::PluginsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles.insert(rCode, "code");
    m_roles.insert(rName, "name");
    m_roles.insert(rDescription, "description");
    m_roles.insert(rThumbnailUrl, "thumbnailUrl");
    m_roles.insert(rInstalled, "installed");
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

    for (int i = 0; i < 20; ++i) {
        Plugin plugin;

        plugin.codeKey = QString("codeKey%1").arg(i);
        plugin.name = "Some plugin name";
        plugin.description =
                "It is a period of civil war. "
                "Rebel spaceships, striking "
                "from a hidden base, have won "
                "their first victory against "
                "the evil Galactic Empire. "
                "During the battle, Rebel "
                "spies managed to steal secret "
                "plans to the Empire's "
                "ultimate weapon, the DEATH "
                "STAR, an armored space "
                "station with enough power to "
                "destroy an entire planet. "
                "Pursued by the Empire's "
                "sinister agents, Princess "
                "Leia races home aboard her "
                "starship, custodian of the "
                "stolen plans that can save "
                "her people and restore "
                "freedom to the galaxy.....";

        plugin.installed = i % 2;
        plugin.hasUpdate = i % 3;
        plugin.thumbnailUrl = QUrl(thumbnailUrlExamples[i % 5]);

        m_plugins.push_back(plugin);
    }

    endResetModel();
}

void PluginsModel::setCurrentPlugin(QString codeKey)
{
    m_currentPluginCodeKey = codeKey;
}

QVariant PluginsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Plugin plugin = m_plugins[index.row()];

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

void PluginsModel::install()
{
    NOT_IMPLEMENTED;
}

void PluginsModel::uninstall()
{
    Ret ret = service()->uninstall(m_currentPluginCodeKey);

    if (!ret) {
        LOGE() << ret.toString();
    }
}

void PluginsModel::update()
{
    NOT_IMPLEMENTED;
}

void PluginsModel::restart()
{
    Ret ret = service()->stop(m_currentPluginCodeKey);

    if (!ret) {
        LOGE() << ret.toString();
        return;
    }

    ret = service()->start(m_currentPluginCodeKey);

    if (!ret) {
        LOGE() << ret.toString();
    }
}

void PluginsModel::openDetails()
{
    int index = itemIndexByCodeKey(m_currentPluginCodeKey);
    std::string url = m_plugins[index].detailsUrl.toString().toStdString();
    Ret ret = interactive()->openUrl(url);

    if (!ret) {
        LOGE() << ret.toString();
    }
}

int PluginsModel::itemIndexByCodeKey(const QString& codeKey) const
{
    for (int i = 0; i < m_plugins.count(); i++) {
        if (m_plugins[i].codeKey == codeKey) {
            return i;
        }
    }

    return 0;
}
