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
      const char* tag;        // for use in xml file
      const char* name;       // translated name
      AccidentalVal offset;   // semitone offset
      int centOffset;
      SymId sym;
      Acc(const char* t, const char* n, AccidentalVal o, int o2, SymId s)
         : tag(t), name(n), offset(o), centOffset(o2), sym(s) {}
      };

static Acc accList[] = {
      Acc("none",                QT_TRANSLATE_NOOP("accidental", "none"),                AccidentalVal::NATURAL, 0,    SymId::noSym),
      Acc("sharp",               QT_TRANSLATE_NOOP("accidental", "sharp"),               AccidentalVal::SHARP,   0,    SymId::accidentalSharp),
      Acc("flat",                QT_TRANSLATE_NOOP("accidental", "flat"),                AccidentalVal::FLAT,    0,    SymId::accidentalFlat),
      Acc("double sharp",        QT_TRANSLATE_NOOP("accidental", "double sharp"),        AccidentalVal::SHARP2,  0,    SymId::accidentalDoubleSharp),
      Acc("double flat",         QT_TRANSLATE_NOOP("accidental", "double flat"),         AccidentalVal::FLAT2,   0,    SymId::accidentalDoubleFlat),
      Acc("natural",             QT_TRANSLATE_NOOP("accidental", "natural"),             AccidentalVal::NATURAL, 0,    SymId::accidentalNatural),

      Acc("flat-slash",          QT_TRANSLATE_NOOP("accidental", "flat-slash"),          AccidentalVal::NATURAL, -50,  SymId::accidentalBakiyeFlat),
      Acc("flat-slash2",         QT_TRANSLATE_NOOP("accidental", "flat-slash2"),         AccidentalVal::NATURAL, 0,    SymId::accidentalBuyukMucennebFlat),
      Acc("mirrored-flat2",      QT_TRANSLATE_NOOP("accidental", "mirrored-flat2"),      AccidentalVal::NATURAL, -150, SymId::accidentalThreeQuarterTonesFlatZimmermann),
      Acc("mirrored-flat",       QT_TRANSLATE_NOOP("accidental", "mirrored-flat"),       AccidentalVal::NATURAL, -50,  SymId::accidentalQuarterToneFlatStein),
      Acc("mirrored-flat-slash", QT_TRANSLATE_NOOP("accidental", "mirrored-flat-slash"), AccidentalVal::NATURAL, 0,    SymId::noSym), //TODO-smufl
      Acc("flat-flat-slash",     QT_TRANSLATE_NOOP("accidental", "flat-flat-slash"),     AccidentalVal::NATURAL, -150, SymId::noSym), //TODO-smufl

      Acc("sharp-slash",         QT_TRANSLATE_NOOP("accidental", "sharp-slash"),         AccidentalVal::NATURAL, 50,   SymId::accidentalQuarterToneSharpStein),
      Acc("sharp-slash2",        QT_TRANSLATE_NOOP("accidental", "sharp-slash2"),        AccidentalVal::NATURAL, 0,    SymId::accidentalBuyukMucennebSharp),
      Acc("sharp-slash3",        QT_TRANSLATE_NOOP("accidental", "sharp-slash3"),        AccidentalVal::NATURAL, 0,    SymId::accidentalKucukMucennebSharp),
      Acc("sharp-slash4",        QT_TRANSLATE_NOOP("accidental", "sharp-slash4"),        AccidentalVal::NATURAL, 150,  SymId::accidentalThreeQuarterTonesSharpStein),

      Acc("sharp arrow up",      QT_TRANSLATE_NOOP("accidental", "sharp arrow up"),      AccidentalVal::NATURAL, 150,  SymId::accidentalThreeQuarterTonesSharpArrowUp),
      Acc("sharp arrow down",    QT_TRANSLATE_NOOP("accidental", "sharp arrow down"),    AccidentalVal::NATURAL, 50,   SymId::accidentalQuarterToneSharpArrowDown),
      Acc("sharp arrow both",    QT_TRANSLATE_NOOP("accidental", "sharp arrow both"),    AccidentalVal::NATURAL, 0,    SymId::noSym), //TODO-smufl
      Acc("flat arrow up",       QT_TRANSLATE_NOOP("accidental", "flat arrow up"),       AccidentalVal::NATURAL, -50,  SymId::accidentalQuarterToneFlatArrowUp),
      Acc("flat arrow down",     QT_TRANSLATE_NOOP("accidental", "flat arrow down"),     AccidentalVal::NATURAL, -150, SymId::accidentalThreeQuarterTonesFlatArrowDown),
      Acc("flat arrow both",     QT_TRANSLATE_NOOP("accidental", "flat arrow both"),     AccidentalVal::NATURAL, 0,    SymId::noSym), //TODO-smufl
      Acc("natural arrow up",    QT_TRANSLATE_NOOP("accidental", "natural arrow up"),    AccidentalVal::NATURAL, 50,   SymId::accidentalQuarterToneSharpNaturalArrowUp),
      Acc("natural arrow down",  QT_TRANSLATE_NOOP("accidental", "natural arrow down"),  AccidentalVal::NATURAL, -50,  SymId::accidentalQuarterToneFlatNaturalArrowDown),
      Acc("natural arrow both",  QT_TRANSLATE_NOOP("accidental", "natural arrow both"),  AccidentalVal::NATURAL, 0,    SymId::noSym), //TODO-smufl

      Acc("sori",                QT_TRANSLATE_NOOP("accidental", "sori"),                AccidentalVal::NATURAL, 50,   SymId::accidentalSori),
      Acc("koron",               QT_TRANSLATE_NOOP("accidental", "koron"),               AccidentalVal::NATURAL, -50,  SymId::accidentalKoron)
      };

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

