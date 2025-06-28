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
#include "fretframechordlistmodel.h"

#include "dom/box.h"
#include "dom/fret.h"

#include "fretframechorditem.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

FretFrameChordListModel::FretFrameChordListModel(QObject* parent)
    : muse::uicomponents::SelectableItemListModel(parent)
{
}

void FretFrameChordListModel::load(FBox* box)
{
    m_fretBox = box;

    QList<Item*> items;

    for (EngravingItem* element : m_fretBox->el()) {
        FretDiagram* diagram = toFretDiagram(element);
        auto chordItem = new FretFrameChordItem(this);
        chordItem->setId(QString::fromStdString(diagram->eid().toStdString()));
        chordItem->setTitle(diagram->harmony()->plainText());
        chordItem->setIsVisible(diagram->visible());

        items << chordItem;
    }

    setItems(items);
}

QVariant FretFrameChordListModel::data(const QModelIndex& index, int role) const
{
    FretFrameChordItem* item = modelIndexToItem(index);
    if (!item) {
        return QVariant();
    }

    switch (role) {
    case ItemRole: return QVariant::fromValue(item);
    default: break;
    }

    return SelectableItemListModel::data(index, role);
}

QHash<int, QByteArray> FretFrameChordListModel::roleNames() const
{
    QHash<int, QByteArray> roles = SelectableItemListModel::roleNames();
    roles[ItemRole] = "item";

    return roles;
}

void FretFrameChordListModel::setChordVisible(int index, bool visible)
{
    if (!m_fretBox) {
        return;
    }

    ElementList diagrams = m_fretBox->el();
    if (index < 0 || index >= static_cast<int>(diagrams.size())) {
        return;
    }

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    const muse::TranslatableString actionName = visible
                                                ? muse::TranslatableString("undoableAction", "Make chord(s) visible")
                                                : muse::TranslatableString("undoableAction", "Make chord(s) invisible");

    notation->undoStack()->prepareChanges(actionName);

    m_fretBox->score()->undoChangeVisible(diagrams[index], visible);

    FretFrameChordItem* item = modelIndexToItem(this->index(index));
    item->setIsVisible(visible);

    notation->undoStack()->commitChanges();
    notation->notationChanged().notify();
}

void FretFrameChordListModel::moveSelectionUp()
{
    if (!m_fretBox) {
        return;
    }

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    const muse::TranslatableString actionName = muse::TranslatableString("undoableAction", "Move chord(s) up");

    notation->undoStack()->prepareChanges(actionName);

    SelectableItemListModel::moveSelectionUp();

    std::vector<EID> newOrderElementsIds;

    for (const Item* item: items()) {
        const FretFrameChordItem* chordItem = dynamic_cast<const FretFrameChordItem*>(item);
        newOrderElementsIds.push_back(EID::fromStdString(chordItem->id().toStdString()));
    }

    m_fretBox->undoReorderElements(newOrderElementsIds);

    notation->undoStack()->commitChanges();
    notation->notationChanged().notify();
}

void FretFrameChordListModel::moveSelectionDown()
{
    if (!m_fretBox) {
        return;
    }

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    const muse::TranslatableString actionName = muse::TranslatableString("undoableAction", "Move chord(s) down");

    notation->undoStack()->prepareChanges(actionName);

    SelectableItemListModel::moveSelectionDown();

    std::vector<EID> newOrderElementsIds;

    for (const Item* item: items()) {
        const FretFrameChordItem* chordItem = dynamic_cast<const FretFrameChordItem*>(item);
        newOrderElementsIds.push_back(EID::fromStdString(chordItem->id().toStdString()));
    }

    m_fretBox->undoReorderElements(newOrderElementsIds);

    notation->undoStack()->commitChanges();
    notation->notationChanged().notify();
}

QItemSelectionModel* FretFrameChordListModel::selectionModel() const
{
    return muse::uicomponents::SelectableItemListModel::selection();
}

FretFrameChordItem* FretFrameChordListModel::modelIndexToItem(const QModelIndex& index) const
{
    return dynamic_cast<FretFrameChordItem*>(item(index));
}
