/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "abstractselectionfiltermodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::notation;

AbstractSelectionFilterModel::AbstractSelectionFilterModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void AbstractSelectionFilterModel::load()
{
    TRACEFUNC;

    beginResetModel();
    loadTypes();
    endResetModel();

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        emit enabledChanged();

        if (currentNotation()) {
            emit dataChanged(index(0), index(rowCount() - 1), { IsSelectedRole, IsIndeterminateRole });
        }
    });
}

QVariant AbstractSelectionFilterModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= rowCount()) {
        return {};
    }

    auto type = m_types[row];

    switch (role) {
    case TitleRole:
        return titleForType(type);

    case IsSelectedRole:
        return isFiltered(type);

    case IsIndeterminateRole:
        return isIndeterminate(type);
    }

    return {};
}

bool AbstractSelectionFilterModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
    if (role != IsSelectedRole) {
        return false;
    }

    if (!currentNotationInteraction()) {
        return false;
    }

    int row = index.row();
    if (row < 0 || row >= rowCount()) {
        return false;
    }

    auto type = m_types[row];
    const bool filtered = data.toBool();

    setFiltered(type, filtered);
    notifyAboutDataChanged(index, type);
    return true;
}

int AbstractSelectionFilterModel::rowCount(const QModelIndex&) const
{
    return m_types.size();
}

QHash<int, QByteArray> AbstractSelectionFilterModel::roleNames() const
{
    return {
        { TitleRole, "title" },
        { IsSelectedRole, "isSelected" },
        { IsIndeterminateRole, "isIndeterminate" }
    };
}

bool AbstractSelectionFilterModel::enabled() const
{
    return currentNotation() != nullptr;
}

INotationPtr AbstractSelectionFilterModel::currentNotation() const
{
    return globalContext()->currentNotation();
}

INotationInteractionPtr AbstractSelectionFilterModel::currentNotationInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

bool AbstractSelectionFilterModel::isFiltered(const SelectionFilterTypesVariant& variant) const
{
    return currentNotationInteraction() ? currentNotationInteraction()->isSelectionTypeFiltered(variant) : false;
}

void AbstractSelectionFilterModel::setFiltered(const SelectionFilterTypesVariant& variant, bool filtered)
{
    if (currentNotationInteraction()) {
        currentNotationInteraction()->setSelectionTypeFiltered(variant, filtered);
    }
}
