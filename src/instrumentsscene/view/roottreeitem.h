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
#ifndef MU_INSTRUMENTSSCENE_ROOTTREEITEM_H
#define MU_INSTRUMENTSSCENE_ROOTTREEITEM_H

#include "abstractinstrumentspaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu::instrumentsscene {
class RootTreeItem : public AbstractInstrumentsPanelTreeItem
{
    Q_OBJECT

public:
    RootTreeItem(notation::IMasterNotationPtr masterNotation, notation::INotationPtr notation, QObject* parent = nullptr);

    MoveParams buildMoveParams(int sourceRow, int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                               int destinationRow) const override;

    void moveChildren(int sourceRow, int count, AbstractInstrumentsPanelTreeItem* destinationParent, int destinationRow,
                      bool updateNotation) override;

    void removeChildren(int row, int count, bool deleteChild) override;
};
}

#endif // MU_INSTRUMENTSSCENE_ROOTTREEITEM_H
