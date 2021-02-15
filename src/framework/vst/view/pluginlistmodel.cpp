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
#include "pluginlistmodel.h"
#include "log.h"

using namespace mu::vst;

PluginListModel::PluginListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

PluginListModel::PluginListModel(std::shared_ptr<VSTScanner> scaner, QObject* parent)
    : QAbstractListModel(parent), m_scanner(scaner)
{
    update();
}

Qt::ItemFlags PluginListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() || !m_scanner) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsSelectable;
}

int PluginListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_scanner) {
        return 0;
    }

    return m_scanner->getPlugins().size();
}

QHash<int, QByteArray> PluginListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[NameRole] = "name";
    names[UidRole]  = "uid";
    return names;
}

QVariant PluginListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_scanner) {
        return QVariant();
    }

    if (index.row() >= m_plugins.size()) {
        LOGE() << "wrong plugin index";
        return QVariant();
    }

    auto plugin = m_plugins.at(index.row());
    QVariant r;
    switch (role) {
    case NameRole:
        r.setValue(QString::fromStdString(plugin.getName()));
        break;
    case UidRole:
        r.setValue(QString::fromStdString(plugin.getId()));
        break;
    }

    return r;
}

void PluginListModel::update()
{
    m_plugins.clear();
    m_scanner->scan();
    for (auto&& p : m_scanner->getPlugins()) {
        m_plugins.push_back(p.second);
    }
}

const Plugin PluginListModel::nullPlugin = Plugin();
const Plugin& PluginListModel::item(unsigned int index)
{
    auto size = static_cast<unsigned int>(m_plugins.size());
    IF_ASSERT_FAILED(index < size) {
        LOGE() << "index out of range";
        return nullPlugin;
    }
    return m_plugins.at(index);
}
