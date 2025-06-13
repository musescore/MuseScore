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
#include "hammeronpullofftappingpagemodel.h"

using namespace mu::notation;
using namespace mu::engraving;

HammerOnPullOffTappingPageModel::HammerOnPullOffTappingPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::hopoShowOnStandardStaves,
                                         StyleId::hopoShowOnTabStaves,
                                         StyleId::hopoUpperCase,
                                         StyleId::hopoShowAll,
                                         StyleId::lhTappingSymbolNormalStave,
                                         StyleId::lhTappingSymbolTab,
                                         StyleId::lhTappingShowItemsNormalStave,
                                         StyleId::lhTappingShowItemsTab,
                                         StyleId::lhTappingSlurTopAndBottomNoteOnTab,
                                         StyleId::rhTappingSymbolNormalStave,
                                         StyleId::rhTappingSymbolTab, })
{
}

StyleItem* HammerOnPullOffTappingPageModel::showOnStandardStaves() const
{
    return styleItem(StyleId::hopoShowOnStandardStaves);
}

StyleItem* HammerOnPullOffTappingPageModel::showOnTabStaves() const
{
    return styleItem(StyleId::hopoShowOnTabStaves);
}

StyleItem* HammerOnPullOffTappingPageModel::hopoUpperCase() const
{
    return styleItem(StyleId::hopoUpperCase);
}

StyleItem* HammerOnPullOffTappingPageModel::hopoShowAll() const
{
    return styleItem(StyleId::hopoShowAll);
}

StyleItem* HammerOnPullOffTappingPageModel::lhTappingSymbolNormalStave() const
{
    return styleItem(StyleId::lhTappingSymbolNormalStave);
}

StyleItem* HammerOnPullOffTappingPageModel::lhTappingSymbolTab() const
{
    return styleItem(StyleId::lhTappingSymbolTab);
}

StyleItem* HammerOnPullOffTappingPageModel::lhTappingShowItemsNormalStave() const
{
    return styleItem(StyleId::lhTappingShowItemsNormalStave);
}

StyleItem* HammerOnPullOffTappingPageModel::lhTappingShowItemsTab() const
{
    return styleItem(StyleId::lhTappingShowItemsTab);
}

StyleItem* HammerOnPullOffTappingPageModel::lhTappingSlurTopAndBottomNoteOnTab() const
{
    return styleItem(StyleId::lhTappingSlurTopAndBottomNoteOnTab);
}

StyleItem* HammerOnPullOffTappingPageModel::rhTappingSymbolNormalStave() const
{
    return styleItem(StyleId::rhTappingSymbolNormalStave);
}

StyleItem* HammerOnPullOffTappingPageModel::rhTappingSymbolTab() const
{
    return styleItem(StyleId::rhTappingSymbolTab);
}
