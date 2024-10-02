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
    QMap<size_t, PadInfo> dummyPads;
    dummyPads.insert(2, { "Bass Drum (Kit): Mid", "S", "B2" });
    dummyPads.insert(0, { "This is a pad with a really long text label that truncates.", "A", "F2" });
    dummyPads.insert(8, { "Hi-hat (Kit)", "J", "G#2" });
    dummyPads.insert(17, { "Splash (Kit)", "X", "F3" });
    dummyPads.insert(11, { "Tom (Kit): High", "L", "A#2" });
    dummyPads.insert(9, { "Tom (Kit): Mid", "K", "F#2" });
    dummyPads.insert(12, { "Crash (Kit)", ";", "D#3" });
    dummyPads.insert(3, { "Snare (Kit)", "D", "D3" });
    dummyPads.insert(15, { "Ride (Kit)", "Z", "E3" });
    dummyPads.insert(5, { "Floor Tom (Kit)", "F", "D2" });

    // Get the largest key and round up to the nearest multiple of 8
    size_t maxIndex = dummyPads.lastKey();
    maxIndex = std::ceil(maxIndex / (double)NUM_COLUMNS) * NUM_COLUMNS;

    QList<PercussionPanelPadModel*> padModels;

    for (size_t i = 0; i < maxIndex; ++i) {
        PercussionPanelPadModel* model = new PercussionPanelPadModel(this);
        const PadInfo info = dummyPads.value(i);
        if (info.isValid()) {
            model->setInstrumentName(info.instrumentName);

            model->setKeyboardShortcut(info.keyboardShortcut);
            model->setMidiNote(info.midiNote);

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
