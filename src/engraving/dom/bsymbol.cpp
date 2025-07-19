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

#include "bsymbol.h"

#include <cmath>

#include "containers.h"

#include "measure.h"
#include "page.h"
#include "score.h"
#include "segment.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   BSymbol
//---------------------------------------------------------

BSymbol::BSymbol(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : EngravingItem(type, parent, f)
{
    m_align = { AlignH::LEFT, AlignV::BASELINE };
}

BSymbol::BSymbol(const BSymbol& s)
    : EngravingItem(s)
{
    m_align = s.m_align;
    for (EngravingItem* e : s.m_leafs) {
        EngravingItem* ee = e->clone();
        ee->setParent(this);
        m_leafs.push_back(ee);
    }
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void BSymbol::add(EngravingItem* e)
{
    if (e->isSymbol() || e->isImage()) {
        e->setParent(this);
        e->setTrack(track());
        m_leafs.push_back(e);
        toBSymbol(e)->setZ(z() - 1);        // draw on top of parent
        e->added();
    } else {
        LOGD("BSymbol::add: unsupported type %s", e->typeName());
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void BSymbol::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    func(data, this);
    for (EngravingItem* e : m_leafs) {
        e->scanElements(data, func, all);
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BSymbol::remove(EngravingItem* e)
{
    if (e->isSymbol() || e->isImage()) {
        if (muse::remove(m_leafs, e)) {
            e->removed();
        } else {
            LOGD("BSymbol::remove: element <%s> not found", e->typeName());
        }
    } else {
        LOGD("BSymbol::remove: unsupported type %s", e->typeName());
    }
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool BSymbol::acceptDrop(EditData& data) const
{
    return data.dropElement->isSymbol() || data.dropElement->isImage();
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* BSymbol::drop(EditData& data)
{
    EngravingItem* el = data.dropElement;
    if (el->isSymbol() || el->isImage()) {
        el->setParent(this);
        PointF p = data.pos - pageBoundingRect().topLeft() - data.dragOffset;
        if (p == PointF()) {
            // offset position so newly added child is visible
            p = PointF(width(), height());
        }
        el->setOffset(p);
        score()->undoAddElement(el);
        return el;
    } else {
        delete el;
    }
    return 0;
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF BSymbol::drag(EditData& ed)
{
    RectF r(canvasBoundingRect());
    for (const EngravingItem* e : m_leafs) {
        r.unite(e->canvasBoundingRect());
    }

    double x = ed.delta.x();
    double y = ed.delta.y();

    double _spatium = spatium();
    if (ed.hRaster) {
        double hRaster = _spatium / MScore::hRaster();
        int n = lrint(x / hRaster);
        x = hRaster * n;
    }
    if (ed.vRaster) {
        double vRaster = _spatium / MScore::vRaster();
        int n = lrint(y / vRaster);
        y = vRaster * n;
    }

    setOffset(PointF(x, y));

    r.unite(canvasBoundingRect());
    for (const EngravingItem* e : m_leafs) {
        r.unite(e->canvasBoundingRect());
    }
    return r;
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

std::vector<LineF> BSymbol::dragAnchorLines() const
{
    return genericDragAnchorLines();
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF BSymbol::pagePos() const
{
    if (explicitParent() && (explicitParent()->type() == ElementType::SEGMENT)) {
        PointF p(pos());
        System* system = segment()->measure()->system();
        if (system) {
            p.ry() += system->staff(staffIdx())->y() + system->y();
        }
        p.rx() = pageX();
        return p;
    } else {
        return EngravingItem::pagePos();
    }
}

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

PointF BSymbol::canvasPos() const
{
    if (explicitParent() && (explicitParent()->type() == ElementType::SEGMENT)) {
        PointF p(pos());
        Segment* s = toSegment(explicitParent());

        System* system = s->measure()->system();
        if (system) {
            staff_idx_t si = staffIdx();
            p.ry() += system->staff(si)->y() + system->y();
            Page* page = system->page();
            if (page) {
                p.ry() += page->y();
            }
        }
        p.rx() = canvasX();
        return p;
    } else {
        return EngravingItem::canvasPos();
    }
}
}
