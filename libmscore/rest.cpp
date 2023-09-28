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
//   restStyle
//---------------------------------------------------------

static const ElementStyle restStyle {
      { Sid::mmRestNumberPos, Pid::MMREST_NUMBER_POS },
      };

//---------------------------------------------------------
//    Rest
//--------------------------------------------------------

Rest::Rest(Score* s)
  : ChordRest(s)
      {
      _mmWidth   = 0;
      _beamMode  = Beam::Mode::NONE;
      _sym       = SymId::restQuarter;
      if (score())
            initElementStyle(&restStyle);
      }

Rest::Rest(Score* s, const TDuration& d)
  : ChordRest(s)
      {
      _mmWidth   = 0;
      _beamMode  = Beam::Mode::NONE;
      _sym       = SymId::restQuarter;
      setDurationType(d);
      if (d.fraction().isValid())
            setTicks(d.fraction());
      if (score())
            initElementStyle(&restStyle);
      }

Rest::Rest(const Rest& r, bool link)
   : ChordRest(r, link)
      {
      if (link) {
            score()->undo(new Link(this, const_cast<Rest*>(&r)));
            setAutoplace(true);
            }
      _gap     = r._gap;
      _sym     = r._sym;
      dotline  = r.dotline;
      _mmWidth = r._mmWidth;
      for (NoteDot* dot : r._dots)
            add(new NoteDot(*dot));
      }

//---------------------------------------------------------
//   Rest::draw
//---------------------------------------------------------

