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
#include "voltaspagemodel.h"

using namespace mu::notation;
using namespace mu::engraving;

VoltasPageModel::VoltasPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::voltaPosAbove,
                                         StyleId::voltaHook,
                                         StyleId::voltaLineWidth,
                                         StyleId::voltaLineStyle,
                                         StyleId::voltaDashLineLen,
                                         StyleId::voltaDashGapLen,
                                         StyleId::voltaAlignStartBeforeKeySig,
                                         StyleId::voltaAlignEndLeftOfBarline, })
{
}

StyleItem* VoltasPageModel::voltaPosAbove() const { return styleItem(StyleId::voltaPosAbove); }
StyleItem* VoltasPageModel::voltaHook() const { return styleItem(StyleId::voltaHook); }
StyleItem* VoltasPageModel::voltaLineWidth() const { return styleItem(StyleId::voltaLineWidth); }
StyleItem* VoltasPageModel::voltaLineStyle() const { return styleItem(StyleId::voltaLineStyle); }
StyleItem* VoltasPageModel::voltaDashLineLen() const { return styleItem(StyleId::voltaDashLineLen); }
StyleItem* VoltasPageModel::voltaDashGapLen() const { return styleItem(StyleId::voltaDashGapLen); }
StyleItem* VoltasPageModel::voltaAlignStartBeforeKeySig() const { return styleItem(StyleId::voltaAlignStartBeforeKeySig); }
StyleItem* VoltasPageModel::voltaAlignEndLeftOfBarline() const { return styleItem(StyleId::voltaAlignEndLeftOfBarline); }
