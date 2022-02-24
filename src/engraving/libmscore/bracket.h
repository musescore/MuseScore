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

#ifndef __BRACKET_H__
#define __BRACKET_H__

#include "engravingitem.h"
#include "bracketItem.h"
#include "infrastructure/draw/painterpath.h"

namespace mu::engraving {
class Factory;
}

namespace Ms {
class MuseScoreView;
class System;
enum class BracketType : signed char;

//---------------------------------------------------------
//   @@ Bracket
//---------------------------------------------------------

class Bracket final : public EngravingItem
{
    BracketItem* _bi;
    qreal ay1;
    qreal h2;

    int _firstStaff;
    int _lastStaff;

    mu::PainterPath path;
    SymId _braceSymbol;
    Shape _shape;

    // horizontal scaling factor for brace symbol. Cannot be equal to magY or depend on h
    // because layout needs width of brace before knowing height of system...
    qreal _magx;
    Measure* _measure = nullptr;

    friend class mu::engraving::Factory;
    Bracket(EngravingItem* parent);

public:

    ~Bracket();

    Bracket* clone() const override { return new Bracket(*this); }

    void setBracketItem(BracketItem* i) { _bi = i; }
    BracketItem* bracketItem() const { return _bi; }

    BracketType bracketType() const { return _bi->bracketType(); }
    static const char* bracketTypeName(BracketType type);

    int firstStaff() const { return _firstStaff; }
    int lastStaff() const { return _lastStaff; }
    void setStaffSpan(int a, int b);

    SymId braceSymbol() const { return _braceSymbol; }
    void setBraceSymbol(const SymId& sym) { _braceSymbol = sym; }
    int column() const { return _bi->column(); }
    int span() const { return _bi->bracketSpan(); }
    qreal magx() const { return _magx; }

    System* system() const { return (System*)explicitParent(); }

    Measure* measure() const { return _measure; }
    void setMeasure(Measure* measure) { _measure = measure; }

    Fraction playTick() const override;

    void setHeight(qreal) override;
    qreal width() const override;

    Shape shape() const override { return _shape; }

    void draw(mu::draw::Painter*) const override;
    void layout() override;

    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;
    void endEdit(EditData&) override;
    void editDrag(EditData&) override;
    void endEditDrag(EditData&) override;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;

    void undoChangeProperty(Pid id, const mu::engraving::PropertyValue& v, PropertyFlags ps) override;
    using EngravingObject::undoChangeProperty;

    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    void setSelected(bool f) override;
};
}     // namespace Ms
#endif
