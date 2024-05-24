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

#ifndef MU_ENGRAVING_ACCIDENTAL_H
#define MU_ENGRAVING_ACCIDENTAL_H

/**
 \file
 Definition of class Accidental
*/

#include <vector>

#include "engravingitem.h"

#include "types.h"

namespace mu::engraving {
class Factory;
class Note;
enum class AccidentalVal : signed char;

//---------------------------------------------------------
//   AccidentalBracket
//---------------------------------------------------------

enum class AccidentalBracket : char {
    NONE,
    PARENTHESIS,
    BRACKET,
    BRACE, //! Deprecated; removed from the UI and kept here only for compatibility purposes
};

//---------------------------------------------------------
//   @@ Accidental
//   @P role        enum  (Accidental.AUTO, .USER) (read only)
//   @P isSmall     bool
//---------------------------------------------------------

class Accidental final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Accidental)
    DECLARE_CLASSOF(ElementType::ACCIDENTAL)

public:

    Accidental* clone() const override { return new Accidental(*this); }

    Note* note() const { return (explicitParent() && explicitParent()->isNote()) ? toNote(explicitParent()) : 0; }

    // Score Tree functions
    EngravingObject* scanParent() const override;

    TranslatableString subtypeUserName() const override;
    void setSubtype(const AsciiStringView& s);
    int subtype() const override { return (int)m_accidentalType; }

    void setAccidentalType(AccidentalType t) { m_accidentalType = t; }
    AccidentalType accidentalType() const { return m_accidentalType; }

    void setRole(AccidentalRole r) { m_role = r; }
    AccidentalRole role() const { return m_role; }

    AccidentalBracket bracket() const { return m_bracket; }
    void setBracket(AccidentalBracket val) { m_bracket = val; }
    bool parentNoteHasParentheses() const;

    bool isSmall() const { return m_isSmall; }
    void setSmall(bool val) { m_isSmall = val; }

    SymId symId() const;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override { setGenerated(false); }

    void undoSetSmall(bool val);

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    static AccidentalVal subtype2value(AccidentalType);               // return effective pitch offset
    static SymId subtype2symbol(AccidentalType);
    static AsciiStringView subtype2name(AccidentalType);
    static AccidentalType value2subtype(AccidentalVal);
    static AccidentalType name2subtype(const AsciiStringView&);
    static bool isMicrotonal(AccidentalType t) { return t > AccidentalType::FLAT3; }
    static double subtype2centOffset(AccidentalType);

    int stackingOrder() const { return ldata()->stackingNumber + m_stackingOrderOffset; }

    int line() const;

    String accessibleInfo() const override;

    void computeMag();

    struct LayoutData : public EngravingItem::LayoutData {
        struct Sym {
            SymId sym;
            double x;
            double y;
            Sym(SymId _sym, double _x, double _y)
                : sym(_sym), x(_x), y(_y) {}
        };

        std::vector<Sym> syms;

        ld_field<int> stackingNumber = { "[Accidental] stackingNumber", 0 };
        ld_field<int> verticalSubgroup = { "[Accidental] verticalSubgroup", 0 };
        ld_field<int> column = { "[Accidental] column", 0 };
        ld_field<std::vector<Accidental*> > octaves = { "[Accidental] octaves", std::vector<Accidental*> {} };
        ld_field<std::vector<Accidental*> > seconds = { "[Accidental] seconds", std::vector<Accidental*> {} };

        bool isValid() const override { return EngravingItem::LayoutData::isValid() && !syms.empty(); }
    };
    DECLARE_LAYOUTDATA_METHODS(Accidental)

    int stackingOrderOffset() const { return m_stackingOrderOffset; }
    void setStackingOrderOffset(int v) { m_stackingOrderOffset = v; }

private:

    friend class Factory;

    Accidental(EngravingItem* parent);

    AccidentalType m_accidentalType = AccidentalType::NONE;
    AccidentalBracket m_bracket = AccidentalBracket::NONE;
    AccidentalRole m_role = AccidentalRole::AUTO;
    bool m_isSmall = false;
    int m_stackingOrderOffset = 0;
};

extern AccidentalVal sym2accidentalVal(SymId id);
} // namespace mu::engraving

#endif
