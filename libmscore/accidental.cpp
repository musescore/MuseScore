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
      AccidentalVal offset;   // semitone offset
      int centOffset;
      SymId sym;
      Acc(AccidentalVal o, int o2, SymId s) : offset(o), centOffset(o2), sym(s) {}
      };

// NOTE: keep this in sync with with AccidentalType enum, watch out for isMicrotonal()
static Acc accList[] = {
      Acc(AccidentalVal::NATURAL, 0,    SymId::noSym),                // NONE
      Acc(AccidentalVal::FLAT,    0,    SymId::accidentalFlat),       // FLAT
      Acc(AccidentalVal::NATURAL, 0,    SymId::accidentalNatural),    // NATURAL
      Acc(AccidentalVal::SHARP,   0,    SymId::accidentalSharp),      // SHARP
      Acc(AccidentalVal::SHARP2,  0,    SymId::accidentalDoubleSharp),// SHARP2
      Acc(AccidentalVal::FLAT2,   0,    SymId::accidentalDoubleFlat), // FLAT2
      //Acc(AccidentalVal::SHARP3,  0,    SymId::accidentalTripleSharp),// SHARP3
      //Acc(AccidentalVal::FLAT3,   0,    SymId::accidentalTripleFlat), // FLAT3
      Acc(AccidentalVal::FLAT,    0,    SymId::accidentalNaturalFlat),  // NATURAL_FLAT
      Acc(AccidentalVal::SHARP,   0,    SymId::accidentalNaturalSharp), // NATURAL_SHARP
      Acc(AccidentalVal::SHARP2,  0,    SymId::accidentalSharpSharp),   // SHARP_SHARP

      // Gould arrow quartertone
      Acc(AccidentalVal::NATURAL, -50,  SymId::accidentalQuarterToneFlatArrowUp),        // FLAT_ARROW_UP
      Acc(AccidentalVal::NATURAL, -150, SymId::accidentalThreeQuarterTonesFlatArrowDown),// FLAT_ARROW_DOWN
      Acc(AccidentalVal::NATURAL, 50,   SymId::accidentalQuarterToneSharpNaturalArrowUp),// NATURAL_ARROW_UP
      Acc(AccidentalVal::NATURAL, -50,  SymId::accidentalQuarterToneFlatNaturalArrowDown), // NATURAL_ARROW_DOWN
      Acc(AccidentalVal::NATURAL, 150,  SymId::accidentalThreeQuarterTonesSharpArrowUp), // SHARP_ARROW_UP
      Acc(AccidentalVal::NATURAL, 50,   SymId::accidentalQuarterToneSharpArrowDown),     // SHARP_ARROW_DOWN
      Acc(AccidentalVal::NATURAL, 250,  SymId::accidentalFiveQuarterTonesSharpArrowUp),    // SHARP2_ARROW_UP
      Acc(AccidentalVal::NATURAL, 150,  SymId::accidentalThreeQuarterTonesSharpArrowDown), // SHARP2_ARROW_DOWN
      Acc(AccidentalVal::NATURAL, -250, SymId::accidentalThreeQuarterTonesFlatArrowUp),    // FLAT2_ARROW_UP
      Acc(AccidentalVal::NATURAL, -150, SymId::accidentalFiveQuarterTonesFlatArrowDown),   // FLAT2_ARROW_DOWN

      // Stein-Zimmermann
      Acc(AccidentalVal::NATURAL, -50,  SymId::accidentalQuarterToneFlatStein), // MIRRORED_FLAT
      Acc(AccidentalVal::NATURAL, -150, SymId::accidentalThreeQuarterTonesFlatZimmermann), // MIRRORED_FLAT2
      Acc(AccidentalVal::NATURAL, 50,   SymId::accidentalQuarterToneSharpStein),       // SHARP_SLASH
      Acc(AccidentalVal::NATURAL, 150,  SymId::accidentalThreeQuarterTonesSharpStein), // SHARP_SLASH4

      //Arel-Ezgi-Uzdilek (AEU)
      Acc(AccidentalVal::NATURAL, 0,    SymId::accidentalBuyukMucennebFlat),  // FLAT_SLASH2
      Acc(AccidentalVal::NATURAL, 0,    SymId::accidentalBakiyeFlat),         // FLAT_SLASH
      Acc(AccidentalVal::NATURAL, 0,    SymId::accidentalKucukMucennebSharp), // SHARP_SLASH3
      Acc(AccidentalVal::NATURAL, 0,    SymId::accidentalBuyukMucennebSharp), // SHARP_SLASH2

      // Extended Helmholtz-Ellis accidentals (just intonation)
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleFlatOneArrowDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalFlatOneArrowDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalNaturalOneArrowDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalSharpOneArrowDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleSharpOneArrowDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleFlatOneArrowUp),

      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalFlatOneArrowUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalNaturalOneArrowUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalSharpOneArrowUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleSharpOneArrowUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleFlatTwoArrowsDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalFlatTwoArrowsDown),

      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalNaturalTwoArrowsDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalSharpTwoArrowsDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleSharpTwoArrowsDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleFlatTwoArrowsUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalFlatTwoArrowsUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalNaturalTwoArrowsUp),

      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalSharpTwoArrowsUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleSharpTwoArrowsUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleFlatThreeArrowsDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalFlatThreeArrowsDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalNaturalThreeArrowsDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalSharpThreeArrowsDown),

      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleSharpThreeArrowsDown),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleFlatThreeArrowsUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalFlatThreeArrowsUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalNaturalThreeArrowsUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalSharpThreeArrowsUp),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleSharpThreeArrowsUp),

      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalLowerOneSeptimalComma),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalRaiseOneSeptimalComma),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalLowerTwoSeptimalCommas),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalRaiseTwoSeptimalCommas),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalLowerOneUndecimalQuartertone),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalRaiseOneUndecimalQuartertone),

      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalLowerOneTridecimalQuartertone),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalRaiseOneTridecimalQuartertone),

      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleFlatEqualTempered),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalFlatEqualTempered),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalNaturalEqualTempered),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalSharpEqualTempered),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalDoubleSharpEqualTempered),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalQuarterFlatEqualTempered),
      Acc(AccidentalVal::NATURAL,    0,    SymId::accidentalQuarterSharpEqualTempered),

      // Persian
      Acc(AccidentalVal::NATURAL, 50,   SymId::accidentalSori),                          // SORI
      Acc(AccidentalVal::NATURAL, -50,  SymId::accidentalKoron),                         // KORON
      };

