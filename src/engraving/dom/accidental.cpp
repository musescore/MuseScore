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

#include "accidental.h"

#include "types/symnames.h"
#include "types/translatablestring.h"

#include "actionicon.h"
#include "note.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   Acc
//---------------------------------------------------------

struct Acc {
    AccidentalVal offset;     // semitone offset
    double centOffset;
    SymId sym;
    Acc(AccidentalVal o, double o2, SymId s)
        : offset(o), centOffset(o2), sym(s) {}
};

// NOTE: keep this in sync with AccidentalType enum in types.h, watch out for isMicrotonal()
static const Acc ACC_LIST[] = {
    Acc(AccidentalVal::NATURAL,    0,   SymId::noSym),                  // NONE
    Acc(AccidentalVal::FLAT,       0,   SymId::accidentalFlat),         // FLAT
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalNatural),      // NATURAL
    Acc(AccidentalVal::SHARP,      0,   SymId::accidentalSharp),        // SHARP
    Acc(AccidentalVal::SHARP2,     0,   SymId::accidentalDoubleSharp),  // SHARP2
    Acc(AccidentalVal::FLAT2,      0,   SymId::accidentalDoubleFlat),   // FLAT2
    Acc(AccidentalVal::SHARP3,     0,   SymId::accidentalTripleSharp),  // SHARP3
    Acc(AccidentalVal::FLAT3,      0,   SymId::accidentalTripleFlat),   // FLAT3
    Acc(AccidentalVal::FLAT,       0,   SymId::accidentalNaturalFlat),  // NATURAL_FLAT
    Acc(AccidentalVal::SHARP,      0,   SymId::accidentalNaturalSharp), // NATURAL_SHARP
    Acc(AccidentalVal::SHARP2,     0,   SymId::accidentalSharpSharp),   // SHARP_SHARP

    // Gould arrow quartertone
    Acc(AccidentalVal::NATURAL,  -50,   SymId::accidentalQuarterToneFlatArrowUp),          // FLAT_ARROW_UP
    Acc(AccidentalVal::NATURAL, -150,   SymId::accidentalThreeQuarterTonesFlatArrowDown),  // FLAT_ARROW_DOWN
    Acc(AccidentalVal::NATURAL,   50,   SymId::accidentalQuarterToneSharpNaturalArrowUp),  // NATURAL_ARROW_UP
    Acc(AccidentalVal::NATURAL,  -50,   SymId::accidentalQuarterToneFlatNaturalArrowDown), // NATURAL_ARROW_DOWN
    Acc(AccidentalVal::NATURAL,  150,   SymId::accidentalThreeQuarterTonesSharpArrowUp),   // SHARP_ARROW_UP
    Acc(AccidentalVal::NATURAL,   50,   SymId::accidentalQuarterToneSharpArrowDown),       // SHARP_ARROW_DOWN
    Acc(AccidentalVal::NATURAL,  250,   SymId::accidentalFiveQuarterTonesSharpArrowUp),    // SHARP2_ARROW_UP
    Acc(AccidentalVal::NATURAL,  150,   SymId::accidentalThreeQuarterTonesSharpArrowDown), // SHARP2_ARROW_DOWN
    Acc(AccidentalVal::NATURAL, -150,   SymId::accidentalThreeQuarterTonesFlatArrowUp),    // FLAT2_ARROW_UP
    Acc(AccidentalVal::NATURAL, -250,   SymId::accidentalFiveQuarterTonesFlatArrowDown),   // FLAT2_ARROW_DOWN
    Acc(AccidentalVal::NATURAL,  -50,   SymId::accidentalArrowDown), // ARROW_DOWN
    Acc(AccidentalVal::NATURAL,   50,   SymId::accidentalArrowUp),   // ARROW_UP

    // Stein-Zimmermann
    Acc(AccidentalVal::NATURAL,  -50,   SymId::accidentalQuarterToneFlatStein),   // MIRRORED_FLAT
    Acc(AccidentalVal::NATURAL, -150,   SymId::accidentalThreeQuarterTonesFlatZimmermann),   // MIRRORED_FLAT2
    Acc(AccidentalVal::NATURAL,   50,   SymId::accidentalQuarterToneSharpStein),         // SHARP_SLASH
    Acc(AccidentalVal::NATURAL,  150,   SymId::accidentalThreeQuarterTonesSharpStein),   // SHARP_SLASH4

    // Arel-Ezgi-Uzdilek (AEU)
    Acc(AccidentalVal::NATURAL,  -89,   SymId::accidentalBuyukMucennebFlat),    // FLAT_SLASH2
    Acc(AccidentalVal::NATURAL,  -44,   SymId::accidentalBakiyeFlat),           // FLAT_SLASH
    Acc(AccidentalVal::NATURAL,   56,   SymId::accidentalKucukMucennebSharp),   // SHARP_SLASH3
    Acc(AccidentalVal::NATURAL,   89,   SymId::accidentalBuyukMucennebSharp),   // SHARP_SLASH2

    // Extended Helmholtz-Ellis accidentals (just intonation)
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleFlatOneArrowDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalFlatOneArrowDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalNaturalOneArrowDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalSharpOneArrowDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleSharpOneArrowDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleFlatOneArrowUp),

    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalFlatOneArrowUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalNaturalOneArrowUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalSharpOneArrowUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleSharpOneArrowUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleFlatTwoArrowsDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalFlatTwoArrowsDown),

    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalNaturalTwoArrowsDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalSharpTwoArrowsDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleSharpTwoArrowsDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleFlatTwoArrowsUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalFlatTwoArrowsUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalNaturalTwoArrowsUp),

    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalSharpTwoArrowsUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleSharpTwoArrowsUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleFlatThreeArrowsDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalFlatThreeArrowsDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalNaturalThreeArrowsDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalSharpThreeArrowsDown),

    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleSharpThreeArrowsDown),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleFlatThreeArrowsUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalFlatThreeArrowsUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalNaturalThreeArrowsUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalSharpThreeArrowsUp),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleSharpThreeArrowsUp),

    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalLowerOneSeptimalComma),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalRaiseOneSeptimalComma),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalLowerTwoSeptimalCommas),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalRaiseTwoSeptimalCommas),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalLowerOneUndecimalQuartertone),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalRaiseOneUndecimalQuartertone),

    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalLowerOneTridecimalQuartertone),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalRaiseOneTridecimalQuartertone),

    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleFlatEqualTempered),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalFlatEqualTempered),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalNaturalEqualTempered),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalSharpEqualTempered),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalDoubleSharpEqualTempered),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalQuarterFlatEqualTempered),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalQuarterSharpEqualTempered),

    Acc(AccidentalVal::NATURAL,   -6.8, SymId::accidentalCombiningLower17Schisma),
    Acc(AccidentalVal::NATURAL,    6.8, SymId::accidentalCombiningRaise17Schisma),
    Acc(AccidentalVal::NATURAL,   -3.4, SymId::accidentalCombiningLower19Schisma),
    Acc(AccidentalVal::NATURAL,    3.4, SymId::accidentalCombiningRaise19Schisma),
    Acc(AccidentalVal::NATURAL,  -16.5, SymId::accidentalCombiningLower23Limit29LimitComma),
    Acc(AccidentalVal::NATURAL,   16.5, SymId::accidentalCombiningRaise23Limit29LimitComma),
    Acc(AccidentalVal::NATURAL,   -1.7, SymId::accidentalCombiningLower31Schisma),
    Acc(AccidentalVal::NATURAL,    1.7, SymId::accidentalCombiningRaise31Schisma),
    Acc(AccidentalVal::NATURAL,  -10.9, SymId::accidentalCombiningLower53LimitComma),
    Acc(AccidentalVal::NATURAL,   10.9, SymId::accidentalCombiningRaise53LimitComma),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalEnharmonicAlmostEqualTo),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalEnharmonicEquals),
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalEnharmonicTilde),

    // Persian
    Acc(AccidentalVal::NATURAL,   33,   SymId::accidentalSori),                            // SORI
    Acc(AccidentalVal::NATURAL,  -67,   SymId::accidentalKoron),                           // KORON

    // Wyschnegradsky
    Acc(AccidentalVal::NATURAL, -167,   SymId::accidentalWyschnegradsky10TwelfthsFlat),
    Acc(AccidentalVal::NATURAL,  167,   SymId::accidentalWyschnegradsky10TwelfthsSharp),
    Acc(AccidentalVal::NATURAL, -183,   SymId::accidentalWyschnegradsky11TwelfthsFlat),
    Acc(AccidentalVal::NATURAL,  183,   SymId::accidentalWyschnegradsky11TwelfthsSharp),
    Acc(AccidentalVal::NATURAL,  -17,   SymId::accidentalWyschnegradsky1TwelfthsFlat),
    Acc(AccidentalVal::NATURAL,   17,   SymId::accidentalWyschnegradsky1TwelfthsSharp),
    Acc(AccidentalVal::NATURAL,  -33,   SymId::accidentalWyschnegradsky2TwelfthsFlat),
    Acc(AccidentalVal::NATURAL,   33,   SymId::accidentalWyschnegradsky2TwelfthsSharp),
    Acc(AccidentalVal::NATURAL,  -50,   SymId::accidentalWyschnegradsky3TwelfthsFlat),
    Acc(AccidentalVal::NATURAL,   50,   SymId::accidentalWyschnegradsky3TwelfthsSharp),
    Acc(AccidentalVal::NATURAL,  -67,   SymId::accidentalWyschnegradsky4TwelfthsFlat),
    Acc(AccidentalVal::NATURAL,   67,   SymId::accidentalWyschnegradsky4TwelfthsSharp),
    Acc(AccidentalVal::NATURAL,  -83,   SymId::accidentalWyschnegradsky5TwelfthsFlat),
    Acc(AccidentalVal::NATURAL,   83,   SymId::accidentalWyschnegradsky5TwelfthsSharp),
    Acc(AccidentalVal::FLAT,       0,   SymId::accidentalWyschnegradsky6TwelfthsFlat),
    Acc(AccidentalVal::SHARP,      0,   SymId::accidentalWyschnegradsky6TwelfthsSharp),
    Acc(AccidentalVal::NATURAL, -116,   SymId::accidentalWyschnegradsky7TwelfthsFlat),
    Acc(AccidentalVal::NATURAL,  116,   SymId::accidentalWyschnegradsky7TwelfthsSharp),
    Acc(AccidentalVal::NATURAL, -133,   SymId::accidentalWyschnegradsky8TwelfthsFlat),
    Acc(AccidentalVal::NATURAL,  133,   SymId::accidentalWyschnegradsky8TwelfthsSharp),
    Acc(AccidentalVal::NATURAL, -150,   SymId::accidentalWyschnegradsky9TwelfthsFlat),
    Acc(AccidentalVal::NATURAL,  150,   SymId::accidentalWyschnegradsky9TwelfthsSharp),

    // the most important (Spartan) Sagittal accidentals
    Acc(AccidentalVal::NATURAL,   -5.8, SymId::accSagittal5v7KleismaDown),
    Acc(AccidentalVal::NATURAL,    5.8, SymId::accSagittal5v7KleismaUp),
    Acc(AccidentalVal::NATURAL,  -21.5, SymId::accSagittal5CommaDown),
    Acc(AccidentalVal::NATURAL,   21.5, SymId::accSagittal5CommaUp),
    Acc(AccidentalVal::NATURAL,  -27.3, SymId::accSagittal7CommaDown),
    Acc(AccidentalVal::NATURAL,   27.3, SymId::accSagittal7CommaUp),
    Acc(AccidentalVal::NATURAL,  -43.0, SymId::accSagittal25SmallDiesisDown),
    Acc(AccidentalVal::NATURAL,   43.0, SymId::accSagittal25SmallDiesisUp),
    Acc(AccidentalVal::NATURAL,  -48.8, SymId::accSagittal35MediumDiesisDown),
    Acc(AccidentalVal::NATURAL,   48.8, SymId::accSagittal35MediumDiesisUp),
    Acc(AccidentalVal::NATURAL,  -53.3, SymId::accSagittal11MediumDiesisDown),
    Acc(AccidentalVal::NATURAL,   53.3, SymId::accSagittal11MediumDiesisUp),
    Acc(AccidentalVal::NATURAL,  -60.4, SymId::accSagittal11LargeDiesisDown),
    Acc(AccidentalVal::NATURAL,   60.4, SymId::accSagittal11LargeDiesisUp),
    Acc(AccidentalVal::NATURAL,  -64.9, SymId::accSagittal35LargeDiesisDown),
    Acc(AccidentalVal::NATURAL,   64.9, SymId::accSagittal35LargeDiesisUp),
    Acc(AccidentalVal::NATURAL,  -70.7, SymId::accSagittalFlat25SUp),
    Acc(AccidentalVal::NATURAL,   70.7, SymId::accSagittalSharp25SDown),
    Acc(AccidentalVal::NATURAL,  -86.4, SymId::accSagittalFlat7CUp),
    Acc(AccidentalVal::NATURAL,   86.4, SymId::accSagittalSharp7CDown),
    Acc(AccidentalVal::NATURAL,  -92.2, SymId::accSagittalFlat5CUp),
    Acc(AccidentalVal::NATURAL,   92.2, SymId::accSagittalSharp5CDown),
    Acc(AccidentalVal::NATURAL, -107.9, SymId::accSagittalFlat5v7kUp),
    Acc(AccidentalVal::NATURAL,  107.9, SymId::accSagittalSharp5v7kDown),
    Acc(AccidentalVal::NATURAL, -113.7, SymId::accSagittalFlat),
    Acc(AccidentalVal::NATURAL,  113.7, SymId::accSagittalSharp),

    // Turkish folk music accidentals
    Acc(AccidentalVal::NATURAL, -22.2, SymId::accidental1CommaFlat),
    Acc(AccidentalVal::NATURAL,  22.2, SymId::accidental1CommaSharp),
    Acc(AccidentalVal::NATURAL, -44.4, SymId::accidental2CommaFlat),
    Acc(AccidentalVal::NATURAL,  44.4, SymId::accidental2CommaSharp),
    Acc(AccidentalVal::NATURAL, -66.7, SymId::accidental3CommaFlat),
    Acc(AccidentalVal::NATURAL,  66.7, SymId::accidental3CommaSharp),
    Acc(AccidentalVal::NATURAL, -88.9, SymId::accidental4CommaFlat),
    //Acc(AccidentalVal::NATURAL,  88.9, SymId::accidentalSharp),
    Acc(AccidentalVal::NATURAL, 111.1, SymId::accidental5CommaSharp),
};

