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
#include "tupletcenteringselectormodel.h"

using namespace mu::notation;
using namespace mu::engraving;

TupletCenteringSelectorModel::TupletCenteringSelectorModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::tupletNumberRythmicCenter })
{
}

StyleItem* TupletCenteringSelectorModel::tupletNumberRythmicCenter() const { return styleItem(StyleId::tupletNumberRythmicCenter); }
