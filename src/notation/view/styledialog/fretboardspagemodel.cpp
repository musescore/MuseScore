/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "fretboardspagemodel.h"

using namespace mu::notation;

FretboardsPageModel::FretboardsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::fretY,
    StyleId::fretMag,
    StyleId::fretOrientation,
    StyleId::fretNutThickness,
    StyleId::fretNumPos,
    StyleId::fretUseCustomSuffix,
    StyleId::fretCustomSuffix,
    StyleId::fretDotSpatiumSize,
    StyleId::barreAppearanceSlur,
    StyleId::barreLineWidth,
    StyleId::fretShowFingerings,
    StyleId::fretStyleExtended,
    StyleId::fretStringSpacing,
    StyleId::fretFretSpacing,
    StyleId::maxFretShiftAbove,
    StyleId::maxFretShiftBelow,
})
{
}

StyleItem* FretboardsPageModel::fretY() const { return styleItem(StyleId::fretY); }
StyleItem* FretboardsPageModel::fretMag() const { return styleItem(StyleId::fretMag); }
StyleItem* FretboardsPageModel::fretOrientation() const { return styleItem(StyleId::fretOrientation); }
StyleItem* FretboardsPageModel::fretNutThickness() const { return styleItem(StyleId::fretNutThickness); }
StyleItem* FretboardsPageModel::fretNumPos() const { return styleItem(StyleId::fretNumPos); }
StyleItem* FretboardsPageModel::fretUseCustomSuffix() const { return styleItem(StyleId::fretUseCustomSuffix); }
StyleItem* FretboardsPageModel::fretCustomSuffix() const { return styleItem(StyleId::fretCustomSuffix); }
StyleItem* FretboardsPageModel::fretDotSpatiumSize() const { return styleItem(StyleId::fretDotSpatiumSize); }
StyleItem* FretboardsPageModel::barreAppearanceSlur() const { return styleItem(StyleId::barreAppearanceSlur); }
StyleItem* FretboardsPageModel::barreLineWidth() const { return styleItem(StyleId::barreLineWidth); }
StyleItem* FretboardsPageModel::fretShowFingerings() const { return styleItem(StyleId::fretShowFingerings); }
StyleItem* FretboardsPageModel::fretStyleExtended() const { return styleItem(StyleId::fretStyleExtended); }
StyleItem* FretboardsPageModel::fretStringSpacing() const { return styleItem(StyleId::fretStringSpacing); }
StyleItem* FretboardsPageModel::fretFretSpacing() const { return styleItem(StyleId::fretFretSpacing); }
StyleItem* FretboardsPageModel::maxFretShiftAbove() const { return styleItem(StyleId::maxFretShiftAbove); }
StyleItem* FretboardsPageModel::maxFretShiftBelow() const { return styleItem(StyleId::maxFretShiftBelow); }
