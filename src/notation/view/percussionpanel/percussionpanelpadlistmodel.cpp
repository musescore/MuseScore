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

#include "notation/utilities/engravingitempreviewpainter.h"
#include "notation/utilities/percussionutilities.h"

using namespace mu::notation;

PercussionPanelPadListModel::PercussionPanelPadListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

PercussionPanelPadListModel::~PercussionPanelPadListModel()
{
    delete m_drumset;
    m_drumset = nullptr;
}

QVariant PercussionPanelPadListModel::data(const QModelIndex& index, int role) const
{
    if (!indexIsValid(index.row())) {
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

void PercussionPanelPadListModel::init()
{
    m_padModels.clear();
    addEmptyRow();
}

void PercussionPanelPadListModel::addEmptyRow()
{
    for (size_t i = 0; i < NUM_COLUMNS; ++i) {
        m_padModels.append(nullptr);
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

void PercussionPanelPadListModel::removeEmptyRows()
{
    bool rowsRemoved = false;
    const int lastRowIndex = numPads() / NUM_COLUMNS - 1;
    for (int i = lastRowIndex; i >= 0; --i) {
        const int numRows = numPads() / NUM_COLUMNS;
        if (rowIsEmpty(i) && numRows > 1) { // never delete the first row
            m_padModels.remove(i * NUM_COLUMNS, NUM_COLUMNS);
            rowsRemoved = true;
        }
    }
    if (rowsRemoved) {
        emit layoutChanged();
        emit numPadsChanged();
    }
}

bool PercussionPanelPadListModel::rowIsEmpty(int row) const
{
    return numEmptySlotsAtRow(row) == NUM_COLUMNS;
}

void PercussionPanelPadListModel::startDrag(int startIndex)
{
    m_dragStartIndex = startIndex;
}

void PercussionPanelPadListModel::endDrag(int endIndex)
{
    if (indexIsValid(m_dragStartIndex) && indexIsValid(endIndex)) {
        movePad(m_dragStartIndex, endIndex);
    } else {
        emit layoutChanged();
    }
    m_dragStartIndex = -1;
}

void PercussionPanelPadListModel::setDrumset(const engraving::Drumset* drumset)
{
    const bool drumsetWasValid = m_drumset;

    delete m_drumset;
    m_drumset = drumset ? new engraving::Drumset(*drumset) : nullptr;

    if (drumsetWasValid ^ bool(m_drumset)) {
        m_hasActivePadsChanged.notify();
    }

    load();
    removeEmptyRows();
}

void PercussionPanelPadListModel::load()
{
    beginResetModel();

    m_padModels.clear();

    if (!m_drumset) {
        // Add an empty row...
        for (size_t i = 0; i < NUM_COLUMNS; ++i) {
            m_padModels.append(nullptr);
        }
        endResetModel();
        emit numPadsChanged();
        return;
    }

    QMap<int /*index*/, PercussionPanelPadModel*> modelsMap;
    QMap<int /*index*/, PercussionPanelPadModel*> modelsToAppend;

    for (int pitch = 0; pitch < mu::engraving::DRUM_INSTRUMENTS; ++pitch) {
        if (!m_drumset->isValid(pitch)) {
            continue;
        }

        PercussionPanelPadModel* model = createPadModelForPitch(pitch);
        const int modelIndex = createModelIndexForPitch(pitch);

        if (modelIndex < 0) {
            // Sometimes a drum won't have a defined row/column - e.g. if it was newly added through the customize kit
            // dialog. In this case we'll save it for later, and append it once the rest of the layout has been decided...
            modelsToAppend.insert(modelsToAppend.size(), model);
            continue;
        }

        modelsMap.insert(modelIndex, model);
    }

    int requiredSize = modelsMap.isEmpty() ? 0 : modelsMap.lastKey() + 1;

    for (int i = 0; i < modelsToAppend.size(); ++i) {
        PercussionPanelPadModel* model = modelsToAppend.value(i);
        engraving::DrumInstrument& drum = m_drumset->drum(model->pitch());
        drum.panelRow = requiredSize / NUM_COLUMNS;
        drum.panelColumn = requiredSize % NUM_COLUMNS;
        modelsMap.insert(requiredSize++, model);
    }

    // Round up to nearest multiple of NUM_COLUMNS
    requiredSize = ((requiredSize + NUM_COLUMNS - 1) / NUM_COLUMNS) * NUM_COLUMNS;

    //! NOTE: There is an argument that m_padModels itself should be a map instead of a list, as this would
    //! prevent the following double work. In practice, however, this makes some other operations (such as
    //! adding and removing rows) more complex and significantly less intuitive.
    m_padModels = QList<PercussionPanelPadModel*>(requiredSize);
    for (int i = 0; i < m_padModels.size(); ++i) {
        m_padModels.replace(i, modelsMap.value(i));
    }

    endResetModel();

    emit numPadsChanged();
}

bool PercussionPanelPadListModel::indexIsValid(int index) const
{
    return index > -1 && index < m_padModels.count();
}

PercussionPanelPadModel* PercussionPanelPadListModel::createPadModelForPitch(int pitch)
{
    IF_ASSERT_FAILED(m_drumset && m_drumset->isValid(pitch)) {
        return nullptr;
    }

    PercussionPanelPadModel* model = new PercussionPanelPadModel(this);
    model->setInstrumentName(m_drumset->name(pitch));

    const QString shortcut = m_drumset->shortcut(pitch) ? QChar(m_drumset->shortcut(pitch)) : QString("-");
    model->setKeyboardShortcut(shortcut);

    model->setPitch(pitch);

    model->padTriggered().onNotify(this, [this, pitch]() {
        m_triggeredChannel.send(pitch);
    });

    model->setNotationPreviewItem(PercussionUtilities::getDrumNoteForPreview(m_drumset, pitch));

    return model;
}

int PercussionPanelPadListModel::createModelIndexForPitch(int pitch) const
{
    IF_ASSERT_FAILED(m_drumset && m_drumset->isValid(pitch)) {
        return -1;
    }

    const int panelRow = m_drumset->panelRow(pitch);
    const int panelColumn = m_drumset->panelColumn(pitch);

    IF_ASSERT_FAILED(panelColumn < NUM_COLUMNS) {
        LOGE() << "Percussion panel - column out of bounds for " << m_drumset->name(pitch);
        return -1;
    }

    if (panelRow < 0 || panelColumn < 0) {
        // No row/column was specified for this pitch...
        return -1;
    }

    const int modelIndex = panelRow * NUM_COLUMNS + panelColumn;

    const PercussionPanelPadModel* existingModel = modelIndex < m_padModels.size() ? m_padModels.at(modelIndex) : nullptr;
    IF_ASSERT_FAILED(!existingModel) {
        const int existingDrumPitch = existingModel->pitch();
        LOGE() << "Percussion panel - error when trying to load pad for " << m_drumset->name(pitch) << "; pad for "
               << m_drumset->name(existingDrumPitch) << " already exists at row " << panelRow << ", column " << panelColumn;
        return -1;
    }

    return modelIndex;
}

void PercussionPanelPadListModel::movePad(int fromIndex, int toIndex)
{
    const int fromRow = fromIndex / NUM_COLUMNS;
    const int toRow = toIndex / NUM_COLUMNS;

    // fromRow will become empty if there's only 1 "occupied" slot, toRow will no longer be empty if it was previously...
    const bool fromRowEmptyChanged = numEmptySlotsAtRow(fromRow) == NUM_COLUMNS - 1;
    const bool toRowEmptyChanged = rowIsEmpty(toRow);

    m_padModels.swapItemsAt(fromIndex, toIndex);
    emit layoutChanged();

    if (fromRowEmptyChanged) {
        emit rowIsEmptyChanged(fromRow, /*isEmpty*/ true);
    }

    if (toRowEmptyChanged) {
        emit rowIsEmptyChanged(toRow, /*isEmpty*/ false);
    }
}

int PercussionPanelPadListModel::numEmptySlotsAtRow(int row) const
{
    int count = 0;
    const size_t rowStartIdx = row * NUM_COLUMNS;
    for (size_t i = rowStartIdx; i < rowStartIdx + NUM_COLUMNS; ++i) {
        if (!m_padModels.at(i)) {
            ++count;
        }
    }
    return count;
}
