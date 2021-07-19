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
#include "noteinputbarcustomisemodel.h"

#include "translation.h"

#include "internal/notationuiactions.h"
#include "workspace/workspacetypes.h"

#include "uicomponents/view/itemmultiselectionmodel.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::workspace;
using namespace mu::ui;
using namespace mu::uicomponents;
using namespace mu::actions;

static const QString NOTE_INPUT_TOOLBAR_NAME("noteInput");

NoteInputBarCustomiseModel::NoteInputBarCustomiseModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_selectionModel = new ItemMultiSelectionModel(this);

    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged,
            [this](const QItemSelection& selected, const QItemSelection& deselected) {
        updateOperationsAvailability();

        QModelIndexList indexes;
        indexes << selected.indexes() << deselected.indexes();
        QSet<QModelIndex> indexesSet(indexes.begin(), indexes.end());
        for (const QModelIndex& index : indexesSet) {
            emit dataChanged(index, index, { SelectedRole });
        }
    });
}

void NoteInputBarCustomiseModel::load()
{
    qDeleteAll(m_items);
    m_items.clear();

    beginResetModel();

    ToolConfig toolConfig = uiConfiguration()->toolConfig(NOTE_INPUT_TOOLBAR_NAME);
    if (!toolConfig.isValid()) {
        toolConfig = NotationUiActions::defaultNoteInputBarConfig();
    }

    for (const ToolConfig::Item& item : toolConfig.items) {
        UiAction action = actionsRegister()->action(item.action);
        m_items << makeItem(action, item.show);
    }

    endResetModel();
}

QVariant NoteInputBarCustomiseModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case ItemRole:
        return QVariant::fromValue(m_items.at(index.row()));
    case SelectedRole:
        return m_selectionModel->isSelected(index);
    }

    return QVariant();
}

int NoteInputBarCustomiseModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int, QByteArray> NoteInputBarCustomiseModel::roleNames() const
{
    static QHash<int, QByteArray> roles = {
        { ItemRole, "itemRole" },
        { SelectedRole, "selectedRole" }
    };

    return roles;
}

bool NoteInputBarCustomiseModel::moveRows(const QModelIndex& sourceParent,
                                          int sourceRow,
                                          int count,
                                          const QModelIndex& destinationParent,
                                          int destinationChild)
{
    int sourceFirstRow = sourceRow;
    int sourceLastRow = sourceRow + count - 1;
    int destinationRow = (sourceLastRow > destinationChild) ? destinationChild : destinationChild + 1;

    beginMoveRows(sourceParent, sourceFirstRow, sourceLastRow, destinationParent, destinationRow);

    int increaseCount = (sourceRow > destinationChild) ? 1 : 0;
    int moveIndex = 0;
    for (int i = 0; i < count; i++) {
        m_items.move(sourceRow + moveIndex, destinationChild + moveIndex);
        moveIndex += increaseCount;
    }

    endMoveRows();

    updateOperationsAvailability();
    saveActions();

    return true;
}

void NoteInputBarCustomiseModel::addSeparatorLine()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    QModelIndex selectedItemIndex = m_selectionModel->selectedIndexes().first();
    if (!selectedItemIndex.isValid()) {
        return;
    }

    QModelIndex prevItemIndex = index(selectedItemIndex.row() - 1);
    if (!prevItemIndex.isValid()) {
        return;
    }

    beginInsertRows(prevItemIndex.parent(), prevItemIndex.row() + 1, prevItemIndex.row() + 1);
    m_items.insert(prevItemIndex.row() + 1, makeSeparatorItem());
    endInsertRows();

    updateOperationsAvailability();
    saveActions();
}

void NoteInputBarCustomiseModel::selectRow(int row)
{
    m_selectionModel->select(index(row));
}

void NoteInputBarCustomiseModel::moveSelectedRowsUp()
{
    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    if (selectedIndexList.isEmpty()) {
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](QModelIndex f, QModelIndex s) -> bool {
        return f.row() < s.row();
    });

    QModelIndex sourceRowFirst = selectedIndexList.first();

    moveRows(sourceRowFirst.parent(), sourceRowFirst.row(), selectedIndexList.count(), sourceRowFirst.parent(), sourceRowFirst.row() - 1);

    updateOperationsAvailability();
}