Accidental::Accidental(Score* s)
   : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      _hasBracket     = false;
      _role           = Role::AUTO;
      _small          = false;
      _accidentalType = Type::NONE;
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
            else if (tag == "subtype") {
                  QString text(e.readElementText());
                  bool isInt;
                  int i = text.toInt(&isInt);
                  if (isInt) {
                        _hasBracket = i & 0x8000;
                        i &= ~0x8000;
                        Type at;
                        switch(i) {
                               case 0:
                                     at = Type::NONE;
                                     break;
                               case 1:
                               case 11:
                                     at = Type::SHARP;
                                     break;
                               case 2:
                               case 12:
                                     at = Type::FLAT;
                                     break;
                               case 3:
                               case 13:
                                     at = Type::SHARP2;
                                     break;
                               case 4:
                               case 14:
                                     at = Type::FLAT2;
                                     break;
                               case 5:
                               case 15:
                                     at = Type::NATURAL;
                                     break;
                               case 6:
                                     at = Type::SHARP;
                                     _hasBracket = true;
                                     break;
                               case 7:
                                     at = Type::FLAT;
                                     _hasBracket = true;
                                     break;
                               case 8:
                                     at = Type::SHARP2;
                                     _hasBracket = true;
                                     break;
                               case 9:
                                     at = Type::FLAT2;
                                     _hasBracket = true;
                                     break;
                               case 10:
                                     at = Type::NATURAL;
                                     _hasBracket = true;
                                     break;
                               case 16:
                                     at = Type::FLAT_SLASH;
                                     break;
                               case 17:
                                     at = Type::FLAT_SLASH2;
                                     break;
                               case 18:
                                     at = Type::MIRRORED_FLAT2;
                                     break;
                               case 19:
                                     at = Type::MIRRORED_FLAT;
                                     break;
                               case 20:
                                     at = Type::MIRRORED_FLAT_SLASH;
                                     break;
                               case 21:
                                     at = Type::FLAT_FLAT_SLASH;
                                     break;
                               case 22:
                                     at = Type::SHARP_SLASH;
                                     break;
                               case 23:
                                     at = Type::SHARP_SLASH2;
                                     break;
                               case 24:
                                     at = Type::SHARP_SLASH3;
                                     break;
                               case 25:
                                     at = Type::SHARP_SLASH4;
                                     break;
                               case 26:
                                     at = Type::SHARP_ARROW_UP;
                                     break;
                               case 27:
                                     at = Type::SHARP_ARROW_DOWN;
                                     break;
                               case 28:
                                     at = Type::SHARP_ARROW_BOTH;
                                     break;
                               case 29:
                                     at = Type::FLAT_ARROW_UP;
                                     break;
                               case 30:
                                     at = Type::FLAT_ARROW_DOWN;
                                     break;
                               case 31:
                                     at = Type::FLAT_ARROW_BOTH;
                                     break;
                               case 32:
                                     at = Type::NATURAL_ARROW_UP;
                                     break;
                               case 33:
                                     at = Type::NATURAL_ARROW_DOWN;
                                     break;
                               case 34:
                                     at = Type::NATURAL_ARROW_BOTH;
                                     break;
                               default:
                                     at = Type::NONE;
                                     break;
                               }
                        setAccidentalType(Type(at));
                        }
                  else
                        setSubtype(text);
                  }
            else if (tag == "role") {
                  int i = e.readInt();
                  if (i == int(Role::AUTO) || i == int(Role::USER))
                        _role = Role(i);
                  }
            else if (tag == "small")
                  _small = e.readInt();
            else if (tag == "offset") {
                  if (score()->mscVersion() > 114)
                        Element::readProperties(e);
                  else
                        e.skipCurrentElement(); // ignore manual layout in older scores
                  }
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
      if (_hasBracket)
            xml.tag("bracket", _hasBracket);
      if (_role != Role::AUTO)
            xml.tag("role", int(_role));
      if (_small)
            xml.tag("small", _small);
      xml.tag("subtype", accList[int(_accidentalType)].tag);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   subTypeUserName
