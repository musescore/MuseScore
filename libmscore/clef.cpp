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


// table must be in sync with enum ClefType
const ClefInfo ClefInfo::clefTable[] = {
// tag    xmlName    line oCh pOff|-lines for sharps---||---lines for flats--|   name
{ "G",    "G",         2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, QT_TRANSLATE_NOOP("clefTable", "Treble clef"),            StaffGroup::STANDARD  },
{ "G8va", "G",         2,  1, 52, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, QT_TRANSLATE_NOOP("clefTable", "Treble clef 8va"),        StaffGroup::STANDARD  },
{ "G15ma","G",         2,  2, 59, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, QT_TRANSLATE_NOOP("clefTable", "Treble clef 15ma"),       StaffGroup::STANDARD  },
{ "G8vb", "G",         2, -1, 38, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, QT_TRANSLATE_NOOP("clefTable", "Treble clef 8vb"),        StaffGroup::STANDARD  },
{ "F",    "F",         4,  0, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, QT_TRANSLATE_NOOP("clefTable", "Bass clef"),              StaffGroup::STANDARD  },
{ "F8vb", "F",         4, -1, 26, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, QT_TRANSLATE_NOOP("clefTable", "Bass clef 8vb"),          StaffGroup::STANDARD  },
{ "F15mb","F",         4, -2, 19, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, QT_TRANSLATE_NOOP("clefTable", "Bass clef 15mb"),         StaffGroup::STANDARD  },
{ "F3",   "F",         3,  0, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, QT_TRANSLATE_NOOP("clefTable", "Baritone clef (F clef)"), StaffGroup::STANDARD  },
{ "F5",   "F",         5,  0, 31, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, QT_TRANSLATE_NOOP("clefTable", "Subbass clef"),           StaffGroup::STANDARD  },
{ "C1",   "C",         1,  0, 43, { 5, 1, 4, 0, 3,-1, 2, 2, 6, 3, 7, 4, 8, 5 }, QT_TRANSLATE_NOOP("clefTable", "Soprano clef"),           StaffGroup::STANDARD  }, // C1
{ "C2",   "C",         2,  0, 41, { 3, 6, 2, 5, 1, 4, 0, 0, 4, 1, 5, 2, 6, 3 }, QT_TRANSLATE_NOOP("clefTable", "Mezzo-soprano clef"),     StaffGroup::STANDARD  }, // C2
{ "C3",   "C",         3,  0, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 }, QT_TRANSLATE_NOOP("clefTable", "Alto clef"),              StaffGroup::STANDARD  }, // C3
{ "C4",   "C",         4,  0, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, QT_TRANSLATE_NOOP("clefTable", "Tenor clef"),             StaffGroup::STANDARD  }, // C4
{ "TAB",  "TAB",       5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, QT_TRANSLATE_NOOP("clefTable", "Tablature"),              StaffGroup::TAB       },
{ "PERC", "percussion",2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, QT_TRANSLATE_NOOP("clefTable", "Percussion"),             StaffGroup::PERCUSSION},
{ "C5",   "C",         5,  0, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, QT_TRANSLATE_NOOP("clefTable", "Baritone clef (C clef)"), StaffGroup::STANDARD  }, // C5
{ "G1",   "G",         1,  0, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, QT_TRANSLATE_NOOP("clefTable", "French violin clef"),     StaffGroup::STANDARD  }, // G4
{ "F8va", "F",         4,  1, 40, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, QT_TRANSLATE_NOOP("clefTable", "Bass clef 8va"),          StaffGroup::STANDARD  }, // F_8VA
{ "F15ma","F",         4,  2, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, QT_TRANSLATE_NOOP("clefTable", "Bass clef 15ma"),         StaffGroup::STANDARD  }, // F_15MA
{ "PERC2","percussion",2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, QT_TRANSLATE_NOOP("clefTable", "Percussion"),             StaffGroup::PERCUSSION}, // PERC2 placeholder
{ "TAB2", "TAB",       5,  0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, QT_TRANSLATE_NOOP("clefTable", "Tablature2"),             StaffGroup::TAB       },
{ "G8vbp","G",         2,  0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, QT_TRANSLATE_NOOP("clefTable", "Treble clef optional 8vb"),StaffGroup::STANDARD }, // G5
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
  : Element(s)
      {
      setFlags(ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);

      _showCourtesy               = true;
      _small                      = false;
      _clefTypes._concertClef     = ClefType::INVALID;
      _clefTypes._transposingClef = ClefType::INVALID;
      curClefType                 = ClefType::G;
      curLines                    = -1;
      curLineDist                 = 1.0;
      symbol                      = 0;
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

Clef::~Clef()
      {
      delete symbol;
      }

//---------------------------------------------------------
//   sym
//---------------------------------------------------------

SymId Clef::sym() const
      {
      return symbol ? symbol->sym() : SymId::noSym;
      }
qreal Clef::symMag() const
      {
      return symbol ? symbol->mag() : 1.0;
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Clef::mag() const
      {
      qreal mag = staff() ? staff()->mag() : 1.0;
      if (_small)
            mag *= score()->style(StyleIdx::smallClefMag).toDouble();
      return mag;
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Clef::setSelected(bool f)
      {
      Element::setSelected(f);
      symbol->setSelected(f);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Clef::layout()
      {
      setPos(QPoint());

      // determine current number of lines and line distance
      int   lines       = 5;              // assume resonable defaults
      qreal lineDist    = 1.0;

      Staff*      stf         = staff();
      StaffType*  staffType   = nullptr;
      Segment*    clefSeg     = static_cast<Segment*>(parent());
      // check clef visibility and type compatibility
      if (clefSeg && stf && stf->staffType()) {
            bool        bHide;
            // check staff type allows clef display
            staffType = stf->staffType();
            bHide = !staffType->genClef();

            // check clef is compatible with staff type group
            int tick = clefSeg->tick();
            if (ClefInfo::staffGroup(clefType()) != staffType->group()) {
                  if (tick > 0 && !generated()) // if clef is not generated, hide it
                        bHide = true;
                  else                          // if generated, replace with initial clef type
                        // TODO : instead of initial staff clef (which is assumed to be compatible)
                        // use the last compatible clef previously found in staff
                        _clefTypes = stf->clefType(0);
                  }

            //
            // courtesy clef
            //
            bool showClef = true;
            // only if there is a clef change
            if (!bHide && tick > 0 ) {
                  Measure* meas = clefSeg->measure();
                  // courtesy clef: end of last measure measure of system
                  bool courtesy = clefSeg->tick() == meas->endTick() && meas->system() && (meas == meas->system()->lastMeasure() || meas->system()->measures().indexOf(meas) == -1);
                  showClef =                    // show this clef if:
                        // it is not a courtesy clef
                        !courtesy
                        // or, if courtesy clef: show if score has courtesy clefs on
                        || ( score()->styleB(StyleIdx::genCourtesyClef)
                              // AND measure is not at the end of a repeat or of a section
                              && !( (meas->repeatFlags() & Repeat::END) || meas->isFinalMeasureOfSection() )
                              // AND this clef has courtesy clef turned on
                              && showCourtesy() );
                  bHide |= !showClef;
                  }

            // if clef not to show or not compatible with staff group
            if (bHide) {
                  delete symbol;         // set empty bbox and do nothing
                  symbol = 0;
                  setbbox(QRectF());
                  return;
                  }

            lines = staffType->lines();         // init values from staff type
            lineDist = staffType->lineDistance().val();
            }

      // if nothing changed since last layout, do nothing
//DEBUG      if (curClefType == clefType() && curLines == lines && curLineDist == lineDist)
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
      qreal yoff     = 0.0;

      delete symbol;
      symbol = 0;

      symbol = new Symbol(score());

      switch (curClefType) {
            case ClefType::G:                              // G clef on 2nd line
                  symbol->setSym(SymId::gClef);
                  yoff = 3.0 * curLineDist;
                  break;
            case ClefType::G1:                             // G clef 8va on 2nd line
                  symbol->setSym(SymId::gClef8va);
                  yoff = 3.0 * curLineDist;
                  break;
            case ClefType::G2:                             // G clef 15ma on 2nd line
                  symbol->setSym(SymId::gClef15ma);
                  yoff = 3.0 * curLineDist;
                  break;
            case ClefType::G3:                             // G clef 8vb on 2nd line
                  symbol->setSym(SymId::gClef8vb);
                  yoff = 3.0 * curLineDist;
                  break;
            case ClefType::F:                              // F clef on penultimate line
                  symbol->setSym(SymId::fClef);
                  yoff = 1.0 * curLineDist;
                  break;
            case ClefType::F8:                             // F clef 8va bassa on penultimate line
                  symbol->setSym(SymId::fClef8vb);
                  yoff = 1.0 * curLineDist;
                  break;
            case ClefType::F15:                            // F clef 15ma bassa on penultimate line
                  symbol->setSym(SymId::fClef15mb);
                  yoff = 1.0 * curLineDist;
                  break;
            case ClefType::F_B:                            // baritone clef
                  symbol->setSym(SymId::fClef);
                  yoff = 2.0 * curLineDist;
                  break;
            case ClefType::F_C:                            // subbass clef
                  symbol->setSym(SymId::fClef);
                  yoff = 0.0;
                  break;
            case ClefType::C1:                             // C clef in 1st line
                  symbol->setSym(SymId::cClef);
                  yoff = 4.0 * curLineDist;
                  break;
            case ClefType::C2:                             // C clef on 2nd line
                  symbol->setSym(SymId::cClef);
                  yoff = 3.0 * curLineDist;
                  break;
            case ClefType::C3:                             // C clef in 3rd line
                  symbol->setSym(SymId::cClef);
                  yoff = 2.0 * curLineDist;
                  break;
            case ClefType::C4:                             // C clef on 4th line
                  symbol->setSym(SymId::cClef);
                  yoff = 1.0 * curLineDist;
                  break;
            case ClefType::C5:                             // C clef on 5th line
                  symbol->setSym(SymId::cClef);
                  yoff = 0.0;
                  break;
            case ClefType::TAB:                            // TAB clef
                  symbol->setSym(SymId::sixStringTabClef);
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = curLineDist * (curLines - 1) * .5;
                  break;
            case ClefType::TAB2:                           // TAB clef alternate style
                  symbol->setSym(SymId::sixStringTabClefSerif);
                  // on tablature, position clef at half the number of spaces * line distance
                  yoff = curLineDist * (curLines - 1) * .5;
                  break;
            case ClefType::PERC:                           // percussion clefs
            case ClefType::PERC2:         // no longer supported: fall back to same glyph as PERC
                  symbol->setSym(SymId::unpitchedPercussionClef1);
                  yoff = curLineDist * (curLines - 1) * 0.5;
                  break;
            case ClefType::G4:                             // G clef in 1st line
                  symbol->setSym(SymId::gClef);
                  yoff = 4.0 * curLineDist;
                  break;
            case ClefType::F_8VA:                          // F clef 8va on penultimate line
                  symbol->setSym(SymId::fClef8va);
                  yoff = 1.0 * curLineDist;
                  break;
            case ClefType::F_15MA:                         // F clef 15ma on penultimate line
                  symbol->setSym(SymId::fClef15ma);
                  yoff = 1.0 * curLineDist;
                  break;
            case ClefType::G5:                              // G clef on 2nd line
                  symbol->setSym(SymId::gClef8vbParens);
                  yoff = 3.0 * curLineDist;
                  break;
            case ClefType::INVALID:
            case ClefType::MAX:
                  return;
            }

      symbol->setMag(smag);
      symbol->layout();
      symbol->setPos(.0, yoff * _spatium);
      symbol->setParent(this);
      symbol->setColor(curColor());
      setbbox(symbol->bbox().translated(symbol->pos()));
      symbol->setSelected(selected());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Clef::draw(QPainter* painter) const
      {
      if (staff() && !staff()->staffType()->genClef())
            return;
      symbol->setColor(curColor());           //??
      QPointF pt(symbol->pos());
      painter->translate(pt);
      symbol->draw(painter);
      painter->translate(-pt);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Clef::acceptDrop(const DropData& data) const
      {
      return (data.element->type() == Element::Type::CLEF
         || (/*!generated() &&*/ data.element->type() == Element::Type::AMBITUS) );
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Clef::drop(const DropData& data)
      {
      Element* e = data.element;
      Clef* c = 0;
      if (e->type() == Element::Type::CLEF) {
            Clef* clef = static_cast<Clef*>(e);
            ClefType stype  = clef->clefType();
            if (clefType() != stype) {
                  score()->undoChangeClef(staff(), segment(), stype);
                  c = this;
                  }
            }
      else if (e->type() == Element::Type::AMBITUS) {
            /*if (!generated())*/ {
                  Measure*    meas  = measure();
                  Segment*    segm  = meas->getSegment(Segment::Type::Ambitus, meas->tick());
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
                  case 19: ct = ClefType::PERC; break;      // PERC2 no longer supported
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
      layout1();
      Element::spatiumChanged(oldValue, newValue);
      }

//---------------------------------------------------------
//   undoSetShowCourtesy
//---------------------------------------------------------

void Clef::undoSetShowCourtesy(bool v)
      {
      score()->undoChangeProperty(this, P_ID::SHOW_COURTESY, v);
      }

//---------------------------------------------------------
//   otherClef
//    try to locate the 'other clef' of a courtesy / main pair
//---------------------------------------------------------

Clef* Clef::otherClef()
      {
      // if not in a clef-segment-measure hierarchy, do nothing
      if (!parent() || parent()->type() != Element::Type::SEGMENT)
            return nullptr;
      Segment* segm = static_cast<Segment*>(parent());
      int segmTick = segm->tick();
      if (!segm->parent() || !segm->parent()->isMeasure())
            return nullptr;
      Measure* meas = static_cast<Measure*>(segm->parent());
      Measure* otherMeas = nullptr;
      Segment* otherSegm = nullptr;
      if (segmTick == meas->tick())                         // if clef segm is measure-initial
            otherMeas = meas->prevMeasure();                // look for a previous measure
      else if (segmTick == meas->tick() + meas->ticks())    // if clef segm is measure-final
            otherMeas = meas->nextMeasure();                // look for a next measure
      if (!otherMeas)
            return nullptr;
      // look for a clef segment in the 'other' measure at the same tick of this clef segment
      otherSegm = otherMeas->findSegment(Segment::Type::Clef, segmTick);
      if (!otherSegm)
            return nullptr;
      // if any 'other' segment found, look for a clef in the same track as this
      return static_cast<Clef*>(otherSegm->element(track()));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Clef::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::SHOW_COURTESY: return showCourtesy();
            case P_ID::SMALL:         return small();
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
            case P_ID::SHOW_COURTESY: _showCourtesy = v.toBool(); break;
            case P_ID::SMALL:         setSmall(v.toBool()); break;
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
            case P_ID::SHOW_COURTESY: return true;
            case P_ID::SMALL:         return false;
            default:              return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* Clef::nextElement()
      {
      return segment()->firstInNextSegments(staffIdx());
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* Clef::prevElement()
      {
      return segment()->lastInPrevSegments(staffIdx());
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Clef::accessibleInfo()
      {
      return qApp->translate("clefTable", ClefInfo::name(clefType()));
      }

}

