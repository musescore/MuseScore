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
#include "measure.h"
#include "ambitus.h"
#include "xml.h"
#include "sym.h"
#include "symbol.h"
#include "system.h"
#include "score.h"
#include "staff.h"
#include "segment.h"
#include "stafftype.h"

namespace Ms {


#define TR(a) QT_TRANSLATE_NOOP("clefTable", a)

// table must be in sync with enum ClefType
const ClefInfo ClefInfo::clefTable[] = {
// tag    xmlName    line oCh pOff|-lines for sharps---||---lines for flats--|  symbol                           | name                                   | valid in staff group
{ "G",    "G",         2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef,                    TR("Treble clef"),                       StaffGroup::STANDARD  },
// { "G15mb","G",         2, -2, 59, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef15mb,                TR("Treble clef 15ma bassa"),                  StaffGroup::STANDARD  },
{ "G15mb","G",         2, -2, 31, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef15mb,                TR("Treble clef 15ma bassa"),                  StaffGroup::STANDARD  },
{ "G8vb", "G",         2, -1, 38, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8vb,                 TR("Treble clef 8va bassa"),                   StaffGroup::STANDARD  },
{ "G8va", "G",         2,  1, 52, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8va,                 TR("Treble clef 8va alta"),                   StaffGroup::STANDARD  },
{ "G15ma","G",         2,  2, 59, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef15ma,                TR("Treble clef 15ma alta"),                  StaffGroup::STANDARD  },
{ "G8vbo","G",         2, -1, 38, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8vbOld,              TR("Double treble clef 8va bassa on 2nd line"),StaffGroup::STANDARD  },
{ "G8vbp","G",         2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8vbParens,           TR("Treble clef optional 8va bassa"),          StaffGroup::STANDARD  },
{ "G1",   "G",         1,  0, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::gClef,                    TR("French violin clef"),                StaffGroup::STANDARD  },
{ "C1",   "C",         1,  0, 43, { 5, 1, 4, 0, 3,-1, 2, 2, 6, 3, 7, 4, 8, 5 }, SymId::cClef,                    TR("Soprano clef"),                      StaffGroup::STANDARD  },
{ "C2",   "C",         2,  0, 41, { 3, 6, 2, 5, 1, 4, 0, 0, 4, 1, 5, 2, 6, 3 }, SymId::cClef,                    TR("Mezzo-soprano clef"),                StaffGroup::STANDARD  },
{ "C3",   "C",         3,  0, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 }, SymId::cClef,                    TR("Alto clef"),                         StaffGroup::STANDARD  },
{ "C4",   "C",         4,  0, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, SymId::cClef,                    TR("Tenor clef"),                        StaffGroup::STANDARD  },
{ "C5",   "C",         5,  0, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, SymId::cClef,                    TR("Baritone clef (C clef)"),            StaffGroup::STANDARD  },
{ "C_19C", "G",        2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::cClefSquare,              TR("C clef, H shape (19th century)"),    StaffGroup::STANDARD  },
{ "C3_F18C", "C",      3,  0, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 }, SymId::cClefFrench,              TR("Alto clef (French, 18th century)"),  StaffGroup::STANDARD  },
{ "C4_F18C", "C",      4,  0, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, SymId::cClefFrench,              TR("Tenor clef (French, 18th century)"), StaffGroup::STANDARD  },
{ "C3_F20C", "C",      3,  0, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 }, SymId::cClefFrench20C,           TR("Alto clef (French, 20th century)"),  StaffGroup::STANDARD  },
{ "C4_F20C", "C",      4,  0, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, SymId::cClefFrench20C,           TR("Tenor clef (French, 20th century)"), StaffGroup::STANDARD  },
{ "F",    "F",         4,  0, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef,                    TR("Bass clef"),                         StaffGroup::STANDARD  },
{ "F15mb","F",         4, -2, 19, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef15mb,                TR("Bass clef 15ma bassa"),                    StaffGroup::STANDARD  },
{ "F8vb", "F",         4, -1, 26, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef8vb,                 TR("Bass clef 8va bassa"),                     StaffGroup::STANDARD  },
{ "F8va", "F",         4,  1, 40, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef8va,                 TR("Bass clef 8va alta"),                     StaffGroup::STANDARD  },
{ "F15ma","F",         4,  2, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef15ma,                TR("Bass clef 15ma alta"),                    StaffGroup::STANDARD  },
{ "F3",   "F",         3,  0, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, SymId::fClef,                    TR("Baritone clef (F clef)"),            StaffGroup::STANDARD  },
{ "F5",   "F",         5,  0, 31, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::fClef,                    TR("Subbass clef"),                      StaffGroup::STANDARD  },
{ "F_F18C","F",        4,  0, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClefFrench,              TR("F clef (French, 18th century)"),     StaffGroup::STANDARD  },
{ "F_19C","F",         4,  0, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, SymId::fClef19thCentury,         TR("F clef (19th century)"),             StaffGroup::STANDARD  },
{ "PERC", "percussion",2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::unpitchedPercussionClef1, TR("Percussion"),                        StaffGroup::PERCUSSION},
{ "PERC2","percussion",2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::unpitchedPercussionClef2, TR("Percussion 2"),                      StaffGroup::PERCUSSION},
{ "TAB",  "TAB",       5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::sixStringTabClef,         TR("Tablature"),                         StaffGroup::TAB       },
{ "TAB4", "TAB",       5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::fourStringTabClef,        TR("Tablature 4 lines"),                 StaffGroup::TAB       },
{ "TAB2", "TAB",       5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::sixStringTabClefSerif,    TR("Tablature Serif"),                   StaffGroup::TAB       },
{ "TAB4_SERIF", "TAB", 5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::fourStringTabClefSerif,   TR("Tablature Serif 4 lines"),           StaffGroup::TAB       },
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
  : Element(s, ElementFlag::ON_STAFF)
      {
      _showCourtesy               = true;
      _small                      = false;
      _clefTypes._concertClef     = ClefType::INVALID;
      _clefTypes._transposingClef = ClefType::INVALID;
      }

