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

#ifndef __BSYMBOL_H__
#define __BSYMBOL_H__

#include "engravingitem.h"

namespace Ms {
//---------------------------------------------------------
//   @@ BSymbol
///    base class for Symbol and Image
//---------------------------------------------------------

class BSymbol : public EngravingItem
{
public:

    Segment* segment() const { return (Segment*)explicitParent(); }

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObject* scanChild(int idx) const override;
    int scanChildCount() const override;
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    BSymbol& operator=(const BSymbol&) = delete;

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;
    void layout() override;
    mu::RectF drag(EditData&) override;

    void writeProperties(XmlWriter& xml) const override;
    bool readProperties(XmlReader&) override;

    Align align() const { return _align; }
    void setAlign(Align a) { _align = a; }

    const QList<EngravingItem*>& leafs() const { return _leafs; }
    QList<EngravingItem*>& leafs() { return _leafs; }
    mu::PointF pagePos() const override;
    mu::PointF canvasPos() const override;
    QVector<mu::LineF> dragAnchorLines() const override;

protected:
    BSymbol(const ElementType& type, EngravingItem* parent, ElementFlags f = ElementFlag::NOTHING);
    BSymbol(const BSymbol&);

private:

    QList<EngravingItem*> _leafs;
    Align _align;
};
}     // namespace Ms
#endif
