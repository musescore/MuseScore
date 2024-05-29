/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "noteinputbarcustomiseitem.h"

#include "translation.h"

#include "internal/notationuiactions.h"

#include "log.h"

using namespace mu::notation;
using namespace muse::workspace;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace muse::actions;

static const QString NOTE_INPUT_TOOLBAR_NAME("noteInput");

NoteInputBarCustomiseModel::NoteInputBarCustomiseModel(QObject* parent)
    : SelectableItemListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void NoteInputBarCustomiseModel::load()
{
    TRACEFUNC;

    QList<Item*> items;

    ToolConfig toolConfig = uiConfiguration()->toolConfig(NOTE_INPUT_TOOLBAR_NAME, NotationUiActions::defaultNoteInputBarConfig());

    for (const ToolConfig::Item& item : toolConfig.items) {
        UiAction action = actionsRegister()->action(item.action);
        items << makeItem(action, item.show);
    }

    setItems(items);
}

QVariant NoteInputBarCustomiseModel::data(const QModelIndex& index, int role) const
{
    NoteInputBarCustomiseItem* item = modelIndexToItem(index);
    if (!item) {
        return QVariant();
    }

    switch (role) {
    case ItemRole: return QVariant::fromValue(item);
    default: break;
    }

    return SelectableItemListModel::data(index, role);
}

QHash<int, QByteArray> NoteInputBarCustomiseModel::roleNames() const
{
    QHash<int, QByteArray> roles = SelectableItemListModel::roleNames();
    roles[ItemRole] = "item";

    return roles;
}

void NoteInputBarCustomiseModel::addSeparatorLine()
{
    if (!hasSelection()) {
        return;
    }

    QModelIndex selectedItemIndex = selection()->selectedIndexes().first();
    if (!selectedItemIndex.isValid()) {
        return;
    }

    QModelIndex prevItemIndex = index(selectedItemIndex.row() - 1);
    if (!prevItemIndex.isValid()) {
        return;
    }

    insertItem(prevItemIndex.row() + 1, makeSeparatorItem());

    onUpdateOperationsAvailability();
    saveActions();
}

QItemSelectionModel* NoteInputBarCustomiseModel::selectionModel() const
{
    return selection();
}

bool NoteInputBarCustomiseModel::isAddSeparatorAvailable() const
{
    return m_isAddSeparatorAvailable;
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
    return dynamic_cast<NoteInputBarCustomiseItem*>(item(index));
}

void NoteInputBarCustomiseModel::onUpdateOperationsAvailability()
{
    TRACEFUNC;

    SelectableItemListModel::onUpdateOperationsAvailability();
    updateRemovingAvailability();
    updateAddSeparatorAvailability();
}

void NoteInputBarCustomiseModel::onRowsMoved()
{
    saveActions();
}

void NoteInputBarCustomiseModel::onRowsRemoved()
{
    saveActions();
}

void NoteInputBarCustomiseModel::updateRemovingAvailability()
{
    TRACEFUNC;

    auto hasActionInSelection = [this](const QModelIndexList& selectedIndexes) {
        for (const QModelIndex& index : selectedIndexes) {
            const NoteInputBarCustomiseItem* item = modelIndexToItem(index);

            if (item && item->type() == NoteInputBarCustomiseItem::ACTION) {
                return true;
            }
        }

        return false;
    };

    QModelIndexList selectedIndexes = selection()->selectedIndexes();
    bool removingAvailable = !selectedIndexes.empty();

    if (removingAvailable) {
        removingAvailable = !hasActionInSelection(selectedIndexes);
    }

    setIsRemovingAvailable(removingAvailable);
}

void NoteInputBarCustomiseModel::updateAddSeparatorAvailability()
{
    TRACEFUNC;

    bool addingAvailable = !selection()->selectedIndexes().empty();
    if (!addingAvailable) {
        setIsAddSeparatorAvailable(addingAvailable);
        return;
    }

    addingAvailable = selection()->selectedIndexes().count() == 1;
    if (!addingAvailable) {
        setIsAddSeparatorAvailable(addingAvailable);
        return;
    }

    QModelIndex selectedItemIndex = selection()->selectedIndexes().first();

    const NoteInputBarCustomiseItem* selectedItem = modelIndexToItem(selectedItemIndex);
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
    item->setTitle(action.title.qTranslatedWithoutMnemonic());
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
    item->setTitle(QString("-------  %1  -------").arg(muse::qtrc("notation", "Separator line")));
    item->setChecked(true); //! NOTE Can't be unchecked
    return item;
}

void NoteInputBarCustomiseModel::saveActions()
{
    TRACEFUNC;

    ToolConfig config;
    for (const Item* item : items()) {
        auto customiseItem = dynamic_cast<const NoteInputBarCustomiseItem*>(item);
        if (!customiseItem) {
            continue;
        }

        ToolConfig::Item citem;
        citem.action = customiseItem->id().toStdString();
        citem.show = customiseItem->checked();
        config.items.append(citem);
    }

    uiConfiguration()->setToolConfig(NOTE_INPUT_TOOLBAR_NAME, config);
}
