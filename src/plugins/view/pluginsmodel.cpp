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

#include "containers.h"
#include "translation.h"
#include "log.h"

#include "shortcuts/shortcutstypes.h"

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
    m_roles.insert(rEnabled, "enabled");
    m_roles.insert(rCategory, "category");
    m_roles.insert(rVersion, "version");
    m_roles.insert(rShortcuts, "shortcuts");
}

void PluginsModel::load()
{
    beginResetModel();
    m_plugins.clear();

    RetVal<PluginInfoMap> plugins = service()->plugins();
    if (!plugins.ret) {
        LOGE() << plugins.ret.toString();
    }

    for (const PluginInfo& plugin : values(plugins.val)) {
        m_plugins << plugin;
    }

    std::sort(m_plugins.begin(), m_plugins.end(), [](const PluginInfo& l, const PluginInfo& r) {
        return l.name < r.name;
    });

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
        //! TODO
        if (plugin.thumbnailUrl.isEmpty()) {
            return "qrc:/qml/MuseScore/Plugins/internal/placeholders/placeholder.jpeg";
        }

        return plugin.thumbnailUrl;
    case rEnabled:
        return plugin.enabled;
    case rCategory:
        return plugin.categoryCode;
    case rVersion:
        return plugin.version.toString();
    case rShortcuts:
        if (!plugin.shortcuts.empty()) {
            return shortcuts::sequencesToNativeText(shortcuts::Shortcut::sequencesFromString(plugin.shortcuts));
        }

        return qtrc("plugins", "Not defined");
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

void PluginsModel::setEnable(const QString& codeKey, bool enable)
{
    Ret ret = service()->setEnable(codeKey, enable);
    emit finished();

    if (!ret) {
        LOGE() << ret.toString();
    }
}

void PluginsModel::editShortcut(QString codeKey)
{
    int index = itemIndexByCodeKey(codeKey);
    if (index == INVALID_INDEX) {
        return;
    }

    UriQuery uri("musescore://preferences");
    uri.addParam("currentPageId", Val("shortcuts"));

    QVariantMap params;
    params["shortcutCodeKey"] = codeKey;
    uri.addParam("params", Val(params));

    RetVal<Val> retVal = interactive()->open(uri);

    if (!retVal.ret) {
        LOGE() << retVal.ret.toString();
    }
}

void PluginsModel::reloadPlugins()
{
    service()->reloadPlugins();
}

QVariantList PluginsModel::categories() const
{
    QVariantList result;

    for (const auto& category: service()->categories()) {
        QVariantMap obj;
        obj["code"] = QString::fromStdString(category.first);
        obj["title"] = QString::fromStdString(category.second);

        result << obj;
    }

    return result;
}

void PluginsModel::updatePlugin(const PluginInfo& plugin)
{
    for (int i = 0; i < m_plugins.count(); ++i) {
        if (m_plugins[i].codeKey == plugin.codeKey) {
            PluginInfo tmp = m_plugins[i];
            m_plugins[i] = plugin;
            m_plugins[i].thumbnailUrl = tmp.thumbnailUrl;
            m_plugins[i].categoryCode = tmp.categoryCode;
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