//---------------------------------------------------------
//   sym2accidentalVal
//---------------------------------------------------------

AccidentalVal sym2accidentalVal(SymId id)
      {
      for (const Acc& a : accList) {
            if (a.sym == id)
                  return a.offset;
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
                  if (i == 0 || i == 1 || i == 2)
                        _bracket = AccidentalBracket(i);
                  }
            else if (tag == "subtype")
                  setSubtype(e.readElementText());
            else if (tag == "role") {
                  AccidentalRole r = AccidentalRole(e.readInt());
                  if (r == AccidentalRole::AUTO || r == AccidentalRole::USER)
                        _role = r;
                  }
            else if (tag == "small")
                  _small = e.readInt();
            else if (Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Accidental::write(XmlWriter& xml) const
      {
      xml.stag(name());
      writeProperty(xml, Pid::ACCIDENTAL_BRACKET);
      writeProperty(xml, Pid::ROLE);
      writeProperty(xml, Pid::SMALL);
      xml.tag("subtype", subtype2name(accidentalType()));
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
//   name2subtype
//---------------------------------------------------------

AccidentalType Accidental::name2subtype(const QString& tag)
      {
      SymId symId = Sym::name2id(tag);
      if (symId == SymId::noSym)
            ; // qDebug("no symbol found");
      else {
            int i = 0;
            for (const Acc& acc : accList) {
                  if (acc.sym == symId)
                        return AccidentalType(i);
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

      QRectF r;
      // TODO: remove Accidental in layout()
      // don't show accidentals for tab or slash notation
      if ((staff() && staff()->isTabStaff(tick())) || (note() && note()->fixed())) {
            setbbox(r);
            return;
            }

      qreal m = parent() ? parent()->mag() : 1.0;
      if (_small)
            m *= score()->styleD(Sid::smallNoteMag);
      setMag(m);

      m = magS();

      if (_bracket != AccidentalBracket::NONE) {
            SymId id = _bracket == AccidentalBracket::PARENTHESIS ? SymId::accidentalParensLeft : SymId::accidentalBracketLeft;
            SymElement e(id, 0.0);
            el.append(e);
            r |= symBbox(id);
            }

      SymId s = symbol();
      qreal x = r.x()+r.width();
      SymElement e(s, x);
      el.append(e);
      r |= symBbox(s).translated(x, 0.0);

      if (_bracket != AccidentalBracket::NONE) {
            SymId id = _bracket == AccidentalBracket::PARENTHESIS ? SymId::accidentalParensRight : SymId::accidentalBracketRight;
            x = r.x()+r.width();
            SymElement e(id, x);
            el.append(e);
            r |= symBbox(id).translated(x, 0.0);
            }
      setbbox(r);
      }

//---------------------------------------------------------
//   value2subtype
//---------------------------------------------------------

AccidentalType Accidental::value2subtype(AccidentalVal v)
      {
      switch(v) {
            case AccidentalVal::NATURAL: return AccidentalType::NONE;
            case AccidentalVal::SHARP:   return AccidentalType::SHARP;
            case AccidentalVal::SHARP2:  return AccidentalType::SHARP2;
            case AccidentalVal::FLAT:    return AccidentalType::FLAT;
            case AccidentalVal::FLAT2:   return AccidentalType::FLAT2;
            default:
                  qFatal("value2subtype: illegal accidental val %d", int(v));
            }
      return AccidentalType::NONE;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Accidental::draw(QPainter* painter) const
      {
      // don't show accidentals for tab or slash notation
      if ((staff() && staff()->isTabStaff(tick())) || (note() && note()->fixed()))
            return;
      painter->setPen(curColor());
      for (const SymElement& e : el)
            score()->scoreFont()->draw(e.sym, painter, magS(), QPointF(e.x, 0.0));
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Accidental::acceptDrop(EditData& data) const
      {
      Element* e = data.element;
      return e->isIcon() && (toIcon(e)->iconType() == IconType::BRACKETS || toIcon(e)->iconType() == IconType::PARENTHESES);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Accidental::drop(EditData& data)
      {
      Element* e = data.element;
      switch(e->type()) {
            case ElementType::ICON :
                  switch(toIcon(e)->iconType()) {
                        case IconType::BRACKETS:
                              undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::BRACKET), PropertyFlags::NOSTYLE);
                              break;
                        case IconType::PARENTHESES:
                              undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::PARENTHESIS), PropertyFlags::NOSTYLE);
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
//   accessibleInfo
//---------------------------------------------------------

QString Accidental::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(Accidental::subtypeUserName());
      }

}

