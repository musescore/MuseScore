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
#include "parttreeitem.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

PartTreeItem::PartTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractLayoutPanelTreeItem(LayoutPanelItemType::PART, masterNotation, notation, parent), Injectable(iocCtxForQmlObject(this))
{
    setIsSelectable(true);

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
    setSettingsAvailable(partExists);
    setSettingsEnabled(partExists);
    setIsExpandable(partExists);
    setIsRemovable(partExists);

    m_part = part;
    m_isInited = true;
}

const Part* PartTreeItem::part() const
{
    return m_part;
}

void PartTreeItem::listenVisibilityChanged()
{
    connect(this, &AbstractLayoutPanelTreeItem::isVisibleChanged, this, [this](bool isVisible) {
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

    for (const AbstractLayoutPanelTreeItem* item : parentItem()->childItems()) {
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

MoveParams PartTreeItem::buildMoveParams(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent,
                                         int destinationRow) const
{
    MoveParams moveParams;

    IDList stavesIds;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        stavesIds.push_back(childAtRow(i)->id());
    }

    moveParams.objectIdListToMove = stavesIds;

    int destinationRowLast = destinationRow;
    INotationParts::InsertMode moveMode = INotationParts::InsertMode::Before;

    // exclude the control item
    if (destinationRow == destinationParent->childCount() - 1) {
        destinationRowLast = destinationRow - 1;
        moveMode = INotationParts::InsertMode::After;
    }

    AbstractLayoutPanelTreeItem* destinationStaffItem = destinationParent->childAtRow(destinationRowLast);

    moveParams.destinationObjectId = destinationStaffItem->id();
    moveParams.insertMode = moveMode;
    moveParams.objectsType = LayoutPanelItemType::STAFF;

    return moveParams;
}

void PartTreeItem::moveChildren(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent,
                                int destinationRow, bool updateNotation)
{
    if (updateNotation) {
        MoveParams moveParams = buildMoveParams(sourceRow, count, destinationParent, destinationRow);
        moveChildrenOnScore(moveParams);
    }

    AbstractLayoutPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow, updateNotation);
}

void PartTreeItem::moveChildrenOnScore(const MoveParams& params)
{
    notation()->parts()->moveStaves(params.objectIdListToMove, params.destinationObjectId, params.insertMode);
}

void PartTreeItem::removeChildren(int row, int count, bool deleteChild)
{
    IDList stavesIds;

    for (int i = row; i < row + count; ++i) {
        stavesIds.push_back(childAtRow(i)->id());
    }

    // Remove the children first, then remove the staves
    // so we don't try to remove them twice when notified by removeStaves()
    AbstractLayoutPanelTreeItem::removeChildren(row, count, deleteChild);

    if (deleteChild) {
        masterNotation()->parts()->removeStaves(stavesIds);
    }
}

bool PartTreeItem::canAcceptDrop(const QVariant& obj) const
{
    if (auto item = dynamic_cast<const AbstractLayoutPanelTreeItem*>(obj.value<QObject*>())) {
        if (item->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            return true;
        }
    }

    return AbstractLayoutPanelTreeItem::canAcceptDrop(obj);
}

QString PartTreeItem::instrumentId() const
{
    return m_part ? m_part->instrumentId().toQString() : QString();
}

void PartTreeItem::replaceInstrument()
{
    InstrumentKey instrumentKey;
    instrumentKey.partId = id();
    instrumentKey.instrumentId = instrumentId();
    instrumentKey.tick = Part::MAIN_INSTRUMENT_TICK;

    RetVal<InstrumentTemplate> templ = selectInstrumentsScenario()->selectInstrument(instrumentKey);
    if (!templ.ret) {
        LOGE() << templ.ret.toString();
        return;
    }

    Instrument instrument = Instrument::fromTemplate(&templ.val);

    const StaffType* staffType = templ.val.staffTypePreset;
    if (!staffType) {
        staffType = StaffType::getDefaultPreset(templ.val.staffGroup);
    }

    masterNotation()->parts()->replaceInstrument(instrumentKey, instrument, staffType);
}

void PartTreeItem::resetAllFormatting()
{
    std::string title = muse::trc("layoutpanel", "Are you sure you want to reset all formatting?");
    std::string body = muse::trc("layoutpanel", "This action can not be undone");

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
