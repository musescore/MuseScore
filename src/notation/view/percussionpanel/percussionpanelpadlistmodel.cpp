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

#include "percussionpanelpadlistmodel.h"

PercussionPanelPadListModel::PercussionPanelPadListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant PercussionPanelPadListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_padModels.count()) {
        return QVariant();
    }

    PercussionPanelPadModel* item = m_padModels.at(index.row());

    switch (role) {
    case PadModelRole: return QVariant::fromValue(item);
    default: break;
    }

    return QVariant();
}

QHash<int, QByteArray> PercussionPanelPadListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { PadModelRole, "padModelRole" },
    };
    return roles;
}

void PercussionPanelPadListModel::load()
{
    m_padModels = createDefaultItems();
    emit layoutChanged();
    emit numPadsChanged();
}

void PercussionPanelPadListModel::addRow()
{
    for (size_t i = 0; i < NUM_COLUMNS; ++i) {
        m_padModels.append(new PercussionPanelPadModel(QObject::parent()));
    }
    emit layoutChanged();
    emit numPadsChanged();
}

void PercussionPanelPadListModel::deleteRow(int row)
{
    m_padModels.remove(row * NUM_COLUMNS, NUM_COLUMNS);
    emit layoutChanged();
    emit numPadsChanged();
}

void PercussionPanelPadListModel::startDrag(int startIndex)
{
    m_dragStartIndex = startIndex;
}

void PercussionPanelPadListModel::endDrag(int endIndex)
{
    movePad(m_dragStartIndex, endIndex);
    m_dragStartIndex = -1;
}

bool PercussionPanelPadListModel::isDragActive() const
{
    return m_dragStartIndex > -1;
}

void PercussionPanelPadListModel::resetLayout()
{
    beginResetModel();
    m_padModels = createDefaultItems();
    endResetModel();

    emit numPadsChanged();
}

QList<PercussionPanelPadModel*> PercussionPanelPadListModel::createDefaultItems()
{
    QMap<size_t, QString> dummyInstruments;
    dummyInstruments.insert(2, "Bass Drum (Kit): Mid");
    dummyInstruments.insert(0, "Bass Drum (Kit): Low");
    dummyInstruments.insert(8, "Hi-hat (Kit)");
    dummyInstruments.insert(17, "Splash (Kit)");
    dummyInstruments.insert(11, "Tom (Kit): High");
    dummyInstruments.insert(9, "Tom (Kit): Mid");
    dummyInstruments.insert(12, "Crash (Kit)");
    dummyInstruments.insert(3, "Snare (Kit)");
    dummyInstruments.insert(15, "Ride (Kit)");
    dummyInstruments.insert(5, "Floor Tom (Kit)");

    // Get the largest key and round up to the nearest multiple of 8
    size_t maxIndex = dummyInstruments.lastKey();
    maxIndex = std::ceil(maxIndex / (double)NUM_COLUMNS) * NUM_COLUMNS;

    QList<PercussionPanelPadModel*> padModels;

    for (size_t i = 0; i < maxIndex; ++i) {
        PercussionPanelPadModel* model = new PercussionPanelPadModel(this);
        if (!dummyInstruments.value(i).isEmpty()) {
            model->setInstrumentName(dummyInstruments.value(i));
            model->setIsEmptySlot(false);
        }
        padModels.append(model);
    }

    return padModels;
}

void PercussionPanelPadListModel::movePad(int fromIndex, int toIndex)
{
    m_padModels.swapItemsAt(fromIndex, toIndex);
    emit layoutChanged();
}