//---------------------------------------------------------

const char* Accidental::subtypeUserName() const
      {
      return accList[int(accidentalType())].name;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Accidental::setSubtype(const QString& tag)
      {
      int n = sizeof(accList)/sizeof(*accList);
      for (int i = 0; i < n; ++i) {
            if (accList[i].tag == tag) {
                  setAccidentalType(Type(i));
                  return;
                  }
            }
      setAccidentalType(Type::NONE);
      }

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

SymId Accidental::symbol() const
      {
      return accList[int(accidentalType())].sym;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Accidental::layout()
      {
      el.clear();

      QRectF r;
      if (staff() && staff()->isTabStaff()) {      //in TAB, accidentals are not shown
            setbbox(QRectF());
            return;
            }

      qreal m = parent() ? parent()->mag() : 1.0;
      if (_small)
            m *= score()->styleD(StyleIdx::smallNoteMag);
      setMag(m);

      m = magS();
      if (_hasBracket) {
            SymElement e(SymId::noteheadParenthesisLeft, 0.0);
            el.append(e);
            r |= symBbox(SymId::noteheadParenthesisLeft);
            }

      SymId s = symbol();
      qreal x = r.x()+r.width();
      SymElement e(s, x);
      el.append(e);
      r |= symBbox(s).translated(x, 0.0);

      if (_hasBracket) {
            x = r.x()+r.width();
            SymElement e(SymId::noteheadParenthesisRight, x);
            el.append(e);
            r |= symBbox(SymId::noteheadParenthesisRight).translated(x, 0.0);
            }
      setbbox(r);
      }

//---------------------------------------------------------
//   subtype2value
//    returns the resulting pitch offset
//---------------------------------------------------------

AccidentalVal Accidental::subtype2value(Type st)
      {
      return accList[int(st)].offset;
      }

//---------------------------------------------------------
//   subtype2name
//---------------------------------------------------------

const char* Accidental::subtype2name(Type st)
      {
      return accList[int(st)].tag;
      }

//---------------------------------------------------------
//   value2subtype
//---------------------------------------------------------

Accidental::Type Accidental::value2subtype(AccidentalVal v)
      {
      switch(v) {
            case AccidentalVal::NATURAL: return Type::NONE;
            case AccidentalVal::SHARP:   return Type::SHARP;
            case AccidentalVal::SHARP2:  return Type::SHARP2;
            case AccidentalVal::FLAT:    return Type::FLAT;
            case AccidentalVal::FLAT2:   return Type::FLAT2;
            default:
                  qFatal("value2subtype: illegal accidental val %hhd", v);
            }
      return Type::NONE;
      }

//---------------------------------------------------------
//   name2subtype
//---------------------------------------------------------

Accidental::Type Accidental::name2subtype(const QString& tag)
      {
      int n = sizeof(accList)/sizeof(*accList);
      for (int i = 0; i < n; ++i) {
            if (accList[i].tag == tag)
                  return Type(i);
            }
      return Type::NONE;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Accidental::draw(QPainter* painter) const
      {
      if (staff() && staff()->isTabStaff())        //in TAB, accidentals are not shown
            return;
      painter->setPen(curColor());
      foreach(const SymElement& e, el)
            score()->scoreFont()->draw(e.sym, painter, magS(), QPointF(e.x, 0.0));
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Accidental::acceptDrop(const DropData& data) const
      {
      Element* e = data.element;
      return e->type() == Element::Type::ICON && static_cast<Icon*>(e)->iconType() == IconType::BRACKETS;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Accidental::drop(const DropData& data)
      {
      Element* e = data.element;
      switch(e->type()) {
            case Element::Type::ICON :
                  if (static_cast<Icon*>(e)->iconType() == IconType::BRACKETS && !_hasBracket)
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
      score()->undoChangeProperty(this, P_ID::ACCIDENTAL_BRACKET, val);
      }

//---------------------------------------------------------
//   undoSetSmall
//---------------------------------------------------------

void Accidental::undoSetSmall(bool val)
      {
      score()->undoChangeProperty(this, P_ID::SMALL, val);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Accidental::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::SMALL:              return _small;
            case P_ID::ACCIDENTAL_BRACKET: return _hasBracket;
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Accidental::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_ID::SMALL:
                  _small = v.toBool();
                  break;
            case P_ID::ACCIDENTAL_BRACKET:
                  _hasBracket = v.toBool();
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      layout();
      score()->setLayoutAll(true);  // spacing changes
      return true;
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Accidental::accessibleInfo()
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(qApp->translate("accidental", Accidental::subtype2name(accidentalType())));
      }

}

