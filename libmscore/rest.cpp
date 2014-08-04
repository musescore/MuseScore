//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "rest.h"
#include "score.h"
#include "xml.h"
#include "style.h"
#include "utils.h"
#include "tuplet.h"
#include "sym.h"
#include "stafftext.h"
#include "articulation.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "undo.h"
#include "staff.h"
#include "harmony.h"
#include "lyrics.h"
#include "segment.h"
#include "stafftype.h"
#include "icon.h"

namespace Ms {

//---------------------------------------------------------
//    Rest
//--------------------------------------------------------

Rest::Rest(Score* s)
  : ChordRest(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      _beamMode  = Beam::Mode::NONE;
      dotline    = -1;
      _sym       = SymId::restQuarter;
      }

Rest::Rest(Score* s, const TDuration& d)
  : ChordRest(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      _beamMode  = Beam::Mode::NONE;
      dotline    = -1;
      _sym       = SymId::restQuarter;
      setDurationType(d);
      if (d.fraction().isValid())
            setDuration(d.fraction());
      }

Rest::Rest(const Rest& r, bool link)
   : ChordRest(r, link)
      {
      if (link)
            linkTo((Rest*)&r);      // HACK!
      _sym     = r._sym;
      dotline  = r.dotline;
      _mmWidth = r._mmWidth;
      }

//---------------------------------------------------------
//   Rest::draw
//---------------------------------------------------------

void Rest::draw(QPainter* painter) const
      {
      if (
         (staff() && staff()->isTabStaff()
         // in tab staff, do not draw rests is rests are off OR if dur. symbols are on
         && (!staff()->staffType()->showRests() || staff()->staffType()->genDurations())
         && !measure()->isMMRest())        // show multi measure rest always
         || generated()
            )
            return;
      qreal _spatium = spatium();

      painter->setPen(curColor());

      if (parent() && measure() && measure()->isMMRest()) {
            //only on voice 1
            if ((track() % VOICES) != 0)
                  return;
            Measure* m = measure();
            int n      = m->mmRestCount();
            qreal pw = _spatium * .7;
            QPen pen(painter->pen());
            pen.setWidthF(pw);
            painter->setPen(pen);

            qreal w  = _mmWidth;
            qreal y  = 0.0;
            qreal x1 = 0.0;
            qreal x2 =  w;
            pw *= .5;
            painter->drawLine(QLineF(x1 + pw, y, x2 - pw, y));

            // draw vertical lines:
            pen.setWidthF(_spatium * .2);
            painter->setPen(pen);
            painter->drawLine(QLineF(x1, y-_spatium, x1, y+_spatium));
            painter->drawLine(QLineF(x2, y-_spatium, x2, y+_spatium));

            painter->setFont(score()->scoreFont()->font());
            QFontMetricsF fm(score()->scoreFont()->font());
            QString s = toTimeSigString(QString("%1").arg(n));
            y  = -_spatium * 1.5 - staff()->height() *.5;
            qreal x = center(x1, x2);
            x -= symBbox(s).width() * .5;
            drawSymbols(s, painter, QPointF(x, y));
            }
      else {
            drawSymbol(_sym, painter);
            int dots = durationType().dots();
            if (dots) {
                  qreal y = dotline * _spatium * .5;
                  for (int i = 1; i <= dots; ++i) {
                        qreal x = symWidth(_sym) + point(score()->styleS(StyleIdx::dotNoteDistance)) * i;
                        drawSymbol(SymId::augmentationDot, painter, QPointF(x, y));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setUserOff, overriden from Element
//    - raster vertical position in spatium units
//    - half rests and whole rests outside the staff are
//      replaced by special symbols with ledger lines
//---------------------------------------------------------

void Rest::setUserOff(const QPointF& o)
      {
      qreal _spatium = spatium();
      int line = lrint(o.y()/_spatium);

      if (_sym == SymId::restWhole && (line <= -2 || line >= 3))
            _sym = SymId::restWholeLegerLine;
      else if (_sym == SymId::restWholeLegerLine && (line > -2 && line < 4))
            _sym = SymId::restWhole;
      else if (_sym == SymId::restHalf && (line <= -3 || line >= 3))
            _sym = SymId::restHalfLegerLine;
      else if (_sym == SymId::restHalfLegerLine && (line > -3 && line < 3))
            _sym = SymId::restHalf;

      Element::setUserOff(QPointF(o.x(), qreal(line) * _spatium));
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Rest::drag(EditData* data)
      {
      QPointF s(data->delta);
      QRectF r(abbox());

      // Limit horizontal drag range
      static const qreal xDragRange = spatium() * 5;
      if (fabs(s.x()) > xDragRange)
            s.rx() = xDragRange * (s.x() < 0 ? -1.0 : 1.0);
      setUserOff(QPointF(s.x(), s.y()));
      layout();
      score()->rebuildBspTree();
      return abbox() | r;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Rest::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      Element::Type type = e->type();
      if (
         (type == Element::Type::ICON && static_cast<Icon*>(e)->iconType() == IconType::SBEAM)
         || (type == Element::Type::ICON && static_cast<Icon*>(e)->iconType() == IconType::MBEAM)
         || (type == Element::Type::ICON && static_cast<Icon*>(e)->iconType() == IconType::NBEAM)
         || (type == Element::Type::ICON && static_cast<Icon*>(e)->iconType() == IconType::BEAM32)
         || (type == Element::Type::ICON && static_cast<Icon*>(e)->iconType() == IconType::BEAM64)
         || (type == Element::Type::ICON && static_cast<Icon*>(e)->iconType() == IconType::AUTOBEAM)
         || (type == Element::Type::ARTICULATION && static_cast<Articulation*>(e)->isFermata())
         || (type == Element::Type::CLEF)
         || (type == Element::Type::STAFF_TEXT)
         || (type == Element::Type::BAR_LINE)
         || (type == Element::Type::BREATH)
         || (type == Element::Type::CHORD)
         || (type == Element::Type::STAFF_STATE)
         || (type == Element::Type::INSTRUMENT_CHANGE)
         || (type == Element::Type::DYNAMIC)
         || (type == Element::Type::HARMONY)
         || (type == Element::Type::TEMPO_TEXT)
         || (type == Element::Type::STAFF_TEXT)
         || (type == Element::Type::REHEARSAL_MARK)
         || (type == Element::Type::FRET_DIAGRAM)
         || (type == Element::Type::SYMBOL)
         ) {
            return true;
            }
      if(type == Element::Type::REPEAT_MEASURE && durationType().type() == TDuration::DurationType::V_MEASURE)
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Rest::drop(const DropData& data)
      {
      Element* e = data.element;
      switch (e->type()) {
            case Element::Type::ARTICULATION:
                  {
                  Articulation* a = static_cast<Articulation*>(e);
                  if (!a->isFermata()
                     || !score()->addArticulation(this, a)) {
                        delete e;
                        e = 0;
                        }
                  }
                  return e;

            case Element::Type::CHORD:
                  {
                  Chord* c              = static_cast<Chord*>(e);
                  Note* n               = c->upNote();
                  MScore::Direction dir = c->stemDirection();
                  // score()->select(0, SelectType::SINGLE, 0);
                  NoteVal nval;
                  nval.pitch = n->pitch();
                  nval.headGroup = n->headGroup();
                  Fraction d = score()->inputState().duration().fraction();
                  if (!d.isZero()) {
                        Segment* seg = score()->setNoteRest(segment(), track(), nval, d, dir);
                        if (seg) {
                              ChordRest* cr = static_cast<ChordRest*>(seg->element(track()));
                              if (cr)
                                    score()->nextInputPos(cr, true);
                              }
                        }
                  delete e;
                  }
                  break;
            case Element::Type::REPEAT_MEASURE:
                  delete e;
                  if (durationType().type() == TDuration::DurationType::V_MEASURE) {
                        measure()->cmdInsertRepeatMeasure(staffIdx());
                        }
                  break;
            default:
                  return ChordRest::drop(data);
            }
      return 0;
      }

//---------------------------------------------------------
//   getSymbol
//---------------------------------------------------------

SymId Rest::getSymbol(TDuration::DurationType type, int line, int lines, int* yoffset)
      {
      *yoffset = 2;
      switch(type) {
            case TDuration::DurationType::V_LONG:
                  return SymId::restLonga;
            case TDuration::DurationType::V_BREVE:
                  return SymId::restDoubleWhole;
            case TDuration::DurationType::V_MEASURE:
                  if (duration() >= Fraction(2, 1))
                        return SymId::restDoubleWhole;
                  // fall trough
            case TDuration::DurationType::V_WHOLE:
                  *yoffset = 1;
                  return (line <= -2 || line >= (lines - 1)) ? SymId::restWholeLegerLine : SymId::restWhole;
            case TDuration::DurationType::V_HALF:
                  return (line <= -3 || line >= (lines - 2)) ? SymId::restHalfLegerLine : SymId::restHalf;
            case TDuration::DurationType::V_QUARTER:
                  return SymId::restQuarter;
            case TDuration::DurationType::V_EIGHT:
                  return SymId::rest8th;
            case TDuration::DurationType::V_16TH:
                  return SymId::rest16th;
            case TDuration::DurationType::V_32ND:
                  return SymId::rest32nd;
            case TDuration::DurationType::V_64TH:
                  return SymId::rest64th;
            case TDuration::DurationType::V_128TH:
                  return SymId::rest128th;
            case TDuration::DurationType::V_256TH:
                  return SymId::rest256th;
            case TDuration::DurationType::V_512TH:
                  return SymId::rest512th;
            case TDuration::DurationType::V_1024TH:
                  return SymId::rest1024th;
            default:
                  qDebug("unknown rest type %hhd", type);
                  return SymId::restQuarter;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Rest::layout()
      {
      _space.setLw(0.0);

      if (parent() && measure() && measure()->isMMRest()) {
            _space.setRw(point(score()->styleS(StyleIdx::minMMRestWidth)));

            static const qreal verticalLineWidth = .2;
            qreal _spatium = spatium();
            qreal h        = _spatium * (2 + verticalLineWidth);
            qreal w        = _mmWidth + _spatium * verticalLineWidth*.5;
            bbox().setRect(-_spatium * verticalLineWidth*.5, -h * .5, w, h);

            // text
            qreal y  = -_spatium * 2.5 - staff()->height() *.5;
            addbbox(QRectF(0, y, w, _spatium * 2));         // approximation
            return;
            }

      rxpos() = 0.0;
      if (staff() && staff()->isTabStaff()) {
            StaffType* tab = staff()->staffType();
            // if rests are shown and note values are shown as duration symbols
            if (tab->showRests() && tab->genDurations()) {
                  TDuration::DurationType type = durationType().type();
                  int                     dots = durationType().dots();
                  // if rest is whole measure, convert into actual type and dot values
                  if (type == TDuration::DurationType::V_MEASURE) {
                        int       ticks = measure()->ticks();
                        TDuration dur   = TDuration(Fraction::fromTicks(ticks)).type();
                        type = dur.type();
                        dots = dur.dots();
                        }
                  // symbol needed; if not exist, create, if exists, update duration
                  if (!_tabDur)
                        _tabDur = new TabDurationSymbol(score(), tab, type, dots);
                  else
                        _tabDur->setDuration(type, dots, tab);
                  _tabDur->setParent(this);
// needed?        _tabDur->setTrack(track());
                  _tabDur->layout();
                  setbbox(_tabDur->bbox());
                  setPos(0.0, 0.0);             // no rest is drawn: reset any position might be set for it
                  _space.setLw(0.0);
                  _space.setRw(width());
                  return;
                  }
            // if no rests or no duration symbols, delete any dur. symbol and chain into standard staff mngmt
            // this is to ensure horiz space is reserved for rest, even if they are not diplayed
            // Rest::draw() will skip their drawing, if not needed
            if(_tabDur) {
                  delete _tabDur;
                  _tabDur = 0;
                  }
            }

      switch(durationType().type()) {
            case TDuration::DurationType::V_64TH:
            case TDuration::DurationType::V_32ND:
                  dotline = -3;
                  break;
            case TDuration::DurationType::V_1024TH:
            case TDuration::DurationType::V_512TH:
            case TDuration::DurationType::V_256TH:
            case TDuration::DurationType::V_128TH:
                  dotline = -5;
                  break;
            default:
                  dotline = -1;
                  break;
            }
      qreal _spatium = spatium();
      int stepOffset = 0;
      if (staff())
            stepOffset = staff()->staffType()->stepOffset();
      int line       = lrint(userOff().y() / _spatium); //  + ((staff()->lines()-1) * 2);
      qreal lineDist = staff() ? staff()->staffType()->lineDistance().val() : 1.0;

      int lines = staff() ? staff()->lines() : 5;
      int lineOffset = computeLineOffset();

      int yo;
      _sym = getSymbol(durationType().type(), line + lineOffset/2, lines, &yo);
      layoutArticulations();
      rypos() = (qreal(yo) + qreal(lineOffset + stepOffset) * .5) * lineDist * _spatium;

      Spatium rs;
      if (dots()) {
            rs = Spatium(score()->styleS(StyleIdx::dotNoteDistance)
               + dots() * score()->styleS(StyleIdx::dotDotDistance));
            }
      if (dots()) {
            rs = Spatium(score()->styleS(StyleIdx::dotNoteDistance)
               + dots() * score()->styleS(StyleIdx::dotDotDistance));
            }
      setbbox(symBbox(_sym));
      qreal symOffset = bbox().x();
      if (symOffset < 0.0)
            _space.setLw(-symOffset);
      _space.setRw(width() + point(rs) + symOffset);
      }

//---------------------------------------------------------
//   centerX
//---------------------------------------------------------

int Rest::computeLineOffset()
      {
      int lineOffset = 0;
      int lines = staff() ? staff()->lines() : 5;
      if (segment() && measure() && measure()->mstaff(staffIdx())->hasVoices) {
            // move rests in a multi voice context
            bool up = (voice() == 0) || (voice() == 2);       // TODO: use style values
            switch(durationType().type()) {
                  case TDuration::DurationType::V_LONG:
                        lineOffset = up ? -3 : 5;
                        break;
                  case TDuration::DurationType::V_BREVE:
                        lineOffset = up ? -3 : 5;
                        break;
                  case TDuration::DurationType::V_MEASURE:
                        if (duration() >= Fraction(2, 1))    // breve symbol
                              lineOffset = up ? -3 : 5;
                        // fall through
                  case TDuration::DurationType::V_WHOLE:
                        lineOffset = up ? -4 : 6;
                        break;
                  case TDuration::DurationType::V_HALF:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TDuration::DurationType::V_QUARTER:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TDuration::DurationType::V_EIGHT:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TDuration::DurationType::V_16TH:
                        lineOffset = up ? -6 : 4;
                        break;
                  case TDuration::DurationType::V_32ND:
                        lineOffset = up ? -6 : 6;
                        break;
                  case TDuration::DurationType::V_64TH:
                        lineOffset = up ? -8 : 6;
                        break;
                  case TDuration::DurationType::V_128TH:
                        lineOffset = up ? -8 : 8;
                        break;
                  case TDuration::DurationType::V_1024TH:
                  case TDuration::DurationType::V_512TH:
                  case TDuration::DurationType::V_256TH:
                        lineOffset = up ? -10 : 6;
                        break;
                  default:
                        break;
                  }
            }
      else {
            switch(durationType().type()) {
                  case TDuration::DurationType::V_LONG:
                  case TDuration::DurationType::V_BREVE:
                  case TDuration::DurationType::V_MEASURE:
                  case TDuration::DurationType::V_WHOLE:
                        if (lines == 1)
                              lineOffset = -2;
                        break;
                  case TDuration::DurationType::V_HALF:
                  case TDuration::DurationType::V_QUARTER:
                  case TDuration::DurationType::V_EIGHT:
                  case TDuration::DurationType::V_16TH:
                  case TDuration::DurationType::V_32ND:
                  case TDuration::DurationType::V_64TH:
                  case TDuration::DurationType::V_128TH:
                  case TDuration::DurationType::V_256TH:
                  case TDuration::DurationType::V_512TH:
                  case TDuration::DurationType::V_1024TH:
                        if (lines == 1)
                              lineOffset = -4;
                        break;
                  default:
                        break;
                  }
            }
      return lineOffset;
      }

//---------------------------------------------------------
//   centerX
//---------------------------------------------------------

qreal Rest::centerX() const
      {
      return symWidth(_sym) * .5;
      }

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Rest::upPos() const
      {
      return symBbox(_sym).y();
      }

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Rest::downPos() const
      {
      return symBbox(_sym).y() + symHeight(_sym);
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Rest::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      func(data, this);
      ChordRest::scanElements(data, func, all);
      }

//---------------------------------------------------------
//   setMMWidth
//---------------------------------------------------------

void Rest::setMMWidth(qreal val)
      {
      _mmWidth = val;
      layout();
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Rest::reset()
      {
      score()->undoChangeProperty(this, P_ID::BEAM_MODE, int(Beam::Mode::NONE));
      ChordRest::reset();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Rest::mag() const
      {
      qreal m = staff()->mag();
      if (small())
            m *= score()->styleD(StyleIdx::smallNoteMag);
      return m;
      }

//---------------------------------------------------------
//   upLine
//---------------------------------------------------------

int Rest::upLine() const
      {
      return lrint((pos().y() + bbox().top() + spatium()) * 2 / spatium());
      }

//---------------------------------------------------------
//   downLine
//---------------------------------------------------------

int Rest::downLine() const
      {
      return lrint((pos().y() + bbox().top() + spatium()) * 2 / spatium());
      }

//---------------------------------------------------------
//   stemPos
//    point to connect stem
//---------------------------------------------------------

QPointF Rest::stemPos() const
      {
      return pagePos();
      }

//---------------------------------------------------------
//   stemPosBeam
//    return stem position of note on beam side
//    return canvas coordinates
//---------------------------------------------------------

QPointF Rest::stemPosBeam() const
      {
      QPointF p(pagePos());
      if (_up)
            p.ry() += bbox().top() + spatium() * 2;
      else
            p.ry() += bbox().bottom() - spatium() * 2;
      return p;
      }

//---------------------------------------------------------
//   stemPosX
//---------------------------------------------------------

qreal Rest::stemPosX() const
      {
      if (_up)
            return bbox().right();
      else
            return bbox().left();
      }
}

