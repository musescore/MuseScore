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
#include "sharedparttreeitem.h"

#include "async/notifylist.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

SharedPartTreeItem::SharedPartTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : PartTreeItem(masterNotation, notation, parent, LayoutPanelItemType::ItemType::SHARED_PART)
{
    setSettingsAvailable(false);
}

void SharedPartTreeItem::init(const notation::Part* part)
{
    PartTreeItem::init(part);

    setSettingsAvailable(false);
    setIsExpandable(true);
}

void SharedPartTreeItem::onScoreChanged(const mu::engraving::ScoreChanges& sc)
{
    PartTreeItem::onScoreChanged(sc);
}

MoveParams SharedPartTreeItem::buildMoveParams(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent,
                                               int destinationRow) const
{
    return PartTreeItem::buildMoveParams(sourceRow, count, destinationParent, destinationRow);
}

void SharedPartTreeItem::moveChildren(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent,
                                      int destinationRow, bool updateNotation)
{
    PartTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow, updateNotation);
}

void SharedPartTreeItem::moveChildrenOnScore(const MoveParams& params)
{
    PartTreeItem::moveChildrenOnScore(params);
}

void SharedPartTreeItem::removeChildren(int row, int count, bool deleteChild)
{
    PartTreeItem::removeChildren(row, count, deleteChild);
}

bool SharedPartTreeItem::canAcceptDrop(const QVariant& obj) const
{
    return PartTreeItem::canAcceptDrop(obj);
}
