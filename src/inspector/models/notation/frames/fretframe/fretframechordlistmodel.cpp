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

void FretFrameChordListModel::load()
{
    if (!m_fretBox) {
        return;
    }

    QList<Item*> items;

    auto harmonyName = [](const Harmony* harmony) {
        QString name;
        for (const HarmonyRenderItem* segment : harmony->ldata()->renderItemList()) {
            if (const TextSegment* textSeg = dynamic_cast<const TextSegment*>(segment)) {
                name += textSeg->text().toQString();
            } else if (const ChordSymbolParen* parenSeg = dynamic_cast<const ChordSymbolParen*>(segment)) {
                name += parenSeg->paren->direction() == DirectionH::LEFT ? u"(" : u")";
            }
        }

        return name;
    };

    for (EngravingItem* element : m_fretBox->orderedElements()) {
        FretDiagram* diagram = toFretDiagram(element);
        auto chordItem = new FretFrameChordItem(this);
        chordItem->setId(QString::fromStdString(diagram->eid().toStdString()));
        chordItem->setTitle(harmonyName(diagram->harmony()));
        chordItem->setIsVisible(diagram->visible());

        items << chordItem;
    }

    setItems(items);
}

void FretFrameChordListModel::setFBox(engraving::FBox* box)
{
    m_fretBox = box;
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

    ElementList diagrams = m_fretBox->orderedElements();
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

    SelectableItemListModel::moveSelectionUp();

    const muse::TranslatableString actionName = muse::TranslatableString("undoableAction", "Move chord(s) up");
    notation->undoStack()->prepareChanges(actionName);

    saveOrder();

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

    SelectableItemListModel::moveSelectionDown();

    const muse::TranslatableString actionName = muse::TranslatableString("undoableAction", "Move chord(s) down");
    notation->undoStack()->prepareChanges(actionName);

    saveOrder();

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

void FretFrameChordListModel::saveOrder()
{
    StringList newOrder;

    for (const Item* item: items()) {
        const FretFrameChordItem* chordItem = dynamic_cast<const FretFrameChordItem*>(item);
        newOrder.push_back(String::fromQString(chordItem->title()));
    }

    m_fretBox->undoReorderElements(newOrder);
}
