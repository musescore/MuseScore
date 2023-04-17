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

#ifndef __PAGE_H__
#define __PAGE_H__

#include <vector>

#include "engravingitem.h"
#include "bsp.h"

namespace mu::engraving {
class RootItem;
class Factory;
class System;
class Text;
class Measure;
class XmlWriter;
class Score;
class MeasureBase;

//---------------------------------------------------------
//   @@ Page
//   @P pagenumber int (read only)
//---------------------------------------------------------

class Page final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Page)
    DECLARE_CLASSOF(ElementType::PAGE)

    std::vector<System*> _systems;
    page_idx_t _no;                        // page number

    BspTree bspTree;
    bool bspTreeValid;

    void doRebuildBspTree();

    friend class Factory;
    Page(RootItem* parent);

    String replaceTextMacros(const String&) const;
    void drawHeaderFooter(mu::draw::Painter*, int area, const String&) const;
    Text* layoutHeaderFooter(int area, const String& ss) const;

public:
    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    Page* clone() const override { return new Page(*this); }
    const std::vector<System*>& systems() const { return _systems; }
    std::vector<System*>& systems() { return _systems; }
    System* system(int idx) { return _systems[idx]; }

    void write(XmlWriter&) const override;

    void appendSystem(System* s);

    page_idx_t no() const { return _no; }
    void setNo(page_idx_t n) { _no = n; }
    bool isOdd() const;
    double tm() const;              // margins in pixel
    double bm() const;
    double lm() const;
    double rm() const;
    double headerExtension() const;
    double footerExtension() const;

    void draw(mu::draw::Painter*) const override;
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    std::vector<EngravingItem*> items(const mu::RectF& r);
    std::vector<EngravingItem*> items(const mu::PointF& p);
    void invalidateBspTree() { bspTreeValid = false; }
    mu::PointF pagePos() const override { return mu::PointF(); }       ///< position in page coordinates
    std::vector<EngravingItem*> elements() const;              ///< list of visible elements
    mu::RectF tbbox();                             // tight bounding box, excluding white space
    Fraction endTick() const;

#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif
};
} // namespace mu::engraving
#endif
