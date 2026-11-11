/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

namespace mu::instrumentsscene {
class RootTreeItem : public AbstractLayoutPanelTreeItem
{
    Q_OBJECT

    QML_ELEMENT;
    QML_UNCREATABLE("Must be created in C++ only")

public:
    RootTreeItem(notation::IMasterNotationPtr masterNotation, notation::INotationPtr notation, QObject* parent = nullptr);

    void moveChildren(int sourceRow, int count, AbstractLayoutPanelTreeItem* destinationParent, int destinationRow,
                      bool updateNotation) override;

    //! Moves the parts and the system markings on the score to match the panel
    void applyToScore();

    void removeChildren(int row, int count, bool deleteChild) override;
};
}
