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
#include "beamspagemodel.h"

using namespace mu::notation;

BeamsPageModel::BeamsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::useWideBeams,
    StyleId::beamWidth,
    StyleId::beamMinLen,
    StyleId::beamNoSlope,
    StyleId::frenchStyleBeams
})
{
}

StyleItem* BeamsPageModel::useWideBeams() const
{
    return styleItem(StyleId::useWideBeams);
}

StyleItem* BeamsPageModel::beamWidth() const
{
    return styleItem(StyleId::beamWidth);
}

StyleItem* BeamsPageModel::beamMinLen() const
{
    return styleItem(StyleId::beamMinLen);
}

StyleItem* BeamsPageModel::beamNoSlope() const
{
    return styleItem(StyleId::beamNoSlope);
}

StyleItem* BeamsPageModel::frenchStyleBeams() const
{
    return styleItem(StyleId::frenchStyleBeams);
}
