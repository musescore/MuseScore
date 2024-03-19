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

#include "extensionsmodel.h"

#include "translation.h"
#include "shortcuts/shortcutstypes.h"

#include "log.h"

using namespace mu::extensions;
using namespace mu::async;

static constexpr int INVALID_INDEX = -1;

ExtensionsListModel::ExtensionsListModel(QObject* parent)
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

void ExtensionsListModel::load()
{
    beginResetModel();

    m_plugins = provider()->manifestList();
    if (m_plugins.empty()) {
        LOGE() << "Not found plugins";
        return;
    }

    std::sort(m_plugins.begin(), m_plugins.end(), [](const Manifest& l, const Manifest& r) {
        return l.title < r.title;
    });

    Channel<Manifest> manifestChanged = provider()->manifestChanged();
    manifestChanged.onReceive(this, [this](const Manifest& plugin) {
        updatePlugin(plugin);
    });

    endResetModel();
}

QVariant ExtensionsListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Manifest plugin = m_plugins.at(index.row());

    switch (role) {
    case rCode:
        return QString::fromStdString(plugin.uri.toString());
    case rName:
        return plugin.title.toQString();
    case rDescription:
        return plugin.description.toQString();
    case rThumbnailUrl:
        if (plugin.thumbnail.empty()) {
            return "qrc:/qml/Muse/Extensions/internal/resources/placeholder.png";
        }

        return QUrl::fromLocalFile(plugin.thumbnail.toQString());
    case rEnabled:
        return plugin.config.enabled;
    case rCategory:
        return plugin.category.toQString();
    case rVersion:
        return plugin.version.toQString();
    case rShortcuts:
        if (!plugin.config.shortcuts.empty()) {
            return shortcuts::sequencesToNativeText(shortcuts::Shortcut::sequencesFromString(plugin.config.shortcuts));
        }

        //: No keyboard shortcut is assigned to this plugin.
        return qtrc("extensions", "Not defined");
    }

    return QVariant();
}

int ExtensionsListModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_plugins.size());
}

QHash<int, QByteArray> ExtensionsListModel::roleNames() const
{
    return m_roles;
}

void ExtensionsListModel::setEnable(const QString& uri, bool enable)
{
    Ret ret = provider()->setEnable(Uri(uri.toStdString()), enable);
    emit finished();

    if (!ret) {
        LOGE() << ret.toString();
    }
}

void ExtensionsListModel::editShortcut(QString codeKey)
{
    int index = itemIndexByCodeKey(codeKey);
    if (index == INVALID_INDEX) {
        return;
    }

    UriQuery uri("musescore://preferences");
    uri.addParam("currentPageId", Val("shortcuts"));

    QVariantMap params;
    params["shortcutCodeKey"] = codeKey;
    uri.addParam("params", Val::fromQVariant(params));

    RetVal<Val> retVal = interactive()->open(uri);

    if (!retVal.ret) {
        LOGE() << retVal.ret.toString();
    }
}

void ExtensionsListModel::reloadPlugins()
{
    provider()->reloadPlugins();
}

QVariantList ExtensionsListModel::categories() const
{
    QVariantList result;

    for (const auto& category : provider()->knownCategories()) {
        QVariantMap obj;
        obj["code"] = QString::fromStdString(category.first);
        obj["title"] = category.second.qTranslated();

        result << obj;
    }

    return result;
}

void ExtensionsListModel::updatePlugin(const Manifest& plugin)
{
    for (size_t i = 0; i < m_plugins.size(); ++i) {
        if (m_plugins.at(i).uri == plugin.uri) {
            Manifest tmp = m_plugins.at(i);
            m_plugins[i] = plugin;
            m_plugins[i].thumbnail = tmp.thumbnail;
            m_plugins[i].category = tmp.category;
            QModelIndex index = createIndex(int(i), 0);
            emit dataChanged(index, index);
            return;
        }
    }
}

int ExtensionsListModel::itemIndexByCodeKey(const QString& uri_) const
{
    Uri uri(uri_.toStdString());
    for (size_t i = 0; i < m_plugins.size(); ++i) {
        if (m_plugins[i].uri == uri) {
            return static_cast<int>(i);
        }
    }

    return INVALID_INDEX;
}