//---------------------------------------------------------
//   sym2accidentalVal
//---------------------------------------------------------

AccidentalVal sym2accidentalVal(SymId id)
{
    for (const Acc& a : ACC_LIST) {
        if (a.sym == id) {
            return a.offset;
        }
    }
    return AccidentalVal::NATURAL;
}

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

Accidental::Accidental(EngravingItem* parent)
    : EngravingItem(ElementType::ACCIDENTAL, parent, ElementFlag::MOVABLE)
{
}

//---------------------------------------------------------
//   subTypeUserName
//---------------------------------------------------------

TranslatableString Accidental::subtypeUserName() const
{
    return SymNames::userNameForSymId(symId());
}

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

SymId Accidental::symId() const
{
    return ACC_LIST[int(accidentalType())].sym;
}

bool Accidental::parentNoteHasParentheses() const
{
    return explicitParent() && parentItem()->isNote() ? toNote(parentItem())->bothParentheses() : false;
}

//---------------------------------------------------------
//   subtype2value
//    returns the resulting pitch offset
//---------------------------------------------------------

AccidentalVal Accidental::subtype2value(AccidentalType st)
{
    return ACC_LIST[int(st)].offset;
}

//---------------------------------------------------------
//   subtype2name
//---------------------------------------------------------

AsciiStringView Accidental::subtype2name(AccidentalType st)
{
    return SymNames::nameForSymId(ACC_LIST[int(st)].sym);
}

