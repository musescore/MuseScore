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

#ifndef __ACCIDENTAL_H__
#define __ACCIDENTAL_H__

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
//   SymElement
//---------------------------------------------------------

struct SymElement {
    SymId sym;
    double x;
    double y;
    SymElement(SymId _sym, double _x, double _y)
        : sym(_sym), x(_x), y(_y) {}
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

    std::vector<SymElement> el;
    AccidentalType _accidentalType { AccidentalType::NONE };
    bool m_isSmall                    { false };
    AccidentalBracket _bracket     { AccidentalBracket::NONE };
    AccidentalRole _role           { AccidentalRole::AUTO };

    friend class Factory;

    Accidental(EngravingItem* parent);

public:

    Accidental* clone() const override { return new Accidental(*this); }

    // Score Tree functions
    EngravingObject* scanParent() const override;

    TranslatableString subtypeUserName() const override;
    void setSubtype(const AsciiStringView& s);
    void setAccidentalType(AccidentalType t) { _accidentalType = t; }

    AccidentalType accidentalType() const { return _accidentalType; }
    AccidentalRole role() const { return _role; }

    int subtype() const override { return (int)_accidentalType; }

    void clearElements() { el.clear(); }
    void addElement(const SymElement& e) { el.push_back(e); }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;
    void layout() override;

    void draw(mu::draw::Painter*) const override;
    bool isEditable() const override { return true; }
    void startEdit(EditData&) override { setGenerated(false); }

    SymId symbol() const;
    Note* note() const { return (explicitParent() && explicitParent()->isNote()) ? toNote(explicitParent()) : 0; }

    AccidentalBracket bracket() const { return _bracket; }
    void setBracket(AccidentalBracket val) { _bracket = val; }

    void setRole(AccidentalRole r) { _role = r; }

    bool isSmall() const { return m_isSmall; }
    void setSmall(bool val) { m_isSmall = val; }

    void undoSetSmall(bool val);

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    static AccidentalVal subtype2value(AccidentalType);               // return effective pitch offset
    static SymId subtype2symbol(AccidentalType);
    static mu::AsciiStringView subtype2name(AccidentalType);
    static AccidentalType value2subtype(AccidentalVal);
    static AccidentalType name2subtype(const mu::AsciiStringView&);
    static bool isMicrotonal(AccidentalType t) { return t > AccidentalType::FLAT3; }

    String accessibleInfo() const override;
};

extern AccidentalVal sym2accidentalVal(SymId id);
} // namespace mu::engraving

#endif
