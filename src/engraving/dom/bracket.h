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

namespace mu::engraving {
class Factory;
class System;
enum class BracketType : signed char;

//---------------------------------------------------------
//   @@ Bracket
//---------------------------------------------------------

class Bracket final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Bracket)
    DECLARE_CLASSOF(ElementType::BRACKET)

public:

    ~Bracket();

    Bracket* clone() const override { return new Bracket(*this); }

    void setBracketItem(BracketItem* i) { m_bi = i; }
    BracketItem* bracketItem() const { return m_bi; }

    BracketType bracketType() const { return m_bi->bracketType(); }

    size_t firstStaff() const { return m_firstStaff; }
    size_t lastStaff() const { return m_lastStaff; }
    void setStaffSpan(size_t a, size_t b);

    SymId braceSymbol() const { return m_braceSymbol; }
    void setBraceSymbol(const SymId& sym) { m_braceSymbol = sym; }
    size_t column() const { return m_bi->column(); }
    size_t span() const { return m_bi->bracketSpan(); }
    double magx() const { return m_magx; }

    System* system() const { return (System*)explicitParent(); }

    Measure* measure() const { return m_measure; }
    void setMeasure(Measure* measure) { m_measure = measure; }

    Fraction playTick() const override;

    const BracketItem* bi() const { return m_bi; }

    bool isEditable() const override;
    bool needStartEditingAfterSelecting() const override;
    void startEdit(EditData&) override;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;
    void endEdit(EditData&) override;
    void editDrag(EditData&) override;
    void endEditDrag(EditData&) override;

    mu::draw::Color color() const override { return m_bi->color(); }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps) override;
    using EngravingObject::undoChangeProperty;

    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    void setSelected(bool f) override;

    struct LayoutData : public EngravingItem::LayoutData {
        SymId braceSymbol = SymId::noSym;

        PainterPath path;
        Shape shape;

        bool isSetBracketHeight() const { return m_bracketHeight.has_value(); }
        void setBracketHeight(double v) { m_bracketHeight.set_value(v); }
        double bracketHeight(LD_ACCESS mode = LD_ACCESS::CHECK) const { return m_bracketHeight.value(mode); }
        double h2(LD_ACCESS mode = LD_ACCESS::CHECK) const { return m_bracketHeight.value(mode) * 0.5; }

        bool isSetBracketWidth() const { return m_bracketWidth.has_value(); }
        void setBracketWidth(double v) { m_bracketWidth.set_value(v); }
        double bracketWidth(LD_ACCESS mode = LD_ACCESS::CHECK) const { return m_bracketWidth.value(mode); }

    private:
        ld_field<double> m_bracketHeight = { "bracketHeight", 0.0 };
        ld_field<double> m_bracketWidth = { "bracketWidth", 0.0 };
    };
    DECLARE_LAYOUTDATA_METHODS(Bracket);

    //! --- DEPRECATED ---
    double h2() const { return ldata()->h2(); }
    //! ------------------

private:
    friend class Factory;

    Bracket(EngravingItem* parent);

    BracketItem* m_bi = nullptr;
    double m_ay1 = 0.0;

    size_t m_firstStaff = 0;
    size_t m_lastStaff = 0;

    SymId m_braceSymbol = SymId::noSym;

    // horizontal scaling factor for brace symbol. Cannot be equal to magY or depend on h
    // because layout needs width of brace before knowing height of system...
    double m_magx = 0.0;
    Measure* m_measure = nullptr;
};
} // namespace mu::engraving
#endif