//---------------------------------------------------------
//   subtype2symbol
//---------------------------------------------------------

SymId Accidental::subtype2symbol(AccidentalType st)
{
    return ACC_LIST[int(st)].sym;
}

//---------------------------------------------------------
//   subtype2centoffset
//---------------------------------------------------------

double Accidental::subtype2centOffset(AccidentalType st)
{
    return ACC_LIST[int(st)].centOffset;
}

int Accidental::line() const
{
    Note* n = note();
    return n ? n->line() : 0;
}

//---------------------------------------------------------
//   name2subtype
//---------------------------------------------------------

AccidentalType Accidental::name2subtype(const AsciiStringView& tag)
{
    SymId symId = SymNames::symIdByName(tag);
    if (symId == SymId::noSym) {
        // LOGD("no symbol found");
    } else {
        int i = 0;
        for (const Acc& acc : ACC_LIST) {
            if (acc.sym == symId) {
                return AccidentalType(i);
            }
            ++i;
        }
    }
    return AccidentalType::NONE;
}

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Accidental::setSubtype(const AsciiStringView& tag)
{
    setAccidentalType(name2subtype(tag));
}

void Accidental::computeMag()
{
    double m = explicitParent() ? parentItem()->mag() : 1.0;
    if (isSmall()) {
        m *= style().styleD(Sid::smallNoteMag);
    }
    mutldata()->setMag(m);
}

