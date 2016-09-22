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

static Acc accList[] = {
      Acc(AccidentalVal::NATURAL, 0,    SymId::noSym),
      Acc(AccidentalVal::SHARP,   0,    SymId::accidentalSharp),
      Acc(AccidentalVal::FLAT,    0,    SymId::accidentalFlat),
      Acc(AccidentalVal::SHARP2,  0,    SymId::accidentalDoubleSharp),
      Acc(AccidentalVal::FLAT2,   0,    SymId::accidentalDoubleFlat),
      Acc(AccidentalVal::NATURAL, 0,    SymId::accidentalNatural),

      Acc(AccidentalVal::NATURAL, -50,  SymId::accidentalBakiyeFlat),
      Acc(AccidentalVal::NATURAL, 0,    SymId::accidentalBuyukMucennebFlat),
      Acc(AccidentalVal::NATURAL, -150, SymId::accidentalThreeQuarterTonesFlatZimmermann),
      Acc(AccidentalVal::NATURAL, -50,  SymId::accidentalQuarterToneFlatStein),
      Acc(AccidentalVal::NATURAL, 0,    SymId::noSym), //TODO-smufl
      Acc(AccidentalVal::NATURAL, -150, SymId::noSym), //TODO-smufl

      Acc(AccidentalVal::NATURAL, 50,   SymId::accidentalQuarterToneSharpStein),
      Acc(AccidentalVal::NATURAL, 0,    SymId::accidentalBuyukMucennebSharp),
      Acc(AccidentalVal::NATURAL, 0,    SymId::accidentalKucukMucennebSharp),
      Acc(AccidentalVal::NATURAL, 150,  SymId::accidentalThreeQuarterTonesSharpStein),

      Acc(AccidentalVal::NATURAL, 150,  SymId::accidentalThreeQuarterTonesSharpArrowUp),
      Acc(AccidentalVal::NATURAL, 50,   SymId::accidentalQuarterToneSharpArrowDown),
      Acc(AccidentalVal::NATURAL, 0,    SymId::noSym), //TODO-smufl
      Acc(AccidentalVal::NATURAL, -50,  SymId::accidentalQuarterToneFlatArrowUp),
      Acc(AccidentalVal::NATURAL, -150, SymId::accidentalThreeQuarterTonesFlatArrowDown),
      Acc(AccidentalVal::NATURAL, 0,    SymId::noSym), //TODO-smufl
      Acc(AccidentalVal::NATURAL, 50,   SymId::accidentalQuarterToneSharpNaturalArrowUp),
      Acc(AccidentalVal::NATURAL, -50,  SymId::accidentalQuarterToneFlatNaturalArrowDown),
      Acc(AccidentalVal::NATURAL, 0,    SymId::noSym), //TODO-smufl

      Acc(AccidentalVal::NATURAL, 50,   SymId::accidentalSori),
      Acc(AccidentalVal::NATURAL, -50,  SymId::accidentalKoron),

      Acc(AccidentalVal::SHARP,   0,    SymId::accidentalNaturalSharp),
      Acc(AccidentalVal::FLAT,    0,    SymId::accidentalNaturalFlat),
      };

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

Accidental::Accidental(Score* s)
   : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      _hasBracket     = false;
      _role           = AccidentalRole::AUTO;
      _small          = false;
      _accidentalType = AccidentalType::NONE;
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
                  if (i == 0 || i == 1)
                        _hasBracket = i;
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

void Accidental::write(Xml& xml) const
      {
      xml.stag(name());
      writeProperty(xml, P_ID::ACCIDENTAL_BRACKET);
      writeProperty(xml, P_ID::ROLE);
      writeProperty(xml, P_ID::SMALL);
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
            qDebug("no symbol found");
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
      // don't show accidentals for tab or slash notation
      if ((staff() && staff()->isTabStaff()) || (note() && note()->fixed())) {
            setbbox(r);
            return;
            }

      qreal m = parent() ? parent()->mag() : 1.0;
      if (_small)
            m *= score()->styleD(StyleIdx::smallNoteMag);
      setMag(m);

      m = magS();
      if (_hasBracket) {
            SymElement e(SymId::accidentalParensLeft, 0.0);
            el.append(e);
            r |= symBbox(SymId::accidentalParensLeft);
            }

      SymId s = symbol();
      qreal x = r.x()+r.width();
      SymElement e(s, x);
      el.append(e);
      r |= symBbox(s).translated(x, 0.0);

      if (_hasBracket) {
            x = r.x()+r.width();
            SymElement e(SymId::accidentalParensRight, x);
            el.append(e);
            r |= symBbox(SymId::accidentalParensRight).translated(x, 0.0);
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
      if ((staff() && staff()->isTabStaff()) || (note() && note()->fixed()))
            return;
      painter->setPen(curColor());
      for (const SymElement& e : el)
            score()->scoreFont()->draw(e.sym, painter, magS(), QPointF(e.x, 0.0));
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Accidental::acceptDrop(const DropData& data) const
      {
      Element* e = data.element;
      return e->isIcon() && toIcon(e)->iconType() == IconType::BRACKETS;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Accidental::drop(const DropData& data)
      {
      Element* e = data.element;
      switch(e->type()) {
            case Element::Type::ICON :
                  if (toIcon(e)->iconType() == IconType::BRACKETS && !_hasBracket)
                        undoSetHasBracket(true);
                  break;

            default:
                  break;
            }
      delete e;
      return 0;
      }

//---------------------------------------------------------
//   undoSetHasBracket
//---------------------------------------------------------

void Accidental::undoSetHasBracket(bool val)
      {
      undoChangeProperty(P_ID::ACCIDENTAL_BRACKET, val);
      }

//---------------------------------------------------------
//   undoSetSmall
//---------------------------------------------------------

void Accidental::undoSetSmall(bool val)
      {
      undoChangeProperty(P_ID::SMALL, val);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Accidental::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::SMALL:              return _small;
            case P_ID::ACCIDENTAL_BRACKET: return _hasBracket;
            case P_ID::ROLE:               return int(role());
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Accidental::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::SMALL:              return false;
            case P_ID::ACCIDENTAL_BRACKET: return false;
            case P_ID::ROLE:               return int(AccidentalRole::AUTO);
            default:
                  return Element::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Accidental::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::SMALL:
                  _small = v.toBool();
                  break;
            case P_ID::ACCIDENTAL_BRACKET:
                  _hasBracket = v.toBool();
                  break;
            case P_ID::ROLE:
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

