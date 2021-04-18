//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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

#include "vstpluginlistmodelexample.h"
#include "log.h"

using namespace mu::vst;

VstPluginListModelExample::VstPluginListModelExample(QObject* parent)
    : QAbstractListModel(parent)
{
}

VstPluginListModelExample::~VstPluginListModelExample()
{
}

void VstPluginListModelExample::load()
{
    repository()->loadAvailablePlugins();

    RetValCh<VstPluginMetaList> pluginMetaList = repository()->pluginsMetaList();

    if (!pluginMetaList.ret) {
        return;
    }

    updatePluginMetaList(pluginMetaList.val);

    pluginMetaList.ch.onReceive(this, [this](const VstPluginMetaList& newMetaList) {
        updatePluginMetaList(newMetaList);
    });
}

void VstPluginListModelExample::showPluginEditor()
{
    if (m_selectedItemIndex < 0 || m_selectedItemIndex > m_pluginMetaList.size() - 1) {
        return;
    }

    VstPluginMeta meta = m_pluginMetaList.at(m_selectedItemIndex);

    interactive()->open("musescore://vst/editor?sync=false&pluginId=" + meta.id);
}

int VstPluginListModelExample::rowCount(const QModelIndex&) const
{
    return m_pluginMetaList.size();
}

QVariant VstPluginListModelExample::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    VstPluginMeta meta = m_pluginMetaList.at(index.row());
    switch (role) {
    case NameRole: return QString::fromStdString(meta.name);
    case IdRole: return QString::fromStdString(meta.id);
    case PathRole: return QString::fromStdString(meta.path);
    default: return QVariant();
    }
}

QHash<int, QByteArray> VstPluginListModelExample::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { NameRole, "nameRole" },
        { IdRole, "idRole" },
        { PathRole, "pathRole" }
    };
    return roles;
}

int VstPluginListModelExample::selectedItemIndex() const
{
    return m_selectedItemIndex;
}

void VstPluginListModelExample::setSelectedItemIndex(int selectedItemIndex)
{
    if (m_selectedItemIndex == selectedItemIndex) {
        return;
    }

    m_selectedItemIndex = selectedItemIndex;
    emit selectedItemIndexChanged(m_selectedItemIndex);
}

void VstPluginListModelExample::updatePluginMetaList(const VstPluginMetaList& newMetaList)
{
    beginResetModel();

    m_pluginMetaList = newMetaList;

    endResetModel();
}
