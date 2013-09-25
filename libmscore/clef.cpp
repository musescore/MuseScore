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

/**
 \file
 Implementation of classes Clef (partial) and ClefList (complete).
*/

#include "clef.h"
#include "xml.h"
#include "sym.h"
#include "symbol.h"
#include "score.h"
#include "staff.h"
#include "segment.h"
#include "stafftype.h"

namespace Ms {

#define TR(a)  QT_TRANSLATE_NOOP("clefTable", a)

// table must be in sync with enum ClefType
const ClefInfo ClefInfo::clefTable[] = {
// tag    xmlName    line oCh pOff|-lines for sharps---||---lines for flats--|   name
{ "G",    "G",         2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef"),            STANDARD_STAFF_GROUP  },
{ "G8va", "G",         2,  1, 52, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef 8va"),        STANDARD_STAFF_GROUP  },
{ "G15ma","G",         2,  2, 59, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef 15ma"),       STANDARD_STAFF_GROUP  },
{ "G8vb", "G",         2, -1, 38, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef 8vb"),        STANDARD_STAFF_GROUP  },
{ "F",    "F",         4,  0, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef"),              STANDARD_STAFF_GROUP  },
{ "F8vb", "F",         4, -1, 26, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 8vb"),          STANDARD_STAFF_GROUP  },
{ "F15mb","F",         4, -2, 19, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 15mb"),         STANDARD_STAFF_GROUP  },
{ "F3",   "F",         3,  0, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, TR("Baritone clef (F clef)"), STANDARD_STAFF_GROUP  },
{ "F5",   "F",         5,  0, 31, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Subbass clef"),           STANDARD_STAFF_GROUP  },
{ "C1",   "C",         1,  0, 43, { 5, 1, 4, 0, 3,-1, 2, 2,-1, 3, 0, 4, 1, 5 }, TR("Soprano clef"),           STANDARD_STAFF_GROUP  }, // C1
{ "C2",   "C",         2,  0, 41, { 3, 6, 2, 5, 1, 4, 0, 0, 4, 1, 5, 2, 6, 3 }, TR("Mezzo-soprano clef"),     STANDARD_STAFF_GROUP  }, // C2
{ "C3",   "C",         3,  0, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 }, TR("Alto clef"),              STANDARD_STAFF_GROUP  }, // C3
{ "C4",   "C",         4,  0, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, TR("Tenor clef"),             STANDARD_STAFF_GROUP  }, // C4
{ "TAB",  "TAB",       5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Tablature"),              TAB_STAFF_GROUP       },
{ "PERC", "percussion",2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Percussion"),             PERCUSSION_STAFF_GROUP},
{ "C5",   "C",         5,  0, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, TR("Baritone clef (C clef)"), STANDARD_STAFF_GROUP  }, // C5
{ "G1",   "G",         1,  0, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("French violin clef"),     STANDARD_STAFF_GROUP  }, // G4
{ "F8va", "F",         4,  1, 40, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 8va"),          STANDARD_STAFF_GROUP  }, // F_8VA
{ "F15ma","F",         4,  2, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 15ma"),         STANDARD_STAFF_GROUP  }, // F_15MA
{ "PERC2","percussion",2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Percussion"),             PERCUSSION_STAFF_GROUP}, // PERC2 placeholder
{ "TAB2", "TAB",       5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Tablature2"),             TAB_STAFF_GROUP       },
      };
#undef TR

//---------------------------------------------------------
//   tag2type
//---------------------------------------------------------

ClefType ClefInfo::tag2type(const QString& s)
      {
      for (unsigned i = 0; i < sizeof(ClefInfo::clefTable)/sizeof(*ClefInfo::clefTable); ++i) {
            if (clefTable[i]._tag == s)
                  return ClefType(i);
            }
      return ClefType::G;
      }

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

Clef::Clef(Score* s)
  : Element(s)
      {
//      setFlags(ELEMENT_SELECTABLE | ELEMENT_ON_STAFF | ELEMENT_MOVABLE);
      setFlags(ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);

      _showCourtesy               = true;
      _small                      = false;
      _clefTypes._concertClef     = ClefType::INVALID;
      _clefTypes._transposingClef = ClefType::INVALID;
      curClefType                 = ClefType::G;
      curLines                    = -1;
      curLineDist                 = 1.0;
      }

Clef::Clef(const Clef& c)
   : Element(c)
      {
      _showCourtesy     = c._showCourtesy;
      _showPreviousClef = c._showPreviousClef;
      _small            = c._small;
      _clefTypes        = c._clefTypes;
      curClefType       = c.curClefType;
      curLines          = c.curLines;
      curLineDist       = c.curLineDist;
      layout1();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Clef::mag() const
      {
      qreal mag = staff() ? staff()->mag() : 1.0;
      if (_small)
            mag *= score()->style(ST_smallClefMag).toDouble();
      return mag;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Clef::addElement(Element* e, qreal x, qreal y)
      {
      e->layout();
      e->setPos(x, y);
      e->setParent(this);
      e->setSelected(selected());
      elements.push_back(e);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Clef::setSelected(bool f)
      {
      Element::setSelected(f);
      foreach(Element* e, elements)
            e->setSelected(f);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Clef::layout()
      {
      // determine current number of lines and line distance
      int lines      = 5;                       // assume a resonable default
      qreal lineDist = 1.0;

      StaffType* staffType;
      if (staff() && staff()->staffType()) {
            staffType = staff()->staffType();
            if (!staffType->genClef()) {        // if no clef, set empty bbox and do nothing
                  setbbox(QRectF());
                  return;
                  }

            // tablatures:
            if (staffType->group() == TAB_STAFF_GROUP) {
                  // if current clef type not compatible with tablature,
                  // set tab clef according to score style
                  if (ClefInfo::staffGroup(clefType()) != TAB_STAFF_GROUP)
                        setClefType( ClefType(score()->styleI(ST_tabClef)) );
                  }
            // all staff types: init values from staff type
            lines = staffType->lines();
            lineDist = staffType->lineDistance().val();
            }

      // if nothing changed since last layout, do nothing
//      if (curClefType == clefType() && curLines == lines && curLineDist == lineDist)
//            return;
      // if something has changed, cache new values and re-layout
      curClefType = clefType();
      curLines    = lines;
      curLineDist = lineDist;
      layout1();
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void Clef::layout1()
      {
      qreal smag     = mag();
      qreal _spatium = spatium();
      qreal msp      = score()->spatium() * smag;
      qreal yoff     = 0.0;

      qDeleteAll(elements);
      elements.clear();

      Symbol* symbol = new Symbol(score());

      switch (curClefType) {
            case ClefType::G:                              // G clef on 2nd line
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0 * curLineDist;
                  break;
            case ClefType::G1:                             // G clef 8va on 2nd line
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0 * curLineDist;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, 1.0 * msp, -5.0 * msp + yoff * _spatium);
                  }
                  break;
            case ClefType::G2:                             // G clef 15ma on 2nd line
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0 * curLineDist;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefOneSym);
                  addElement(number, .6 * msp, -5.0 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefFiveSym);
                  addElement(number, 1.4 * msp, -5.0 * msp + yoff * _spatium);
                  }
                  break;
            case ClefType::G3:                             // G clef 8va bassa on 2nd line
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0 * curLineDist;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, 1.0 * msp, 4.0 * msp + yoff * _spatium);
                  }
                  break;
            case ClefType::F:                              // F clef on penultimate line
                  symbol->setSym(bassclefSym);
                  yoff = 1.0 * curLineDist;
                  break;
            case ClefType::F8:                             // F clef 8va bassa on penultimate line
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0 * curLineDist;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, .5* msp, 4.5 * msp + yoff * _spatium);
                  }
                  break;
            case ClefType::F15:                            // F clef 15ma bassa on penultimate line
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0 * curLineDist;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefOneSym);
                  addElement(number, .3* msp, 4.5 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefFiveSym);
                  addElement(number, 1.1 * msp, 4.5 * msp + yoff * _spatium);
                  }
                  break;
            case ClefType::F_B:                            // baritone clef
                  symbol->setSym(bassclefSym);
                  yoff = 2.0 * curLineDist;
                  break;
            case ClefType::F_C:                            // subbass clef
                  symbol->setSym(bassclefSym);
                  yoff = 0.0;
                  break;
            case ClefType::C1:                             // C clef in 1st line
                  symbol->setSym(altoclefSym);
                  yoff = 4.0 * curLineDist;
                  break;
            case ClefType::C2:                             // C clef on 2nd line
                  symbol->setSym(altoclefSym);
                  yoff = 3.0 * curLineDist;
                  break;
            case ClefType::C3:                             // C clef in 3rd line
                  symbol->setSym(altoclefSym);
                  yoff = 2.0 * curLineDist;
                  break;
            case ClefType::C4:                             // C clef on 4th line
                  symbol->setSym(altoclefSym);
                  yoff = 1.0 * curLineDist;
                  break;
            case ClefType::C5:                             // C clef on 5th line
                  symbol->setSym(altoclefSym);
                  yoff = 0.0;
                  break;
            case ClefType::TAB:                            // TAB clef
                  symbol->setSym(tabclefSym);
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = curLineDist * (curLines - 1) * .5;
                  break;                              // TAB clef alternate style
            case ClefType::TAB2:
                  symbol->setSym(tabclef2Sym);
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = curLineDist * (curLines - 1) * .5;
                  break;
            case ClefType::PERC:                           // percussion clefs
            case ClefType::PERC2:
                  symbol->setSym(percussionclefSym);
                  yoff = curLineDist * (curLines - 1) * 0.5;
                  break;
            case ClefType::G4:                             // G clef in 1st line
                  symbol->setSym(trebleclefSym);
                  yoff = 4.0 * curLineDist;
                  break;
            case ClefType::F_8VA:                          // F clef 8va on penultimate line
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0 * curLineDist;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, .5 * msp, -1.5 * msp + yoff * _spatium);
                  }
                  break;
            case ClefType::F_15MA:                         // F clef 15ma on penultimate line
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0 * curLineDist;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefOneSym);
                  addElement(number, .3* msp, -1.5 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefFiveSym);
                  addElement(number, 1.1 * msp, -1.5 * msp + yoff * _spatium);
                  }
                  break;
            case ClefType::INVALID:
            case ClefType::MAX:
                  return;
            }

      symbol->setMag(smag);
      symbol->layout();
      addElement(symbol, .0, yoff * _spatium);
      setbbox(QRectF());
      for (auto i = elements.begin(); i != elements.end(); ++i) {
            Element* e = *i;
            e->setColor(curColor());
            addbbox(e->bbox().translated(e->pos()));
            e->setSelected(selected());
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Clef::draw(QPainter* painter) const
      {
      if (staff() && !staff()->staffType()->genClef())
	      return;
      QColor color(curColor());
      foreach(Element* e, elements) {
            e->setColor(color);
            QPointF pt(e->pos());
            painter->translate(pt);
            e->draw(painter);
            painter->translate(-pt);
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Clef::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == CLEF;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Clef::drop(const DropData& data)
      {
      Element* e = data.element;
      Clef* c = 0;
      if (e->type() == CLEF) {
            Clef* clef = static_cast<Clef*>(e);
            ClefType stype  = clef->clefType();
            if (clefType() != stype) {
                  score()->undoChangeClef(staff(), segment(), stype);
                  c = this;
                  }
            }
      delete e;
      return c;
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void Clef::setSmall(bool val)
      {
      if (val != _small) {
            _small = val;
            curClefType = ClefType::INVALID;
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Clef::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setClefType(clefType(e.readElementText()));
            else if (tag == "concertClefType")
                  _clefTypes._concertClef = Clef::clefType(e.readElementText());
            else if (tag == "transposingClefType")
                  _clefTypes._transposingClef = Clef::clefType(e.readElementText());
            else if (tag == "showCourtesyClef")
                  _showCourtesy = e.readInt();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      if (score()->mscVersion() < 113)
            setUserOff(QPointF());
      if (clefType() == ClefType::INVALID)
            setClefType(ClefType::G);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Clef::write(Xml& xml) const
      {
      xml.stag(name());
      if(_clefTypes._concertClef != ClefType::INVALID)
            xml.tag("concertClefType", ClefInfo::tag(_clefTypes._concertClef));
      if(_clefTypes._transposingClef != ClefType::INVALID)
            xml.tag("transposingClefType", ClefInfo::tag(_clefTypes._transposingClef));
      if (!_showCourtesy)
            xml.tag("showCourtesyClef", _showCourtesy);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int Clef::tick() const
      {
      return segment() ? segment()->tick() : 0;
      }

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Clef::setClefType(const QString& s)
      {
      ClefType ct = clefType(s);
      if (ct == ClefType::INVALID) {
            qDebug("Clef::setSubtype: unknown: <%s>\n", qPrintable(s));
            ct = ClefType::G;
            }
      setClefType(ct);
      }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefType Clef::clefType(const QString& s)
      {
      ClefType ct = ClefType::G;
      bool ok;
      int i = s.toInt(&ok);
      if (ok) {
            //
            // convert obsolete old coding
            //
            switch (i) {
                  default:
                  case  0: ct = ClefType::G; break;
                  case  1: ct = ClefType::G1; break;
                  case  2: ct = ClefType::G2; break;
                  case  3: ct = ClefType::G3; break;
                  case  4: ct = ClefType::F; break;
                  case  5: ct = ClefType::F8; break;
                  case  6: ct = ClefType::F15; break;
                  case  7: ct = ClefType::F_B; break;
                  case  8: ct = ClefType::F_C; break;
                  case  9: ct = ClefType::C1; break;
                  case 10: ct = ClefType::C2; break;
                  case 11: ct = ClefType::C3; break;
                  case 12: ct = ClefType::C4; break;
                  case 13: ct = ClefType::TAB; break;
                  case 14: ct = ClefType::PERC; break;
                  case 15: ct = ClefType::C5; break;
                  case 16: ct = ClefType::G4; break;
                  case 17: ct = ClefType::F_8VA; break;
                  case 18: ct = ClefType::F_15MA; break;
                  case 19: ct = ClefType::PERC2; break;
                  case 20: ct = ClefType::TAB2; break;
                  }
            }
      else
            ct = ClefInfo::tag2type(s);
      return ct;
      }

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Clef::setClefType(ClefType i)
      {
      if (score()->concertPitch()) {
            _clefTypes._concertClef = i;
            if (_clefTypes._transposingClef == ClefType::INVALID)
                  _clefTypes._transposingClef = i;

            }
      else {
            _clefTypes._transposingClef = i;
            if (_clefTypes._concertClef == ClefType::INVALID)
                  _clefTypes._concertClef = i;
            }
      }

//---------------------------------------------------------
//   setConcertClef
//---------------------------------------------------------

void Clef::setConcertClef(ClefType val)
      {
      _clefTypes._concertClef = val;
      }

//---------------------------------------------------------
//   setTransposingClef
//---------------------------------------------------------

void Clef::setTransposingClef(ClefType val)
      {
      _clefTypes._transposingClef = val;
      }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefType Clef::clefType() const
      {
      if (score()->concertPitch())
            return _clefTypes._concertClef;
      else
            return _clefTypes._transposingClef;
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Clef::spatiumChanged(qreal oldValue, qreal newValue)
      {
      layout1();
      Element::spatiumChanged(oldValue, newValue);
      }

//---------------------------------------------------------
//   undoSetShowCourtesy
//---------------------------------------------------------

void Clef::undoSetShowCourtesy(bool v)
      {
      score()->undoChangeProperty(this, P_SHOW_COURTESY, v);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Clef::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SHOW_COURTESY: return showCourtesy();
            case P_SMALL:         return small();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Clef::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_SHOW_COURTESY: _showCourtesy = v.toBool(); break;
            case P_SMALL:         setSmall(v.toBool()); break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Clef::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_SHOW_COURTESY: return true;
            case P_SMALL:         return false;
            default:              return Element::propertyDefault(id);
            }
      }

}

