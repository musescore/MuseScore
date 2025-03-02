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

#include "extensionslistmodel.h"

#include "translation.h"
#include "shortcuts/shortcutstypes.h"

#include "log.h"

using namespace muse::extensions;
using namespace muse::async;

static constexpr int INVALID_INDEX = -1;

ExtensionsListModel::ExtensionsListModel(QObject* parent)
    : QAbstractListModel(parent), Injectable(muse::iocCtxForQmlObject(this))
{
    m_roles = {
        { rUri, "uri" },
        { rName, "name" },
        { rDescription, "description" },
        { rThumbnailUrl, "thumbnailUrl" },
        { rEnabled, "enabled" },
        { rCategory, "category" },
        { rVersion, "version" },
        { rShortcuts, "shortcuts" },
    };
}

void ExtensionsListModel::load()
{
    provider()->manifestListChanged().onNotify(this, [this]() {
        load();
    });

    beginResetModel();

    m_plugins = provider()->manifestList();
    if (m_plugins.empty()) {
        LOGE() << "Not found plugins";
        endResetModel();
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
    case rUri:
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
        return plugin.enabled();
    case rCategory:
        return plugin.category.toQString();
    case rVersion:
        if (plugin.version.empty()) {
            //: No version is specified for this plugin.
            return muse::qtrc("extensions", "Not specified");
        }
        return plugin.version.toQString();
    case rShortcuts: {
        std::vector<std::string> shortcuts;
        for (const auto& action : plugin.actions) {
            actions::ActionCode code = makeActionCode(plugin.uri, action.code);
            shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut(code);
            shortcuts.insert(shortcuts.end(), shortcut.sequences.cbegin(), shortcut.sequences.cend());
        }

        if (!shortcuts.empty()) {
            return shortcuts::sequencesToNativeText(shortcuts);
        }

        //: No keyboard shortcut is assigned to this plugin.
        return muse::qtrc("extensions", "Not defined");
    }
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

const std::vector<ExecPoint>& ExtensionsListModel::execPoints(const QString& uri) const
{
    if (m_execPointsCache.uri != uri) {
        m_execPointsCache.uri = uri;
        m_execPointsCache.points = provider()->execPoints(Uri(uri.toStdString()));
    }
    return m_execPointsCache.points;
}

int ExtensionsListModel::currentExecPointIndex(const QString& uri) const
{
    ExecPointName currentName;
    Manifest m = provider()->manifest(Uri(uri.toStdString()));
    IF_ASSERT_FAILED(m.actions.size() > 0) {
        return 0;
    }

    //! NOTE For complex extensions, execution point selection is not currently supported.
    if (m.actions.size() > 1) {
        currentName = m.enabled() ? EXEC_MANUALLY : EXEC_DISABLED;
    } else {
        currentName = m.config.aconfig(m.actions.at(0).code).execPoint;
    }

    const std::vector<ExecPoint>& points = execPoints(uri);
    for (size_t i = 0; i < points.size(); ++i) {
        if (points.at(i).name == currentName) {
            return int(i);
        }
    }
    return 0;
}

QVariantList ExtensionsListModel::execPointsModel(const QString& uri) const
{
    QVariantList model;
    const std::vector<ExecPoint>& points = execPoints(uri);
    for (const ExecPoint& p : points) {
        QVariantMap item;
        item["text"] = p.title.qTranslated();
        item["value"] = QString::fromStdString(p.name);
        model << item;
    }
    return model;
}

void ExtensionsListModel::selectExecPoint(const QString& uri, int index)
{
    const std::vector<ExecPoint>& points = execPoints(uri);

    provider()->setExecPoint(Uri(uri.toStdString()), points.at(index).name);

    emit finished();
}

void ExtensionsListModel::editShortcut(const QString& extensionUri)
{
    int index = itemIndexByUri(extensionUri);
    if (index == INVALID_INDEX) {
        return;
    }

    QString actionCodeBase
        = QString::fromStdString(makeActionCodeBase(Uri(extensionUri.toStdString())));

    UriQuery preferencesUri("muse://preferences");
    preferencesUri.addParam("currentPageId", Val("shortcuts"));

    QVariantMap params;
    params["shortcutCodeKey"] = actionCodeBase;
    preferencesUri.addParam("params", Val::fromQVariant(params));

    RetVal<Val> retVal = interactive()->open(preferencesUri);

    if (!retVal.ret) {
        LOGE() << retVal.ret.toString();
    }
}

void ExtensionsListModel::reloadPlugins()
{
    provider()->reloadExtensions();
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

int ExtensionsListModel::itemIndexByUri(const QString& uri_) const
{
    Uri uri(uri_.toStdString());
    for (size_t i = 0; i < m_plugins.size(); ++i) {
        if (m_plugins[i].uri == uri) {
            return static_cast<int>(i);
        }
    }

    return INVALID_INDEX;
}
