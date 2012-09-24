//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: beam.cpp 5656 2012-05-21 15:36:47Z wschweer $
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "beam.h"
#include "segment.h"
#include "score.h"
#include "chord.h"
#include "sig.h"
#include "style.h"
#include "note.h"
#include "tuplet.h"
#include "system.h"
#include "tremolo.h"
#include "measure.h"
#include "undo.h"
#include "staff.h"
#include "stafftype.h"
#include "stem.h"
#include "hook.h"
#include "mscore.h"
#include "icon.h"

//---------------------------------------------------------
//   BeamFragment
//    position of primary beam
//    idx 0 - MScore::AUTO or MScore::DOWN
//        1 - MScore::UP
//---------------------------------------------------------

struct BeamFragment {
      qreal py1[2];
      qreal py2[2];
      };

//---------------------------------------------------------
//   BeamHint
//    beam hint for autobeamer
//---------------------------------------------------------

struct BeamHint {
      Fraction noteLen;
      Fraction prevNoteLen; // zero = all notes
      Fraction timeSig;     // valid for this timesig; zero = valid for all
      Fraction pos;

      BeamHint(Fraction sig, Fraction p, Fraction len, Fraction prevLen)
         : noteLen(len), prevNoteLen(prevLen), timeSig(sig), pos(p) {}
      };

//---------------------------------------------------------
//   endBeam
//---------------------------------------------------------

