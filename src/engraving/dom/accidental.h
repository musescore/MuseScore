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

#pragma once

#include <vector>

#include "engravingitem.h"

namespace mu::engraving {
class Factory;
class Note;
enum class AccidentalVal : signed char;

// NOTE: keep this in sync with accList array in accidentals.cpp
enum class AccidentalType : unsigned char {
    ///.\{
    NONE,
    FLAT,
    NATURAL,
    SHARP,
    SHARP2,
    FLAT2,
    SHARP3,
    FLAT3,
    NATURAL_FLAT,
    NATURAL_SHARP,
    SHARP_SHARP,

    // Gould arrow quartertone
    FLAT_ARROW_UP,
    FLAT_ARROW_DOWN,
    NATURAL_ARROW_UP,
    NATURAL_ARROW_DOWN,
    SHARP_ARROW_UP,
    SHARP_ARROW_DOWN,
    SHARP2_ARROW_UP,
    SHARP2_ARROW_DOWN,
    FLAT2_ARROW_UP,
    FLAT2_ARROW_DOWN,
    ARROW_DOWN,
    ARROW_UP,

    // Stein-Zimmermann
    MIRRORED_FLAT,
    MIRRORED_FLAT2,
    SHARP_SLASH,
    SHARP_SLASH4,

    // Arel-Ezgi-Uzdilek (AEU)
    FLAT_SLASH2,
    FLAT_SLASH,
    SHARP_SLASH3,
    SHARP_SLASH2,

    // Extended Helmholtz-Ellis accidentals (just intonation)
    DOUBLE_FLAT_ONE_ARROW_DOWN,
    FLAT_ONE_ARROW_DOWN,
    NATURAL_ONE_ARROW_DOWN,
    SHARP_ONE_ARROW_DOWN,
    DOUBLE_SHARP_ONE_ARROW_DOWN,
    DOUBLE_FLAT_ONE_ARROW_UP,

    FLAT_ONE_ARROW_UP,
    NATURAL_ONE_ARROW_UP,
    SHARP_ONE_ARROW_UP,
    DOUBLE_SHARP_ONE_ARROW_UP,
    DOUBLE_FLAT_TWO_ARROWS_DOWN,
    FLAT_TWO_ARROWS_DOWN,

    NATURAL_TWO_ARROWS_DOWN,
    SHARP_TWO_ARROWS_DOWN,
    DOUBLE_SHARP_TWO_ARROWS_DOWN,
    DOUBLE_FLAT_TWO_ARROWS_UP,
    FLAT_TWO_ARROWS_UP,
    NATURAL_TWO_ARROWS_UP,

    SHARP_TWO_ARROWS_UP,
    DOUBLE_SHARP_TWO_ARROWS_UP,
    DOUBLE_FLAT_THREE_ARROWS_DOWN,
    FLAT_THREE_ARROWS_DOWN,
    NATURAL_THREE_ARROWS_DOWN,
    SHARP_THREE_ARROWS_DOWN,

    DOUBLE_SHARP_THREE_ARROWS_DOWN,
    DOUBLE_FLAT_THREE_ARROWS_UP,
    FLAT_THREE_ARROWS_UP,
    NATURAL_THREE_ARROWS_UP,
    SHARP_THREE_ARROWS_UP,
    DOUBLE_SHARP_THREE_ARROWS_UP,

    LOWER_ONE_SEPTIMAL_COMMA,
    RAISE_ONE_SEPTIMAL_COMMA,
    LOWER_TWO_SEPTIMAL_COMMAS,
    RAISE_TWO_SEPTIMAL_COMMAS,
    LOWER_ONE_UNDECIMAL_QUARTERTONE,
    RAISE_ONE_UNDECIMAL_QUARTERTONE,

    LOWER_ONE_TRIDECIMAL_QUARTERTONE,
    RAISE_ONE_TRIDECIMAL_QUARTERTONE,

    DOUBLE_FLAT_EQUAL_TEMPERED,
    FLAT_EQUAL_TEMPERED,
    NATURAL_EQUAL_TEMPERED,
    SHARP_EQUAL_TEMPERED,
    DOUBLE_SHARP_EQUAL_TEMPERED,
    QUARTER_FLAT_EQUAL_TEMPERED,
    QUARTER_SHARP_EQUAL_TEMPERED,

    FLAT_17,
    SHARP_17,
    FLAT_19,
    SHARP_19,
    FLAT_23,
    SHARP_23,
    FLAT_31,
    SHARP_31,
    FLAT_53,
    SHARP_53,
    EQUALS_ALMOST,
    EQUALS,
    TILDE,

    // Persian
    SORI,
    KORON,

    // Wyschnegradsky
    TEN_TWELFTH_FLAT,
    TEN_TWELFTH_SHARP,
    ELEVEN_TWELFTH_FLAT,
    ELEVEN_TWELFTH_SHARP,
    ONE_TWELFTH_FLAT,
    ONE_TWELFTH_SHARP,
    TWO_TWELFTH_FLAT,
    TWO_TWELFTH_SHARP,
    THREE_TWELFTH_FLAT,
    THREE_TWELFTH_SHARP,
    FOUR_TWELFTH_FLAT,
    FOUR_TWELFTH_SHARP,
    FIVE_TWELFTH_FLAT,
    FIVE_TWELFTH_SHARP,
    SIX_TWELFTH_FLAT,
    SIX_TWELFTH_SHARP,
    SEVEN_TWELFTH_FLAT,
    SEVEN_TWELFTH_SHARP,
    EIGHT_TWELFTH_FLAT,
    EIGHT_TWELFTH_SHARP,
    NINE_TWELFTH_FLAT,
    NINE_TWELFTH_SHARP,

    // (Spartan) Sagittal
    SAGITTAL_5V7KD,
    SAGITTAL_5V7KU,
    SAGITTAL_5CD,
    SAGITTAL_5CU,
    SAGITTAL_7CD,
    SAGITTAL_7CU,
    SAGITTAL_25SDD,
    SAGITTAL_25SDU,
    SAGITTAL_35MDD,
    SAGITTAL_35MDU,
    SAGITTAL_11MDD,
    SAGITTAL_11MDU,
    SAGITTAL_11LDD,
    SAGITTAL_11LDU,
    SAGITTAL_35LDD,
    SAGITTAL_35LDU,
    SAGITTAL_FLAT25SU,
    SAGITTAL_SHARP25SD,
    SAGITTAL_FLAT7CU,
    SAGITTAL_SHARP7CD,
    SAGITTAL_FLAT5CU,
    SAGITTAL_SHARP5CD,
    SAGITTAL_FLAT5V7KU,
    SAGITTAL_SHARP5V7KD,
    SAGITTAL_FLAT,
    SAGITTAL_SHARP,

    // Turkish folk music
    ONE_COMMA_FLAT,
    ONE_COMMA_SHARP,
    TWO_COMMA_FLAT,
    TWO_COMMA_SHARP,
    THREE_COMMA_FLAT,
    THREE_COMMA_SHARP,
    FOUR_COMMA_FLAT,
    //FOUR_COMMA_SHARP,
    FIVE_COMMA_SHARP,

    END
    ///\}
};

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
}

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::AccidentalType)
#endif
