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

#include "mscoreview.h"
#include "score.h"
#include "page.h"
#include "text.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   elementLower
//---------------------------------------------------------

static bool elementLower(const EngravingItem* e1, const EngravingItem* e2)
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

EngravingItem* MuseScoreView::elementAt(const PointF& p) const
{
    std::vector<EngravingItem*> el = elementsAt(p);
    EngravingItem* e = el.front();
    if (e && e->isPage()) {
        e = *std::next(el.begin());
    }
    return e;
}

//---------------------------------------------------------
//   point2page
//---------------------------------------------------------

Page* MuseScoreView::point2page(const PointF& p) const
{
    if (score()->linearMode()) {
        return score()->pages().empty() ? 0 : score()->pages().front();
    }
    for (Page* page : score()->pages()) {
        if (page->ldata()->bbox().translated(page->pos()).contains(p)) {
            return page;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   elementsAt
//    p is in canvas coordinates
//---------------------------------------------------------

const std::vector<EngravingItem*> MuseScoreView::elementsAt(const PointF& p) const
{
    std::vector<EngravingItem*> el;

    Page* page = point2page(p);
    if (page) {
        el = page->items(p - page->pos());
        std::sort(el.begin(), el.end(), elementLower);
    }

    return el;
}

EngravingItem* MuseScoreView::elementNear(const PointF& pos) const
{
    std::vector<EngravingItem*> near = elementsNear(pos);
    if (near.empty()) {
        return nullptr;
    }
    return near.front();
}

const std::vector<EngravingItem*> MuseScoreView::elementsNear(const PointF& pos) const
{
    std::vector<EngravingItem*> ll;
    Page* page = point2page(pos);
    if (!page) {
        return ll;
    }

    PointF p = pos - page->pos();
    double w = selectionProximity();
    RectF r(p.x() - w, p.y() - w, 3.0 * w, 3.0 * w);

    std::vector<EngravingItem*> el = page->items(r);
    for (int i = 0; i < MAX_HEADERS; i++) {
        if (score()->headerText(i) != nullptr) {
            el.push_back(score()->headerText(i));
        }
    }
    for (int i = 0; i < MAX_FOOTERS; i++) {
        if (score()->footerText(i) != nullptr) {
            el.push_back(score()->footerText(i));
        }
    }
    for (EngravingItem* e : el) {
        e->itemDiscovered = 0;
        if (!e->selectable() || e->isPage()) {
            continue;
        }
        if (e->contains(p)) {
            ll.push_back(e);
        }
    }
    size_t n = ll.size();
    if ((n == 0) || ((n == 1) && (ll[0]->isMeasure()))) {
        //
        // if no relevant element hit, look nearby
        //
        for (EngravingItem* e : el) {
            if (e->isPage() || !e->selectable()) {
                continue;
            }
            if (e->intersects(r)) {
                ll.push_back(e);
            }
        }
    }
    if (!ll.empty()) {
        std::sort(ll.begin(), ll.end(), elementLower);
    }
    return ll;
}
}
