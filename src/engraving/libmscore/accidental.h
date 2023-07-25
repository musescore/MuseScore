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

    void draw(mu::draw::Painter*) const override;
    bool isEditable() const override { return true; }
    void startEdit(EditData&) override { setGenerated(false); }

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
    static double subtype2centOffset(AccidentalType);

    String accessibleInfo() const override;

    void computeMag();

    struct LayoutData {
        struct Sym {
            SymId sym;
            double x;
            double y;
            Sym(SymId _sym, double _x, double _y)
                : sym(_sym), x(_x), y(_y) {}
        };

        std::vector<Sym> syms;
        RectF bbox;

        bool isValid() const { return !syms.empty(); }
        void invalidate() { syms.clear(); }
    };

    const LayoutData& layoutData() const { return m_layoutData; }
    void setLayoutData(const LayoutData& data);

    //! -- Old interface --
    void clearElements() { m_layoutData.syms.clear(); }
    void addElement(const LayoutData::Sym& s) { m_layoutData.syms.push_back(s); }
    //! -----------

private:

    friend class Factory;

    Accidental(EngravingItem* parent);

    AccidentalType m_accidentalType = AccidentalType::NONE;
    AccidentalBracket m_bracket = AccidentalBracket::NONE;
    AccidentalRole m_role = AccidentalRole::AUTO;
    bool m_isSmall = false;

    LayoutData m_layoutData;
};

extern AccidentalVal sym2accidentalVal(SymId id);
} // namespace mu::engraving

#endif