//---------------------------------------------------------
//   value2subtype
//---------------------------------------------------------

AccidentalType Accidental::value2subtype(AccidentalVal v)
{
    switch (v) {
    case AccidentalVal::NATURAL: return AccidentalType::NONE;
    case AccidentalVal::SHARP:   return AccidentalType::SHARP;
    case AccidentalVal::SHARP2:  return AccidentalType::SHARP2;
    case AccidentalVal::SHARP3:  return AccidentalType::SHARP3;
    case AccidentalVal::FLAT:    return AccidentalType::FLAT;
    case AccidentalVal::FLAT2:   return AccidentalType::FLAT2;
    case AccidentalVal::FLAT3:   return AccidentalType::FLAT3;
    default:
        ASSERT_X(u"value2subtype: illegal accidental val: " + String::number(int(v)));
    }
    return AccidentalType::NONE;
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Accidental::acceptDrop(EditData& data) const
{
    EngravingItem* e = data.dropElement;

    if (e->type() == ElementType::ACCIDENTAL) {
        return true;
    }

    if (e->isActionIcon()) {
        ActionIconType type = toActionIcon(e)->actionType();
        return type == ActionIconType::PARENTHESES
               || type == ActionIconType::BRACKETS;
    }

    return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Accidental::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    switch (e->type()) {
    case ElementType::ACCIDENTAL:
        score()->changeAccidental(note(), toAccidental(e)->accidentalType());
        break;
    case ElementType::ACTION_ICON:
        switch (toActionIcon(e)->actionType()) {
        case ActionIconType::PARENTHESES:
            undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::PARENTHESIS), PropertyFlags::NOSTYLE);
            break;
        case ActionIconType::BRACKETS:
            undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::BRACKET), PropertyFlags::NOSTYLE);
            break;
        default:
            LOGD("unknown icon type");
            break;
        }
        break;
    default:
        break;
    }
    delete e;
    return nullptr;
}

