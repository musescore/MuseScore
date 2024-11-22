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
#include "tieplacementselector.h"

using namespace mu::notation;

TiePlacementSelectorModel::TiePlacementSelectorModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::tiePlacementSingleNote,
    StyleId::tiePlacementChord,
    StyleId::tieDotsPlacement })
{
}

StyleItem* TiePlacementSelectorModel::placementSingleNotes() const
{
    return styleItem(StyleId::tiePlacementSingleNote);
}

StyleItem* TiePlacementSelectorModel::placementChords() const
{
    return styleItem(StyleId::tiePlacementChord);
}

StyleItem* TiePlacementSelectorModel::placementDots() const
{
    return styleItem(StyleId::tieDotsPlacement);
}