Clef::Clef(const Clef& c)
   : Element(c)
      {
      _showCourtesy = c._showCourtesy;
      _small        = c._small;
      _clefTypes    = c._clefTypes;
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Clef::mag() const
      {
      qreal mag = staff() ? staff()->mag(tick()) : 1.0;
      if (_small)
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
            StaffType* st = staffType();
            bool show     = st->genClef();        // check staff type allows clef display

            // check clef is compatible with staff type group:
            if (ClefInfo::staffGroup(clefType()) != st->group()) {
                  if (tick > Fraction(0,1) && !generated()) // if clef is not generated, hide it
                        show = false;
                  else                          // if generated, replace with initial clef type
                        // TODO : instead of initial staff clef (which is assumed to be compatible)
                        // use the last compatible clef previously found in staff
                        _clefTypes = staff()->clefType(Fraction(0,1));
                  }

            Measure* meas = clefSeg->measure();
            if (meas && meas->system() && !score()->lineMode()) {
                  const auto& ml = meas->system()->measures();
                  bool found = (std::find(ml.begin(), ml.end(), meas) != ml.end());
                  bool courtesy = (tick == meas->endTick() && (meas == meas->system()->lastMeasure() || !found));
                  if (courtesy && (!showCourtesy() || !score()->styleB(Sid::genCourtesyClef) || meas->isFinalMeasureOfSection()))
                        show = false;
                  }
            // if clef not to show or not compatible with staff group
            if (!show) {
                  setbbox(QRectF());
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
            yoff = lineDist * (lines - ClefInfo::line(clefType()));
            }

      switch (clefType()) {
            case ClefType::C_19C:                            // 19th C clef is like a G clef
                  yoff = lineDist * 1.5;
                  break;
            case ClefType::TAB:                            // TAB clef
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = lineDist * (lines - 1) * .5;
                  break;
            case ClefType::TAB4:                            // TAB clef 4 strings
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = lineDist * (lines - 1) * .5;
                  break;
            case ClefType::TAB_SERIF:                           // TAB clef alternate style
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = lineDist * (lines - 1) * .5;
                  break;
            case ClefType::TAB4_SERIF:                           // TAB clef alternate style
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = lineDist * (lines - 1) * .5;
                  break;
            case ClefType::PERC:                           // percussion clefs
                  yoff = lineDist * (lines - 1) * 0.5;
                  break;
            case ClefType::PERC2:
                  yoff = lineDist * (lines - 1) * 0.5;
                  break;
            case ClefType::INVALID:
            case ClefType::MAX:
                  qDebug("Clef::layout: invalid type");
                  return;
            default:
                  break;
            }
      // clefs are right aligned to Segment
      QRectF r(symBbox(symId));
      setPos(0.0, yoff * _spatium + (stepOffset * -_spatium));

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
      if (val != _small) {
            _small = val;
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
      if (_clefTypes._concertClef != ClefType::INVALID)
            xml.tag("concertClefType", ClefInfo::tag(_clefTypes._concertClef));
      if (_clefTypes._transposingClef != ClefType::INVALID)
            xml.tag("transposingClefType", ClefInfo::tag(_clefTypes._transposingClef));
      if (!_showCourtesy)
            xml.tag("showCourtesyClef", _showCourtesy);
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
      if (segmTick == meas->tick())                         // if clef segm is measure-initial
            otherMeas = meas->prevMeasure();                // look for a previous measure
      else if (segmTick == meas->tick() + meas->ticks())    // if clef segm is measure-final
            otherMeas = meas->nextMeasure();                // look for a next measure
      if (!otherMeas)
            return nullptr;
      // look for a clef segment in the 'other' measure at the same tick of this clef segment
      otherSegm = otherMeas->findSegment(SegmentType::Clef | SegmentType::HeaderClef, segmTick);
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
            case Pid::SHOW_COURTESY: return showCourtesy();
            case Pid::SMALL:         return small();
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

