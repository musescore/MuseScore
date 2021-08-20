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

#include "mscoreview.h"
#include "score.h"
#include "page.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   elementLower
//---------------------------------------------------------

static bool elementLower(const Element* e1, const Element* e2)
{
    if (!e1->selectable()) {
        return false;
    }
    if (!e2->selectable()) {
        return true;
    }
    return e1->z() < e2->z();
}

//---------------------------------------------------------
//   elementAt
//---------------------------------------------------------

Element* MuseScoreView::elementAt(const mu::PointF& p)
{
    QList<Element*> el = elementsAt(p);
    Element* e = el.value(0);
    if (e && e->isPage()) {
        e = el.value(1);
    }
    return e;
}

//---------------------------------------------------------
//   point2page
//---------------------------------------------------------

Page* MuseScoreView::point2page(const mu::PointF& p)
{
    if (score()->layoutMode() == LayoutMode::LINE) {
        return score()->pages().isEmpty() ? 0 : score()->pages().front();
    }
    foreach (Page* page, score()->pages()) {
        if (page->bbox().translated(page->pos()).contains(p)) {
            return page;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   elementsAt
//    p is in canvas coordinates
//---------------------------------------------------------

const QList<Element*> MuseScoreView::elementsAt(const mu::PointF& p)
{
    QList<Element*> el;

    Page* page = point2page(p);
    if (page) {
        el = page->items(p - page->pos());
        std::sort(el.begin(), el.end(), elementLower);
    }
    return el;
}
}