static BeamHint endBeamList[] = {
      // in 2 2 time
      //  end beams each 1 2 note

      BeamHint(Fraction(2,2), Fraction(1,2), Fraction(0,0), Fraction(0,0)),

      // in 3 2 time:
      //   end beams each 1 2 note
      //   end beams with 16th notes each 1 4 note
      //   end beams with 32th notes each 1 8 note

      //       noteLen   timesig  position

      BeamHint(Fraction(3,2), Fraction(1,2), Fraction(0,0), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(2,2), Fraction(0,0), Fraction(0,0)),

      BeamHint(Fraction(3,2), Fraction(1,4), Fraction(1,16), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(1,2), Fraction(1,16), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(3,4), Fraction(1,16), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(1,1), Fraction(1,16), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(5,4), Fraction(1,16), Fraction(0,0)),

      BeamHint(Fraction(3,2), Fraction(1,8), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(1,4), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(3,8), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(1,2), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(5,8), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(3,4), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(7,8), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(1,1), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(9,8), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(5,4), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(3,2), Fraction(11,8),Fraction(1,32), Fraction(0,0)),

      BeamHint(Fraction(2,4), Fraction(0,0), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(2,4), Fraction(1,4), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(2,4), Fraction(1,9), Fraction(1,32), Fraction(0,0)),
      BeamHint(Fraction(2,4), Fraction(3,8), Fraction(1,32), Fraction(0,0)),

      BeamHint(Fraction(3,4), Fraction(1,4), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,2), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,4), Fraction(1,16),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,2), Fraction(1,16),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,4), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(3,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(1,2), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(3,4), Fraction(5,8), Fraction(1,32),  Fraction(0,0)),

      BeamHint(Fraction(12,16), Fraction(3,8),  Fraction(0, 0), Fraction(0,0)),
      BeamHint(Fraction(12,16), Fraction(3,16), Fraction(1,16), Fraction(0,0)),
      BeamHint(Fraction(12,16), Fraction(6,16), Fraction(1,8),  Fraction(0,0)),
      BeamHint(Fraction(12,16), Fraction(9,16), Fraction(1,8),  Fraction(0,0)),
      BeamHint(Fraction(12,16), Fraction(9,16), Fraction(1,16), Fraction(0,0)),

      BeamHint(Fraction(4,4), Fraction(1,2), Fraction(0,0),   Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(1,4), Fraction(1,12),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(3,4), Fraction(1,12),  Fraction(0,0)),

      BeamHint(Fraction(4,4), Fraction(1,4), Fraction(1,8),  Fraction(1,16)),  // ws
      BeamHint(Fraction(4,4), Fraction(2,4), Fraction(1,8),  Fraction(1,16)),  // ws
      BeamHint(Fraction(4,4), Fraction(3,4), Fraction(1,8),  Fraction(1,16)),  // ws

      BeamHint(Fraction(4,4), Fraction(1,4), Fraction(1,16),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(3,4), Fraction(1,16),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(1,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(1,4), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(3,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(5,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(3,4), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,4), Fraction(7,8), Fraction(1,32),  Fraction(0,0)),

      BeamHint(Fraction(5,4), Fraction(3,4), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(6,4), Fraction(3,4), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(3,8), Fraction(3,8), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(4,8), Fraction(0,0), Fraction(0,0),   Fraction(0,0)),
      BeamHint(Fraction(4,8), Fraction(1,4), Fraction(0,0),   Fraction(0,0)),
      BeamHint(Fraction(4,8), Fraction(1,8), Fraction(1,32),  Fraction(0,0)),
      BeamHint(Fraction(4,8), Fraction(3,8), Fraction(1,32),  Fraction(0,0)),

      BeamHint(Fraction(6,8), Fraction(3,8), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(9,8), Fraction(3,8), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(9,8), Fraction(3,4), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(12,8), Fraction(3,8), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(12,8), Fraction(3,4), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(12,8), Fraction(9,8), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(15,8), Fraction(3,8), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(15,8), Fraction(3,4), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(15,8), Fraction(9,8), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(15,8), Fraction(6,8), Fraction(0,0),  Fraction(0,0)),

      BeamHint(Fraction(4,16), Fraction(0,0), Fraction(0,0),  Fraction(0,0)),
      BeamHint(Fraction(4,16), Fraction(1,8), Fraction(0,0),  Fraction(0,0))
      };

//---------------------------------------------------------
//   endBeam
//    return true if beam should be ended
//---------------------------------------------------------

bool endBeam(const Fraction& ts, ChordRest* cr, ChordRest* prevCr)
      {
      int p = cr->tick() - cr->measure()->tick();
      if (cr->tuplet() && !cr->tuplet()->elements().isEmpty()) {
            if (cr->tuplet()->elements().front() == cr)     // end beam at tuplet
                  return true;
            return false;
            }
      Fraction l  = cr->duration();
      Fraction pl = prevCr ? prevCr->duration() : Fraction(0,1);
      for (unsigned i = 0; i < sizeof(endBeamList)/sizeof(*endBeamList); ++i) {
            const BeamHint& h = endBeamList[i];
            if (!h.timeSig.isZero() && (!h.timeSig.identical(ts)))
                  continue;
            if (!h.noteLen.isZero() && (h.noteLen != l))
                  continue;
            if (!h.prevNoteLen.isZero() && (h.prevNoteLen != pl))
                  continue;
            if (!h.pos.isZero()) {
                  int pos = h.pos.ticks();
                  if (pos != p)
                        continue;
                  }
            else {            // if (h.pos.numerator() == 0) {   // stop on every beat
                  int len = (4 * MScore::division) / h.timeSig.denominator();
                  if (p % len) {
                        continue;
                        }
                  }
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::Beam(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_SELECTABLE);
      _direction       = MScore::AUTO;
      _up              = true;
      _distribute      = false;
      _userModified[0] = false;
      _userModified[1] = false;
      _grow1           = 1.0;
      _grow2           = 1.0;
      editFragment     = 0;
      }

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::Beam(const Beam& b)
   : Element(b)
      {
      _elements     = b._elements;
      foreach(QLineF* bs, b.beamSegments)
            beamSegments.append(new QLineF(*bs));
      _direction       = b._direction;
      _up              = b._up;
      _distribute      = b._distribute;
      _userModified[0] = b._userModified[0];
      _userModified[1] = b._userModified[1];
      _grow1           = b._grow1;
      _grow2           = b._grow2;
      foreach(BeamFragment* f, b.fragments)
            fragments.append(new BeamFragment(*f));
      minMove          = b.minMove;
      maxMove          = b.maxMove;
      isGrace          = b.isGrace;
      cross            = b.cross;
      maxDuration      = b.maxDuration;
      slope            = b.slope;
      }

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::~Beam()
      {
      //
      // delete all references from chords
      //
      foreach(ChordRest* cr, _elements)
            cr->setBeam(0);
      qDeleteAll(beamSegments);
      qDeleteAll(fragments);
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF Beam::pagePos() const
      {
      System* system = static_cast<System*>(parent());
      if (system == 0)
            return pos();
      qreal yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Beam::add(ChordRest* a)
      {
      a->setBeam(this);
      if (!_elements.contains(a)) {
            //
            // insert element in same order as it appears
            // in the score
            //
            if (a->segment() && !_elements.isEmpty()) {
                  for (int i = 0; i < _elements.size(); ++i) {
                        Segment* s = _elements[i]->segment();
                        if ((s->tick() > a->segment()->tick())
                           || ((s->tick() == a->segment()->tick()) && (a->segment()->next(Segment::SegChordRest) == s))
                           )  {
                              _elements.insert(i, a);
                              return;
                              }
                        }
                  }
            _elements.append(a);
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Beam::remove(ChordRest* a)
      {
      if (!_elements.removeOne(a))
            qDebug("Beam::remove(): cannot find ChordRest");
      a->setBeam(0);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Beam::draw(QPainter* painter) const
      {
      if (staff()->isTabStaff()) {
            if (staff()->staffType()->slashStyle())
                  return;
            }
      painter->setBrush(QBrush(curColor()));
      painter->setPen(Qt::NoPen);
      qreal lw2 = point(score()->styleS(ST_beamWidth)) * .5 * mag();
      foreach (const QLineF* bs, beamSegments) {
            QPolygonF pg;
               pg << QPointF(bs->x1(), bs->y1()-lw2)
                  << QPointF(bs->x2(), bs->y2()-lw2)
                  << QPointF(bs->x2(), bs->y2()+lw2)
                  << QPointF(bs->x1(), bs->y1()+lw2);
            painter->drawPolygon(pg, Qt::OddEvenFill);
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Beam::move(qreal x, qreal y)
      {
      Element::move(x, y);
      foreach (QLineF* bs, beamSegments)
            bs->translate(x, y);
      }

//---------------------------------------------------------
//   writeMusicXml
//---------------------------------------------------------

// needed only for dump beam contents
// #include "rest.h"

void Beam::writeMusicXml(Xml& xml, ChordRest* cr) const
      {
/*
      qDebug("Beam::writeMusicXml(cr=%p)\n", cr);
      // dump beam contents
      foreach(ChordRest* crst, _elements) {
            if (crst->type() == CHORD) {
                  Chord* c = static_cast<Chord*>(crst);
                  qDebug(" chord %p tick=%d durtype=%d beams=%d\n", c, c->tick(), c->duration().type(), c->beams());
                  }
            else if (crst->type() == REST) {
                  Rest* r = static_cast<Rest*>(crst);
                  qDebug(" rest %p tick=%d durtype=%d beams=%d\n", r, r->tick(), r->duration().type(), r->beams());
                  }
            else {
                  qDebug(" type=%d %p tick=%d\n", crst->type(), crst, crst->tick());
                  }
            }
      // end dump beam contents
*/
      int idx = _elements.indexOf(cr);
      if (idx == -1) {
            qDebug("Beam::writeMusicXml(): cannot find ChordRest\n");
            return;
            }
      int blp = -1; // beam level previous chord
      int blc = -1; // beam level current chord
      int bln = -1; // beam level next chord
      // find beam level previous chord
      for (int i = idx - 1; blp == -1 && i >= 0; --i) {
            ChordRest* crst = _elements[i];
            if (crst->type() == CHORD)
                  blp = (static_cast<Chord*>(crst))->beams();
            }
      // find beam level current chord
      if (cr->type() == CHORD)
            blc = (static_cast<Chord*>(cr))->beams();
      // find beam level next chord
      for (int i = idx + 1; bln == -1 && i < _elements.size(); ++i) {
            ChordRest* crst = _elements[i];
            if (crst->type() == CHORD)
                  bln = (static_cast<Chord*>(crst))->beams();
            }
//      qDebug(" blp=%d blc=%d bln=%d\n", blp, blc, bln);
      for (int i = 1; i <= blc; ++i) {
            QString s;
            if (blp < i && bln >= i) s = "begin";
            else if (blp < i && bln < i) {
                  if (bln > 0) s = "forward hook";
                  else if (blp > 0) s = "backward hook";
                  }
            else if (blp >= i && bln < i) s = "end";
            else if (blp >= i && bln >= i) s = "continue";
            if (s != "")
                  xml.tag(QString("beam number=\"%1\"").arg(i), s);
            }
      }

//---------------------------------------------------------
//   twoBeamedNotes
//    calculate stem direction of two beamed notes
//    return true if two beamed notes found
//---------------------------------------------------------

bool Beam::twoBeamedNotes()
      {
      if ((_elements.size() != 2)
         || (_elements[0]->type() != CHORD)
         || _elements[1]->type() != CHORD) {
            return false;
            }
      const Chord* c1 = static_cast<const Chord*>(_elements[0]);
      const Chord* c2 = static_cast<const Chord*>(_elements[1]);
      if (c1->notes().size() != 1 || c2->notes().size() != 1)
            return false;
      int dist1 = c1->upNote()->line() - 4;
      int dist2 = c2->upNote()->line() - 4;
      if ((dist1 == -dist2) || (-dist1 == dist2)) {
            _up = false;
            Segment* s = c1->segment();
            s = s->prev1(Segment::SegChordRest);
            if (s && s->element(c1->track())) {
                  Chord* c = static_cast<Chord*>(s->element(c1->track()));
                  if ((c->type() == CHORD) && c->beam())
                        _up = c->beam()->up();
                  }
            }
      else if (qAbs(dist1) > qAbs(dist2))
            _up = dist1 > 0;
      else
            _up = dist2 > 0;
      return true;
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void Beam::layout1()
      {
      //delete old segments
      qDeleteAll(beamSegments);
      beamSegments.clear();

      maxDuration.setType(TDuration::V_INVALID);
      Chord* c1 = 0;
      Chord* c2 = 0;

      if (staff()->isTabStaff()) {
            //TABULATURES: all beams (and related chords) are:
            //    UP or DOWN according to TAB duration position
            //    slope 0
            _up   = !((StaffTypeTablature*)staff()->staffType())->stemsDown();
            slope = 0.0;
            cross = isGrace = false;
            foreach(ChordRest* cr, _elements) {
                  if (cr->type() == CHORD) {
                        // set members maxDuration, c1, c2
                        if (!maxDuration.isValid() || (maxDuration < cr->durationType()))
                              maxDuration = cr->durationType();
                        c2 = static_cast<Chord*>(cr);
                        if (c2->noteType() != NOTE_NORMAL)
                              isGrace = true;
                        if (c1 == 0)
                              c1 = c2;
                        }
                  }
            }
      else {
            //PITCHED STAVES
            minMove = 1000;
            maxMove = -1000;
            isGrace = false;

            int upCount = 0;
            int mUp     = 0;
            int mDown   = 0;

            foreach(ChordRest* cr, _elements) {
                  if (cr->type() == CHORD) {
                        c2 = static_cast<Chord*>(cr);
                        if (c2->line() != 4)
                              upCount += c2->up() ? 1 : -1;
                        if (c2->noteType() != NOTE_NORMAL)
                              isGrace = true;
                        if (c1 == 0)
                              c1 = c2;
                        int i = c2->staffMove();
                        if (i < minMove)
                              minMove = i;
                        if (i > maxMove)
                              maxMove = i;
                        int line = c2->upNote()->line();
                        if ((line - 4) > mUp)
                              mUp = line - 4;
                        line = c2->downNote()->line();
                        if (4 - line > mDown)
                              mDown = 4 - line;
                        }
                  if (!maxDuration.isValid() || (maxDuration < cr->durationType()))
                        maxDuration = cr->durationType();
                  }
            //
            // determine beam stem direction
            //
            if (_direction != MScore::AUTO) {
                  _up = _direction == MScore::UP;
                  }
            else {
                  ChordRest* cr = _elements[0];
                  Measure* m = cr->measure();
                  if (m->hasVoices(cr->staffIdx())) {
                        switch(cr->voice()) {
                              case 0:  _up = (score()->style(ST_stemDir1).toDirection() == MScore::UP); break;
                              case 1:  _up = (score()->style(ST_stemDir2).toDirection() == MScore::UP); break;
                              case 2:  _up = (score()->style(ST_stemDir3).toDirection() == MScore::UP); break;
                              case 3:  _up = (score()->style(ST_stemDir4).toDirection() == MScore::UP); break;
                              }
                        }
                  else if (!twoBeamedNotes()) {
                        // highest or lowest note determines stem direction
                        // down-stems is preferred if equal
                        _up = mUp > mDown;
                        }
                  }

            cross   = minMove < maxMove;
            int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
            slope   = 0.0;

            if (cross || _userModified[idx]) {
                  //
                  // guess stem direction for every chord
                  //
#if 0
                  foreach(ChordRest* cr, _elements) {
                        if (cr->type() != CHORD)
                              continue;
                        Chord* c  = static_cast<Chord*>(cr);
                        int move = c->staffMove();
                        if (move == 0)
                              c->setUp(maxMove ? false : true);
                        else if (move > 0)
                              c->setUp(true);
                        else if (move < 0)
                              c->setUp(false);
                        }
#endif
                  }
            else {
                  foreach(ChordRest* cr, _elements)
                        cr->setUp(_up);
                  }
            }     // end of if/else(tablature)
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Beam::layout()
      {
      System* system = _elements.front()->measure()->system();
      setParent(system);

      QList<ChordRest*> crl;

      int n = 0;
      foreach(ChordRest* cr, _elements) {
            if (cr->measure()->system() != system) {
                  SpannerSegmentType st;
                  if (n == 0)
                        st = SEGMENT_BEGIN;
                  else
                        st = SEGMENT_MIDDLE;
                  ++n;
                  if (fragments.size() < n)
                        fragments.append(new BeamFragment);
                  layout2(crl, st, n-1);
                  crl.clear();
                  system = cr->measure()->system();
                  }
            crl.append(cr);
            }
      if (!crl.isEmpty()) {
            SpannerSegmentType st;
            if (n == 0)
                  st = SEGMENT_SINGLE;
            else
                  st = SEGMENT_END;
            if (fragments.size() < (n+1))
                  fragments.append(new BeamFragment);
            layout2(crl, st, n);
            }

      setbbox(QRectF());
      qreal lw2 = point(score()->styleS(ST_beamWidth)) * .5 * mag();
      foreach(const QLineF* bs, beamSegments) {
            QPolygonF a(4);
            a[0] = QPointF(bs->x1(), bs->y1()-lw2);
            a[1] = QPointF(bs->x2(), bs->y2()-lw2);
            a[2] = QPointF(bs->x2(), bs->y2()+lw2);
            a[3] = QPointF(bs->x1(), bs->y1()+lw2);
            addbbox(a.boundingRect());
            }
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath Beam::shape() const
      {
      QPainterPath pp;
      qreal lw2 = point(score()->styleS(ST_beamWidth)) * .5 * mag();
      foreach(const QLineF* bs, beamSegments) {
            QPolygonF a(5);
            a[0] = QPointF(bs->x1(), bs->y1()-lw2);
            a[1] = QPointF(bs->x2(), bs->y2()-lw2);
            a[2] = QPointF(bs->x2(), bs->y2()+lw2);
            a[3] = QPointF(bs->x1(), bs->y1()+lw2);
            a[4] = QPointF(bs->x1(), bs->y1()-lw2);
            pp.addPolygon(a);
            }
      return pp;
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

bool Beam::contains(const QPointF& p) const
      {
      return shape().contains(p - pagePos());
      }

//---------------------------------------------------------
//   absLimit
//---------------------------------------------------------

inline qreal absLimit(qreal val, qreal limit)
      {
      if (val > limit)
            return limit;
      if (val < -limit)
            return -limit;
      return val;
      }

//---------------------------------------------------------
//   noSlope
//---------------------------------------------------------

bool Beam::noSlope(const QList<Chord*>& cl)
      {
      if (cl.size() < 2)
            return true;

      int l1 = cl.front()->line();
      int le = cl.back()->line();

      // look for some pattern
      if (cl.size() == 4) {
            int l2 = cl[1]->line();
            int l3 = cl[2]->line();

            if ((l1 < le) && (l2 > l1) && (l2 > l3) && (l3 > le)) {
                  return true;
                  }
            if ((l1 == l3) && (l2 == le))
                  return true;
            }
      else if (cl.size() == 6) {
            int l2 = cl[1]->line();
            int l3 = cl[2]->line();
            int l4 = cl[3]->line();
            int l5 = cl[4]->line();
            if ((l2 > l1) && (l3 > l2) && (l1 == l4) && (l2 == l5) && (l3 == le))
                  return true;
            }
      //
      //    concave beams have a slope of 0.0
      //
      bool sameLine = true;

      slope = 0.0;
      if (cl.size() >= 3) {
            int l4 = cl[1]->line(_up);
            for (int i = 1; i < cl.size()-1; ++i) {
                  int l3 = cl[i]->line(_up);
                  if (l3 != l4)
                        sameLine = false;
                  if (_up) {
                        if (l3 < l1 && l3 < le)
                              return true;
                        }
                  else {
                        if (l3 > l1 && l3 > le)
                              return true;
                        }
                  }
            if (sameLine && (l1 == l4 || le == l4)) {
                  if (_up) {
                        if (l1 == l4 && l1 < le)
                              return true;
                        if (le == l4 && le < l1)
                              return true;
                        }
                  else {
                        if (l1 == l4 && l1 > le)
                              return true;
                        else if (le == l4 && le > l1)
                              return true;
                        }
                  }
            }
      return l1 == le;
      }

//---------------------------------------------------------
//   BeamMetric
//---------------------------------------------------------

struct Bm
      {
      char l;     // stem len   in 1/4 spatium units
      char s;     // beam slant in 1/4 spatium units
      Bm() : l(0), s(0) {}
      Bm(char a, char b) : l(a), s(b) {}
      static int key(int a, int b, int c) { return ((a & 0xff) << 16) | ((b & 0xff) << 8) | (c & 0xff); }
      };

static QHash<int, Bm> bMetrics;

//---------------------------------------------------------
//   initBeamMetrics
//---------------------------------------------------------

#define B(a,b,c,d,e) bMetrics[Bm::key(a, b, c)] = Bm(d, e);

static void initBeamMetrics()
      {
      // up  step1 step2 stemLen1 slant
      //                 (- up)   (- up)
      // =================================== C
      B(1,  10, 10, -12,  0);
      B(0,   3,  3,  11,  0);
      B(1,   3,  3, -11,  0);

      B(1,  10,  9, -12, -1);
      B(1,  10,  8, -12, -4);
      B(1,  10,  7, -12, -5);
      B(1,  10,  6, -15, -5);
      B(1,  10,  5, -16, -5);
      B(1,  10,  4, -20, -4);
      B(1,  10,  3, -20, -5);

      B(1,  10,  11, -12, 1);
      B(1,  10,  12, -13, 2);      // F
      B(1,  10,  13, -13, 2);
      B(1,  10,  14, -13, 2);
      B(1,  10,  15, -13, 2);

      B(1,  3,  4, -11, 1);
      B(1,  3,  5, -11, 2);
      B(1,  3,  6, -11, 4);
      B(1,  3,  7, -11, 5);
      B(1,  3,  8, -11, 5);
      B(1,  3,  9, -11, 5);
      B(1,  3, 10, -11, 5);

      B(0, -4, -3,  15, 1);
      B(0, -4, -2,  15, 2);
      B(0, -4, -1,  15, 2);
      B(0, -4,  0,  15, 5);
      B(0, -4,  1,  16, 5);
      B(0, -4,  2,  20, 4);
      B(0, -4,  3,  20, 5);

      B(0,  3,  4,  13, 1);
      B(0,  3,  5,  13, 2);
      B(0,  3,  6,  14, 4);
      B(0,  3,  7,  13, 4);
      B(0,  3,  8,  13, 6);

      B(0,  3,  2,  11, -1);
      B(0,  3,  1,  11, -2);
      B(0,  3,  0,  11, -5);
      B(0,  3, -1,  11, -5);
      B(0,  3, -2,  11, -5);
      B(0,  3, -3,  11, -5);
      B(0,  3, -4,  11, -5);

      // =================================== D
      B(1,  9,  9,  -13, 0);
      B(0,  2,  2,   12, 0);
      B(1,  2,  2,  -11, 0);

      B(1,  9,  8,  -13, -1);
      B(1,  9,  7,  -13, -2);
      B(1,  9,  6,  -13, -5);
      B(1,  9,  5,  -14, -5);
      B(1,  9,  4,  -16, -6);
      B(1,  9,  3,  -17, -5);
      B(1,  9,  2,  -17, -8);

      B(1,  9, 10,  -11, 1);
      B(1,  9, 11,  -11, 2);
      B(1,  9, 12,  -11, 2);
      B(1,  9, 13,  -11, 2);
      B(1,  9, 14,  -11, 2);
      B(1,  9, 15,  -11, 2);

      B(1,  2, 3,   -12, 1);
      B(1,  2, 4,   -12, 2);
      B(1,  2, 5,   -12, 4);
      B(1,  2, 6,   -12, 5);
      B(1,  2, 7,   -11, 5);
      B(1,  2, 8,   -12, 5);
      B(1,  2, 9,   -12, 8);

      B(0, -5,-4,   16, 2);
      B(0, -5,-3,   16, 2);
      B(0, -5,-2,   16, 2);
      B(0, -5,-1,   16, 2);
      B(0, -5, 0,   16, 4);
      B(0, -5, 1,   16, 5);
      B(0, -5, 2,   16, 5);

      B(0,  2, 3,   12, 1);
      B(0,  2, 4,   12, 4);
      B(0,  2, 5,   13, 4);  // F
      B(0,  2, 6,   15, 5);
      B(0,  2, 7,   13, 6);
      B(0,  2, 8,   16, 8);
      B(0,  2, 9,   16, 8);

      B(0,  2,  1,   12, -1);
      B(0,  2,  0,   12, -4);
      B(0,  2, -1,   12, -5);
      B(0,  2, -2,   12, -5);
      B(0,  2, -3,   12, -4);
      B(0,  2, -4,   12, -4);
      B(0,  2, -5,   12, -5);

      // =================================== E
      B(1, 8, 8,  -12, 0);
      B(0, 1, 1,   13, 0);
      B(1, 1, 1,   -9, 0);

      B(1, 8, 7, -12, -1);
      B(1, 8, 6, -12, -4);
      B(1, 8, 5, -12, -5);
      B(1, 8, 4, -15, -5);
      B(1, 8, 3, -16, -5);
      B(1, 8, 2, -17, -6);
      B(1, 8, 1, -19, -6);

      B(1, 15, 11, -21, -1);
      B(1, 15, 10, -21, -1);
      B(1, 15,  9, -21, -1);
      B(1, 15,  8, -21, -1);

      B(1,  1,  8, -11,  6);
      B(1,  1,  7, -11,  6);
      B(1,  1,  6, -12,  6);

      B(1,  8,  9, -12,  1);
      B(1,  8, 10, -12,  4);
      B(1,  8, 11, -12,  5);
      B(1,  8, 12, -12,  5);
      B(1,  8, 13, -12,  4);
      B(1,  8, 14, -12,  5);
      B(1,  8, 15, -12,  1);

      B(0,  1,  0, 11,  -1);
      B(0,  1, -1, 11,  -2);
      B(0,  1, -2, 11,  -5);
      B(0,  1, -3, 11,  -5);
      B(0,  1, -4, 11,  -5);
      B(0,  1, -5, 11,  -5);
      B(0,  1, -6, 11,  -5);

      B(0, 1, 2, 13, 1);
      B(0, 1, 3, 13, 2);
      B(0, 1, 4, 13, 5);
      B(0, 1, 5, 14, 5);
      B(0, 1, 6, 15, 5);
      B(0, 1, 7, 17, 5);
      B(0, 1, 8, 17, 8);

      B(0, -6, -2,  19, 2);
      B(0, -6, -1,  19, 4);
      B(0, -6,  0,  20, 4);
      B(0, -6,  1,  20, 5);

      B(0, 8, 3, 9,  -6);
      B(0, 8, 2, 12, -8);
      B(0, 8, 1, 12, -8);

      // =================================== F
      B(1, 7, 7,-13, 0);      //F
      B(0, 0, 0, 12, 0);
      B(0, 7, 7, 10, 0);

      B(1, 7, 6, -13, -1);
      B(1, 7, 5, -13, -2);
      B(1, 7, 4, -13, -5);
      B(1, 7, 3, -14, -5);
      B(1, 7, 2, -15, -6);
      B(1, 7, 1, -17, -6);
      B(1, 7, 0, -18, -8);

      B(1, 14, 10, -19, -2);
      B(1, 14,  9, -19, -2);
      B(1, 14,  8, -20, -4);
      B(1, 14,  7, -20, -5);

      B(1,  0,  5,  -9, 6);
      B(1,  0,  6, -12, 8);
      B(1,  0,  7, -12, 8);

      B(1, 7,  8, -11, 1);
      B(1, 7,  9, -11, 2);
      B(1, 7, 10, -11, 5);
      B(1, 7, 11, -11, 5);
      B(1, 7, 12, -11, 5);
      B(1, 7, 13, -11, 5);
      B(1, 7, 14, -11, 5);

      B(0, 0, -1, 12, -1);
      B(0, 0, -2, 12, -4);
      B(0, 0, -3, 12, -5);
      B(0, 0, -4, 12, -5);
      B(0, 0, -5, 12, -4);
      B(0, 0, -6, 12, -4);
      B(0, 0, -7, 12, -4);

      B(0, 0, 1, 12, 1);
      B(0, 0, 2, 12, 4);
      B(0, 0, 3, 12, 5);
      B(0, 0, 4, 15, 5);
      B(0, 0, 5, 16, 5);
      B(0, 0, 6, 17, 5);
      B(0, 0, 7, 19, 6);

      B(0, -7, -3, 21, 2);
      B(0, -7, -2, 21, 2);
      B(0, -7, -1, 21, 2);
      B(0, -7,  0, 22, 4);

      B(0, 7, 2, 12, -6);
      B(0, 7, 1, 11, -6);
      B(0, 7, 0, 11, -6);

      // =================================== G
      B(1,  6,  6, -12, 0);
      B(0, -1, -1,  13, 0);
      B(0,  6,  6,  11, 0);

      B(1, 6,  5, -12, -1);
      B(1, 6,  4, -12, -4);
      B(1, 6,  3, -13, -4);
      B(1, 6,  2, -15, -5);
      B(1, 6,  1, -13, -7);
      B(1, 6,  0, -16, -8);
      B(1, 6, -1, -16, -8);

      B(1, 13, 10, -17, -2);
      B(1, 13,  9, -17, -2);
      B(1, 13,  8, -18, -4);
      B(1, 13,  7, -18, -5);
      B(1, 13,  6, -21, -5);

      B(1, -1, 6, -10, 8);

      B(1, 6,  7, -12, 1);
      B(1, 6,  8, -12, 4);
      B(1, 6,  9, -12, 5);
      B(1, 6, 10, -12, 5);
      B(1, 6, 11, -12, 4);
      B(1, 6, 12, -12, 5);
      B(1, 6, 13, -12, 5);

      B(0, -1, -2, 11, -1);
      B(0, -1, -3, 11, -2);
      B(0, -1, -4, 11, -2);
      B(0, -1, -5, 11, -2);
      B(0, -1, -6, 11, -2);
      B(0, -1, -7, 11, -2);

      B(0, -1,  0, 13, 1);
      B(0, -1,  1, 13, 2);
      B(0, -1,  2, 13, 5);
      B(0, -1,  3, 14, 5);
      B(0, -1,  4, 17, 6);
      B(0, -1,  5, 18, 5);
      B(0, -1,  6, 18, 8);

      B(0,  6,  5, 12, -4);
      B(0,  6,  4, 12, -4);
      B(0,  6,  3, 12, -4);
      B(0,  6,  2, 12, -6);
      B(0,  6,  1, 11, -6);
      B(0,  6,  0, 12, -7);
      B(0,  6, -1, 12, -8);

      // =================================== A
      B(1,  5,  5, -11, 0);
      B(0, -2, -2,  12, 0);
      B(0,  5,  5,  11, 0);

      B(1,  5,  4, -13, -1);
      B(1,  5,  3, -13, -2);
      B(1,  5,  2, -14, -4);
      B(1,  5,  1, -14, -4);
      B(1,  5,  0, -13, -6);

      B(1, 12, 11, -15, -1);
      B(1, 12, 10, -15, -2);
      B(1, 12,  9, -15, -2);
      B(1, 12,  8, -15, -5);
      B(1, 12,  7, -16, -5);
      B(1, 12,  6, -20, -4);
      B(1, 12,  5, -20, -5);

      B(1,  5,  6, -11,  1);
      B(1,  5,  7, -11,  2);
      B(1,  5,  8, -11,  5);
      B(1,  5,  9, -11,  5);
      B(1,  5, 10, -11,  5);
      B(1,  5, 11, -11,  5);
      B(1,  5, 12, -11,  5);

      B(0, -2, -1, 12, 1);
      B(0, -2,  0, 12, 4);
      B(0, -2,  1, 12, 5);
      B(0, -2,  2, 15, 5);
      B(0, -2,  3, 16, 5);
      B(0, -2,  4, 20, 4);
      B(0, -2,  5, 20, 5);

      B(0, -2, -3, 12, -1);
      B(0, -2, -4, 13, -2);
      B(0, -2, -5, 13, -2);
      B(0, -2, -6, 13, -2);
      B(0, -2, -7, 13, -2);

      B(0,  5,  4, 11, -1);
      B(0,  5,  3, 11, -2);
      B(0,  5,  2, 11, -4);
      B(0,  5,  1, 11, -5);
      B(0,  5,  0, 11, -5);
      B(0,  5, -1, 11, -5);
      B(0,  5, -2, 11, -5);

      // =================================== B
      B(1,  4,  4, -12, 0);
      B(1, 11, 11, -13, 0);
      B(0,  4,  4,  12, 0);
      B(0, -3, -3,  13, 0);

      B(1, 11, 10, -13, -1);
      B(1, 11,  9, -13, -2);
      B(1, 11,  8, -13, -5);
      B(1, 11,  7, -14, -5);
      B(1, 11,  6, -18, -4);
      B(1, 11,  5, -18, -5);
      B(1, 11,  4, -21, -5);

      B(1,  4,  3, -12, -1);
      B(1,  4,  2, -12, -4);
      B(1,  4,  1, -14, -4);
      B(1,  4,  0, -16, -4);

      B(1, 11, 12, -14, 1);
      B(1, 11, 13, -14, 1);
      B(1, 11, 14, -14, 1);
      B(1, 11, 15, -15, 2);
      B(1, 11, 16, -15, 2);

      B(1,  4,  5, -12, 1);
      B(1,  4,  6, -12, 4);
      B(1,  4,  7, -12, 5);
      B(1,  4,  8, -12, 5);
      B(1,  4,  9, -13, 6);
      B(1,  4, 10, -12, 4);
      B(1,  4, 11, -12, 5);

      B(0,  4,  3, 12, -1);
      B(0,  4,  2, 12, -4);
      B(0,  4,  1, 12, -5);
      B(0,  4,  0, 12, -5);
      B(0,  4, -1, 13, -6);
      B(0,  4, -2, 12, -4);
      B(0,  4, -3, 12, -5);

      B(0,  4,  5, 12, 1);
      B(0,  4,  6, 12, 4);

      B(0, -3, -4, 14, -1);
      B(0, -3, -5, 14, -1);
      B(0, -3, -6, 14, -1);
      B(0, -3, -7, 15, -2);
      B(0, -3, -8, 15, -2);
      B(0, -3, -9, 15, -2);

      B(0, -3, -2, 13, 1);
      B(0, -3, -1, 13, 2);
      B(0, -3,  0, 13, 5);
      B(0, -3,  1, 14, 5);
      B(0, -3,  2, 18, 4);
      B(0, -3,  3, 18, 5);
      B(0, -3,  4, 21, 5);
      }

//---------------------------------------------------------
//   beamMetric1
//    table driven
//---------------------------------------------------------

static Bm beamMetric1(bool up, char l1, char l2)
      {
      static int initialized = false;
      if (!initialized) {
            initBeamMetrics();
            initialized = true;
            }
      return bMetrics[Bm::key(up, l1, l2)];
      }

//---------------------------------------------------------
//   adjust
//    adjust stem len for notes between start-end
//---------------------------------------------------------

static int adjust(qreal _spatium4, int slant, const QList<Chord*>& cl)
      {
      int n           = cl.size();
      const Chord* c1 = cl[0];
      const Chord* c2 = cl[n-1];

      QPointF p1(c1->stemPosBeam());   // canvas coordinates
      qreal slope = (slant * _spatium4) / (c2->stemPosBeam().x() - p1.x());
      int ml = -1000;
      if (c1->up()) {
            for (int i = 1; i < n; ++i) {
                  QPointF p3(cl[i]->stemPosBeam());
                  qreal yUp   = p1.y() + (p3.x() - p1.x()) * slope;
                  int l       = lrint((yUp - p3.y()) / _spatium4);
                  ml          = qMax(ml, l);
                  }
            }
      else {
            for (int i = 1; i < n; ++i) {
                  const Chord* c3 = cl[i];
                  QPointF p3(c3->stemPosBeam());
                  qreal yUp   = p1.y() + (p3.x() - p1.x()) * slope;
                  int l       = lrint((p3.y() - yUp) / _spatium4);
                  ml          = qMax(ml, l);
                  }
            }
      return (ml > 0) ? ml : 0;
      }

//---------------------------------------------------------
//   adjust2
//    adjust stem position for single beams
//---------------------------------------------------------

static void adjust2(Bm& bm, const Chord* c1)
      {
      static const int dd[4][4] = {
            // St   H  --   S
            {0,  0,  1,  0},     // St
            {0,  0, -1,  0},     // S
            {1,  1,  1, -1},     // --
            {0,  0, -1,  0}      // H
            };
      int ys = bm.l + c1->line() * 2;
      int e1 = qAbs((ys  + 1000) % 4);
      int e2 = qAbs((ys + 1000 + bm.s) % 4);
      bm.l  -= dd[e1][e2];
      }

//---------------------------------------------------------
//   minSlant
//---------------------------------------------------------

static int minSlant(uint interval)
      {
      static const int minSlantTable[] = { 0, 1, 2, 4, 5 };
      if (interval > 4)
            return 5;
      return minSlantTable[interval];
      }

//---------------------------------------------------------
//   maxSlant
//---------------------------------------------------------

static int maxSlant(uint interval)
      {
      static const int maxSlantTable[] = { 0, 1, 4, 5, 5, 6, 7, 8 };
      if (interval > 7)
            return 8;
      return maxSlantTable[interval];
      }

//---------------------------------------------------------
//   slantTable
//---------------------------------------------------------

static int* slantTable(uint interval)
      {
      static int t[8][5] = {
            { 0, -1,  0,  0,  0 },
            { 1, -1,  0,  0,  0 },
            { 3,  4,  2, -1,  0 },
            { 4,  5, -1,  0,  0 },
            { 5, -1,  0,  0,  0 },
            { 5,  6, -1,  0,  0 },
            { 6,  5,  7, -1,  0 },
            { 6,  7,  5,  8, -1 },
            };
      if (interval > 7)
            interval = 7;
      return &t[interval][0] ;
      }

//---------------------------------------------------------
//   computeStemLen
//---------------------------------------------------------

void Beam::computeStemLen(const QList<Chord*>& cl, qreal& py1, int beamLevels)
      {
      qreal _spatium  = spatium();
      qreal _spatium4 = _spatium * .25;
      const Chord* c1 = cl.front();
      const Chord* c2 = cl.back();
      qreal dx        = c2->pagePos().x() - c1->pagePos().x();
      bool zeroSlant  = noSlope(cl);

      int l1 = c1->line() * 2;
      int l2 = c2->line() * 2;

      Bm bm;
      if (beamLevels == 1) {
            bm = beamMetric1(_up, l1 / 2, l2 / 2);
            if (bm.l && !(zeroSlant && cl.size() > 2)) {
                  if (cl.size() > 2) {
                        if (_up)
                              bm.l = -12 - adjust(_spatium4, bm.s, cl);
                        else
                              bm.l = 12 + adjust(_spatium4, bm.s, cl);
                        adjust2(bm, c1);
                        }
                  }
            else {
                  int* st = slantTable(zeroSlant ? 0 : qAbs((l2 - l1) / 2));
                  int ll1;
                  if (_up) {
                        ll1 = l1 - ((l1 & 3) ? 11 : 12);
                        int ll1m = l1 - 10;
                        int rll1 = ll1;
                        if ((l1 > 20) && (l2 > 20)) {
                              st = slantTable(zeroSlant ? 0 : 1);
                              rll1 = (zeroSlant || (l2 < l1)) ? 9 : 8;
                              }
                        for (int n = 0; ; ll1--) {
                              int i;
                              for (i = 0; st[i] != -1; ++i) {
                                    int slant = (l2 > l1) ? st[i] : -st[i];
                                    int lll1  = qMin(rll1, ll1m - n - adjust(_spatium4, slant, cl));
                                    int ll2   = lll1 + slant;
                                    static bool ba[4][4] = {
                                          { true,  true,  false, true },
                                          { true,  true,  false, true },
                                          { false, false, false, true },
                                          { true,  true,  false, true }
                                          };
                                    if (ba[lll1 & 3][ll2 & 3]) {
                                          ll1 = lll1;
                                          bm.s = slant;
                                          break;
                                          }
                                    }
                              if (st[i] != -1)
                                    break;
                              if (++n > 4) {
                                    printf("beam note not found 1\n");
                                    break;
                                    }
                              }
                        }
                  else {
                        ll1 = ((l1 & 3) ? 11 : 12) + l1;
                        int rll1 = ll1;
                        if ((l1 < -4) && (l2 < -4)) {
                              // extend to middle line, slant is always 0 <= 1
                              st = slantTable(zeroSlant ? 0 : 1);
                              rll1 = (zeroSlant || (l2 > l1)) ? 7 : 8;
                              }
                        for (int n = 0;;ll1++) {
                              int i;
                              for (i = 0; st[i] != -1; ++i) {
                                    int slant = (l2 > l1) ? st[i] : -st[i];
                                    int lll1  = qMax(rll1, ll1 + adjust(_spatium4, slant, cl));
                                    int e1    = lll1 & 3;
                                    int ll2   = lll1 + slant;
                                    int e2    = ll2 & 3;
                                    static bool ba[4][4] = {
                                          { true,  true,  false, true },
                                          { true,  true,  false, true },
                                          { false, false, false, true },
                                          { true,  true,  false, true }
                                          };
                                    if (ba[e1][e2]) {
                                          ll1 = lll1;
                                          bm.s = slant;
                                          break;
                                          }
                                    }
                              if (st[i] != -1)
                                    break;
                              if (++n > 4) {
                                    printf("beam not found 2\n");
                                    break;
                                    }
                              }
                        }
                  bm.l = ll1 - l1;
                  }
            }
      else if (beamLevels == 2) {
            int minS, maxS;
            if (zeroSlant)
                  minS = maxS = 0;
            else {
                  uint interval = qAbs((l2 - l1) / 2);
                  minS          = minSlant(interval);
                  maxS          = maxSlant(interval);
                  }
            int ll1;
            if (_up) {
                  ll1 = l1 - 12;     // sp minimum to primary beam
                  int rll1 = ll1;
                  if ((l1 > 20) && (l2 > 20)) {
                        minS = zeroSlant ? 0 : 1;
                        maxS = minS;
                        rll1 = (zeroSlant || (l2 < l1)) ? 9 : 8;
                        }
                  for (int n = 0; ; ll1--) {
                        int i;
                        for (i = minS; i <= maxS; ++i) {
                              int slant = (l2 > l1) ? i : -i;
                              int lll1  = qMin(rll1, ll1 - adjust(_spatium4, slant, cl));
                              int ll2   = lll1 + slant;
                              static bool ba[4][4] = {
                                    { true,  true,  false, false  },
                                    { true,  true,  false, false },
                                    { false, false, false, false },
                                    { false, false, false, false }
                                    };
                              if (ba[lll1 & 3][ll2 & 3]) {
                                    ll1 = lll1;
                                    break;
                                    }
                              }
                        if (i <= maxS) {
                              bm.s = l2 > l1 ? i : -i;
                              break;
                              }
                        if (++n > 4) {
                              printf("beam note not found 1 %d-%d\n", minS, maxS);
                              break;
                              }
                        }
                  }
            else {
                  ll1       = 12 + l1;
                  int rll1  = ll1;
                  bool down = l2 > l1;
                  if ((l1 < -4) && (l2 < -4)) {
                        // extend to middle line, slant is always 0 <= 1
                        minS = zeroSlant ? 0 : 1;
                        maxS = minS;
                        rll1 = (zeroSlant || down) ? 7 : 8;
                        }
                  for (int n = 0;;ll1++) {
                        int i;
                        for (i = minS; i <= maxS; ++i) {
                              int slant = down ? i : -i;
                              int lll1  = qMax(rll1, ll1 + adjust(_spatium4, slant, cl));
                              int ll2   = lll1 + slant;
                              static bool ba[4][4] = {
                                    { true,  false, false, true  },
                                    { false, false, false, false },
                                    { false, false, false, false },
                                    { true,  false, false, true }
                                    };
                              if (ba[lll1 & 3][ll2 & 3]) {
                                    ll1 = lll1;
                                    bm.s = slant;
                                    break;
                                    }
                              }
                        if (i <= maxS)
                              break;
                        if (++n > 4) {
                              printf("beam not found 2\n");
                              break;
                              }
                        }
                  }
            bm.l = ll1 - l1;
            }
      else if (beamLevels == 3) {
            int slant;
            bool outside;
            if (zeroSlant) {
                  outside = (_up && qMin(l1, l2) <= 10) ||
                     (!_up && qMax(l1, l2) >= 6);
                  slant = 0;
                  }
            else {
                  outside = (_up && (l1 <= 10) && (l2 <= 10)) ||
                     (!_up && (l1 >= 6) && (l2 >= 6));
                  if (outside)
                        slant = *slantTable(qAbs(l1-l2) / 2);
                  else
                        slant = 4;
                  if (l1 > l2)
                        slant = -slant;
                  }
            int ll1;
            if (_up) {
                  static const int t[4] = { 3, 0, 1, 2 };
                  ll1 = l1 - 15 - adjust(_spatium4, slant, cl);
                  ll1 = qMin(ll1, 5);
                  if (!outside)
                        ll1 -= t[ll1 & 3];      // extend to sit on line
                  }
            else {
                  ll1 = 15 + l1 + adjust(_spatium4, slant, cl);
                  ll1 = qMax(ll1, 11);
                  if (!outside)
                        ll1 += 3 - (ll1 & 3);   // extend to hang on line
                  }
            bm.s = slant;
            bm.l = ll1 - l1;
            }
      else if (beamLevels == 4) {
            int slant = zeroSlant ? 0 : (l2 > l1 ? 4 : -4);
            int ll1;
            if (_up) {
                  ll1 = l1 - 17 - adjust(_spatium4, slant, cl);
                  ll1 = qMin(ll1, 1);
                  static const int t[4] = { 3, 0, 1, 2 };
                  ll1 -= t[ll1 & 3];      // extend to sit on line
                  }
            else {
                  ll1 = 17 + l1 + adjust(_spatium4, slant, cl);
                  ll1 = qMax(ll1, 15);
                  ll1 += 3 - (ll1 & 3);   // extend to hang on line
                  }
            bm.s = slant;
            bm.l = ll1 - l1;
            }
      else { // if (beamLevels > 4) {
            static const int t[] = { 0, 0, 4, 4, 8, 12, 16 }; // spatium4 added to stem len
            int n = t[beamLevels] + 12;
            bm.s = 0;
            if (_up) {
                  bm.l = -n;
                  bm.l -= adjust(_spatium4, bm.s, cl);
                  }
            else {
                  bm.l += n;
                  bm.l += adjust(_spatium4, bm.s, cl);
                  }
            }
      slope   = (bm.s * _spatium4) / dx;
      py1 += ((c1->line(_up) - c1->line(!_up)) * 2 + bm.l) * _spatium4;
      }

//---------------------------------------------------------
//   layout2
//---------------------------------------------------------

void Beam::layout2(QList<ChordRest*>crl, SpannerSegmentType, int frag)
      {
      if (_distribute)
            score()->respace(&crl);       // fix horizontal spacing of stems

      QList<Chord*> cl;
      foreach(ChordRest* cr, crl) {
            if (cr->type() == CHORD)
                  cl.append(static_cast<Chord*>(cr));
            }
      if (cl.isEmpty())                   // no chords?
            return;
      const Chord* c1 = cl.front();       // first chord in beam
      const Chord* c2 = cl.back();        // last chord in beam

      int beamLevels = 1;
      foreach(Chord* c, cl) {
            int bl        = c->durationType().hooks();
            beamLevels    = qMax(beamLevels, bl);
            }

      BeamFragment* f = fragments[frag];
      int dIdx        = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
      qreal& py1      = f->py1[dIdx];
      qreal& py2      = f->py2[dIdx];

      qreal _spatium   = spatium();
      QPointF canvPos(pagePos());
      qreal beamMinLen = point(score()->styleS(ST_beamMinLen));
      qreal graceMag   = score()->styleD(ST_graceNoteMag);

      // style values ST_beamDistance and ST_beamWidth not used
      if (beamLevels == 4)
            _beamDist = (2.5 / 3.0) * _spatium;
      else
            _beamDist = 0.75 * _spatium;

      if (isGrace) {
            _beamDist *= graceMag;
            setMag(graceMag);
            beamMinLen *= graceMag;
            }
      else
            setMag(1.0);

      if (staff()->isTabStaff()) {
            qreal y;                // vert. pos. of beam, relative to staff (top line = 0)
            StaffTypeTablature* tab = (StaffTypeTablature*)staff()->staffType();
            if(tab->stemsDown()) {
                  _up   = false;
                  y     = (tab->lines() - 1) * tab->lineDistance().val()
                              + STAFFTYPE_TAB_DEFAULTSTEMDIST_DN + STAFFTYPE_TAB_DEFAULTSTEMLEN_DN;
                  }
            else {
                  _up   = true;
                  y     = -STAFFTYPE_TAB_DEFAULTSTEMDIST_UP - STAFFTYPE_TAB_DEFAULTSTEMLEN_UP;
                  }
            y *= _spatium;
            py1 = y;
            py2 = y;
            }
      else {
            //
            // PITCHED STAVES: SETMScore::UP
            //
            qreal px1 = c1->stemPos().x();
            qreal px2 = c2->stemPos().x();
            if (_userModified[dIdx]) {
                  py1 += canvPos.y();
                  py2 += canvPos.y();

                  qreal beamY = py1;
                  slope       = (py2 - py1) / (px2 - px1);
                  //
                  // set stem direction for every chord
                  //
                  foreach (Chord* c, cl) {
                        QPointF p = c->upNote()->pagePos();
                        qreal y1  = beamY + (p.x() - px1) * slope;
                        bool nup  = y1 < p.y();
                        if (c->up() != nup) {
                              c->setUp(nup);
                              // guess was wrong, have to relayout
                              score()->layoutChords1(c->segment(), c->staffIdx());
                              }
                        }
                  _up = cl.front()->up();
                  }
            else if (cross) {
                  qreal beamY   = 0.0;  // y position of main beam start
                  qreal y1   = -200000;
                  qreal y2   = 200000;
                  foreach(const Chord* c, cl) {
                        qreal y  = c->upNote()->pagePos().y();
                        y1       = qMax(y1, y);
                        y2       = qMin(y2, y);
                        }
                  if (y1 > y2)
                        beamY = y2 + (y1 - y2) * .5;
                  else
                        beamY = _up ? y2 : y1;
                  py1 = beamY;
                  //
                  // set stem direction for every chord
                  //
                  foreach(Chord* c, cl) {
                        qreal y  = c->upNote()->pagePos().y();
                        bool nup = beamY < y;
                        if (c->up() != nup) {
                              c->setUp(nup);
                              // guess was wrong, have to relayout
                              score()->layoutChords1(c->segment(), c->staffIdx());
                              }
                        }

                  qreal yDownMax = -300000;
                  qreal yUpMin   = 300000;
                  foreach(const Chord* cr, cl) {
                        bool _up = cr->up();
                        qreal y = (_up ? cr->upNote() : cr->downNote())->stemPos(_up).y();
                        if (_up)
                              yUpMin = qMin(y, yUpMin);
                        else
                              yDownMax = qMax(y, yDownMax);
                        }
                  qreal slant = _spatium;
                  if (cl.front()->up())
                        slant = -slant;
                  py1   = yUpMin + (yDownMax - yUpMin) * .5 - slant * .5;
                  slope = slant / (px2 - px1);
                  }
            else {
                  py1 = c1->stemPos().y();
                  py2 = c2->stemPos().y();
                  computeStemLen(cl, py1, beamLevels);
                  }
            py2 = (px2 - px1) * slope + py1;
            py1 -= canvPos.y();
            py2 -= canvPos.y();
            }

      //---------------------------------------------
      //   create beam segments
      //---------------------------------------------

      qreal x1 = cl[0]->stemPos().x() - canvPos.x();
      int n    = cl.size();

      int baseLevel = 0;
      for (int beamLevel = 0; beamLevel < beamLevels; ++beamLevel) {
            bool growDown = _up || cross;
            for (int i = 0; i < n; ++i) {
                  Chord* cr1 = cl[i];
                  int l = cr1->durationType().hooks() - 1;
                  if (l >= beamLevel) {
                        int c1 = i;
                        ++i;
                        for (; i < n; ++i) {
                              Chord* c = cl[i];
                              int l = c->durationType().hooks() - 1;
                              bool b32 = (beamLevel >= 1) && (c->beamMode() == BEAM_BEGIN32);
                              bool b64 = (beamLevel >= 2) && (c->beamMode() == BEAM_BEGIN64);
                              if (l >= beamLevel && (b32 || b64)) {
                                    ++i;
                                    break;
                                    }
                              if (l < beamLevel)
                                    break;
                              }

                        int bl = growDown ? beamLevel : -beamLevel;

                        Chord* cr2 = cl[i-1];
                        if (c1 && (cr1->up() == cr2->up())) {
                              QPointF stemPos(cr1->stemPos());
                              qreal x2 = stemPos.x() - canvPos.x();
                              qreal y1 = (x2 - x1) * slope + py1 + canvPos.y();
                              qreal y2 = stemPos.y();

                              if ((y1 < y2) != growDown)
                                    bl = baseLevel - (beamLevel + 1);
                              }
                        int c2 = i;
                        if (c1 == 0 && c2 == n)
                              ++baseLevel;

                        qreal stemWidth  = point(score()->styleS(ST_stemWidth));
                        qreal x2   = cr1->stemPos().x() - canvPos.x();
                        qreal x3;

                        if ((c2 - c1) > 1) {
                              Chord* cr2 = cl[c2-1];
                              // create segment
                              x3 = cr2->stemPos().x() - canvPos.x();

                              if (staff()->isTabStaff() || cr1->up())
                                    x2 -= stemWidth;
//                              if ( !(staff()->isTabStaff() || cr2->up()) )
                              else
                                    x3 += stemWidth;
                              }
                        else {
                              // create broken segment
                              int n = cl.size();
                              qreal len = point(score()->styleS(ST_beamMinLen));
                              //
                              // find direction
                              //
                              if (c1 == 0)                // point to right
                                    ;
                              else if (c1 == n - 1)       // point to left
                                    len = -len;
                              else {
                                    // 0 < c1 < (n-1)
                                    Fraction a  = cl[c1-1]->duration();
                                    Fraction b  = cr1->duration();
                                    Fraction c  = cl[c1+1]->duration();
                                    Fraction ab = (a + b).reduced();
                                    Fraction bc = (b + c).reduced();

                                    if (ab.denominator() < bc.denominator())
                                          len = -len;
                                    else if (ab.denominator() == bc.denominator()) {
                                          if (a.reduced().denominator() < b.reduced().denominator())
                                                len = -len;
                                          }
                                    }
                              bool stemUp = cr1->up();
                              if (stemUp && len > 0)
                                    x2 -= stemWidth;
                              else if (!stemUp && len < 0)
                                    x2 += stemWidth;
                              x3 = x2 + len;
                              }
                        qreal yo   = py1 + bl * _beamDist * _grow1;
                        qreal ly1  = (x2 - x1) * slope + yo;
                        qreal ly2  = (x3 - x1) * slope + yo;
                        if (!qIsFinite(x2) || !qIsFinite(ly1)
                           || !qIsFinite(x3) || !qIsFinite(ly2))
                              qDebug("bad beam segment");
                        else
                              beamSegments.push_back(new QLineF(x2, ly1, x3, ly2));
                        --i;
                        }
                  }
            }

      //
      //  create stems
      //
      for (int i = 0; i < n; ++i) {
            Chord* cr  = cl[i];
            Stem* stem = cr->stem();
            if (!stem) {
                  stem = new Stem(score());
                  cr->setStem(stem);
                  }
            if (cr->hook())
                  score()->undoRemoveElement(cr->hook());

            QPointF stemPos(cr->stemPos());
            qreal x2 = stemPos.x() - canvPos.x();
            qreal y1 = (x2 - x1) * slope + py1 + canvPos.y();
            qreal y2 = stemPos.y();
            if (y2 < y1) {
                  // search bottom beam
                  qreal by = -1000000.0;
                  foreach(QLineF* l, beamSegments) {
                        if (x2 >= l->x1() && x2 <= l->x2()) {
                              qreal y = (x2 - l->x1()) * slope + l->y1();
                              by = qMax(by, y);
                              }
                        }
                  y1 = by + canvPos.y();
                  }
            else {
                  // search top beam
                  qreal by = 1000000.0;
                  foreach(QLineF* l, beamSegments) {
                        if (x2 >= l->x1() && x2 <= l->x2()) {
                              qreal y = (x2 - l->x1()) * slope + l->y1();
                              by = qMin(by, y);
                              }
                        }
                  y1 = by + canvPos.y();
                  }

            stem->setLen(y2 - y1);
            stem->setPos(stemPos - cr->pagePos());
            //
            // layout stem slash for acciacatura
            //
            if ((i == 0) && cr->noteType() == NOTE_ACCIACCATURA) {
                  StemSlash* stemSlash = cr->stemSlash();
                  if (!stemSlash) {
                        stemSlash = new StemSlash(score());
                        cr->add(stemSlash);
                        }
                  stemSlash->layout();
                  }
            else
                  cr->setStemSlash(0);

            Tremolo* tremolo = cr->tremolo();
            if (tremolo)
                  tremolo->layout();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Beam::write(Xml& xml) const
      {
      if (_elements.isEmpty())
            return;
      xml.stag(QString("Beam id=\"%1\"").arg(_id));
      Element::writeProperties(xml);

      writeProperty(xml, P_STEM_DIRECTION);
      writeProperty(xml, P_DISTRIBUTE);
      writeProperty(xml, P_GROW_LEFT);
      writeProperty(xml, P_GROW_RIGHT);

      int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
      if (_userModified[idx]) {
            qreal _spatium = spatium();
            foreach(BeamFragment* f, fragments) {
                  xml.stag("Fragment");
                  xml.tag("y1", f->py1[idx] / _spatium);
                  xml.tag("y2", f->py2[idx] / _spatium);
                  xml.etag();
                  }
            }
#ifndef NDEBUG
      //
      // this info is used for regression testing
      // l1/l2 is the beam position of the layout engine
      //
      if (score()->testMode()) {
            qreal _spatium4 = spatium() * .25;
            foreach(BeamFragment* f, fragments) {
                  xml.tag("l1", int(lrint(f->py1[idx] / _spatium4)));
                  xml.tag("l2", int(lrint(f->py2[idx] / _spatium4)));
                  }
            }
#endif
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Beam::read(const QDomElement& de)
      {
      QPointF p1, p2;
      qreal _spatium = spatium();
      _id = de.attribute("id").toInt();
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "StemDirection")
                  setProperty(P_STEM_DIRECTION, ::getProperty(P_STEM_DIRECTION, e));
            else if (tag == "distribute")
                  setDistribute(val.toInt());
            else if (tag == "growLeft")
                  setGrowLeft(val.toDouble());
            else if (tag == "growRight")
                  setGrowRight(val.toDouble());
            else if (tag == "y1") {
                  if (fragments.isEmpty())
                        fragments.append(new BeamFragment);
                  BeamFragment* f = fragments.back();
                  int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  f->py1[idx] = val.toDouble() * _spatium;
                  }
            else if (tag == "y2") {
                  if (fragments.isEmpty())
                        fragments.append(new BeamFragment);
                  BeamFragment* f = fragments.back();
                  int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  f->py2[idx] = val.toDouble() * _spatium;
                  }
            else if (tag == "Fragment") {
                  BeamFragment* f = new BeamFragment;
                  int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
                  _userModified[idx] = true;
                  qreal _spatium = spatium();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        const QString& tag(ee.tagName());
                        qreal v = ee.text().toDouble() * _spatium;
                        if (tag == "y1")
                              f->py1[idx] = v;
                        else if (tag == "y2")
                              f->py2[idx] = v;
                        else
                              domError(ee);
                        }
                  fragments.append(f);
                  }
#ifndef NDEBUG
            else if (tag == "l1" || tag == "l2")      // ignore
                  ;
#endif
            else if (tag == "subtype")          // obsolete
                  ;
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Beam::editDrag(const EditData& ed)
      {
      int idx  = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
      qreal dy = ed.delta.y();
      BeamFragment* f = fragments[editFragment];
      if (ed.curGrip == 0)
            f->py1[idx] += dy;
      f->py2[idx] += dy;
      _userModified[idx] = true;
      setGenerated(false);
      layout1();
      layout();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Beam::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 2;
      int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
      BeamFragment* f = fragments[editFragment];

      Chord* c1;
      Chord* c2;
      int n = _elements.size();
      for (int i = 0; i < n; ++i) {
            if (_elements[i]->type() == CHORD) {
                  c1 = static_cast<Chord*>(_elements[i]);
                  break;
                  }
            }
      for (int i = n-1; i >= 0; --i) {
            if (_elements[i]->type() == CHORD) {
                  c2 = static_cast<Chord*>(_elements[i]);
                  break;
                  }
            }
      int y = pagePos().y();
      grip[0].translate(QPointF(c1->stemPos().x(), f->py1[idx] + y));
      grip[1].translate(QPointF(c2->stemPos().x(), f->py2[idx] + y));
      }

//---------------------------------------------------------
//   setBeamDirection
//---------------------------------------------------------

void Beam::setBeamDirection(MScore::Direction d)
      {
      _direction = d;
      if (d != MScore::AUTO)
            _up = d == MScore::UP;
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Beam::toDefault()
      {
      if (distribute())
            score()->undoChangeProperty(this, P_DISTRIBUTE, false);
      if (growLeft() != 1.0)
            score()->undoChangeProperty(this, P_GROW_LEFT, 1.0);
      if (growRight() != 1.0)
            score()->undoChangeProperty(this, P_GROW_RIGHT, 1.0);
      if (userModified()) {
            score()->undoChangeProperty(this, P_BEAM_POS, QVariant(beamPos()));
            score()->undoChangeProperty(this, P_USER_MODIFIED, false);
            }
      if (beamDirection() != MScore::AUTO)
            score()->undoChangeProperty(this, P_STEM_DIRECTION, int(MScore::AUTO));
      setGenerated(true);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Beam::startEdit(MuseScoreView*, const QPointF& p)
      {
      QPointF pt(p - pagePos());
      qreal ydiff = 100000000.0;
      int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
      int i = 0;
      editFragment = 0;
      foreach (BeamFragment* f, fragments) {
            qreal d = fabs(f->py1[idx] - pt.y());
            if (d < ydiff) {
                  ydiff = d;
                  editFragment = i;
                  }
            ++i;
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Beam::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return (e->type() == ICON) && ((static_cast<Icon*>(e)->subtype() == ICON_FBEAM1)
         || (static_cast<Icon*>(e)->subtype() == ICON_FBEAM2));
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Beam::drop(const DropData& data)
      {
      Icon* e = static_cast<Icon*>(data.element);
      if (e->type() != ICON)
            return 0;
      qreal g1;
      qreal g2;

      if (e->subtype() == ICON_FBEAM1) {
            g1 = 1.0;
            g2 = 0.0;
            }
      else if (e->subtype() == ICON_FBEAM2) {
            g1 = 0.0;
            g2 = 1.0;
            }
      else
            return 0;
      if (g1 != growLeft())
            score()->undoChangeProperty(this, P_GROW_LEFT, g1);
      if (g2 != growRight())
            score()->undoChangeProperty(this, P_GROW_RIGHT, g2);
      return 0;
      }

//---------------------------------------------------------
//   beamPos
//    misuse QPointF for y1-y2 real values
//---------------------------------------------------------

QPointF Beam::beamPos() const
      {
      if (fragments.isEmpty())
            return QPointF(0.0, 0.0);
      BeamFragment* f = fragments.back();
      int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
      qreal _spatium = spatium();
      return QPointF(f->py1[idx] / _spatium, f->py2[idx] / _spatium);
      }

//---------------------------------------------------------
//   setBeamPos
//---------------------------------------------------------

void Beam::setBeamPos(const QPointF& bp)
      {
      if (fragments.isEmpty())
            fragments.append(new BeamFragment);
      BeamFragment* f = fragments.back();
      int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
      _userModified[idx] = true;
      setGenerated(false);
      qreal _spatium = spatium();
      f->py1[idx] = bp.x() * _spatium;
      f->py2[idx] = bp.y() * _spatium;
      }

//---------------------------------------------------------
//   userModified
//---------------------------------------------------------

bool Beam::userModified() const
      {
      int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
      return _userModified[idx];
      }

//---------------------------------------------------------
//   setUserModified
//---------------------------------------------------------

void Beam::setUserModified(bool val)
      {
      int idx = (_direction == MScore::AUTO || _direction == MScore::DOWN) ? 0 : 1;
      _userModified[idx] = val;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Beam::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_STEM_DIRECTION: return int(beamDirection());
            case P_DISTRIBUTE:     return distribute();
            case P_GROW_LEFT:      return growLeft();
            case P_GROW_RIGHT:     return growRight();
            case P_USER_MODIFIED:  return userModified();
            case P_BEAM_POS:       return beamPos();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Beam::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_STEM_DIRECTION:
                  setBeamDirection(MScore::Direction(v.toInt()));
                  break;
            case P_DISTRIBUTE:
                  setDistribute(v.toBool());
                  break;
            case P_GROW_LEFT:
                  setGrowLeft(v.toDouble());
                  break;
            case P_GROW_RIGHT:
                  setGrowRight(v.toDouble());
                  break;
            case P_USER_MODIFIED:
                  setUserModified(v.toBool());
                  break;
            case P_BEAM_POS:
                  if (userModified())
                        setBeamPos(v.toPointF());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Beam::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_STEM_DIRECTION: return int(MScore::AUTO);
            case P_DISTRIBUTE:     return false;
            case P_GROW_LEFT:      return 1.0;
            case P_GROW_RIGHT:     return 1.0;
            case P_USER_MODIFIED:  return false;
            case P_BEAM_POS:       return beamPos();
            default:               return Element::propertyDefault(id);
            }
      }

