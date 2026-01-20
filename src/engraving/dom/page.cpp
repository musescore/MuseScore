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

#include "page.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

#include "factory.h"
#include "measurebase.h"
#include "mscore.h"
#include "score.h"
#include "system.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

Page::Page(RootItem* parent)
    : EngravingItem(ElementType::PAGE, parent, ElementFlag::NOT_SELECTABLE)
{
    m_bspTreeValid = false;
}

//---------------------------------------------------------
//   items
//---------------------------------------------------------

std::vector<EngravingItem*> Page::items(const RectF& rect)
{
    if (!m_bspTreeValid) {
        doRebuildBspTree();
    }
    return bspTree.items(rect);
}

std::vector<EngravingItem*> Page::items(const PointF& point)
{
    if (!m_bspTreeValid) {
        doRebuildBspTree();
    }
    return bspTree.items(point);
}

//---------------------------------------------------------
//   appendSystem
//---------------------------------------------------------

void Page::appendSystem(System* s)
{
    s->moveToPage(this);
    m_systems.push_back(s);
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr Page::createAccessible()
{
    return std::make_shared<AccessibleItem>(this, AccessibleItem::Group);
}

#endif

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Page::scanElements(std::function<void(EngravingItem*)> func)
{
    for (System* s :m_systems) {
        for (MeasureBase* m : s->measures()) {
            m->scanElements(func);
        }
        s->scanElements(func);
    }

    func(this);

    for (int i = 0; i < MAX_HEADERS; i++) {
        if (Text* t = headerText(i)) {
            func(t);
        }
    }
    for (int i = 0; i < MAX_FOOTERS; i++) {
        if (Text* t = footerText(i)) {
            func(t);
        }
    }
}

//---------------------------------------------------------
//   doRebuildBspTree
//---------------------------------------------------------

void Page::doRebuildBspTree()
{
    int n = 0;
    auto countElements = [&](EngravingItem* item) {
        if (item->collectForDrawing()) {
            ++n;
        }
    };

    scanElements(countElements);

    RectF r;
    if (score()->linearMode()) {
        double w = 0.0;
        double h = 0.0;
        if (!m_systems.empty()) {
            h = m_systems.front()->height();
            if (!m_systems.front()->measures().empty()) {
                MeasureBase* mb = m_systems.front()->measures().back();
                w = mb->x() + mb->width();
            }
        }
        r = RectF(0.0, 0.0, w, h);
    } else {
        r = pageBoundingRect();
    }

    bspTree.initialize(r, n);

    auto bspInsert = [&] (EngravingItem* item) {
        if (item->collectForDrawing()) {
            bspTree.insert(item);
        }
    };
    scanElements(bspInsert);

    m_bspTreeValid = true;
}

//---------------------------------------------------------
//   isOdd
//---------------------------------------------------------

bool Page::isOdd() const
{
    return (m_pageNumber + 1 + score()->pageNumberOffset()) & 1;
}

//---------------------------------------------------------
//   elements
//---------------------------------------------------------

std::vector<EngravingItem*> Page::elements() const
{
    return getChildren(false);
}

//---------------------------------------------------------
//   tm
//---------------------------------------------------------

double Page::tm() const
{
    return ((!style().styleB(Sid::pageTwosided) || isOdd())
            ? style().styleD(Sid::pageOddTopMargin) : style().styleD(Sid::pageEvenTopMargin)) * DPI;
}

//---------------------------------------------------------
//   bm
//---------------------------------------------------------

double Page::bm() const
{
    return ((!style().styleB(Sid::pageTwosided) || isOdd())
            ? style().styleD(Sid::pageOddBottomMargin) : style().styleD(Sid::pageEvenBottomMargin)) * DPI;
}

//---------------------------------------------------------
//   lm
//---------------------------------------------------------

double Page::lm() const
{
    return ((!style().styleB(Sid::pageTwosided) || isOdd())
            ? style().styleD(Sid::pageOddLeftMargin) : style().styleD(Sid::pageEvenLeftMargin)) * DPI;
}

//---------------------------------------------------------
//   rm
//---------------------------------------------------------

double Page::rm() const
{
    return (style().styleD(Sid::pageWidth) - style().styleD(Sid::pagePrintableWidth)) * DPI - lm();
}

//---------------------------------------------------------
//   tbbox
//    calculates and returns smallest rectangle containing all (visible) page elements
//---------------------------------------------------------

RectF Page::tbbox() const
{
    double x1 = width();
    double x2 = 0.0;
    double y1 = height();
    double y2 = 0.0;
    const std::vector<EngravingItem*> el = elements();
    for (EngravingItem* e : el) {
        if (e == this || !e->isPrintable()) {
            continue;
        }
        RectF ebbox = e->pageBoundingRect();
        if (ebbox.left() < x1) {
            x1 = ebbox.left();
        }
        if (ebbox.right() > x2) {
            x2 = ebbox.right();
        }
        if (ebbox.top() < y1) {
            y1 = ebbox.top();
        }
        if (ebbox.bottom() > y2) {
            y2 = ebbox.bottom();
        }
    }
    if (x1 < x2 && y1 < y2) {
        return RectF(x1, y1, x2 - x1, y2 - y1);
    } else {
        return pageBoundingRect();
    }
}

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

Fraction Page::endTick() const
{
    return m_systems.empty() ? Fraction(-1, 1) : m_systems.back()->measures().back()->endTick();
}
