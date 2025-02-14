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

void PercussionPanelPadListModel::addEmptyRow(bool focusFirstInNewRow)
{
    for (size_t i = 0; i < NUM_COLUMNS; ++i) {
        m_padModels.append(nullptr);
    }
    emit layoutChanged();
    emit numPadsChanged();

    if (focusFirstInNewRow) {
        const int indexToFocus = numPads() - NUM_COLUMNS;
        emit padFocusRequested(indexToFocus);
    }
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

void PercussionPanelPadListModel::startPadSwap(int startIndex)
{
    m_padSwapStartIndex = startIndex;
}

void PercussionPanelPadListModel::endPadSwap(int endIndex)
{
    const auto endSwap = [this, endIndex]() {
        m_padSwapStartIndex = -1;
        emit padFocusRequested(endIndex);
    };

    if (m_padSwapStartIndex == endIndex || !indexIsValid(m_padSwapStartIndex) || !indexIsValid(endIndex)) {
        // Put everything back where it was...
        emit layoutChanged();
        endSwap();
        return;
    }

    movePad(m_padSwapStartIndex, endIndex);

    //! NOTE: "Pad swap options" and the associated dialog were dropped from percussion panel MVP (version 4.5).
    //! See PR #25810 when re-implementing...
    endSwap();
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

mu::engraving::Drumset PercussionPanelPadListModel::constructDefaultLayout(const engraving::Drumset& defaultDrumset) const
{
    //! NOTE: The idea of this method is take a "default" (template) drumset, find matching drums in the current drumset, and evaluate/return
    //! the default panel layout based on this information. The reason we can't simply revert to the default drumset in its entirety is that
    //! there's no guarantee that all drums will match (the user may have added some of their own drums, removed some, tweaked some, etc). Any
    //! drums that aren't accounted for in the default drumset are appended chromatically once the rest of the layout has been decided...

    mu::engraving::Drumset defaultLayout = *m_drumset;

    int highestIndex = -1;
    QList<int /*pitch*/> noTemplateFound;

    for (int pitch = 0; pitch < mu::engraving::DRUM_INSTRUMENTS; ++pitch) {
        if (defaultDrumset.isValid(pitch) && !defaultLayout.isValid(pitch)) {
            // Pitch was deleted - restore it...
            defaultLayout.drum(pitch) = defaultDrumset.drum(pitch);
            continue;
        }
        //! NOTE: Pitch + drum name isn't exactly the most robust identifier, but this will probably change with the new percussion ID system
        if (!defaultDrumset.isValid(pitch) || defaultLayout.name(pitch) != defaultDrumset.name(pitch)) {
            // Drum is valid, but we can't find a template for it. Set the position chromatically later...
            noTemplateFound.emplaceBack(pitch);
            continue;
        }

        const int templateRow = defaultDrumset.drum(pitch).panelRow;
        const int templateColumn = defaultDrumset.drum(pitch).panelColumn;

        defaultLayout.drum(pitch).panelRow = templateRow;
        defaultLayout.drum(pitch).panelColumn = templateColumn;

        const int modelIndex = templateRow * NUM_COLUMNS + templateColumn;

        if (modelIndex > highestIndex) {
            highestIndex = modelIndex;
        }
    }

    for (int pitch : noTemplateFound) {
        ++highestIndex;
        defaultLayout.drum(pitch).panelRow = highestIndex / NUM_COLUMNS;
        defaultLayout.drum(pitch).panelColumn = highestIndex % NUM_COLUMNS;
    }

    return defaultLayout;
}

void PercussionPanelPadListModel::focusFirstActivePad()
{
    for (int i = 0; i < m_padModels.size(); i++) {
        if (m_padModels.at(i)) {
            emit padFocusRequested(i);
            return;
        }
    }
}

void PercussionPanelPadListModel::focusLastActivePad()
{
    for (int i = m_padModels.size() - 1; i >= 0; --i) {
        if (m_padModels.at(i)) {
            emit padFocusRequested(i);
            return;
        }
    }
}

int PercussionPanelPadListModel::nextAvailableIndex(int pitch) const
{
    const int currentModelIndex = getModelIndexForPitch(pitch);
    for (int candidateIndex = currentModelIndex + 1; candidateIndex != currentModelIndex; ++candidateIndex) {
        if (candidateIndex == m_padModels.size()) {
            // Wrap around
            candidateIndex = 0;
        }
        if (!m_padModels.at(candidateIndex)) {
            return candidateIndex;
        }
    }
    return m_padModels.size();
}

int PercussionPanelPadListModel::nextAvailablePitch(int pitch) const
{
    for (int candidatePitch = pitch + 1; candidatePitch != pitch; ++candidatePitch) {
        if (candidatePitch == mu::engraving::DRUM_INSTRUMENTS) {
            // Wrap around
            candidatePitch = 0;
        }
        if (!m_drumset->isValid(candidatePitch)) {
            return candidatePitch;
        }
    }
    return -1;
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

    model->setPadName(m_drumset->name(pitch));
    model->setKeyboardShortcut(m_drumset->shortcut(pitch));
    model->setPitch(pitch);

    model->padActionTriggered().onReceive(this, [this, pitch](PercussionPanelPadModel::PadAction action) {
        m_padActionRequestChannel.send(action, pitch);
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

muse::RetVal<muse::Val> PercussionPanelPadListModel::openPadSwapDialog()
{
    const bool moveMidiNotesAndShortcuts = configuration()->percussionPanelMoveMidiNotesAndShortcuts();

    muse::UriQuery query("musescore://notation/percussionpanelpadswap?sync=true&modal=true");
    query.addParam("moveMidiNotesAndShortcuts", muse::Val(moveMidiNotesAndShortcuts));
    muse::RetVal<muse::Val> rv = interactive()->open(query);

    const QVariantMap vals = rv.val.toQVariant().toMap();
    if (!rv.ret) {
        return rv;
    }

    const bool rememberMyChoice = vals["rememberMyChoice"].toBool();
    if (rememberMyChoice) {
        configuration()->setPercussionPanelMoveMidiNotesAndShortcuts(moveMidiNotesAndShortcuts);
    }
    configuration()->setShowPercussionPanelPadSwapDialog(!rememberMyChoice);

    return rv;
}

void PercussionPanelPadListModel::swapMidiNotesAndShortcuts(int fromIndex, int toIndex)
{
    PercussionPanelPadModel* fromModel = m_padModels.at(fromIndex);
    PercussionPanelPadModel* toModel = m_padModels.at(toIndex);

    const int tempPitch = fromModel->pitch();
    const QString tempShortcut = fromModel->keyboardShortcut();

    fromModel->setPitch(toModel->pitch());
    fromModel->setKeyboardShortcut(toModel->keyboardShortcut());

    toModel->setPitch(tempPitch);
    toModel->setKeyboardShortcut(tempShortcut);
}

int PercussionPanelPadListModel::getModelIndexForPitch(int pitch) const
{
    IF_ASSERT_FAILED(m_drumset && m_drumset->isValid(pitch)) {
        return -1;
    }

    for (int i = 0; i < m_padModels.size(); ++i) {
        const PercussionPanelPadModel* model = m_padModels.at(i);
        if (model && model->pitch() == pitch) {
            return i;
        }
    }

    return -1;
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
