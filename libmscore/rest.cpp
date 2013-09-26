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
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);
      _beamMode  = BeamMode::NONE;
      dotline    = -1;
      _sym       = rest4Sym;
      }

Rest::Rest(Score* s, const TDuration& d)
  : ChordRest(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);
      _beamMode  = BeamMode::NONE;
      dotline    = -1;
      _sym       = rest4Sym;
      setDurationType(d);
      if (d.fraction().isValid())
            setDuration(d.fraction());
      }

//---------------------------------------------------------
//   Rest::draw
//---------------------------------------------------------

void Rest::draw(QPainter* painter) const
      {
      if ( (staff() && staff()->isTabStaff()
            // in tab staff, do not draw rests is rests are off OR if dur. symbols are on
            && ( !((StaffTypeTablature*)staff()->staffType())->showRests()
                  || ((StaffTypeTablature*)staff()->staffType())->genDurations()) )
            || generated())
            return;
      qreal _spatium = spatium();

      painter->setPen(curColor());

      if (parent() && measure() && measure()->isMMRest()) {
            //only on voice 1
            if((track() % VOICES) != 0)
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

#ifdef USE_GLYPHS
            QRawFont rfont = fontId2RawFont(0);
            rfont.setPixelSize(20.0 * spatium()/(PPI * SPATIUM20));

            QGlyphRun glyphs;
            QVector<quint32> idx = rfont.glyphIndexesForString(QString("%1").arg(n));
            glyphs.setGlyphIndexes(idx);
            QVector<QPointF> adv = rfont.advancesForGlyphIndexes(idx);
            adv.insert(0, QPointF());
            glyphs.setPositions(adv);
            glyphs.setRawFont(rfont);
            QRectF r = glyphs.boundingRect();
            y  = -_spatium * .5 - (staff()->height()*.5);
            painter->drawGlyphRun(QPointF((x2 - x1) * .5 + x1 - r.width() * .5, y), glyphs);
#else
            QFont font = fontId2font(0);
            font.setPixelSize(lrint(20.0 * spatium()/(PPI * SPATIUM20)));
            painter->setFont(font);
            QFontMetricsF fm(font);
            y  = -_spatium * .5 - (staff()->height()*.5) - fm.ascent();
            painter->drawText(QRectF(center(x1, x2), y, .0, .0),
               Qt::AlignHCenter|Qt::TextDontClip,
               QString("%1").arg(n));
#endif
            }
      else {
            qreal mag = magS();
            symbols[score()->symIdx()][_sym].draw(painter, mag);
            int dots = durationType().dots();
            if (dots) {
                  qreal y = dotline * _spatium * .5;
                  for (int i = 1; i <= dots; ++i) {
                        qreal x = symbols[score()->symIdx()][_sym].width(mag)
                                   + point(score()->styleS(ST_dotNoteDistance)) * i;
                        symbols[score()->symIdx()][dotSym].draw(painter, mag, QPointF(x, y));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setUserOffset
//    - raster vertical position in spatium units
//    - half rests and whole rests outside the staff are
//      replaced by special symbols with ledger lines
//---------------------------------------------------------

void Rest::setUserOffset(qreal x, qreal y)
      {
      qreal _spatium = spatium();
      int line = lrint(y/_spatium);

      if (_sym == wholerestSym && (line <= -2 || line >= 3))
            _sym = outsidewholerestSym;
      else if (_sym == outsidewholerestSym && (line > -2 && line < 4))
            _sym = wholerestSym;
      else if (_sym == halfrestSym && (line <= -3 || line >= 3))
            _sym = outsidehalfrestSym;
      else if (_sym == outsidehalfrestSym && (line > -3 && line < 3))
            _sym = halfrestSym;

      setUserOff(QPointF(x, qreal(line) * _spatium));
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Rest::drag(const EditData& data)
      {
      QPointF s(data.pos);
      QRectF r(abbox());

      // Limit horizontal drag range
      static const qreal xDragRange = spatium() * 5;
      if (fabs(s.x()) > xDragRange)
            s.rx() = xDragRange * (s.x() < 0 ? -1.0 : 1.0);
      setUserOffset(s.x(), s.y());
      layout();
      score()->rebuildBspTree();
      return abbox() | r;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Rest::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      int type = e->type();
      if (
         (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_SBEAM)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_MBEAM)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_NBEAM)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_BEAM32)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_BEAM64)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_AUTOBEAM)
         || (type == ARTICULATION && static_cast<Articulation*>(e)->articulationType() == Articulation_Fermata)
         || (type == CLEF)
         || (type == STAFF_TEXT)
         || (type == BAR_LINE)
         || (type == BREATH)
         || (type == CHORD)
         || (type == STAFF_STATE)
         || (type == INSTRUMENT_CHANGE)
         || (type == DYNAMIC)
         || (type == HARMONY)
         || (type == TEMPO_TEXT)
         || (type == STAFF_TEXT)
         || (type == REHEARSAL_MARK)
         || (type == FRET_DIAGRAM)
         || (type == SYMBOL)
         ) {
            return true;
            }
      if(type == REPEAT_MEASURE && durationType().type() == TDuration::V_MEASURE)
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
            case ARTICULATION:
                  {
                  Articulation* a = static_cast<Articulation*>(e);
                  if (a->articulationType() != Articulation_Fermata
                     || !score()->addArticulation(this, a)) {
                        delete e;
                        e = 0;
                        }
                  }
                  return e;

            case CHORD:
                  {
                  Chord* c      = static_cast<Chord*>(e);
                  Note* n       = c->upNote();
                  MScore::Direction dir = c->stemDirection();
                  // score()->select(0, SELECT_SINGLE, 0);
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
            case REPEAT_MEASURE:
                  delete e;
                  if (durationType().type() == TDuration::V_MEASURE) {
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

int Rest::getSymbol(TDuration::DurationType type, int line, int lines, int* yoffset)
      {
      *yoffset = 2;
      switch(type) {
            case TDuration::V_LONG:
                  return longarestSym;
            case TDuration::V_BREVE:
                  return breverestSym;
            case TDuration::V_MEASURE:
                  if (duration() >= Fraction(2, 1))
                        return breverestSym;
                  // fall trough
            case TDuration::V_WHOLE:
                  *yoffset = 1;
                  return (line <= -2 || line >= (lines - 1)) ? outsidewholerestSym : wholerestSym;
            case TDuration::V_HALF:
                  return (line <= -3 || line >= (lines - 2)) ? outsidehalfrestSym : halfrestSym;
            case TDuration::V_QUARTER:
                  return rest4Sym;
            case TDuration::V_EIGHT:
                  return rest8Sym;
            case TDuration::V_16TH:
                  return rest16Sym;
            case TDuration::V_32ND:
                  return rest32Sym;
            case TDuration::V_64TH:
                  return rest64Sym;
            case TDuration::V_128TH:
                  return rest128Sym;
            case TDuration::V_256TH:
                  qDebug("Rest: no symbol for 1/256");
                  return rest128Sym;
            default:
                  qDebug("unknown rest type %d", type);
                  return rest4Sym;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Rest::layout()
      {
      _space.setLw(0.0);

      if (parent() && measure() && measure()->isMMRest()) {
            _space.setRw(point(score()->styleS(ST_minMMRestWidth)));
            return;
            }

      rxpos() = 0.0;
      if (staff() && staff()->isTabStaff()) {
            StaffTypeTablature* tab = (StaffTypeTablature*)staff()->staffType();
            // if rests are shown and note values are shown as duration symbols
            if(tab->showRests() && tab->genDurations()) {
                  TDuration::DurationType type = durationType().type();
                  int                     dots = durationType().dots();
                  // if rest is whole measure, convert into actual type and dot values
                  if (type == TDuration::V_MEASURE) {
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
            case TDuration::V_64TH:
            case TDuration::V_32ND:
                  dotline = -3;
                  break;
            case TDuration::V_256TH:
            case TDuration::V_128TH:
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
            rs = Spatium(score()->styleS(ST_dotNoteDistance)
               + dots() * score()->styleS(ST_dotDotDistance));
            }
      if (dots()) {
            rs = Spatium(score()->styleS(ST_dotNoteDistance)
               + dots() * score()->styleS(ST_dotDotDistance));
            }
      setbbox(symbols[score()->symIdx()][_sym].bbox(magS()));
      _space.setRw(width() + point(rs));
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
                  case TDuration::V_LONG:
                        lineOffset = up ? -3 : 5;
                        break;
                  case TDuration::V_BREVE:
                        lineOffset = up ? -3 : 5;
                        break;
                  case TDuration::V_MEASURE:
                        if (duration() >= Fraction(2, 1))    // breve symbol
                              lineOffset = up ? -3 : 5;
                        // fall through
                  case TDuration::V_WHOLE:
                        lineOffset = up ? -4 : 6;
                        break;
                  case TDuration::V_HALF:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TDuration::V_QUARTER:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TDuration::V_EIGHT:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TDuration::V_16TH:
                        lineOffset = up ? -6 : 4;
                        break;
                  case TDuration::V_32ND:
                        lineOffset = up ? -6 : 6;
                        break;
                  case TDuration::V_64TH:
                        lineOffset = up ? -8 : 6;
                        break;
                  case TDuration::V_128TH:
                        lineOffset = up ? -8 : 8;
                        break;
                  case TDuration::V_256TH:             // not available
                        lineOffset = up ? -10 : 6;
                        break;
                  default:
                        break;
                  }
            }
      else {
            switch(durationType().type()) {
                  case TDuration::V_LONG:
                  case TDuration::V_BREVE:
                  case TDuration::V_MEASURE:
                  case TDuration::V_WHOLE:
                        if (lines == 1)
                              lineOffset = -2;
                        break;
                  case TDuration::V_HALF:
                  case TDuration::V_QUARTER:
                  case TDuration::V_EIGHT:
                  case TDuration::V_16TH:
                  case TDuration::V_32ND:
                  case TDuration::V_64TH:
                  case TDuration::V_128TH:
                  case TDuration::V_256TH:             // not available
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
      return symbols[score()->symIdx()][_sym].width(magS())*.5;
      }

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Rest::upPos() const
      {
      return symbols[score()->symIdx()][_sym].bbox(magS()).y();
      }

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Rest::downPos() const
      {
      return symbols[score()->symIdx()][_sym].bbox(magS()).y() + symbols[score()->symIdx()][_sym].height(magS());
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
      Segment* s = segment();
      if (s && s->measure() && s->measure()->isMMRest()) {
            qreal _spatium = spatium();
            qreal h = _spatium * 2;
            qreal w = _mmWidth;                       // + 1*lineWidth of vertical lines
            bbox().setRect(0.0, -h * .5, w, h);
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Rest::reset()
      {
      score()->undoChangeProperty(this, P_BEAM_MODE, int(BeamMode::NONE));
      ChordRest::reset();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Rest::mag() const
      {
      qreal m = staff()->mag();
      if (small())
            m *= score()->styleD(ST_smallNoteMag);
      return m;
      }
}

