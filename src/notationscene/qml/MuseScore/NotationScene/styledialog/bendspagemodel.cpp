/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "bendspagemodel.h"

using namespace mu::notation;

BendsPageModel::BendsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::guitarBendUseFull,
                                         StyleId::guitarBendLineWidth,
                                         StyleId::guitarBendLineWidthTab,
                                         StyleId::guitarBendArrowWidth,
                                         StyleId::guitarBendArrowHeight,
                                         StyleId::guitarDivesAboveStaff,
                                         StyleId::useCueSizeFretForGraceBends,
                                         StyleId::useFractionCharacters,
                                         StyleId::guitarBendLineWidth,
                                         StyleId::alignPreBendAndPreDiveToGraceNote,
                                         StyleId::guitarDiveLineWidth,
                                         StyleId::guitarDiveLineWidthTab,
                                         StyleId::whammyBarText,
                                         StyleId::whammyBarLineStyle,
                                         StyleId::whammyBarDashLineLen,
                                         StyleId::whammyBarDashGapLen,
                                         StyleId::whammyBarLineWidth, })
{
}

StyleItem* BendsPageModel::guitarBendUseFull() const
{
    return styleItem(StyleId::guitarBendUseFull);
}

StyleItem* BendsPageModel::guitarBendLineWidth() const
{
    return styleItem(StyleId::guitarBendLineWidth);
}

StyleItem* BendsPageModel::guitarBendLineWidthTab() const
{
    return styleItem(StyleId::guitarBendLineWidthTab);
}

StyleItem* BendsPageModel::guitarBendArrowWidth() const
{
    return styleItem(StyleId::guitarBendArrowWidth);
}

StyleItem* BendsPageModel::guitarBendArrowHeight() const
{
    return styleItem(StyleId::guitarBendArrowHeight);
}

StyleItem* BendsPageModel::guitarDivesAboveStaff() const
{
    return styleItem(StyleId::guitarDivesAboveStaff);
}

StyleItem* BendsPageModel::useCueSizeFretForGraceBends() const
{
    return styleItem(StyleId::useCueSizeFretForGraceBends);
}

StyleItem* BendsPageModel::useFractionCharacters() const
{
    return styleItem(StyleId::useFractionCharacters);
}

StyleItem* BendsPageModel::alignPreBendAndPreDiveToGraceNote() const
{
    return styleItem(StyleId::alignPreBendAndPreDiveToGraceNote);
}

StyleItem* BendsPageModel::guitarDiveLineWidth() const
{
    return styleItem(StyleId::guitarDiveLineWidth);
}

StyleItem* BendsPageModel::guitarDiveLineWidthTab() const
{
    return styleItem(StyleId::guitarDiveLineWidthTab);
}

StyleItem* BendsPageModel::whammyBarText() const
{
    return styleItem(StyleId::whammyBarText);
}

StyleItem* BendsPageModel::whammyBarLineStyle() const
{
    return styleItem(StyleId::whammyBarLineStyle);
}

StyleItem* BendsPageModel::whammyBarDashLineLen() const
{
    return styleItem(StyleId::whammyBarDashLineLen);
}

StyleItem* BendsPageModel::whammyBarDashGapLen() const
{
    return styleItem(StyleId::whammyBarDashGapLen);
}

StyleItem* BendsPageModel::whammyBarLineWidth() const
{
    return styleItem(StyleId::whammyBarLineWidth);
}
