/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "instrumentnamespagemodel.h"

using namespace mu::notation;
using namespace mu::engraving;

InstrumentNamesPageModel::InstrumentNamesPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::instrumentNamesAlignLong,
    StyleId::instrumentNamesAlignShort,
    StyleId::instrumentNamesStackVertically,
    StyleId::windsNameByGroup,
    StyleId::vocalsNameByGroup,
    StyleId::stringsNameByGroup,
    StyleId::othersNameByGroup,
})
{
}

StyleItem* InstrumentNamesPageModel::instrumentNamesAlignLong() const
{
    return styleItem(StyleId::instrumentNamesAlignLong);
}

StyleItem* InstrumentNamesPageModel::instrumentNamesAlignShort() const
{
    return styleItem(StyleId::instrumentNamesAlignShort);
}

StyleItem* InstrumentNamesPageModel::instrumentNamesStackVertically() const
{
    return styleItem(StyleId::instrumentNamesStackVertically);
}

StyleItem* InstrumentNamesPageModel::windsNameByGroup() const
{
    return styleItem(StyleId::windsNameByGroup);
}

StyleItem* InstrumentNamesPageModel::vocalsNameByGroup() const
{
    return styleItem(StyleId::vocalsNameByGroup);
}

StyleItem* InstrumentNamesPageModel::stringsNameByGroup() const
{
    return styleItem(StyleId::stringsNameByGroup);
}

StyleItem* InstrumentNamesPageModel::othersNameByGroup() const
{
    return styleItem(StyleId::othersNameByGroup);
}