void NoteInputBarCustomiseModel::moveSelectedRowsDown()
{
    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    if (selectedIndexList.isEmpty()) {
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](QModelIndex f, QModelIndex s) -> bool {
        return f.row() < s.row();
    });

    QModelIndex sourceRowFirst = selectedIndexList.first();
    QModelIndex sourceRowLast = selectedIndexList.last();

    moveRows(sourceRowFirst.parent(), sourceRowFirst.row(), selectedIndexList.count(), sourceRowFirst.parent(), sourceRowLast.row() + 1);
}

void NoteInputBarCustomiseModel::removeSelectedRows()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    QList<NoteInputBarCustomiseItem*> actionsToRemove;

    for (const QModelIndex& selectedIndex : m_selectionModel->selectedIndexes()) {
        actionsToRemove << m_items[selectedIndex.row()];
    }

    for (NoteInputBarCustomiseItem* action : actionsToRemove) {
        int index = m_items.indexOf(action);

        beginRemoveRows(QModelIndex(), index, index);
        m_items.removeAt(index);
        endRemoveRows();
    }

    updateOperationsAvailability();
    saveActions();
}

bool NoteInputBarCustomiseModel::isSelected(int row) const
{
    QModelIndex rowIndex = index(row);
    return m_selectionModel->isSelected(rowIndex);
}

QItemSelectionModel* NoteInputBarCustomiseModel::selectionModel() const
{
    return m_selectionModel;
}

bool NoteInputBarCustomiseModel::isMovingUpAvailable() const
{
    return m_isMovingUpAvailable;
}

bool NoteInputBarCustomiseModel::isMovingDownAvailable() const
{
    return m_isMovingDownAvailable;
}

bool NoteInputBarCustomiseModel::isRemovingAvailable() const
{
    return m_isRemovingAvailable;
}

bool NoteInputBarCustomiseModel::isAddSeparatorAvailable() const
{
    return m_isAddSeparatorAvailable;
}

void NoteInputBarCustomiseModel::setIsMovingUpAvailable(bool isMovingUpAvailable)
{
    if (m_isMovingUpAvailable == isMovingUpAvailable) {
        return;
    }

    m_isMovingUpAvailable = isMovingUpAvailable;
    emit isMovingUpAvailableChanged(m_isMovingUpAvailable);
}

void NoteInputBarCustomiseModel::setIsMovingDownAvailable(bool isMovingDownAvailable)
{
    if (m_isMovingDownAvailable == isMovingDownAvailable) {
        return;
    }

    m_isMovingDownAvailable = isMovingDownAvailable;
    emit isMovingDownAvailableChanged(m_isMovingDownAvailable);
}

void NoteInputBarCustomiseModel::setIsRemovingAvailable(bool isRemovingAvailable)
{
    if (m_isRemovingAvailable == isRemovingAvailable) {
        return;
    }

    m_isRemovingAvailable = isRemovingAvailable;
    emit isRemovingAvailableChanged(m_isRemovingAvailable);
}

void NoteInputBarCustomiseModel::setIsAddSeparatorAvailable(bool isAddSeparatorAvailable)
{
    if (m_isAddSeparatorAvailable == isAddSeparatorAvailable) {
        return;
    }

    m_isAddSeparatorAvailable = isAddSeparatorAvailable;
    emit isAddSeparatorAvailableChanged(m_isAddSeparatorAvailable);
}

NoteInputBarCustomiseItem* NoteInputBarCustomiseModel::modelIndexToItem(const QModelIndex& index) const
{
    return m_items.at(index.row());
}

void NoteInputBarCustomiseModel::updateOperationsAvailability()
{
    updateRearrangementAvailability();
    updateRemovingAvailability();
    updateAddSeparatorAvailability();
}

void NoteInputBarCustomiseModel::updateRearrangementAvailability()
{
    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    if (selectedIndexList.isEmpty()) {
        updateMovingUpAvailability();
        updateMovingDownAvailability();
        return;
    }

    QModelIndex selectedIndex = selectedIndexList.first();
    updateMovingUpAvailability(selectedIndex);
    updateMovingDownAvailability(selectedIndex);
}

