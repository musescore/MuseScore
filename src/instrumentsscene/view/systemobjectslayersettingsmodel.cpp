/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "systemobjectslayersettingsmodel.h"

#include "layoutpanelutils.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

SystemObjectsLayerSettingsModel::SystemObjectsLayerSettingsModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

int SystemObjectsLayerSettingsModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_systemObjectGroups.size());
}

QVariant SystemObjectsLayerSettingsModel::data(const QModelIndex& modelIndex, int role) const
{
    const size_t index = static_cast<size_t>(modelIndex.row());
    if (index >= m_systemObjectGroups.size()) {
        return QVariant();
    }

    const SystemObjectsGroup& group = m_systemObjectGroups.at(index);

    switch (role) {
    case TitleRole: return translatedSystemObjectsGroupName(group).toQString();
    case VisibilityRole: return isSystemObjectsGroupVisible(group);
    }

    return QVariant();
}

QHash<int, QByteArray> SystemObjectsLayerSettingsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { TitleRole, "title" },
        { VisibilityRole, "visible" },
    };

    return roles;
}

void SystemObjectsLayerSettingsModel::load(const QString& staffId)
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    const Staff* staff = notation->parts()->staff(staffId);
    if (!staff) {
        return;
    }

    beginResetModel();
    m_systemObjectGroups = collectSystemObjectGroups(staff);
    endResetModel();
}

void SystemObjectsLayerSettingsModel::setSystemObjectsGroupVisible(int index, bool visible)
{
    const size_t idx = static_cast<size_t>(index);
    if (idx >= m_systemObjectGroups.size()) {
        return;
    }

    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    SystemObjectsGroup& group = m_systemObjectGroups.at(idx);

    const muse::TranslatableString actionName = visible
                                                ? TranslatableString("undoableAction", "Make system object(s) visible")
                                                : TranslatableString("undoableAction", "Make system object(s) invisible");

    notation->undoStack()->prepareChanges(actionName);

    for (EngravingItem* item : group.items) {
        item->undoSetVisible(visible);
    }

    notation->undoStack()->commitChanges();
    notation->notationChanged().notify();

    QModelIndex modelIdx = createIndex(index, 0);
    emit dataChanged(modelIdx, modelIdx, { VisibilityRole });
}