void Rest::draw(QPainter* painter) const
      {
      const StaffType* stt = staff() ? staff()->staffTypeForElement(this) : nullptr;
      if (
         (stt && stt->isTabStaff()
         // in tab staff, do not draw rests is rests are off OR if dur. symbols are on
         && (!stt->showRests() || stt->genDurations())
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

            // draw number
            int n = measure()->mmRestCount();
            std::vector<SymId>&& s = toTimeSigString(QString("%1").arg(n));
            QRectF numberBox = symBbox(s);
            qreal y = _mmRestNumberPos * spatium() - staff()->height() * .5;
            qreal x = (_mmWidth - numberBox.width()) * .5;
            drawSymbols(s, painter, QPointF(x, y));

            // draw horizontal line
            qreal pw = _spatium * .7; // line width
            QPen pen(painter->pen());
            pen.setWidthF(pw);
            painter->setPen(pen);
            qreal x1 = pw * .5; // half of the line width
            qreal x2 = _mmWidth - x1;

            // avoid painting the line when it collides with the number.
            if ((y + (numberBox.height() * .5 )) > -x1  && (y - (numberBox.height() * .5 )) < x1) {
                  qreal gapDistance = numberBox.width() * .5 + _spatium;
                  qreal midpoint = (x1 + x2) * .5;
                  painter->drawLine(QLineF(x1, 0.0, midpoint - gapDistance, 0.0));
                  painter->drawLine(QLineF(midpoint + gapDistance, 0.0, x2, 0.0));
                  }
            else {
                  painter->drawLine(QLineF(x1, 0.0, x2, 0.0));
                  }

            // draw vertical lines
            pen.setWidthF(_spatium * .2);
            painter->setPen(pen);
            painter->drawLine(QLineF(0.0, -_spatium, 0.0, _spatium));
            painter->drawLine(QLineF(_mmWidth, -_spatium, _mmWidth,  _spatium));
            }
      else
            drawSymbol(_sym, painter);
      }

//---------------------------------------------------------
//   setOffset, overridden from Element
//    (- raster vertical position in spatium units) -> no
//    - half rests and whole rests outside the staff are
//      replaced by special symbols with ledger lines
//---------------------------------------------------------

void Rest::setOffset(const QPointF& o)
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

      Element::setOffset(o);
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Rest::drag(EditData& ed)
      {
      // don't allow drag for Measure Rests, because they can't be easily laid out in correct position while dragging
      if (measure() && durationType().type() == TDuration::DurationType::V_MEASURE)
            return QRectF();

      QPointF s(ed.delta);
      QRectF r(abbox());

      // Limit horizontal drag range
      static const qreal xDragRange = spatium() * 5;
      if (fabs(s.x()) > xDragRange)
            s.rx() = xDragRange * (s.x() < 0 ? -1.0 : 1.0);
      setOffset(QPointF(s.x(), s.y()));
      layout();
      score()->rebuildBspTree();
      return abbox() | r;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Rest::acceptDrop(EditData& data) const
      {
      Element* e = data.dropElement;
      ElementType type = e->type();
      if (
            (type == ElementType::ICON && toIcon(e)->iconType() == IconType::SBEAM)
         || (type == ElementType::ICON && toIcon(e)->iconType() == IconType::MBEAM)
         || (type == ElementType::ICON && toIcon(e)->iconType() == IconType::NBEAM)
         || (type == ElementType::ICON && toIcon(e)->iconType() == IconType::BEAM32)
         || (type == ElementType::ICON && toIcon(e)->iconType() == IconType::BEAM64)
         || (type == ElementType::ICON && toIcon(e)->iconType() == IconType::AUTOBEAM)
         || (type == ElementType::FERMATA)
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
         || (type == ElementType::REHEARSAL_MARK)
         || (type == ElementType::FRET_DIAGRAM)
         || (type == ElementType::TREMOLOBAR)
         || (type == ElementType::IMAGE)
         || (type == ElementType::SYMBOL)
         || (type == ElementType::REPEAT_MEASURE && durationType().type() == TDuration::DurationType::V_MEASURE)
         ) {
            return true;
            }
      // prevent 'hanging' slurs, avoid crash on tie
      return type != ElementType::SLUR && type != ElementType::TIE && e->isSpanner();
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Rest::drop(EditData& data)
      {
      Element* e = data.dropElement;
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

            case ElementType::CHORD: {
                  Chord* c = toChord(e);
                  Note* n  = c->upNote();
                  Direction dir = c->stemDirection();
                  // score()->select(0, SelectType::SINGLE, 0);
                  NoteVal nval;
                  nval.pitch = n->pitch();
                  nval.headGroup = n->headGroup();
                  Fraction d = score()->inputState().ticks();
                  if (!d.isZero()) {
                        Segment* seg = score()->setNoteRest(segment(), track(), nval, d, dir);
                        if (seg) {
                              ChordRest* cr = toChordRest(seg->element(track()));
                              if (cr)
                                    score()->nextInputPos(cr, false);
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
                  if (ticks() >= Fraction(2, 1))
                        return SymId::restDoubleWhole;
                  // fall through
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
//   layoutMMRest
//---------------------------------------------------------

void Rest::layoutMMRest(qreal val)
      {
//      static const qreal verticalLineWidth = .2;

      qreal _spatium = spatium();
      _mmWidth       = val;
//      qreal h        = _spatium * (2 + verticalLineWidth);
//      qreal w        = _mmWidth + _spatium * verticalLineWidth * .5;
//      bbox().setRect(-_spatium * verticalLineWidth * .5, -h * .5, w, h);
      bbox().setRect(0.0, -_spatium, _mmWidth, _spatium * 2);

      // text
      addbbox(mmRestNumberRect());
}

//---------------------------------------------------------
//   mmRestNumberRect
///   returns the mmrest number's bounding rectangle
//---------------------------------------------------------

QRectF Rest::mmRestNumberRect() const
      {
      int n = measure()->mmRestCount();
      std::vector<SymId>&& s = toTimeSigString(QString("%1").arg(n));

      QRectF r = symBbox(s);
      qreal y = _mmRestNumberPos * spatium() - staff()->height() * .5;
      qreal x = (_mmWidth - r.width()) * .5;

      r.translate(QPointF(x, y));
      return r;
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
      qreal _spatium = spatium();
      if (measure() && measure()->isMMRest()) {
            _mmWidth = score()->styleP(Sid::minMMRestWidth) * mag();
            // setbbox(QRectF(0.0, -_spatium, _mmWidth, 2.0 * _spatium));
            return;
            }

      rxpos() = 0.0;
      const StaffType* stt = staffType();
      if (stt && stt->isTabStaff()) {
            // if rests are shown and note values are shown as duration symbols
            if (stt->showRests() && stt->genDurations()) {
                  TDuration::DurationType type = durationType().type();
                  int                     dots = durationType().dots();
                  // if rest is whole measure, convert into actual type and dot values
                  if (type == TDuration::DurationType::V_MEASURE && measure()) {
                        Fraction ticks = measure()->ticks();
                        TDuration dur  = TDuration(ticks).type();
                        type           = dur.type();
                        dots           = dur.dots();
                        }
                  // symbol needed; if not exist, create, if exists, update duration
                  if (!_tabDur)
                        _tabDur = new TabDurationSymbol(score(), stt, type, dots);
                  else
                        _tabDur->setDuration(type, dots, stt);
                  _tabDur->setParent(this);
// needed?        _tabDur->setTrack(track());
                  _tabDur->layout();
                  setbbox(_tabDur->bbox());
                  setPos(0.0, 0.0);             // no rest is drawn: reset any position might be set for it
                  return;
                  }
            // if no rests or no duration symbols, delete any dur. symbol and chain into standard staff mngmt
            // this is to ensure horiz space is reserved for rest, even if they are not displayed
            // Rest::draw() will skip their drawing, if not needed
            if(_tabDur) {
                  delete _tabDur;
                  _tabDur = 0;
                  }
            }

      dotline = Rest::getDotline(durationType().type());

      qreal yOff       = offset().y();
      const Staff* stf = staff();
      const StaffType*  st = stf ? stf->staffTypeForElement(this) : 0;
      qreal lineDist = st ? st->lineDistance().val() : 1.0;
      int userLine   = yOff == 0.0 ? 0 : lrint(yOff / (lineDist * _spatium));
      int lines      = st ? st->lines() : 5;
      int lineOffset = computeLineOffset(lines);

      int yo;
      _sym = getSymbol(durationType().type(), lineOffset / 2 + userLine, lines, &yo);
      rypos() = (qreal(yo) + qreal(lineOffset) * .5) * lineDist * _spatium;
      setbbox(symBbox(_sym));
      layoutDots();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Rest::layoutDots()
      {
      checkDots();
      qreal x = symWidth(_sym) + score()->styleP(Sid::dotNoteDistance) * mag();
      qreal dx = score()->styleP(Sid::dotDotDistance) * mag();
      qreal y = dotline * spatium() * .5;
      for (NoteDot* dot : _dots) {
            dot->layout();
            dot->setPos(x, y);
            x += dx;
            }
      }

//---------------------------------------------------------
//   checkDots
//---------------------------------------------------------

void Rest::checkDots()
      {
      int n = dots() - int(_dots.size());
      for (int i = 0; i < n; ++i) {
            NoteDot* dot = new NoteDot(score());
            dot->setParent(this);
            dot->setVisible(visible());
            score()->undoAddElement(dot);
            }
      if (n < 0) {
            for (int i = 0; i < -n; ++i)
                  score()->undoRemoveElement(_dots.back());
            }
      }

//---------------------------------------------------------
//   dot
//---------------------------------------------------------

NoteDot* Rest::dot(int n)
      {
      checkDots();
      return _dots[n];
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
      bool offsetVoices = s && measure() && (voice() > 0 || measure()->hasVoices(staffIdx(), tick(), actualTicks()));
      if (offsetVoices && voice() == 0) {
            // do not offset voice 1 rest if there exists a matching invisible rest in voice 2;
            Element* e = s->element(track() + 1);
            if (e && e->isRest() && !e->visible() && !toRest(e)->isGap()) {
                  Rest* r = toRest(e);
                  if (r->globalTicks() == globalTicks()) {
                        offsetVoices = false;
                        }
                  }
            }

      if (offsetVoices && voice() < 2) {
            // in slash notation voices 1 and 2 are not offset outside the staff
            // if the staff contains slash notation then only offset rests in voices 3 and 4
            int baseTrack = staffIdx() * VOICES;
            for (int v = 0; v < VOICES; ++v) {
                  Element* e = s->element(baseTrack + v);
                  if (e && e->isChord() && toChord(e)->slash()) {
                        offsetVoices = false;
                        break;
                        }
                  }
            }

      if (offsetVoices && staff()->mergeMatchingRests()) {
            // automatically merge matching rests if nothing in any other voice
            // this is not always the right thing to do do, but is useful in choral music
            // and can be enabled via a staff property
            bool matchFound = false;
            int baseTrack = staffIdx() * VOICES;
            for (int v = 0; v < VOICES; ++v) {
                  if (v == voice())
                        continue;
                  Element* e = s->element(baseTrack + v);
                  // try to find match in any other voice
                  if (e) {
                        if (e->type() == ElementType::REST) {
                              Rest* r = toRest(e);
                              if (r->globalTicks() == globalTicks()) {
                                    matchFound = true;
                                    continue;
                                    }
                              }
                        // no match found; no sense looking for anything else
                        matchFound = false;
                        break;
                        }
                  }
            if (matchFound)
                  offsetVoices = false;
            }

      int lineOffset    = 0;
      int assumedCenter = 4;
      int actualCenter  = (lines - 1);
      int centerDiff    = actualCenter - assumedCenter;

      if (offsetVoices) {
            // move rests in a multi voice context
            bool up = (voice() == 0) || (voice() == 2);     // TODO: use style values
            
            // Calculate extra offset to move rests above the highest resp. below the lowest note
            // of this segment (for measure rests, of the whole measure) in all opposite voices. 
            // Ignore stems and articulations, because which multi-voice they are at the opposite end.
            int upOffset = up ? 1 : 0;
            int line = up ? 10 : -10;

            // For compatibility reasons apply automatic collision avoidance only if y-offset is unchanged 
            if (qFuzzyIsNull(offset().y()) && autoplace()) {
                  int firstTrack = staffIdx() * 4;
                  int extraOffsetForFewLines = lines < 5 ? 2 : 0;
                  bool isMeasureRest = durationType().type() == TDuration::DurationType::V_MEASURE;
                  Segment* seg = isMeasureRest ? measure()->first() : s;
                  while (seg) {
                        for (const int& track : { firstTrack + upOffset, firstTrack + 2 + upOffset }) {
                              Element* e = seg->element(track);
                              if (e && e->isChord()) {
                                    Chord* chord = toChord(e);
                                    StaffGroup staffGroup = staff()->staffType(chord->tick())->group();
                                    for (Note* note : chord->notes()) {
                                          int nline = staffGroup == StaffGroup::TAB
                                                ? note->string() * 2
                                                : note->line();
                                          nline = nline - centerDiff;
                                          if (up && nline <= line) {
                                                line = nline - extraOffsetForFewLines; 
                                                if (note->accidentalType() != AccidentalType::NONE)
                                                      line--;
                                                }
                                          else if (!up && nline >= line) {
                                                line = nline + extraOffsetForFewLines;
                                                if (note->accidentalType() != AccidentalType::NONE)
                                                      line++;
                                                }
                                          }
                                    }
                              }
                        seg = isMeasureRest ? seg->next() : nullptr;
                        }
                  }

            switch(durationType().type()) {
                  case TDuration::DurationType::V_LONG:
                        lineOffset = up ? -3 : 5;
                        lineOffset += up ? (line < 5 ? line - 5 : 0) : (line > 5 ? line - 5 : 0);
                        break;
                  case TDuration::DurationType::V_BREVE:
                        lineOffset = up ? -3 : 5;
                        lineOffset += up ? (line < 3 ? line - 3 : 0) : (line > 5 ? line - 5 : 0);
                        break;
                  case TDuration::DurationType::V_MEASURE:
                        if (ticks() >= Fraction(2, 1)) {  // breve symbol
                              lineOffset = up ? -3 : 5;
                              lineOffset += up ? (line < 3 ? line - 3 : 0) : (line > 5 ? line - 4 : 0);
                              }
                        else {
                              lineOffset = up ? -4 : 6;     // whole symbol
                              lineOffset += up ? (line < 3 ? line - 2 : 0) : (line > 6 ? line - 5 : 0);
                              }
                        break;
                  case TDuration::DurationType::V_WHOLE:
                        lineOffset = up ? -4 : 6;
                        lineOffset += up ? (line < 3 ? line - 2 : 0) : (line > 6 ? line - 5 : 0);
                        break;
                  case TDuration::DurationType::V_HALF:
                        lineOffset = up ? -4 : 4;
                        lineOffset += up ? (line < 2 ? line - 3 : 0) : (line > 5 ? line - 4 : 0);
                        break;
                  case TDuration::DurationType::V_QUARTER:
                        lineOffset = up ? -4 : 4;
                        lineOffset += up ? (line < 5 ? line - 4 : 0) : (line > 3 ? line - 3 : 0);
                        break;
                  case TDuration::DurationType::V_EIGHTH:
                        lineOffset = up ? -4 : 4;
                        lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
                        break;
                  case TDuration::DurationType::V_16TH:
                        lineOffset = up ? -6 : 4;
                        lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
                        break;
                  case TDuration::DurationType::V_32ND:
                        lineOffset = up ? -6 : 6;
                        lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
                        break;
                  case TDuration::DurationType::V_64TH:
                        lineOffset = up ? -8 : 6;
                        lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
                        break;
                  case TDuration::DurationType::V_128TH:
                        lineOffset = up ? -8 : 8;
                        lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
                        break;
                  case TDuration::DurationType::V_1024TH:
                  case TDuration::DurationType::V_512TH:
                  case TDuration::DurationType::V_256TH:
                        lineOffset = up ? -10 : 6;
                        lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
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
      for (NoteDot* dot : _dots)
            dot->scanElements(data, func, all);
      if (!isGap())
            func(data, this);
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Rest::setTrack(int val)
      {
      ChordRest::setTrack(val);
      for (NoteDot* dot : _dots)
            dot->setTrack(val);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Rest::mag() const
      {
      qreal m = staff() ? staff()->mag(this) : 1.0;
      if (isSmall())
            m *= score()->styleD(Sid::smallNoteMag);
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
            p.ry() += bbox().top() + spatium() * 1.5;
      else
            p.ry() += bbox().bottom() - spatium() * 1.5;
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
//   rightEdge
//---------------------------------------------------------

qreal Rest::rightEdge() const
      {
      return x() + width();
      }

qreal Rest::centerX() const
      {
      SymId sym = this->sym();
      const auto& bbox = symBbox(sym);
      return symWidth(sym) / 2 + bbox.bottomLeft().x();
      }

//---------------------------------------------------------
//   accent
//---------------------------------------------------------

bool Rest::accent()
      {
      return (voice() >= 2 && isSmall());
      }

//---------------------------------------------------------
//   setAccent
//---------------------------------------------------------

void Rest::setAccent(bool flag)
      {
      undoChangeProperty(Pid::SMALL, flag);
      if (voice() % 2 == 0) {
            if (flag) {
                  qreal yOffset = -(bbox().bottom());
                  if (durationType() >= TDuration::DurationType::V_HALF)
                        yOffset -= staff()->spatium(tick()) * 0.5;
                  // undoChangeProperty(Pid::OFFSET, QPointF(0.0, yOffset));
                  rypos() += yOffset;
                  }
            else {
                  // undoChangeProperty(Pid::OFFSET, QPointF());  TODO::check
                  }
            }
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Rest::accessibleInfo() const
      {
      QString voice = QObject::tr("Voice: %1").arg(QString::number(track() % VOICES + 1));
      return QObject::tr("%1; Duration: %2; %3").arg(Element::accessibleInfo(), durationUserName(), voice);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Rest::screenReaderInfo() const
      {
      Measure* m = measure();
      bool voices = m ? m->hasVoices(staffIdx()) : false;
      QString voice = voices ? QObject::tr("Voice: %1").arg(QString::number(track() % VOICES + 1)) : "";
      return QString("%1 %2 %3").arg(Element::accessibleInfo(), durationUserName(), voice);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Rest::add(Element* e)
      {
      e->setParent(this);
      e->setTrack(track());

      switch(e->type()) {
            case ElementType::NOTEDOT:
                  _dots.push_back(toNoteDot(e));
                  break;
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
            case ElementType::NOTEDOT:
                  _dots.pop_back();
                  break;
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
      writeBeam(xml);
      xml.stag(this);
      writeStyledProperties(xml);
      ChordRest::writeProperties(xml);
      el().write(xml);
      bool write_dots = false;
      for (NoteDot* dot : _dots)
            if (!dot->offset().isNull() || !dot->visible() || dot->color() != Qt::black || dot->visible() != visible()) {
                  write_dots = true;
                  break;
                  }
      if (write_dots)
            for (NoteDot* dot: _dots)
                  dot->write(xml);
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
            else if (tag == "NoteDot") {
                  NoteDot* dot = new NoteDot(score());
                  dot->read(e);
                  add(dot);
                  }
            else if (readStyledProperty(e, tag))
                  ;
            else if (ChordRest::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Rest::localSpatiumChanged(qreal oldValue, qreal newValue)
      {
      ChordRest::localSpatiumChanged(oldValue, newValue);
      for (Element* e : _dots)
            e->localSpatiumChanged(oldValue, newValue);
      for (Element* e : el())
            e->localSpatiumChanged(oldValue, newValue);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Rest::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::GAP:
                  return false;
            case Pid::MMREST_NUMBER_POS:
                  return score()->styleV(Sid::mmRestNumberPos);
            default:
                  return ChordRest::propertyDefault(propertyId);
            }
      }

//————————————————————————————
//   resetProperty
//————————————————————————————

void Rest::resetProperty(Pid id)
      {
      setProperty(id, propertyDefault(id));
      return;
      }

//————————————————————————————
//   getPropertyStyle
//————————————————————————————

Sid Rest::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::MMREST_NUMBER_POS)
            return Sid::mmRestNumberPos;
      return ChordRest::getPropertyStyle(pid);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Rest::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::GAP:
                  return _gap;
            case Pid::MMREST_NUMBER_POS:
                  return _mmRestNumberPos;
            default:
                  return ChordRest::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Rest::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::GAP:
                  _gap = v.toBool();
                  triggerLayout();
                  break;
            case Pid::VISIBLE:
                  setVisible(v.toBool());
                  triggerLayout();
                  break;
            case Pid::OFFSET:
                  score()->addRefresh(canvasBoundingRect());
                  setOffset(v.toPointF());
                  layout();
                  score()->addRefresh(canvasBoundingRect());
                  if (measure() && durationType().type() == TDuration::DurationType::V_MEASURE)
                         measure()->triggerLayout();
                  triggerLayout();
                  break;
            case Pid::MMREST_NUMBER_POS:
                  _mmRestNumberPos = v.toDouble();
                  triggerLayout();
                  break;
            default:
                  return ChordRest::setProperty(propertyId, v);
            }
      return true;
      }

//---------------------------------------------------------
//   undoChangeDotsVisible
//---------------------------------------------------------

void Rest::undoChangeDotsVisible(bool v)
      {
      for (NoteDot* dot : _dots)
            dot->undoChangeProperty(Pid::VISIBLE, QVariant(v));
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* Rest::nextElement()
      {
      return ChordRest::nextElement();
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* Rest::prevElement()
      {
      return ChordRest::prevElement();
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape Rest::shape() const
      {
      Shape shape;
      if (!_gap) {
            shape.add(ChordRest::shape());
            if (measure() && measure()->isMMRest()) {
                  qreal _spatium = spatium();
                  shape.add(QRectF(0.0, -_spatium, _mmWidth, 2.0 * _spatium));

                  shape.add(mmRestNumberRect());
                  }
            else
#ifndef NDEBUG
                  shape.add(bbox(), name());
#else
                  shape.add(bbox());
#endif
            for (NoteDot* dot : _dots)
                  shape.add(symBbox(SymId::augmentationDot).translated(dot->pos()));
            }
      for (Element* e : el()) {
            if (e->addToSkyline())
                  shape.add(e->shape().translated(e->pos()));
            }
      return shape;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Rest::editDrag(EditData& editData)
      {
      Segment* seg = segment();

      if (editData.modifiers & Qt::ShiftModifier) {
            const Spatium deltaSp = Spatium(editData.delta.x() / spatium());
            seg->undoChangeProperty(Pid::LEADING_SPACE, seg->extraLeadingSpace() + deltaSp);
            }
      else {
            setOffset(offset() + editData.evtDelta);
            }
      triggerLayout();
      }

}
