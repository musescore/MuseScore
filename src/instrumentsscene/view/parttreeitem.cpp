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
#include "parttreeitem.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

using ItemType = InstrumentsTreeItemType::ItemType;

PartTreeItem::PartTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractInstrumentsPanelTreeItem(ItemType::PART, masterNotation, notation, parent), Injectable(iocCtxForQmlObject(this))
{
    listenVisibilityChanged();
}

void PartTreeItem::init(const notation::Part* masterPart)
{
    IF_ASSERT_FAILED(masterPart) {
        return;
    }

    const Part* part = notation()->parts()->part(masterPart->id());
    bool partExists = part != nullptr;
    bool visible = partExists && part->show();

    if (!partExists) {
        part = masterPart;
    }

    setId(part->id());
    setTitle(part->instrument()->nameAsPlainText());
    setIsVisible(visible);
    setIsEditable(partExists);
    setIsExpandable(partExists);
    setIsRemovable(partExists);

    m_instrumentId = part->instrumentId();
    m_isInited = true;
}

bool PartTreeItem::isSelectable() const
{
    return true;
}

void PartTreeItem::listenVisibilityChanged()
{
    connect(this, &AbstractInstrumentsPanelTreeItem::isVisibleChanged, this, [this](bool isVisible) {
        if (!m_isInited) {
            return;
        }

        INotationPartsPtr parts = notation()->parts();
        if (!parts) {
            return;
        }

        if (parts->partExists(id())) {
            parts->setPartVisible(id(), isVisible);
        } else if (isVisible) {
            createAndAddPart(id());
        }
    });
}

void PartTreeItem::createAndAddPart(const ID& masterPartId)
{
    const Part* masterPart = masterNotation()->parts()->part(masterPartId);
    if (!masterPart) {
        return;
    }

    size_t index = resolveNewPartIndex(masterPartId);

    notation()->parts()->insertPart(masterPart->clone(), index);
}

size_t PartTreeItem::resolveNewPartIndex(const ID& partId) const
{
    IF_ASSERT_FAILED(parentItem()) {
        return 0;
    }

    bool partFound = false;
    ID firstVisiblePartId;

    for (const AbstractInstrumentsPanelTreeItem* item : parentItem()->childItems()) {
        if (item->id() == partId) {
            partFound = true;
            continue;
        }

        if (!partFound) {
            continue;
        }

        if (item->isVisible()) {
            firstVisiblePartId = item->id();
            break;
        }
    }

    auto parts = notation()->parts()->partList();

    for (size_t i = 0; i < parts.size(); ++i) {
        if (ID(parts[i]->id()) == firstVisiblePartId) {
            return i;
        }
    }

    return parts.size();
}

MoveParams PartTreeItem::buildMoveParams(int sourceRow, int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                                         int destinationRow) const
{
    MoveParams moveParams;

    IDList stavesIds;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        stavesIds.push_back(childAtRow(i)->id());
    }

    moveParams.childIdListToMove = stavesIds;

    int destinationRowLast = destinationRow;
    INotationParts::InsertMode moveMode = INotationParts::InsertMode::Before;

    // exclude the control item
    if (destinationRow == destinationParent->childCount() - 1) {
        destinationRowLast = destinationRow - 1;
        moveMode = INotationParts::InsertMode::After;
    }

    AbstractInstrumentsPanelTreeItem* destinationStaffItem = destinationParent->childAtRow(destinationRowLast);

    moveParams.destinationParentId = destinationStaffItem->id();
    moveParams.insertMode = moveMode;

    return moveParams;
}

void PartTreeItem::moveChildren(int sourceRow, int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                                int destinationRow, bool updateNotation)
{
    if (updateNotation) {
        MoveParams moveParams = buildMoveParams(sourceRow, count, destinationParent, destinationRow);
        notation()->parts()->moveStaves(moveParams.childIdListToMove, moveParams.destinationParentId, moveParams.insertMode);
    }
    AbstractInstrumentsPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow, updateNotation);
}

void PartTreeItem::removeChildren(int row, int count, bool deleteChild)
{
    IDList stavesIds;

    for (int i = row; i < row + count; ++i) {
        stavesIds.push_back(childAtRow(i)->id());
    }

    if (deleteChild) {
        masterNotation()->parts()->removeStaves(stavesIds);
    }

    AbstractInstrumentsPanelTreeItem::removeChildren(row, count, deleteChild);
}

QString PartTreeItem::instrumentId() const
{
    return m_instrumentId;
}

void PartTreeItem::replaceInstrument()
{
    InstrumentKey instrumentKey;
    instrumentKey.partId = id();
    instrumentKey.instrumentId = m_instrumentId;
    instrumentKey.tick = Part::MAIN_INSTRUMENT_TICK;

    RetVal<Instrument> selectedInstrument = selectInstrumentsScenario()->selectInstrument(instrumentKey);
    if (!selectedInstrument.ret) {
        LOGE() << selectedInstrument.ret.toString();
        return;
    }

    const Instrument& newInstrument = selectedInstrument.val;
    masterNotation()->parts()->replaceInstrument(instrumentKey, newInstrument);
}

void PartTreeItem::resetAllFormatting()
{
    std::string title = muse::trc("instruments", "Are you sure you want to reset all formatting?");
    std::string body = muse::trc("instruments", "This action can not be undone");

    IInteractive::Button button = interactive()->question(title, body, {
        IInteractive::Button::No,
        IInteractive::Button::Yes
    }).standardButton();

    if (button != IInteractive::Button::Yes) {
        return;
    }

    const Part* masterPart = masterNotation()->parts()->part(id());
    notation()->parts()->replacePart(id(), masterPart->clone());
}
