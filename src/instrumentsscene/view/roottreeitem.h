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

#pragma once

#include "abstractlayoutpaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu::instrumentsscene {
class RootTreeItem : public AbstractLayoutPanelTreeItem
{
    Q_OBJECT

public:
    RootTreeItem(notation::IMasterNotationPtr masterNotation, notation::INotationPtr notation, QObject* parent = nullptr);

    MoveParams buildMoveParams(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent, int destinationRow) const override;

    void moveChildren(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent, int destinationRow,
                      bool updateNotation) override;

    void moveChildrenOnScore(const MoveParams& params) override;

    void removeChildren(int row, int count, bool deleteChild) override;

private:
    bool partsOrderWillBeChanged(int sourceRow, int count, int destinationRow) const;

    MoveParams buildSystemObjectsMoveParams(int sourceRow, int count, int destinationRow) const;
    MoveParams buildPartsMoveParams(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent, int destinationRow) const;
};
}
