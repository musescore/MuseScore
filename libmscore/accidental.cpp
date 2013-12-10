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
#include "staff.h"
#include "undo.h"

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

#define TRA(x) QT_TRANSLATE_NOOP("accidental", x)
static Acc accList[] = {
      Acc("none",                TRA("none"),                NATURAL, 0,    SymId::noSym),
      Acc("sharp",               TRA("sharp"),               SHARP,   0,    SymId::accidentalSharp),
      Acc("flat",                TRA("flat"),                FLAT,    0,    SymId::accidentalFlat),
      Acc("double sharp",        TRA("double sharp"),        SHARP2,  0,    SymId::accidentalDoubleSharp),
      Acc("double flat",         TRA("double flat"),         FLAT2,   0,    SymId::accidentalDoubleFlat),
      Acc("natural",             TRA("natural"),             NATURAL, 0,    SymId::accidentalNatural),

      Acc("flat-slash",          TRA("flat-slash"),          NATURAL, -50,  SymId::accidentalBakiyeFlat),
      Acc("flat-slash2",         TRA("flat-slash2"),         NATURAL, 0,    SymId::accidentalBuyukMucennebFlat),
      Acc("mirrored-flat2",      TRA("mirrored-flat2"),      NATURAL, -150, SymId::accidentalThreeQuartersFlat2),
      Acc("mirrored-flat",       TRA("mirrored-flat"),       NATURAL, -50,  SymId::accidentalQuarterFlat4),
      Acc("mirrored-flat-slash", TRA("mirrored-flat-slash"), NATURAL, 0,    SymId::noSym), //TODO-smufl
      Acc("flat-flat-slash",     TRA("flat-flat-slash"),     NATURAL, -150, SymId::noSym), //TODO-smufl

      Acc("sharp-slash",         TRA("sharp-slash"),         NATURAL, 50,   SymId::accidentalQuarterSharp4),
      Acc("sharp-slash2",        TRA("sharp-slash2"),        NATURAL, 0,    SymId::accidentalBuyukMucennebSharp),
      Acc("sharp-slash3",        TRA("sharp-slash3"),        NATURAL, 0,    SymId::accidentalKucukMucennebSharp),
      Acc("sharp-slash4",        TRA("sharp-slash4"),        NATURAL, 150,  SymId::accidentalThreeQuartersSharp2),

      Acc("sharp arrow up",      TRA("sharp arrow up"),      NATURAL, 150,  SymId::accidentalThreeQuartersSharpArrowUp),
      Acc("sharp arrow down",    TRA("sharp arrow down"),    NATURAL, 50,   SymId::accidentalQuarterSharpArrowDown),
      Acc("sharp arrow both",    TRA("sharp arrow both"),    NATURAL, 0,    SymId::noSym), //TODO-smufl
      Acc("flat arrow up",       TRA("flat arrow up"),       NATURAL, -50,  SymId::accidentalQuarterFlatArrowUp),
      Acc("flat arrow down",     TRA("flat arrow down"),     NATURAL, -150, SymId::accidentalThreeQuartersFlatArrowDown),
      Acc("flat arrow both",     TRA("flat arrow both"),     NATURAL, 0,    SymId::noSym), //TODO-smufl
      Acc("natural arrow up",    TRA("natural arrow up"),    NATURAL, 50,   SymId::accidentalQuarterSharpNaturalArrowUp),
      Acc("natural arrow down",  TRA("natural arrow down"),  NATURAL, -50,  SymId::accidentalQuarterFlatNaturalArrowDown),
      Acc("natural arrow both",  TRA("natural arrow both"),  NATURAL, 0,    SymId::noSym), //TODO-smufl

      Acc("sori",                TRA("sori"),                NATURAL, 50,   SymId::accidentalSori),
      Acc("koron",               TRA("koron"),               NATURAL, -50,  SymId::accidentalKoron)
      };
#undef TRA

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