void NoteInputBarCustomiseModel::updateMovingUpAvailability(const QModelIndex& selectedRowIndex)
{
    bool isRowInBoundaries = selectedRowIndex.isValid() ? selectedRowIndex.row() > 0 : false;
    setIsMovingUpAvailable(isRowInBoundaries);
}

void NoteInputBarCustomiseModel::updateMovingDownAvailability(const QModelIndex& selectedRowIndex)
{
    bool isRowInBoundaries = selectedRowIndex.isValid() ? selectedRowIndex.row() < rowCount() - 1 : false;
    setIsMovingDownAvailable(isRowInBoundaries);
}

void NoteInputBarCustomiseModel::updateRemovingAvailability()
{
    auto hasActionInSelection = [this](const QModelIndexList& selectedIndexes) {
        for (const QModelIndex& index : selectedIndexes) {
            NoteInputBarCustomiseItem* item = modelIndexToItem(index);
            if (item && item->type() == NoteInputBarCustomiseItem::ACTION) {
                return true;
            }
        }

        return false;
    };

    QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();
    bool removingAvailable = !selectedIndexes.empty();

    if (removingAvailable) {
        removingAvailable = !hasActionInSelection(selectedIndexes);
    }

    setIsRemovingAvailable(removingAvailable);
}

void NoteInputBarCustomiseModel::updateAddSeparatorAvailability()
{
    bool addingAvailable = !m_selectionModel->selectedIndexes().empty();
    if (!addingAvailable) {
        setIsAddSeparatorAvailable(addingAvailable);
        return;
    }

    addingAvailable = m_selectionModel->selectedIndexes().count() == 1;
    if (!addingAvailable) {
        setIsAddSeparatorAvailable(addingAvailable);
        return;
    }

    QModelIndex selectedItemIndex = m_selectionModel->selectedIndexes().first();

    NoteInputBarCustomiseItem* selectedItem = modelIndexToItem(selectedItemIndex);
    addingAvailable = selectedItem && selectedItem->type() == NoteInputBarCustomiseItem::ACTION;

    if (!addingAvailable) {
        setIsAddSeparatorAvailable(addingAvailable);
        return;
    }

    QModelIndex prevItemIndex = index(selectedItemIndex.row() - 1);
    addingAvailable = prevItemIndex.isValid();
    if (addingAvailable) {
        NoteInputBarCustomiseItem* prevItem = modelIndexToItem(prevItemIndex);
        addingAvailable = prevItem && prevItem->type() == NoteInputBarCustomiseItem::ACTION;
    }

    setIsAddSeparatorAvailable(addingAvailable);
}

NoteInputBarCustomiseItem* NoteInputBarCustomiseModel::makeItem(const UiAction& action, bool checked)
{
    if (action.code.empty()) {
        return makeSeparatorItem();
    }

    NoteInputBarCustomiseItem* item = new NoteInputBarCustomiseItem(NoteInputBarCustomiseItem::ItemType::ACTION, this);
    item->setId(QString::fromStdString(action.code));
    item->setTitle(action.title);
    item->setIcon(action.iconCode);
    item->setChecked(checked);

    connect(item, &NoteInputBarCustomiseItem::checkedChanged, this, [this](bool) {
        saveActions();
    });

    return item;
}

NoteInputBarCustomiseItem* NoteInputBarCustomiseModel::makeSeparatorItem()
{
    NoteInputBarCustomiseItem* item = new NoteInputBarCustomiseItem(NoteInputBarCustomiseItem::ItemType::SEPARATOR, this);
    item->setTitle(qtrc("notation", "-------  Separator line  -------"));
    item->setChecked(true); //! NOTE Can't be unchecked
    return item;
}

void NoteInputBarCustomiseModel::saveActions()
{
    ToolConfig config;
    for (const NoteInputBarCustomiseItem* item : m_items) {
        ToolConfig::Item citem;
        citem.action = item->id().toStdString();
        citem.show = item->checked();
        config.items.append(citem);
    }

    uiConfiguration()->setToolConfig(NOTE_INPUT_TOOLBAR_NAME, config);
}
