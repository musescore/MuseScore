/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "slursandtiespagemodel.h"

using namespace mu::notation;

SlursAndTiesPageModel::SlursAndTiesPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::slurEndWidth,
    StyleId::slurMidWidth,
    StyleId::slurDottedWidth,
    StyleId::slurMinDistance,
    StyleId::angleHangingSlursAwayFromStaff,
    StyleId::tieEndWidth,
    StyleId::tieMidWidth,
    StyleId::tieDottedWidth,
    StyleId::tieMinDistance,
    StyleId::minTieLength,
    StyleId::minHangingTieLength,
    StyleId::tiePlacementSingleNote,
    StyleId::tiePlacementChord,
    StyleId::tieDotsPlacement,
    StyleId::minLaissezVibLength,
    StyleId::laissezVibUseSmuflSym
})
{
}

StyleItem* SlursAndTiesPageModel::slurEndWidth() const
{
    return styleItem(StyleId::slurEndWidth);
}

StyleItem* SlursAndTiesPageModel::slurMidWidth() const
{
    return styleItem(StyleId::slurMidWidth);
}

StyleItem* SlursAndTiesPageModel::slurDottedWidth() const
{
    return styleItem(StyleId::slurDottedWidth);
}

StyleItem* SlursAndTiesPageModel::slurMinDistance() const
{
    return styleItem(StyleId::slurMinDistance);
}

StyleItem* SlursAndTiesPageModel::angleHangingSlursAwayFromStaff() const
{
    return styleItem(StyleId::angleHangingSlursAwayFromStaff);
}

StyleItem* SlursAndTiesPageModel::tieEndWidth() const
{
    return styleItem(StyleId::tieEndWidth);
}

StyleItem* SlursAndTiesPageModel::tieMidWidth() const
{
    return styleItem(StyleId::tieMidWidth);
}

StyleItem* SlursAndTiesPageModel::tieDottedWidth() const
{
    return styleItem(StyleId::tieDottedWidth);
}

StyleItem* SlursAndTiesPageModel::tieMinDistance() const
{
    return styleItem(StyleId::tieMinDistance);
}

StyleItem* SlursAndTiesPageModel::minTieLength() const
{
    return styleItem(StyleId::minTieLength);
}

StyleItem* SlursAndTiesPageModel::minHangingTieLength() const
{
    return styleItem(StyleId::minHangingTieLength);
}

StyleItem* SlursAndTiesPageModel::tiePlacementSingleNote() const
{
    return styleItem(StyleId::tiePlacementSingleNote);
}

StyleItem* SlursAndTiesPageModel::tiePlacementChord() const
{
    return styleItem(StyleId::tiePlacementChord);
}

StyleItem* SlursAndTiesPageModel::tieDotsPlacement() const
{
    return styleItem(StyleId::tieDotsPlacement);
}

StyleItem* SlursAndTiesPageModel::minLaissezVibLength() const
{
    return styleItem(StyleId::minLaissezVibLength);
}

StyleItem* SlursAndTiesPageModel::laissezVibUseSmuflSym() const
{
    return styleItem(StyleId::laissezVibUseSmuflSym);
}
