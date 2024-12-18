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

#ifndef MU_ENGRAVING_PAGE_H
#define MU_ENGRAVING_PAGE_H

#include <vector>

#include "engravingitem.h"
#include "bsp.h"
#include "text.h"

namespace mu::engraving {
class RootItem;
class Factory;
class System;
class Text;
class Measure;

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

public:
    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    Page* clone() const override { return new Page(*this); }
    const std::vector<System*>& systems() const { return m_systems; }
    std::vector<System*>& systems() { return m_systems; }
    System* system(size_t idx) { return m_systems[idx]; }
    const System* system(size_t idx) const { return m_systems.at(idx); }

    void appendSystem(System* s);

    page_idx_t no() const { return m_no; }
    void setNo(page_idx_t n) { m_no = n; }
    bool isOdd() const;
    double tm() const;              // margins in pixel
    double bm() const;
    double lm() const;
    double rm() const;
    double headerExtension() const;
    double footerExtension() const;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    std::vector<EngravingItem*> items(const RectF& r);
    std::vector<EngravingItem*> items(const PointF& p);
    void invalidateBspTree() { m_bspTreeValid = false; }
    PointF pagePos() const override { return PointF(); }       ///< position in page coordinates
    std::vector<EngravingItem*> elements() const;              ///< list of visible elements
    RectF tbbox() const;                             // tight bounding box, excluding white space
    Fraction endTick() const;

#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif

    Text* layoutHeaderFooter(int area, const String& s) const;

private:

    friend class Factory;
    Page(RootItem* parent);

    void doRebuildBspTree();
    TextBlock replaceTextMacros(const TextBlock&) const;
    const CharFormat formatForMacro(const String&) const;

    std::vector<System*> m_systems;
    page_idx_t m_no = 0;                        // page number

    BspTree bspTree;
    bool m_bspTreeValid = false;
};
} // namespace mu::engraving
#endif
