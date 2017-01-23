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
#include "segment.h"
#include "stafftype.h"
#include "icon.h"
#include "image.h"

namespace Ms {

//---------------------------------------------------------
//    Rest
//--------------------------------------------------------

Rest::Rest(Score* s)
  : ChordRest(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      _beamMode  = Beam::Mode::NONE;
      _gap       = false;
      _sym       = SymId::restQuarter;
      }

Rest::Rest(Score* s, const TDuration& d)
  : ChordRest(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      _beamMode  = Beam::Mode::NONE;
      _sym       = SymId::restQuarter;
      _gap       = false;
      setDurationType(d);
      if (d.fraction().isValid())
            setDuration(d.fraction());
      }

Rest::Rest(const Rest& r, bool link)
   : ChordRest(r, link)
      {
      if (link)
            score()->undo(new Link(const_cast<Rest*>(&r), this));
      _gap     = r._gap;
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
         (staff() && staff()->isTabStaff(tick())
         // in tab staff, do not draw rests is rests are off OR if dur. symbols are on
         && (!staff()->staffType(tick())->showRests() || staff()->staffType(tick())->genDurations())
         && (!measure() || !measure()->isMMRest()))        // show multi measure rest always
         || generated()
            )
            return;
      qreal _spatium = spatium();

      painter->setPen(curColor());