//---------------------------------------------------------
//   undoSetSmall
//---------------------------------------------------------

void Accidental::undoSetSmall(bool val)
{
    undoChangeProperty(Pid::SMALL, val);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Accidental::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ACCIDENTAL_TYPE:    return int(m_accidentalType);
    case Pid::SMALL:              return m_isSmall;
    case Pid::ACCIDENTAL_BRACKET: return int(bracket());
    case Pid::ACCIDENTAL_ROLE:    return role();
    case Pid::ACCIDENTAL_STACKING_ORDER_OFFSET: return stackingOrderOffset();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Accidental::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ACCIDENTAL_TYPE:    return int(AccidentalType::NONE);
    case Pid::SMALL:              return false;
    case Pid::ACCIDENTAL_BRACKET: return int(AccidentalBracket::NONE);
    case Pid::ACCIDENTAL_ROLE:    return AccidentalRole::AUTO;
    case Pid::ACCIDENTAL_STACKING_ORDER_OFFSET: return 0;
    default:
        return EngravingItem::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Accidental::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::ACCIDENTAL_TYPE:
        setAccidentalType(AccidentalType(v.toInt()));
        break;
    case Pid::SMALL:
        m_isSmall = v.toBool();
        break;
    case Pid::ACCIDENTAL_BRACKET:
        m_bracket = AccidentalBracket(v.toInt());
        break;
    case Pid::ACCIDENTAL_ROLE:
        m_role = v.value<AccidentalRole>();
        break;
    case Pid::ACCIDENTAL_STACKING_ORDER_OFFSET:
        setStackingOrderOffset(v.toInt());
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Accidental::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedSubtypeUserName());
}
}
