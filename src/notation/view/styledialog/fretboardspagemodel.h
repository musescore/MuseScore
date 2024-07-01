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

#ifndef MU_NOTATION_FRETBOARDSPAGEMODEL_H
#define MU_NOTATION_FRETBOARDSPAGEMODEL_H

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class FretboardsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * fretY READ fretY CONSTANT)
    Q_PROPERTY(StyleItem * fretMag READ fretMag CONSTANT)
    Q_PROPERTY(StyleItem * fretOrientation READ fretOrientation CONSTANT)
    Q_PROPERTY(StyleItem * fretNutThickness READ fretNutThickness CONSTANT)
    Q_PROPERTY(StyleItem * fretNumPos READ fretNumPos CONSTANT)
    Q_PROPERTY(StyleItem * fretUseCustomSuffix READ fretUseCustomSuffix CONSTANT)
    Q_PROPERTY(StyleItem * fretCustomSuffix READ fretCustomSuffix CONSTANT)
    Q_PROPERTY(StyleItem * fretDotSpatiumSize READ fretDotSpatiumSize CONSTANT)
    Q_PROPERTY(StyleItem * barreAppearanceSlur READ barreAppearanceSlur CONSTANT)
    Q_PROPERTY(StyleItem * barreLineWidth READ barreLineWidth CONSTANT)
    Q_PROPERTY(StyleItem * fretShowFingerings READ fretShowFingerings CONSTANT)
    Q_PROPERTY(StyleItem * fretStyleExtended READ fretStyleExtended CONSTANT)
    Q_PROPERTY(StyleItem * fretStringSpacing READ fretStringSpacing CONSTANT)
    Q_PROPERTY(StyleItem * fretFretSpacing READ fretFretSpacing CONSTANT)
    Q_PROPERTY(StyleItem * maxFretShiftAbove READ maxFretShiftAbove CONSTANT)
    Q_PROPERTY(StyleItem * maxFretShiftBelow READ maxFretShiftBelow CONSTANT)

public:
    explicit FretboardsPageModel(QObject* parent = nullptr);

    StyleItem* fretY() const;
    StyleItem* fretMag() const;
    StyleItem* fretOrientation() const;
    StyleItem* fretNutThickness() const;
    StyleItem* fretNumPos() const;
    StyleItem* fretUseCustomSuffix() const;
    StyleItem* fretCustomSuffix() const;
    StyleItem* fretDotSpatiumSize() const;
    StyleItem* barreAppearanceSlur() const;
    StyleItem* barreLineWidth() const;
    StyleItem* fretShowFingerings() const;
    StyleItem* fretStyleExtended() const;
    StyleItem* fretStringSpacing() const;
    StyleItem* fretFretSpacing() const;
    StyleItem* maxFretShiftAbove() const;
    StyleItem* maxFretShiftBelow() const;
};
}

#endif // MU_NOTATION_FRETBOARDSPAGEMODEL_H