      if (measure() && measure()->isMMRest()) {
            //only on voice 1
            if (track() % VOICES)
                  return;
            Measure* m = measure();
            int n      = m->mmRestCount();
            qreal pw   = _spatium * .7;
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

            std::vector<Ms::SymId> s = toTimeSigString(QString("%1").arg(n));
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
                  qreal dnd = point(score()->styleS(StyleIdx::dotNoteDistance)) * mag();
                  qreal ddd = point(score()->styleS(StyleIdx::dotDotDistance)) * mag();
                  for (int i = 1; i <= dots; ++i) {
                        qreal x = symWidth(_sym) + dnd + ddd * (i - 1);
                        drawSymbol(SymId::augmentationDot, painter, QPointF(x, y));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setUserOff, overriden from Element
//    (- raster vertical position in spatium units) -> no
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

      Element::setUserOff(o);
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

bool Rest::acceptDrop(const DropData& data) const
      {
      Element* e = data.element;
      ElementType type = e->type();
      if (
            (type == ElementType::ICON && static_cast<Icon*>(e)->iconType() == IconType::SBEAM)
         || (type == ElementType::ICON && static_cast<Icon*>(e)->iconType() == IconType::MBEAM)
         || (type == ElementType::ICON && static_cast<Icon*>(e)->iconType() == IconType::NBEAM)
         || (type == ElementType::ICON && static_cast<Icon*>(e)->iconType() == IconType::BEAM32)
         || (type == ElementType::ICON && static_cast<Icon*>(e)->iconType() == IconType::BEAM64)
         || (type == ElementType::ICON && static_cast<Icon*>(e)->iconType() == IconType::AUTOBEAM)
         || (type == ElementType::ARTICULATION && static_cast<Articulation*>(e)->isFermata())
         || (type == ElementType::CLEF)
         || (type == ElementType::KEYSIG)
         || (type == ElementType::TIMESIG)
         || (type == ElementType::SYSTEM_TEXT)
         || (type == ElementType::STAFF_TEXT)
         || (type == ElementType::BAR_LINE)
         || (type == ElementType::BREATH)
         || (type == ElementType::CHORD)
         || (type == ElementType::NOTE)
         || (type == ElementType::STAFF_STATE)
         || (type == ElementType::INSTRUMENT_CHANGE)
         || (type == ElementType::DYNAMIC)
         || (type == ElementType::HARMONY)
         || (type == ElementType::TEMPO_TEXT)
         || (type == ElementType::STAFF_TEXT)
         || (type == ElementType::REHEARSAL_MARK)
         || (type == ElementType::FRET_DIAGRAM)
         || (type == ElementType::TREMOLOBAR)
         || (type == ElementType::IMAGE)
         || (type == ElementType::SYMBOL)
         || (type == ElementType::REPEAT_MEASURE && durationType().type() == TDuration::DurationType::V_MEASURE)
         ) {
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Rest::drop(const DropData& data)
      {
      Element* e = data.element;
      switch (e->type()) {
            case ElementType::ARTICULATION:
                  {
                  Articulation* a = toArticulation(e);
                  if (!a->isFermata() || !score()->addArticulation(this, a)) {
                        delete e;
                        e = 0;
                        }
                  }
                  return e;

            case ElementType::CHORD:
                  {
                  Chord* c              = static_cast<Chord*>(e);
                  Note* n               = c->upNote();
                  Direction dir = c->stemDirection();
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
            case ElementType::REPEAT_MEASURE:
                  delete e;
                  if (durationType().type() == TDuration::DurationType::V_MEASURE) {
                        measure()->cmdInsertRepeatMeasure(staffIdx());
                        }
                  break;

            case ElementType::SYMBOL:
            case ElementType::IMAGE:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  return e;

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
            case TDuration::DurationType::V_EIGHTH:
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
                  qDebug("unknown rest type %d", int(type));
                  return SymId::restQuarter;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Rest::layout()
      {
      if (_gap)
            return;
      for (Element* e : el())
            e->layout();
      if (measure() && measure()->isMMRest()) {
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
      if (staff() && staff()->isTabStaff(tick())) {
            StaffType* tab = staff()->staffType(tick());
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

      dotline = Rest::getDotline(durationType().type());

      qreal _spatium = spatium();
      qreal yOff     = userOff().y();
      Staff* stf     = staff();
      StaffType*  st = stf->staffType(tick());
      qreal lineDist = st ? st->lineDistance().val() : 1.0;
      int userLine   = yOff == 0.0 ? 0 : lrint(yOff / (lineDist * _spatium));
      int lines      = st ? st->lines() : 5;
      int lineOffset = computeLineOffset(lines);

      int yo;
      _sym = getSymbol(durationType().type(), lineOffset / 2 + userLine, lines, &yo);
      layoutArticulations();
      rypos() = (qreal(yo) + qreal(lineOffset) * .5) * lineDist * _spatium;
      setbbox(symBbox(_sym));
      }

//---------------------------------------------------------
//   getDotline
//---------------------------------------------------------

int Rest::getDotline(TDuration::DurationType durationType)
      {
      int dl = -1;
      switch(durationType) {
            case TDuration::DurationType::V_64TH:
            case TDuration::DurationType::V_32ND:
                  dl = -3;
                  break;
            case TDuration::DurationType::V_1024TH:
            case TDuration::DurationType::V_512TH:
            case TDuration::DurationType::V_256TH:
            case TDuration::DurationType::V_128TH:
                  dl = -5;
                  break;
            default:
                  dl = -1;
                  break;
            }
      return dl;
      }

//---------------------------------------------------------
//   computeLineOffset
//---------------------------------------------------------

int Rest::computeLineOffset(int lines)
      {
      Segment* s = segment();
      bool offsetVoices = s && measure() && measure()->hasVoices(staffIdx());
      if (offsetVoices && voice() == 0) {
            // do not offset voice 1 rest if there exists a matching invisible rest in voice 2;
            Element* e = s->element(track() + 1);
            if (e && e->isRest() && (!e->visible() || toRest(e)->isGap())) {
                  Rest* r = static_cast<Rest*>(e);
                  if (r->globalDuration() == globalDuration()) {
                        offsetVoices = false;
                        }
                  }
            }
#if 0
      if (offsetVoices && staff()->mergeMatchingRests()) {
            // automatically merge matching rests in voices 1 & 2 if nothing in any other voice
            // this is not always the right thing to do do, but is useful in choral music
            // and perhaps could be made enabled via a staff property
            // so choral staves can be treated differently than others
            bool matchFound = false;
            bool nothingElse = true;
            int baseTrack = staffIdx() * VOICES;
            for (int v = 0; v < VOICES; ++v) {
                  if (v == voice())
                        continue;
                  Element* e = s->element(baseTrack + v);
                  if (v <= 1) {
                        // try to find match in other voice (1 or 2)
                        if (e && e->type() == ElementType::REST) {
                              Rest* r = static_cast<Rest*>(e);
                              if (r->globalDuration() == globalDuration()) {
                                    matchFound = true;
                                    continue;
                                    }
                              }
                        // no match found; no sense looking for anything else
                        break;
                        }
                  else {
                        // if anything in another voice, do not merge
                        if (e) {
                              nothingElse = false;
                              break;
                              }
                        }
                  }
            if (matchFound && nothingElse)
                  offsetVoices = false;
            }
#endif

      int lineOffset    = 0;
      int assumedCenter = 4;
      int actualCenter  = (lines - 1);
      int centerDiff    = actualCenter - assumedCenter;

      if (offsetVoices) {
            // move rests in a multi voice context
            bool up = (voice() == 0) || (voice() == 2);     // TODO: use style values
            switch(durationType().type()) {
                  case TDuration::DurationType::V_LONG:
                        lineOffset = up ? -3 : 5;
                        break;
                  case TDuration::DurationType::V_BREVE:
                        lineOffset = up ? -3 : 5;
                        break;
                  case TDuration::DurationType::V_MEASURE:
                        if (duration() >= Fraction(2, 1))   // breve symbol
                              lineOffset = up ? -3 : 5;
                        else
                              lineOffset = up ? -4 : 6;     // whole symbol
                        break;
                  case TDuration::DurationType::V_WHOLE:
                        lineOffset = up ? -4 : 6;
                        break;
                  case TDuration::DurationType::V_HALF:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TDuration::DurationType::V_QUARTER:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TDuration::DurationType::V_EIGHTH:
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

            // adjust offsets for staves with other than five lines
            if (lines != 5) {
                  lineOffset += centerDiff;
                  if (centerDiff & 1) {
                        // round to line
                        if (lines == 2 && staff() && staff()->lineDistance(tick()) < 2.0)
                              ;                                         // leave alone
                        else if (lines <= 6)
                              lineOffset += lineOffset > 0 ? -1 : 1;    // round inward
                        else
                              lineOffset += lineOffset > 0 ? 1 : -1;    // round outward
                        }
                  }
            }
      else {
            // Gould says to center rests on middle line or space
            // but subjectively, many rests look strange centered on a space
            // so we do it for 2-line staves only
            if (centerDiff & 1 && lines != 2)
                  centerDiff += 1;  // round down

            lineOffset = centerDiff;
            switch(durationType().type()) {
                  case TDuration::DurationType::V_LONG:
                  case TDuration::DurationType::V_BREVE:
                  case TDuration::DurationType::V_MEASURE:
                  case TDuration::DurationType::V_WHOLE:
                        if (lineOffset & 1)
                              lineOffset += 1;  // always round to nearest line
                        else if (lines <= 3)
                              lineOffset += 2;  // special case - move down for 1-line or 3-line staff
                        break;
                  case TDuration::DurationType::V_HALF:
                        if (lineOffset & 1)
                              lineOffset += 1;  // always round to nearest line
                        break;
                  default:
                        break;
                  }
            }
      // DEBUG: subtract this off only to be added back in layout()?
      // that would throw off calculation of when ledger lines are needed
      //if (staff())
      //      lineOffset -= staff()->staffType()->stepOffset();
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
      ChordRest::scanElements(data, func, all);
      for (Element* e : el())
            e->scanElements(data, func, all);
      if (!isGap())
            func(data, this);
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
      undoChangeProperty(P_ID::BEAM_MODE, int(Beam::Mode::NONE));
      ChordRest::reset();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Rest::mag() const
      {
      qreal m = staff()->mag(tick());
      if (small())
            m *= score()->styleD(StyleIdx::smallNoteMag);
      return m;
      }

//---------------------------------------------------------
//   upLine
//---------------------------------------------------------

int Rest::upLine() const
      {
      qreal _spatium = spatium();
      return lrint((pos().y() + bbox().top() + _spatium) * 2 / _spatium);
      }

//---------------------------------------------------------
//   downLine
//---------------------------------------------------------

int Rest::downLine() const
      {
      qreal _spatium = spatium();
      return lrint((pos().y() + bbox().top() + _spatium) * 2 / _spatium);
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

//---------------------------------------------------------
//   accent
//---------------------------------------------------------

bool Rest::accent()
      {
      return (voice() >= 2 && small());
      }

//---------------------------------------------------------
//   setAccent
//---------------------------------------------------------

void Rest::setAccent(bool flag)
      {
      undoChangeProperty(P_ID::SMALL, flag);
      if (voice() % 2 == 0) {
            if (flag) {
                  qreal yOffset = -(bbox().bottom());
                  if (durationType() >= TDuration::DurationType::V_HALF)
                        yOffset -= staff()->spatium(tick()) * 0.5;
                  undoChangeProperty(P_ID::USER_OFF, QPointF(0.0, yOffset));
                  }
            else {
                  undoChangeProperty(P_ID::USER_OFF, QPointF());
                  }
            }
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Rest::accessibleInfo() const
      {
      QString voice = tr("Voice: %1").arg(QString::number(track() % VOICES + 1));
      return tr("%1; Duration: %2; %3").arg(Element::accessibleInfo()).arg(durationUserName()).arg(voice);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Rest::screenReaderInfo() const
      {
      QString voice = tr("Voice: %1").arg(QString::number(track() % VOICES + 1));
      return QString("%1 %2 %3").arg(Element::accessibleInfo()).arg(durationUserName()).arg(voice);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Rest::add(Element* e)
      {
      e->setParent(this);
      e->setTrack(track());

      switch(e->type()) {
            case ElementType::SYMBOL:
            case ElementType::IMAGE:
                  el().push_back(e);
                  break;
            default:
                  ChordRest::add(e);
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Rest::remove(Element* e)
      {
      switch(e->type()) {
            case ElementType::SYMBOL:
            case ElementType::IMAGE:
                  if (!el().remove(e))
                        qDebug("Rest::remove(): cannot find %s", e->name());
                  break;
            default:
                  ChordRest::remove(e);
                  break;
            }
      }

//--------------------------------------------------
//   Rest::write
//---------------------------------------------------------

void Rest::write(XmlWriter& xml) const
      {
      if (_gap)
            return;
      xml.stag(name());
      ChordRest::writeProperties(xml);
      el().write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Rest::read
//---------------------------------------------------------

void Rest::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->setTrack(track());
                  s->read(e);
                  add(s);
                  }
            else if (tag == "Image") {
                  if (MScore::noImages)
                        e.skipCurrentElement();
                  else {
                        Image* image = new Image(score());
                        image->setTrack(track());
                        image->read(e);
                        add(image);
                        }
                  }
            else if (ChordRest::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Rest::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::GAP:
                  return _gap;
            default:
                  return ChordRest::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Rest::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::GAP:
                  return false;
            default:
                  return ChordRest::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Rest::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::GAP:
                  _gap = v.toBool();
                  score()->setLayout(tick());
                  break;

            case P_ID::USER_OFF:
                  score()->addRefresh(canvasBoundingRect());
                  setUserOff(v.toPointF());
                  layout();
                  score()->addRefresh(canvasBoundingRect());
                  if (beam())
                        score()->setLayout(tick());
                  break;
            default:
                  return ChordRest::setProperty(propertyId, v);
            }
      return true;
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape Rest::shape() const
      {
      Shape shape;
      if (!_gap) {
            shape.add(ChordRest::shape());
            if (parent() && measure() && measure()->isMMRest())
                  shape.add(QRectF(0.0, 0.0, score()->styleP(StyleIdx::minMMRestWidth), height()));
            else
                  shape.add(bbox());
            }
      return shape;
      }

}
