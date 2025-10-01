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

#include "editscoreproperties.h"

#include "../dom/score.h"
#include "../dom/scoreorder.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   ChangeMetaTags
//---------------------------------------------------------

void ChangeMetaTags::flip(EditData*)
{
    std::map<String, String> t = score->metaTags();
    score->setMetaTags(metaTags);
    metaTags = t;
}

//---------------------------------------------------------
//   ChangeMetaText
//---------------------------------------------------------

void ChangeMetaText::flip(EditData*)
{
    String s = score->metaTag(id);
    score->setMetaTag(id, text);
    text = s;
}

//---------------------------------------------------------
//   ChangeScoreOrder
//---------------------------------------------------------

void ChangeScoreOrder::flip(EditData*)
{
    ScoreOrder s = score->scoreOrder();
    score->setScoreOrder(order);
    order = s;
}

//---------------------------------------------------------
//   ChangePageNumberOffset
//---------------------------------------------------------

void ChangePageNumberOffset::flip(EditData*)
{
    int po = score->pageNumberOffset();

    score->setPageNumberOffset(pageOffset);
    score->setLayoutAll();

    pageOffset = po;
}
