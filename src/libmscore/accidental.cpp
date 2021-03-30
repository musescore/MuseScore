//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "accidental.h"
#include "note.h"
#include "symbol.h"
#include "sym.h"
#include "score.h"
#include "icon.h"
#include "staff.h"
#include "undo.h"
#include "xml.h"

namespace Ms {
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

// NOTE: keep this in sync with with AccidentalType enum in types.h, watch out for isMicrotonal()
static Acc accList[] = {
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
    Acc(AccidentalVal::NATURAL, -250,   SymId::accidentalThreeQuarterTonesFlatArrowUp),    // FLAT2_ARROW_UP
    Acc(AccidentalVal::NATURAL, -150,   SymId::accidentalFiveQuarterTonesFlatArrowDown),   // FLAT2_ARROW_DOWN
    Acc(AccidentalVal::NATURAL,  -50,   SymId::accidentalArrowDown), // ARROW_DOWN
    Acc(AccidentalVal::NATURAL,   50,   SymId::accidentalArrowUp),   // ARROW_UP

    // Stein-Zimmermann
    Acc(AccidentalVal::NATURAL,  -50,   SymId::accidentalQuarterToneFlatStein),   // MIRRORED_FLAT
    Acc(AccidentalVal::NATURAL, -150,   SymId::accidentalThreeQuarterTonesFlatZimmermann),   // MIRRORED_FLAT2
    Acc(AccidentalVal::NATURAL,   50,   SymId::accidentalQuarterToneSharpStein),         // SHARP_SLASH
    Acc(AccidentalVal::NATURAL,  150,   SymId::accidentalThreeQuarterTonesSharpStein),   // SHARP_SLASH4

    // Arel-Ezgi-Uzdilek (AEU)
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalBuyukMucennebFlat),    // FLAT_SLASH2
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalBakiyeFlat),           // FLAT_SLASH
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalKucukMucennebSharp),   // SHARP_SLASH3
    Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalBuyukMucennebSharp),   // SHARP_SLASH2

    // Extended Helmholtz-Ellis accidentals (just intonation)
    Acc(AccidentalVal::NATURAL,    0, SymId::accidentalDoubleFlatOneArrowDown),
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
    //Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalEnharmonicAlmostEqualTo),
    //Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalEnharmonicEquals),
    //Acc(AccidentalVal::NATURAL,    0,   SymId::accidentalEnharmonicTilde),

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
    for (const Acc& a : accList) {
        if (a.sym == id) {
            return a.offset;
        }
    }
    return AccidentalVal::NATURAL;
}

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

