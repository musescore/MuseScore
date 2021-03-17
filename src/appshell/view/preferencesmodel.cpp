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
#include "preferencesmodel.h"

#include "log.h"
#include "translation.h"
#include "ui/view/iconcodes.h"

using namespace mu::appshell;
using namespace mu::ui;

PreferencesModel::PreferencesModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

PreferencesModel::~PreferencesModel()
{
    delete m_rootItem;
    m_rootItem = nullptr;
}

QModelIndex PreferencesModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    PreferencePageItem* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = modelIndexToItem(parent);
    }

    if (!parentItem) {
        return QModelIndex();
    }

    PreferencePageItem* childItem = parentItem->childAtRow(row);

    if (childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex PreferencesModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    PreferencePageItem* childItem = modelIndexToItem(child);
    PreferencePageItem* parentItem = qobject_cast<PreferencePageItem*>(childItem->parentItem());

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int PreferencesModel::rowCount(const QModelIndex& parent) const
{
    PreferencePageItem* parentItem;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = modelIndexToItem(parent);
    }

    if (!parentItem) {
        return 0;
    }

    return parentItem->childCount();
}

int PreferencesModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant PreferencesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() && role != ItemRole) {
        return QVariant();
    }

    PreferencePageItem* item = modelIndexToItem(index);

    if (!item) {
        return QVariant();
    }

    return QVariant::fromValue(qobject_cast<QObject*>(item));
}

QHash<int, QByteArray> PreferencesModel::roleNames() const
{
    return { { ItemRole, "itemRole" } };
}

QString PreferencesModel::currentMenuId() const
{
    return m_currentMenuId;
}

void PreferencesModel::load()
{
    beginResetModel();

    m_rootItem = new PreferencePageItem();

    QList<PreferencePageItem*> generalItems {
        makeItem("general-start", qtrc("appshell", "Programme Start")),
        makeItem("general-folders", qtrc("appshell", "Folders"))
    };

    QList<PreferencePageItem*> items {
        makeItem("general", qtrc("appshell", "General"), IconCode::Code::SETTINGS_COG, generalItems),
        makeItem("appearance", qtrc("appshell", "Appearance"), IconCode::Code::VISIBILITY_ON),
        makeItem("canvas", qtrc("appshell", "Canvas"), IconCode::Code::NEW_FILE),
        makeItem("note-input", qtrc("appshell", "Note Input"), IconCode::Code::EDIT),
        makeItem("midi-device-mapping", qtrc("appshell", "Midi Device Mapping"), IconCode::Code::MIDI_INPUT),
        makeItem("score", qtrc("appshell", "Score"), IconCode::Code::SCORE),
        makeItem("io", qtrc("appshell", "I/O"), IconCode::Code::AUDIO),
        makeItem("import", qtrc("appshell", "Import"), IconCode::Code::IMPORT),
        makeItem("shortcuts", qtrc("appshell", "Shortcuts"), IconCode::Code::SHORTCUTS),
        makeItem("update", qtrc("appshell", "Update"), IconCode::Code::UPDATE),
        makeItem("advanced", qtrc("appshell", "Advanced"), IconCode::Code::CONFIGURE)
    };

    for (PreferencePageItem* item: items) {
        m_rootItem->appendChild(item);
    }

    endResetModel();
}

void PreferencesModel::resetFactorySettings()
{
    NOT_IMPLEMENTED;
}

bool PreferencesModel::apply()
{
    NOT_IMPLEMENTED;
    return false;
}

void PreferencesModel::setCurrentMenuId(QString currentMenuId)
{
    if (m_currentMenuId == currentMenuId) {
        return;
    }

    m_currentMenuId = currentMenuId;
    emit currentMenuIdChanged(m_currentMenuId);
}

PreferencePageItem* PreferencesModel::makeItem(const QString& id, const QString& title, mu::ui::IconCode::Code icon,
                                               const QList<PreferencePageItem*>& children) const
{
    PreferencePageItem* item = new PreferencePageItem();
    item->setId(id);
    item->setTitle(title);
    item->setIcon(icon);

    for (PreferencePageItem* child: children) {
        item->appendChild(child);
    }

    return item;
}

PreferencePageItem* PreferencesModel::modelIndexToItem(const QModelIndex& index) const
{
    return static_cast<PreferencePageItem*>(index.internalPointer());
}
