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

#include "ambitus.h"
#include "clef.h"
#include "measure.h"
#include "part.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stafftype.h"
#include "sym.h"
#include "symbol.h"
#include "system.h"
#include "xml.h"

namespace Ms {


// table must be in sync with enum ClefType in clef.h
const ClefInfo ClefInfo::clefTable[] = {
// tag    xmlName    line oCh pOff|-lines for sharps---||---lines for flats--|  symbol                           | name                                   | valid in staff group
{ "G",    "G",         2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef,                    QT_TRANSLATE_NOOP("clefTable", "Treble clef"),                       StaffGroup::STANDARD  },
// { "G15mb","G",         2, -2, 59, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef15mb,                QT_TRANSLATE_NOOP("clefTable", "Treble clef 15ma bassa"),                  StaffGroup::STANDARD  },
{ "G15mb","G",         2, -2, 31, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef15mb,                QT_TRANSLATE_NOOP("clefTable", "Treble clef 15ma bassa"),                  StaffGroup::STANDARD  },
{ "G8vb", "G",         2, -1, 38, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8vb,                 QT_TRANSLATE_NOOP("clefTable", "Treble clef 8va bassa"),                   StaffGroup::STANDARD  },
{ "G8va", "G",         2,  1, 52, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8va,                 QT_TRANSLATE_NOOP("clefTable", "Treble clef 8va alta"),                   StaffGroup::STANDARD  },
{ "G15ma","G",         2,  2, 59, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef15ma,                QT_TRANSLATE_NOOP("clefTable", "Treble clef 15ma alta"),                  StaffGroup::STANDARD  },
{ "G8vbo","G",         2, -1, 38, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8vbOld,              QT_TRANSLATE_NOOP("clefTable", "Double treble clef 8va bassa on 2nd line"),StaffGroup::STANDARD  },
{ "G8vbp","G",         2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8vbParens,           QT_TRANSLATE_NOOP("clefTable", "Treble clef optional 8va bassa"),          StaffGroup::STANDARD  },
{ "G1",   "G",         1,  0, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::gClef,                    QT_TRANSLATE_NOOP("clefTable", "French violin clef"),                StaffGroup::STANDARD  },
{ "C1",   "C",         1,  0, 43, { 5, 1, 4, 0, 3,-1, 2, 2, 6, 3, 7, 4, 8, 5 }, SymId::cClef,                    QT_TRANSLATE_NOOP("clefTable", "Soprano clef"),                      StaffGroup::STANDARD  },
{ "C2",   "C",         2,  0, 41, { 3, 6, 2, 5, 1, 4, 0, 0, 4, 1, 5, 2, 6, 3 }, SymId::cClef,                    QT_TRANSLATE_NOOP("clefTable", "Mezzo-soprano clef"),                StaffGroup::STANDARD  },
{ "C3",   "C",         3,  0, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 }, SymId::cClef,                    QT_TRANSLATE_NOOP("clefTable", "Alto clef"),                         StaffGroup::STANDARD  },
{ "C4",   "C",         4,  0, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, SymId::cClef,                    QT_TRANSLATE_NOOP("clefTable", "Tenor clef"),                        StaffGroup::STANDARD  },
{ "C5",   "C",         5,  0, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, SymId::cClef,                    QT_TRANSLATE_NOOP("clefTable", "Baritone clef (C clef)"),            StaffGroup::STANDARD  },
{ "C_19C", "G",        2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::cClefSquare,              QT_TRANSLATE_NOOP("clefTable", "C clef, H shape (19th century)"),    StaffGroup::STANDARD  },
{ "C1_F18C", "C",      1,  0, 43, { 5, 1, 4, 0, 3,-1, 2, 2, 6, 3, 7, 4, 8, 5 }, SymId::cClefFrench,              QT_TRANSLATE_NOOP("clefTable", "Soprano clef (French, 18th century)"), StaffGroup::STANDARD  },
{ "C3_F18C", "C",      3,  0, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 }, SymId::cClefFrench,              QT_TRANSLATE_NOOP("clefTable", "Alto clef (French, 18th century)"),  StaffGroup::STANDARD  },
{ "C4_F18C", "C",      4,  0, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, SymId::cClefFrench,              QT_TRANSLATE_NOOP("clefTable", "Tenor clef (French, 18th century)"), StaffGroup::STANDARD  },
{ "C1_F20C", "C",      1,  0, 43, { 5, 1, 4, 0, 3,-1, 2, 2, 6, 3, 7, 4, 8, 5 }, SymId::cClefFrench20C,           QT_TRANSLATE_NOOP("clefTable", "Soprano clef (French, 20th century)"),  StaffGroup::STANDARD  },
{ "C3_F20C", "C",      3,  0, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 }, SymId::cClefFrench20C,           QT_TRANSLATE_NOOP("clefTable", "Alto clef (French, 20th century)"),  StaffGroup::STANDARD  },
{ "C4_F20C", "C",      4,  0, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, SymId::cClefFrench20C,           QT_TRANSLATE_NOOP("clefTable", "Tenor clef (French, 20th century)"), StaffGroup::STANDARD  },
{ "F",    "F",         4,  0, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef,                    QT_TRANSLATE_NOOP("clefTable", "Bass clef"),                         StaffGroup::STANDARD  },
{ "F15mb","F",         4, -2, 19, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef15mb,                QT_TRANSLATE_NOOP("clefTable", "Bass clef 15ma bassa"),                    StaffGroup::STANDARD  },
{ "F8vb", "F",         4, -1, 26, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef8vb,                 QT_TRANSLATE_NOOP("clefTable", "Bass clef 8va bassa"),                     StaffGroup::STANDARD  },
{ "F8va", "F",         4,  1, 40, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef8va,                 QT_TRANSLATE_NOOP("clefTable", "Bass clef 8va alta"),                     StaffGroup::STANDARD  },
{ "F15ma","F",         4,  2, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef15ma,                QT_TRANSLATE_NOOP("clefTable", "Bass clef 15ma alta"),                    StaffGroup::STANDARD  },
{ "F3",   "F",         3,  0, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, SymId::fClef,                    QT_TRANSLATE_NOOP("clefTable", "Baritone clef (F clef)"),            StaffGroup::STANDARD  },
{ "F5",   "F",         5,  0, 31, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::fClef,                    QT_TRANSLATE_NOOP("clefTable", "Subbass clef"),                      StaffGroup::STANDARD  },
{ "F_F18C","F",        4,  0, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClefFrench,              QT_TRANSLATE_NOOP("clefTable", "F clef (French, 18th century)"),     StaffGroup::STANDARD  },
{ "F_19C","F",         4,  0, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef19thCentury,         QT_TRANSLATE_NOOP("clefTable", "F clef (19th century)"),             StaffGroup::STANDARD  },
{ "PERC", "percussion",2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::unpitchedPercussionClef1, QT_TRANSLATE_NOOP("clefTable", "Percussion"),                        StaffGroup::PERCUSSION},
{ "PERC2","percussion",2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::unpitchedPercussionClef2, QT_TRANSLATE_NOOP("clefTable", "Percussion 2"),                      StaffGroup::PERCUSSION},
{ "TAB",  "TAB",       5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::sixStringTabClef,         QT_TRANSLATE_NOOP("clefTable", "Tablature"),                         StaffGroup::TAB       },
{ "TAB4", "TAB",       5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::fourStringTabClef,        QT_TRANSLATE_NOOP("clefTable", "Tablature 4 lines"),                 StaffGroup::TAB       },
{ "TAB2", "TAB",       5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::sixStringTabClefSerif,    QT_TRANSLATE_NOOP("clefTable", "Tablature Serif"),                   StaffGroup::TAB       },
{ "TAB4_SERIF", "TAB", 5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::fourStringTabClefSerif,   QT_TRANSLATE_NOOP("clefTable", "Tablature Serif 4 lines"),           StaffGroup::TAB       },
// new clefs to be added between here
{ "C4_8VB", "C",       4, -1, 30, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, SymId::cClef8vb,                 QT_TRANSLATE_NOOP("clefTable", "Tenor clef 8va bassa"),              StaffGroup::STANDARD  },
{ "G8vbc", "G",        2, -1, 38, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8vbCClef,            QT_TRANSLATE_NOOP("clefTable", "G clef ottava bassa with C clef"),   StaffGroup::STANDARD  },
// and here in oder to not break TAB clef style
      };

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
  : Element(s, ElementFlag::ON_STAFF), symId(SymId::noSym)
      {}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Clef::mag() const
      {
      qreal mag = staff() ? staff()->mag(tick()) : 1.0;
      if (m_isSmall)
            mag *= score()->styleD(Sid::smallClefMag);
      return mag;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Clef::layout()
      {
      // determine current number of lines and line distance
      int   lines;
      qreal lineDist;
      Segment* clefSeg  = segment();
      int stepOffset;

      // check clef visibility and type compatibility
      if (clefSeg && staff()) {
            Fraction tick = clefSeg->tick();
            const StaffType* st = staff()->staffType(tick);
            bool show     = st->genClef();        // check staff type allows clef display
            StaffGroup staffGroup = st->group();

            // if not tab, use instrument->useDrumset to set staffGroup (to allow pitched to unpitched in same staff)
            if ( staffGroup != StaffGroup::TAB)
                  staffGroup = staff()->part()->instrument(this->tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;

            // check clef is compatible with staff type group:
            if (ClefInfo::staffGroup(clefType()) != staffGroup) {
                  if (tick > Fraction(0,1) && !generated()) // if clef is not generated, hide it
                        show = false;
                  else                          // if generated, replace with initial clef type
                        // TODO : instead of initial staff clef (which is assumed to be compatible)
                        // use the last compatible clef previously found in staff
                        _clefTypes = staff()->clefType(Fraction(0,1));
                  }

            // if clef not to show or not compatible with staff group
            if (!show) {
                  setbbox(QRectF());
                  symId = SymId::noSym;
                  qDebug("Clef::layout(): invisible clef at tick %d(%d) staff %d",
                     segment()->tick().ticks(), segment()->tick().ticks()/1920, staffIdx());
                  return;
                  }
            lines      = st->lines();         // init values from staff type
            lineDist   = st->lineDistance().val();
            stepOffset = st->stepOffset();
            }
      else {
            lines      = 5;
            lineDist   = 1.0;
            stepOffset = 0;
            }

      qreal _spatium = spatium();
      qreal yoff     = 0.0;
      if (clefType() !=  ClefType::INVALID && clefType() !=  ClefType::MAX) {
            symId = ClefInfo::symId(clefType());
            yoff = lineDist * (5 - ClefInfo::line(clefType())); 
            }
      else
            symId = SymId::noSym;

      switch (clefType()) {
            case ClefType::C_19C:                            // 19th C clef is like a G clef
                  yoff = lineDist * 1.5;
                  break;
            case ClefType::TAB:                            // TAB clef
            case ClefType::TAB4:                            // TAB clef 4 strings
            case ClefType::TAB_SERIF:                           // TAB clef alternate style
            case ClefType::TAB4_SERIF:                           // TAB clef alternate style
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = lineDist * (lines - 1) * .5;
                  stepOffset = 0; //  ignore stepOffset for TAB and pecussion clefs
                  break;
            case ClefType::PERC:                           // percussion clefs
            case ClefType::PERC2:
                  yoff = lineDist * (lines - 1) * 0.5;
                  stepOffset = 0;
                  break;
            case ClefType::INVALID:
            case ClefType::MAX:
                  qDebug("Clef::layout: invalid type");
                  return;
            default:
                  break;
            }
      // clefs on palette or at start of system/measure are left aligned
      // other clefs are right aligned
      QRectF r(symBbox(symId));
      qreal x = segment() && segment()->rtick().isNotZero() ? -r.right() : 0.0;
      setPos(x, yoff * _spatium + (stepOffset * 0.5 * _spatium));

      setbbox(r);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Clef::draw(QPainter* painter) const
      {
      if (symId == SymId::noSym || (staff() && !const_cast<const Staff*>(staff())->staffType(tick())->genClef()))
            return;
      painter->setPen(curColor());
      drawSymbol(symId, painter);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Clef::acceptDrop(EditData& data) const
      {
      return (data.dropElement->type() == ElementType::CLEF
         || (/*!generated() &&*/ data.dropElement->type() == ElementType::AMBITUS) );
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Clef::drop(EditData& data)
      {
      Element* e = data.dropElement;
      Clef* c = 0;
      if (e->isClef()) {
            Clef* clef = toClef(e);
            ClefType stype  = clef->clefType();
            if (clefType() != stype) {
                  score()->undoChangeClef(staff(), this, stype);
                  c = this;
                  }
            }
      else if (e->isAmbitus()) {
            /*if (!generated())*/ {
                  Measure*    meas  = measure();
                  Segment*    segm  = meas->getSegment(SegmentType::Ambitus, meas->tick());
                  if (segm->element(track()))
                        score()->undoRemoveElement(segm->element(track()));
                  Ambitus* r = new Ambitus(score());
                  r->setParent(segm);
                  r->setTrack(track());
                  score()->undoAddElement(r);
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
      if (val != m_isSmall) {
            m_isSmall = val;
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Clef::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "concertClefType")
                  _clefTypes._concertClef = Clef::clefType(e.readElementText());
            else if (tag == "transposingClefType")
                  _clefTypes._transposingClef = Clef::clefType(e.readElementText());
            else if (tag == "showCourtesyClef")
                  _showCourtesy = e.readInt();
            else if (tag == "forInstrumentChange")
                  _forInstrumentChange = e.readBool();
            else if (tag == "isHeader")   // Mu4.2+ compatibility
                  e.skipCurrentElement(); // skip, don't log
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      if (clefType() == ClefType::INVALID)
            setClefType(ClefType::G);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Clef::write(XmlWriter& xml) const
      {
      xml.stag(this);
      writeProperty(xml, Pid::CLEF_TYPE_CONCERT);
      writeProperty(xml, Pid::CLEF_TYPE_TRANSPOSING);
      if (!_showCourtesy)
            xml.tag("showCourtesyClef", _showCourtesy);
      if (_forInstrumentChange)
            xml.tag("forInstrumentChange", _forInstrumentChange);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Clef::setClefType(const QString& s)
      {
      ClefType ct = clefType(s);
      if (ct == ClefType::INVALID) {
            qDebug("Clef::setSubtype: unknown: <%s>", qPrintable(s));
            ct = ClefType::G;
            }
      setClefType(ct);
      }

//---------------------------------------------------------
//   clefTypeName
//---------------------------------------------------------

const char* Clef::clefTypeName()
      {
      return ClefInfo::tag(clefType());
      }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefType Clef::clefType(const QString& s)
      {
      return ClefInfo::tag2type(s);
      }

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Clef::setClefType(ClefType i)
      {
      if (concertPitch()) {
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
      if (concertPitch())
            return _clefTypes._concertClef;
      else
            return _clefTypes._transposingClef;
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Clef::spatiumChanged(qreal oldValue, qreal newValue)
      {
      Element::spatiumChanged(oldValue, newValue);
      layout();
      }

//---------------------------------------------------------
//   undoSetShowCourtesy
//---------------------------------------------------------

void Clef::undoSetShowCourtesy(bool v)
      {
      undoChangeProperty(Pid::SHOW_COURTESY, v);
      }

//---------------------------------------------------------
//   otherClef
//    try to locate the 'other clef' of a courtesy / main pair
//---------------------------------------------------------

Clef* Clef::otherClef()
      {
      // if not in a clef-segment-measure hierarchy, do nothing
      if (!parent() || !parent()->isSegment())
            return nullptr;
      Segment* segm = toSegment(parent());
      if (!segm->parent() || !segm->parent()->isMeasure())
            return 0;
      Measure* meas = toMeasure(segm->parent());
      Measure* otherMeas = nullptr;
      Segment* otherSegm = nullptr;
      Fraction segmTick  = segm->tick();
      SegmentType type = SegmentType::Clef;
      if (segmTick == meas->tick() && segm->segmentType() == SegmentType::HeaderClef) // if clef segm is measure-initial
            otherMeas = meas->prevMeasure();                                          // look for a previous measure
      else if (segmTick == meas->tick() + meas->ticks()) {                            // if clef segm is measure-final
            otherMeas = meas->nextMeasure();                                          // look for a next measure
            type = SegmentType::HeaderClef;
            }
      if (!otherMeas)
            return nullptr;
      // look for a clef segment in the 'other' measure at the same tick of this clef segment
      otherSegm = otherMeas->findSegment(type, segmTick);
      if (!otherSegm)
            return nullptr;
      // if any 'other' segment found, look for a clef in the same track as this
      return toClef(otherSegm->element(track()));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Clef::getProperty(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::CLEF_TYPE_CONCERT:     return int(_clefTypes._concertClef);
            case Pid::CLEF_TYPE_TRANSPOSING: return int(_clefTypes._transposingClef);
            case Pid::SHOW_COURTESY: return showCourtesy();
            case Pid::SMALL:         return isSmall();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Clef::setProperty(Pid propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case Pid::CLEF_TYPE_CONCERT:
                  setConcertClef(ClefType(v.toInt()));
                  break;
            case Pid::CLEF_TYPE_TRANSPOSING:
                  setTransposingClef(ClefType(v.toInt()));
                  break;
            case Pid::SHOW_COURTESY: _showCourtesy = v.toBool(); break;
            case Pid::SMALL:         setSmall(v.toBool()); break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Clef::propertyDefault(Pid id) const
      {
      switch(id) {
            case Pid::CLEF_TYPE_CONCERT:     return int(ClefType::INVALID);
            case Pid::CLEF_TYPE_TRANSPOSING: return int(ClefType::INVALID);
            case Pid::SHOW_COURTESY: return true;
            case Pid::SMALL:         return false;
            default:              return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* Clef::nextSegmentElement()
      {
      return segment()->firstInNextSegments(staffIdx());
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* Clef::prevSegmentElement()
      {
      return segment()->lastInPrevSegments(staffIdx());
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Clef::accessibleInfo() const
      {
      return qApp->translate("clefTable", ClefInfo::name(clefType()));
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Clef::clear()
      {
      setbbox(QRectF());
      symId = SymId::noSym;
      }

}