Accidental::Accidental(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _hasBracket     = false;
      _role           = ACC_AUTO;
      _small          = false;
      _accidentalType = ACC_NONE;
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
                        switch(i) {
                               case 0:
                                     i = ACC_NONE;
                                     break;
                               case 1:
                               case 11:
                                     i = ACC_SHARP;
                                     break;
                               case 2:
                               case 12:
                                     i = ACC_FLAT;
                                     break;
                               case 3:
                               case 13:
                                     i = ACC_SHARP2;
                                     break;
                               case 4:
                               case 14:
                                     i = ACC_FLAT2;
                                     break;
                               case 5:
                               case 15:
                                     i = ACC_NATURAL;
                                     break;
                               case 6:
                                     i = ACC_SHARP;
                                     _hasBracket = true;
                                     break;
                               case 7:
                                     i = ACC_FLAT;
                                     _hasBracket = true;
                                     break;
                               case 8:
                                     i = ACC_SHARP2;
                                     _hasBracket = true;
                                     break;
                               case 9:
                                     i = ACC_FLAT2;
                                     _hasBracket = true;
                                     break;
                               case 10:
                                     i = ACC_NATURAL;
                                     _hasBracket = true;
                                     break;
                               case 16:
                                     i = ACC_FLAT_SLASH;
                                     break;
                               case 17:
                                     i = ACC_FLAT_SLASH2;
                                     break;
                               case 18:
                                     i = ACC_MIRRORED_FLAT2;
                                     break;
                               case 19:
                                     i = ACC_MIRRORED_FLAT;
                                     break;
                               case 20:
                                     i = ACC_MIRRIRED_FLAT_SLASH;
                                     break;
                               case 21:
                                     i = ACC_FLAT_FLAT_SLASH;
                                     break;
                               case 22:
                                     i = ACC_SHARP_SLASH;
                                     break;
                               case 23:
                                     i = ACC_SHARP_SLASH2;
                                     break;
                               case 24:
                                     i = ACC_SHARP_SLASH3;
                                     break;
                               case 25:
                                     i = ACC_SHARP_SLASH4;
                                     break;
                               case 26:
                                     i = ACC_SHARP_ARROW_UP;
                                     break;
                               case 27:
                                     i = ACC_SHARP_ARROW_DOWN;
                                     break;
                               case 28:
                                     i = ACC_SHARP_ARROW_BOTH;
                                     break;
                               case 29:
                                     i = ACC_FLAT_ARROW_UP;
                                     break;
                               case 30:
                                     i = ACC_FLAT_ARROW_DOWN;
                                     break;
                               case 31:
                                     i = ACC_FLAT_ARROW_BOTH;
                                     break;
                               case 32:
                                     i = ACC_NATURAL_ARROW_UP;
                                     break;
                               case 33:
                                     i = ACC_NATURAL_ARROW_DOWN;
                                     break;
                               case 34:
                                     i = ACC_NATURAL_ARROW_BOTH;
                                     break;
                               default:
                                     i = 0;
                                     break;
                               }
                        setAccidentalType(AccidentalType(i));
                        }
                  else
                        setSubtype(text);
                  }
            else if (tag == "role") {
                  int i = e.readInt();
                  if (i == ACC_AUTO || i == ACC_USER)
                        _role = AccidentalRole(i);
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
      if (_hasBracket)
            xml.tag("bracket", _hasBracket);
      if (_role != ACC_AUTO)
            xml.tag("role", _role);
      if (_small)
            xml.tag("small", _small);
      xml.tag("subtype", accList[_accidentalType].tag);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   subTypeUserName
//---------------------------------------------------------

const char* Accidental::subtypeUserName() const
      {
      return accList[accidentalType()].name;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Accidental::setSubtype(const QString& tag)
      {
      int n = sizeof(accList)/sizeof(*accList);
      for (int i = 0; i < n; ++i) {
            if (accList[i].tag == tag) {
                  setAccidentalType(AccidentalType(i));
                  return;
                  }
            }
      setAccidentalType(ACC_NONE);
      }

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

SymId Accidental::symbol() const
      {
      return accList[accidentalType()].sym;
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
            m *= score()->styleD(ST_smallNoteMag);
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

AccidentalVal Accidental::subtype2value(AccidentalType st)
      {
      return accList[st].offset;
      }

//---------------------------------------------------------
//   subtype2name
//---------------------------------------------------------

const char* Accidental::subtype2name(AccidentalType st)
      {
      return accList[st].tag;
      }

//---------------------------------------------------------
//   value2subtype
//---------------------------------------------------------

Accidental::AccidentalType Accidental::value2subtype(AccidentalVal v)
      {
      switch(v) {
            case NATURAL: return ACC_NONE;
            case SHARP:   return ACC_SHARP;
            case SHARP2:  return ACC_SHARP2;
            case FLAT:    return ACC_FLAT;
            case FLAT2:   return ACC_FLAT2;
            default:
                  qDebug("value2subtype: illegal accidental val %d\n", v);
                  abort();
            }
      return ACC_NONE;
      }

//---------------------------------------------------------
//   name2subtype
//---------------------------------------------------------

Accidental::AccidentalType Accidental::name2subtype(const QString& tag)
      {
      int n = sizeof(accList)/sizeof(*accList);
      for (int i = 0; i < n; ++i) {
            if (accList[i].tag == tag)
                  return AccidentalType(i);
            }
      return ACC_NONE;
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

bool Accidental::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == ACCIDENTAL_BRACKET;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Accidental::drop(const DropData& data)
      {
      Element* e = data.element;
      switch(e->type()) {
            case ACCIDENTAL_BRACKET:
                  if (!_hasBracket)
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
      score()->undoChangeProperty(this, P_ACCIDENTAL_BRACKET, val);
      }

//---------------------------------------------------------
//   undoSetSmall
//---------------------------------------------------------

void Accidental::undoSetSmall(bool val)
      {
      score()->undoChangeProperty(this, P_SMALL, val);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Accidental::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SMALL:              return _small;
            case P_ACCIDENTAL_BRACKET: return _hasBracket;
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
            case P_SMALL:
                  _small = v.toBool();
                  break;
            case P_ACCIDENTAL_BRACKET:
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
//   AccidentalBracket
//---------------------------------------------------------

AccidentalBracket::AccidentalBracket(Score* s)
   : Compound(s)
      {
      Symbol* s1 = new Symbol(score());
      Symbol* s2 = new Symbol(score());
      s1->setSym(SymId::noteheadParenthesisLeft);
      s2->setSym(SymId::noteheadParenthesisRight);
      addElement(s1, -s1->bbox().x(), 0.0);
      addElement(s2, s2->bbox().width() - s2->bbox().x(), 0.0);
      }

}