Accidental::Accidental(Score* s)
    : Element(s, ElementFlag::MOVABLE)
{
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Accidental::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "bracket") {
            int i = e.readInt();
            if (i == 0 || i == 1 || i == 2) {
                _bracket = AccidentalBracket(i);
            }
        } else if (tag == "subtype") {
            setSubtype(e.readElementText());
        } else if (tag == "role") {
            AccidentalRole r = AccidentalRole(e.readInt());
            if (r == AccidentalRole::AUTO || r == AccidentalRole::USER) {
                _role = r;
            }
        } else if (tag == "small") {
            _small = e.readInt();
        } else if (Element::readProperties(e)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Accidental::write(XmlWriter& xml) const
{
    xml.stag(this);
    writeProperty(xml, Pid::ACCIDENTAL_BRACKET);
    writeProperty(xml, Pid::ROLE);
    writeProperty(xml, Pid::SMALL);
    writeProperty(xml, Pid::ACCIDENTAL_TYPE);
    Element::writeProperties(xml);
    xml.etag();
}

//---------------------------------------------------------
//   subTypeUserName
//---------------------------------------------------------

QString Accidental::subtypeUserName() const
{
    return Sym::id2userName(symbol());
}

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

SymId Accidental::symbol() const
{
    return accList[int(accidentalType())].sym;
}

//---------------------------------------------------------
//   subtype2value
//    returns the resulting pitch offset
//---------------------------------------------------------

AccidentalVal Accidental::subtype2value(AccidentalType st)
{
    return accList[int(st)].offset;
}

//---------------------------------------------------------
//   subtype2name
//---------------------------------------------------------

const char* Accidental::subtype2name(AccidentalType st)
{
    return Sym::id2name(accList[int(st)].sym);
}

//---------------------------------------------------------
//   subtype2symbol
//---------------------------------------------------------

SymId Accidental::subtype2symbol(AccidentalType st)
{
    return accList[int(st)].sym;
}

//---------------------------------------------------------
//   name2subtype
//---------------------------------------------------------

AccidentalType Accidental::name2subtype(const QString& tag)
{
    SymId symId = Sym::name2id(tag);
    if (symId == SymId::noSym) {
        // qDebug("no symbol found");
    } else {
        int i = 0;
        for (const Acc& acc : accList) {
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

void Accidental::setSubtype(const QString& tag)
{
    setAccidentalType(name2subtype(tag));
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Accidental::layout()
{
    el.clear();

    // TODO: remove Accidental in layout()
    // don't show accidentals for tab or slash notation
    if (onTabStaff() || (note() && note()->fixed())) {
        setbbox(QRectF());
        return;
    }

    qreal m = parent() ? parent()->mag() : 1.0;
    if (_small) {
        m *= score()->styleD(Sid::smallNoteMag);
    }
    setMag(m);

    // if the accidental is standard (doubleflat, flat, natural, sharp or double sharp)
    // and it has either no bracket or parentheses, then we have glyphs straight from smufl.
    if (_bracket == AccidentalBracket::NONE
        || (_bracket == AccidentalBracket::PARENTHESIS
            && (_accidentalType == AccidentalType::FLAT
                || _accidentalType == AccidentalType::NATURAL
                || _accidentalType == AccidentalType::SHARP
                || _accidentalType == AccidentalType::SHARP2
                || _accidentalType == AccidentalType::FLAT2))) {
        layoutSingleGlyphAccidental();
    } else {
        layoutMultiGlyphAccidental();
    }
}

void Accidental::layoutSingleGlyphAccidental()
{
    QRectF r;

    SymId s = symbol();
    if (_bracket == AccidentalBracket::PARENTHESIS) {
        switch (_accidentalType) {
        case AccidentalType::FLAT2:
            s = SymId::accidentalDoubleFlatParens;
            break;
        case AccidentalType::FLAT:
            s = SymId::accidentalFlatParens;
            break;
        case AccidentalType::NATURAL:
            s = SymId::accidentalNaturalParens;
            break;
        case AccidentalType::SHARP:
            s = SymId::accidentalSharpParens;
            break;
        case AccidentalType::SHARP2:
            s = SymId::accidentalDoubleSharpParens;
            break;
        default:
            break;
        }
        if (!score()->scoreFont()->isValid(s)) {
            layoutMultiGlyphAccidental();
            return;
        }
    }

    SymElement e(s, 0.0, 0.0);
    el.append(e);
    r |= symBbox(s);
    setbbox(r);
}

void Accidental::layoutMultiGlyphAccidental()
{
    qreal margin = score()->styleP(Sid::bracketedAccidentalPadding);
    QRectF r;
    qreal x = 0.0;

    // should always be true
    if (_bracket != AccidentalBracket::NONE) {
        SymId id = SymId::noSym;
        switch (_bracket) {
        case AccidentalBracket::PARENTHESIS:
            id = SymId::accidentalParensLeft;
            break;
        case AccidentalBracket::BRACKET:
            id = SymId::accidentalBracketLeft;
            break;
        case AccidentalBracket::BRACE:
            id = SymId::accidentalCombiningOpenCurlyBrace;
            break;
        case AccidentalBracket::NONE: // can't happen
            break;
        }
        SymElement se(id, 0.0, _bracket == AccidentalBracket::BRACE ? spatium() * 0.4 : 0.0);
        el.append(se);
        r |= symBbox(id);
        x += symAdvance(id) + margin;
    }

    SymId s = symbol();
    SymElement e(s, x, 0.0);
    el.append(e);
    r |= symBbox(s).translated(x, 0.0);

    // should always be true
    if (_bracket != AccidentalBracket::NONE) {
        x += symAdvance(s) + margin;
        SymId id = SymId::noSym;
        switch (_bracket) {
        case AccidentalBracket::PARENTHESIS:
            id = SymId::accidentalParensRight;
            break;
        case AccidentalBracket::BRACKET:
            id = SymId::accidentalBracketRight;
            break;
        case AccidentalBracket::BRACE:
            id = SymId::accidentalCombiningCloseCurlyBrace;
            break;
        case AccidentalBracket::NONE: // can't happen
            break;
        }
        SymElement se(id, x, _bracket == AccidentalBracket::BRACE ? spatium() * 0.4 : 0.0);
        el.append(se);
        r |= symBbox(id).translated(x, 0.0);
    }
    setbbox(r);
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
        qFatal("value2subtype: illegal accidental val %d", int(v));
    }
    return AccidentalType::NONE;
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Accidental::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    // don't show accidentals for tab or slash notation
    if (onTabStaff() || (note() && note()->fixed())) {
        return;
    }

    painter->setPen(curColor());
    for (const SymElement& e : el) {
        score()->scoreFont()->draw(e.sym, painter, magS(), QPointF(e.x, e.y));
    }
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Accidental::acceptDrop(EditData& data) const
{
    Element* e = data.dropElement;
    return e->isIcon()
           && (toIcon(e)->iconType() == IconType::BRACKETS || toIcon(e)->iconType() == IconType::PARENTHESES
               || toIcon(e)->iconType() == IconType::BRACES);
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Accidental::drop(EditData& data)
{
    Element* e = data.dropElement;
    switch (e->type()) {
    case ElementType::ICON:
        switch (toIcon(e)->iconType()) {
        case IconType::BRACKETS:
            undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::BRACKET), PropertyFlags::NOSTYLE);
            break;
        case IconType::PARENTHESES:
            undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::PARENTHESIS), PropertyFlags::NOSTYLE);
            break;
        case IconType::BRACES:
            undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::BRACE), PropertyFlags::NOSTYLE);
            break;
        default:
            qDebug("unknown icon type");
            break;
        }
        break;
    default:
        break;
    }
    delete e;
    return 0;
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

QVariant Accidental::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ACCIDENTAL_TYPE:    return int(_accidentalType);
    case Pid::SMALL:              return _small;
    case Pid::ACCIDENTAL_BRACKET: return int(bracket());
    case Pid::ROLE:               return int(role());
    default:
        return Element::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Accidental::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ACCIDENTAL_TYPE:    return int(AccidentalType::NONE);
    case Pid::SMALL:              return false;
    case Pid::ACCIDENTAL_BRACKET: return int(AccidentalBracket::NONE);
    case Pid::ROLE:               return int(AccidentalRole::AUTO);
    default:
        return Element::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Accidental::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::ACCIDENTAL_TYPE:
        setAccidentalType(AccidentalType(v.toInt()));
        break;
    case Pid::SMALL:
        _small = v.toBool();
        break;
    case Pid::ACCIDENTAL_BRACKET:
        _bracket = AccidentalBracket(v.toInt());
        break;
    case Pid::ROLE:
        _role = v.value<AccidentalRole>();
        break;
    default:
        return Element::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Accidental::propertyId(const QStringRef& xmlName) const
{
    if (xmlName == propertyName(Pid::ACCIDENTAL_TYPE)) {
        return Pid::ACCIDENTAL_TYPE;
    }
    return Element::propertyId(xmlName);
}

//---------------------------------------------------------
//   propertyUserValue
//---------------------------------------------------------

QString Accidental::propertyUserValue(Pid pid) const
{
    switch (pid) {
    case Pid::ACCIDENTAL_TYPE:
        return subtypeUserName();
    default:
        return Element::propertyUserValue(pid);
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Accidental::accessibleInfo() const
{
    return QString("%1: %2").arg(Element::accessibleInfo(), Accidental::subtypeUserName());
}
}
