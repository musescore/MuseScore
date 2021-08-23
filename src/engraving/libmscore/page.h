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

#include "config.h"
#include "element.h"
#include "bsp.h"

namespace Ms {
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

class Page final : public Element
{
    QList<System*> _systems;
    int _no;                        // page number
#ifdef USE_BSP
    BspTree bspTree;
    void doRebuildBspTree();
#endif
    bool bspTreeValid;

    QString replaceTextMacros(const QString&) const;
    void drawHeaderFooter(mu::draw::Painter*, int area, const QString&) const;

public:
    Page(Score*);
    ~Page();

    // Score Tree functions
    ScoreElement* treeParent() const override;
    ScoreElement* treeChild(int idx) const override;
    int treeChildCount() const override;

    Page* clone() const override { return new Page(*this); }
    ElementType type() const override { return ElementType::PAGE; }
    const QList<System*>& systems() const { return _systems; }
    QList<System*>& systems() { return _systems; }
    System* system(int idx) { return _systems[idx]; }

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    void appendSystem(System* s);

    int no() const { return _no; }
    void setNo(int n) { _no = n; }
    bool isOdd() const;
    qreal tm() const;              // margins in pixel
    qreal bm() const;
    qreal lm() const;
    qreal rm() const;

    void draw(mu::draw::Painter*) const override;
    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;

    QList<Element*> items(const mu::RectF& r);
    QList<Element*> items(const mu::PointF& p);
    void invalidateBspTree() { bspTreeValid = false; }
    mu::PointF pagePos() const override { return mu::PointF(); }       ///< position in page coordinates
    QList<Element*> elements() const;           ///< list of visible elements
    mu::RectF tbbox();                             // tight bounding box, excluding white space
    Fraction endTick() const;
};
}     // namespace Ms
#endif
