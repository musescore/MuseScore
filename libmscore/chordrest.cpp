//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: chordrest.cpp 5609 2012-05-07 19:54:46Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "chordrest.h"
#include "chord.h"
#include "xml.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "tuplet.h"
#include "score.h"
#include "sym.h"
#include "slur.h"
#include "beam.h"
#include "breath.h"
#include "barline.h"
#include "articulation.h"
#include "tempo.h"
#include "tempotext.h"
#include "note.h"
#include "arpeggio.h"
#include "dynamic.h"
#include "stafftext.h"
#include "sig.h"
#include "clef.h"
#include "lyrics.h"
#include "segment.h"
#include "stafftype.h"
#include "undo.h"
#include "stem.h"
#include "harmony.h"
#include "figuredbass.h"
#include "icon.h"

//---------------------------------------------------------
//   hasArticulation
//---------------------------------------------------------

Articulation* ChordRest::hasArticulation(const Articulation* aa)
      {
      int idx = aa->subtype();
      foreach(Articulation* a, _articulations) {
            if (idx == a->subtype())
                  return a;
            }
      return 0;
      }

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::ChordRest(Score* s)
   : DurationElement(s)
      {
      _beam        = 0;
      _small       = false;
      _beamMode    = BEAM_AUTO;
      _up          = true;
      _staffMove   = 0;
      _tabDur      = 0;
      _spannerFor  = 0;
      _spannerBack = 0;
      }

ChordRest::ChordRest(const ChordRest& cr)
   : DurationElement(cr)
      {
      _durationType = cr._durationType;
      _staffMove    = cr._staffMove;
      _tabDur       = 0;  // tab sur. symb. depends upon context: can't be
                          // simply copied from another CR

      foreach(Articulation* a, cr._articulations) {    // make deep copy
            Articulation* na = new Articulation(*a);
            na->setParent(this);
            na->setTrack(track());
            _articulations.append(na);
            }

      _beam      = 0;
      _beamMode  = cr._beamMode;
      _up        = cr._up;
      _small     = cr._small;
      _space     = cr._space;

      foreach(Lyrics* l, cr._lyricsList) {        // make deep copy
            if (l == 0)
                  continue;
            Lyrics* nl = new Lyrics(*l);
            nl->setParent(this);
            nl->setTrack(track());
            _lyricsList.append(nl);
            }
      _spannerFor  = 0;
      _spannerBack = 0;
      }

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::~ChordRest()
      {
      foreach(Articulation* a,  _articulations)
            delete a;
      foreach(Lyrics* l, _lyricsList)
            delete l;
      if(_tabDur)
            delete _tabDur;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void ChordRest::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (_beam && (_beam->elements().front() == this))
            _beam->scanElements(data, func, all);
      foreach(Articulation* a, _articulations)
            func(data, a);
      foreach(Lyrics* l, _lyricsList) {
            if (l)
                  l->scanElements(data, func, all);
            }
      DurationElement* de = this;
      while (de->tuplet() && de->tuplet()->elements().front() == de) {
            func(data, de->tuplet());
            de = de->tuplet();
            }
      if (_tabDur)
            func(data, _tabDur);
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void ChordRest::writeProperties(Xml& xml) const
      {
      DurationElement::writeProperties(xml);

      //
      // BeamMode default:
      //    REST  - BEAM_NO
      //    CHORD - BEAM_AUTO
      //
      if ((type() == REST && _beamMode != BEAM_NO)
         || (type() == CHORD && _beamMode != BEAM_AUTO)) {
            QString s;
            switch(_beamMode) {
                  case BEAM_AUTO:    s = "auto"; break;
                  case BEAM_BEGIN:   s = "begin"; break;
                  case BEAM_MID:     s = "mid"; break;
                  case BEAM_END:     s = "end"; break;
                  case BEAM_NO:      s = "no"; break;
                  case BEAM_BEGIN32: s = "begin32"; break;
                  case BEAM_BEGIN64: s = "begin64"; break;
                  case BEAM_INVALID: s = "?"; break;
                  }
            xml.tag("BeamMode", s);
            }
      writeProperty(xml, P_SMALL);
      if (durationType().dots())
            xml.tag("dots", durationType().dots());
      if (_staffMove)
            xml.tag("move", _staffMove);
      if (durationType().isValid())
            xml.tag("durationType", durationType().name());

      if (!duration().isZero() && (!durationType().fraction().isValid()
         || (durationType().fraction() != duration())))
            xml.fTag("duration", duration());
      foreach(const Articulation* a, _articulations)
            a->write(xml);
      //
      // ignore spanner with id==-1 as they are outside of the write range
      //
      for(Spanner* s = _spannerFor; s; s = s->next()) {
            if (s->id() != -1)
                  xml.tagE(QString("Slur type=\"start\" number=\"%1\"").arg(s->id()+1));
            }
      for(Spanner* s = _spannerBack; s; s = s->next()) {
            if (s->id() != -1)
                  xml.tagE(QString("Slur type=\"stop\" number=\"%1\"").arg(s->id()+1));
            }
#ifndef NDEBUG
      if (_beam && (score()->testMode() || !_beam->generated()))
            xml.tag("Beam", _beam->id());
#else
      if (!xml.clipboardmode && _beam && !_beam->generated())
            xml.tag("Beam", _beam->id());
#endif
      foreach(Lyrics* lyrics, _lyricsList) {
            if (lyrics)
                  lyrics->write(xml);
            }
      Fraction t(globalDuration());
      if (staff())
            t *= staff()->timeStretch(xml.curTick);
      xml.curTick += t.ticks();
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ChordRest::readProperties(const QDomElement& e, QList<Tuplet*>* tuplets, QList<Spanner*>* spanner)
      {
      if (DurationElement::readProperties(e, tuplets, spanner))
            return true;
      const QString& tag(e.tagName());
      const QString& val(e.text());

      if (tag == "BeamMode") {
            int bm = BEAM_AUTO;
            if (val == "auto")
                  bm = BEAM_AUTO;
            else if (val == "begin")
                  bm = BEAM_BEGIN;
            else if (val == "mid")
                  bm = BEAM_MID;
            else if (val == "end")
                  bm = BEAM_END;
            else if (val == "no")
                  bm = BEAM_NO;
            else if (val == "begin32")
                  bm = BEAM_BEGIN32;
            else if (val == "begin64")
                  bm = BEAM_BEGIN64;
            else
                  bm = BeamMode(val.toInt());
            _beamMode = BeamMode(bm);
            }
      else if (tag == "Attribute" || tag == "Articulation") {     // obsolete: "Attribute"
            Articulation* atr = new Articulation(score());
            atr->read(e);
            add(atr);
            }
      else if (tag == "leadingSpace") {
//            if (MScore::debugMode)
                  qDebug("ChordRest: leadingSpace obsolete"); // _extraLeadingSpace = Spatium(val.toDouble());
            }
      else if (tag == "trailingSpace") {
//            if (MScore::debugMode)
                  qDebug("ChordRest: trailingSpace obsolete"); // _extraTrailingSpace = Spatium(val.toDouble());
            }
      else if (tag == "Beam") {
            Beam* beam = 0;
            foreach(Beam* b, score()->beams) {
                  if (b->id() == val.toInt()) {
                        beam = b;
                        break;
                        }
                  }
            if (beam)
                  beam->add(this);        // also calls this->setBeam(beam)
            else
                  qDebug("Beam id %d not found", val.toInt());
            }
      else if (tag == "small")
            _small = val.toInt();
      else if (tag == "Slur") {
            if (spanner) {
                  int id = e.attribute("number").toInt();
                  QString type = e.attribute("type");
                  Slur* slur = 0;
                  foreach(Spanner* s, *spanner) {
                        if (s->id() == id) {
                              slur = static_cast<Slur*>(s);
                              break;
                              }
                        }
                  if (!slur)
                        qDebug("ChordRest::read(): Slur id %d not found", id);
                  else {
                        if (type == "start") {
                              slur->setStartElement(this);
                              addSlurFor(slur);
                              }
                        else if (type == "stop") {
                              slur->setEndElement(this);
                              addSlurBack(slur);
                              }
                        else
                              qDebug("ChordRest::read(): unknown Slur type <%s>", qPrintable(type));
                        }
                  }
            }
      else if (tag == "durationType") {
            setDurationType(val);
            if (durationType().type() != TDuration::V_MEASURE) {
                  if ((type() == REST) &&
                              // for backward compatibility, convert V_WHOLE rests to V_MEASURE
                              // if long enough to fill a measure.
                              // OTOH, freshly created (un-initialized) rests have numerator == 0 (< 4/4)
                              // (see Fraction() constructor in fraction.h; this happens for instance
                              // when pasting selection from clipboard): they should not be converted
                              duration().numerator() != 0 &&
                              // rest durations are initialized to full measure duration when
                              // created upon reading the <Rest> tag (see Measure::read() )
                              // so a V_WHOLE rest in a measure of 4/4 or less => V_MEASURE
                              (durationType()==TDuration::V_WHOLE && duration() <= Fraction(4, 4)) ) {
                        // old pre 2.0 scores: convert
                        setDurationType(TDuration::V_MEASURE);
                        }
                  else  // not from old score: set duration fraction from duration type
                        setDuration(durationType().fraction());
                  }
            else {
                  if (score()->mscVersion() < 115) {
                        SigEvent e = score()->sigmap()->timesig(score()->curTick);
                        setDuration(e.timesig());
                        }
                  }
            }
      else if (tag == "duration")
            setDuration(readFraction(e));
      else if (tag == "ticklen") {      // obsolete (version < 1.12)
            int mticks = score()->sigmap()->timesig(score()->curTick).timesig().ticks();
            int i = val.toInt();
            if (i == 0)
                  i = mticks;
            // if ((type() == REST) && (mticks == i || (durationType()==TDuration::V_WHOLE && mticks != 1920))) {
            if ((type() == REST) && (mticks == i)) {
                  setDurationType(TDuration::V_MEASURE);
                  setDuration(Fraction::fromTicks(i));
                  }
            else {
                  Fraction f = Fraction::fromTicks(i);
                  setDuration(f);
                  setDurationType(TDuration(f));
                  }
            }
      else if (tag == "dots")
            setDots(val.toInt());
      else if (tag == "move")
            _staffMove = val.toInt();
      else if (tag == "Lyrics" /*|| tag == "FiguredBass"*/) {
            Element* element = Element::name2Element(tag, score());
            element->setTrack(score()->curTrack);
            element->read(e);
            add(element);
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void ChordRest::setSmall(bool val)
      {
      _small   = val;
      qreal m = 1.0;
      if (_small)
            m = score()->styleD(ST_smallNoteMag);
      if (staff()->small())
            m *= score()->styleD(ST_smallStaffMag);
      setMag(m);
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void ChordRest::setMag(qreal val)
      {
      Element::setMag(val);
      foreach(Articulation* a, _articulations)
            a->setMag(val);
      }

//---------------------------------------------------------
//   layoutArticulations
//    called from chord()->layout()
//---------------------------------------------------------

void ChordRest::layoutArticulations()
      {
      if (parent() == 0 || _articulations.isEmpty())
            return;
      qreal _spatium  = spatium();
      if (type() == CHORD) {
            if (_articulations.size() == 1) {
                  static_cast<Chord*>(this)->layoutArticulation(_articulations[0]);
                  return;
                  }
            if (_articulations.size() == 2) {
                  //
                  // staccato | tenuto + marcato
                  //
                  Articulation* a1 = _articulations[0];
                  Articulation* a2 = _articulations[1];
                  int st1 = a1->subtype();
                  int st2 = a2->subtype();

                  if ((st2 == Articulation_Tenuto || st2 == Articulation_Staccato)
                     && (st1 == Articulation_Marcato)) {
                        qSwap(a1, a2);
                        qSwap(st1, st2);
                        }
                  if ((st1 == Articulation_Tenuto || st1 == Articulation_Staccato)
                     && (st2 == Articulation_Marcato)) {
                        QPointF pt = static_cast<Chord*>(this)->layoutArticulation(a1);
                        pt.ry() += a1->up() ? -_spatium * .5 : _spatium * .5;
                        a2->layout();
                        a2->setUp(a1->up());
                        a2->setPos(pt);
                        a2->adjustReadPos();
                        return;
                        }
                  //
                  // staccato | tenuto + sforzato
                  //
                  if ((st2 == Articulation_Tenuto || st2 == Articulation_Staccato)
                     && (st1 == Articulation_Sforzatoaccent)) {
                        qSwap(a1, a2);
                        qSwap(st1, st2);
                        }
                  if ((st1 == Articulation_Tenuto || st1 == Articulation_Staccato)
                     && (st2 == Articulation_Sforzatoaccent)) {
                        QPointF pt = static_cast<Chord*>(this)->layoutArticulation(a1);
                        pt.ry() += a1->up() ? -_spatium * .7 : _spatium * .7;
                        a2->layout();
                        a2->setUp(a1->up());
                        a2->setPos(pt);
                        a2->adjustReadPos();
                        return;
                        }
                  }
            }
      qreal x         = centerX();
      qreal distance0 = score()->styleS(ST_propertyDistance).val()     * _spatium;
      qreal distance1 = score()->styleS(ST_propertyDistanceHead).val() * _spatium;
      qreal distance2 = score()->styleS(ST_propertyDistanceStem).val() * _spatium;

      qreal chordTopY = upPos();    // note position of highest note
      qreal chordBotY = downPos();  // note position of lowest note

      qreal staffTopY = -distance2;
      qreal staffBotY = staff()->height() + distance2;

      // avoid collisions of staff articulations with chord notes:
      // gap between note and staff articulation is distance0 + 0.5 spatium

      if (type() == CHORD) {
            Chord* chord = static_cast<Chord*>(this);
            Stem* stem   = chord->stem();
            if (stem) {
                  qreal y = stem->pos().y() + pos().y();
                  if (up() && stem->stemLen() < 0.0)
                        y += stem->stemLen();
                  else if (!up() && stem->stemLen() > 0.0)
                        y -= stem->stemLen();

                  if (beam()) {
                        qreal bw = score()->styleS(ST_beamWidth).val() * _spatium;
                        y += up() ? -bw : bw;
                        }
                  if (up())
                        staffTopY = qMin(staffTopY, qreal(y - 0.5 * _spatium));
                  else
                        staffBotY = qMax(staffBotY, qreal(y + 0.5 * _spatium));
                  }
            }

      staffTopY = qMin(staffTopY, qreal(chordTopY - distance0 - 0.5 * _spatium));
      staffBotY = qMax(staffBotY, qreal(chordBotY + distance0 + 0.5 * _spatium));

      qreal dy = 0.0;

      int n = _articulations.size();
      for (int i = 0; i < n; ++i) {
            Articulation* a = _articulations.at(i);
            //
            // determine Direction
            //
            if (a->direction() != MScore::AUTO) {
                  a->setUp(a->direction() == MScore::UP);
                  }
            else {
                  if (a->anchor() == A_CHORD)
                        a->setUp(!up());
                  else
                        a->setUp(a->anchor() == A_TOP_STAFF || a->anchor() == A_TOP_CHORD);
                  }
            }

      //
      //    pass 1
      //    place tenuto and staccato
      //

      for (int i = 0; i < n; ++i) {
            Articulation* a = _articulations.at(i);
            a->layout();
            ArticulationAnchor aa = a->anchor();

            if ((a->subtype() != Articulation_Tenuto)
               && (a->subtype() != Articulation_Staccato))
                  continue;

            if (aa != A_CHORD && aa != A_TOP_CHORD && aa != A_BOTTOM_CHORD)
                  continue;

            bool bottom;
            if ((aa == A_CHORD) && measure()->hasVoices(a->staffIdx()))
                  bottom = !up();
            else
                  bottom = (aa == A_BOTTOM_CHORD) || (aa == A_CHORD && up());
            bool headSide = bottom == up();

            dy += distance1;
            qreal y;
            Chord* chord = static_cast<Chord*>(this);
            if (bottom) {
                  int line = downLine();
                  y = chordBotY + dy;
                  if (!headSide && type() == CHORD && chord->stem()) {
                        Stem* stem = chord->stem();
                        y          = chordTopY + stem->stemLen();
                        if (chord->beam())
                              y += score()->styleS(ST_beamWidth).val() * _spatium * .5;
                        x          = stem->pos().x();
                        int line   = lrint((y+0.5*_spatium) / _spatium);
                        if (line <= 4)    // align between staff lines
                              y = line * _spatium + _spatium * .5;
                        else
                              y += _spatium;
                        }
                  else {
                        int lines = (staff()->lines() - 1) * 2;
                        if (line < lines)
                              y = (line & ~1) + 3;
                        else
                              y = line + 2;
                        y *= _spatium * .5;
                        }
                  }
            else {
                  int line = upLine();
                  y = chordTopY - dy;
                  if (!headSide && type() == CHORD && chord->stem()) {
                        Stem* stem = chord->stem();
                        y          = chordBotY + stem->stemLen();
                        if (chord->beam())
                              y -= score()->styleS(ST_beamWidth).val() * _spatium * .5;
                        x          = stem->pos().x();
                        int line   = lrint((y-0.5*_spatium) / _spatium);
                        if (line >= 0)    // align between staff lines
                              y = line * _spatium - _spatium * .5;
                        else
                              y -= _spatium;
                        }
                  else {
                        if (line > 0)
                              y = ((line+1) & ~1) - 3;
                        else
                              y = line - 2;
                        y *= _spatium * .5;
                        }
                  }
            dy += _spatium * .5;
            a->setPos(x, y);
            }

      // reserve space for slur
      bool botGap = false;
      bool topGap = false;

      for (Spanner* sp = _spannerFor; sp; sp = sp->next()) {
            if (sp->type() != SLUR)
                  continue;
            Slur* s = static_cast<Slur*>(sp);
            if (s->up())
                  topGap = true;
            else
                  botGap = true;
            }
      for (Spanner* sp = _spannerBack; sp; sp = sp->next()) {
            if (sp->type() != SLUR)
                  continue;
            Slur* s = static_cast<Slur*>(sp);
            if (s->up())
                  topGap = true;
            else
                  botGap = true;
            }
      if (botGap)
            chordBotY += _spatium;
      if (topGap)
            chordTopY -= _spatium;

      //
      //    pass 2
      //    place all articulations with anchor at chord/rest
      //
      n = _articulations.size();
      for (int i = 0; i < n; ++i) {
            Articulation* a = _articulations.at(i);
            a->layout();
            ArticulationAnchor aa = a->anchor();
            if ((a->subtype() == Articulation_Tenuto)
               || (a->subtype() == Articulation_Staccato))
                  continue;

            if (aa != A_CHORD && aa != A_TOP_CHORD && aa != A_BOTTOM_CHORD)
                  continue;

            // for tenuto and staccate check for staff line collision
            bool staffLineCT = a->subtype() == Articulation_Tenuto
                               || a->subtype() == Articulation_Staccato;

//            qreal sh = a->bbox().height() * mag();
            bool bottom = (aa == A_BOTTOM_CHORD) || (aa == A_CHORD && up());

            dy += distance1;
            if (bottom) {
                  qreal y = chordBotY + dy;
                  if (staffLineCT && (y <= staffBotY -.1 - dy)) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4) {
                              y  += _spatium * .5;
                              dy += _spatium * .5;
                              }
                        }
                  a->setPos(x, y); // - a->bbox().y() + a->bbox().height() * .5);
                  }
            else {
                  qreal y = chordTopY - dy;
                  if (staffLineCT && (y >= (staffTopY +.1 + dy))) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4) {
                              y  -= _spatium * .5;
                              dy += _spatium * .5;
                              }
                        }
                  a->setPos(x, y); // + a->bbox().y() - a->bbox().height() * .5);
                  }
            }

      //
      //    pass 3
      //    now place all articulations with staff top or bottom anchor
      //
      qreal dyTop = staffTopY;
      qreal dyBot = staffBotY;

/*      if ((upPos() - _spatium) < dyTop)
            dyTop = upPos() - _spatium;
      if ((downPos() + _spatium) > dyBot)
            dyBot = downPos() + _spatium;
  */
      for (int i = 0; i < n; ++i) {
            Articulation* a = _articulations.at(i);
            ArticulationAnchor aa = a->anchor();
            if (aa == A_TOP_STAFF || aa == A_BOTTOM_STAFF) {
                  if (a->up()) {
                        a->setPos(x, dyTop);
                        dyTop -= distance0;
                        }
                  else {
                        a->setPos(x, dyBot);
                        dyBot += distance0;
                        }
                  }
            a->adjustReadPos();
            }
      }

//---------------------------------------------------------
//   addSpannerBack
//---------------------------------------------------------

void ChordRest::addSpannerBack(Spanner* e)
      {
      e->setNext(_spannerBack);
      _spannerBack = e;
      }

//---------------------------------------------------------
//   removeSpannerBack
//---------------------------------------------------------

bool ChordRest::removeSpannerBack(Spanner* e)
      {
      Spanner* sp = _spannerBack;
      Spanner* prev = 0;
      while (sp) {
            if (sp == e) {
                  if (prev)
                        prev->setNext(sp->next());
                  else
                        _spannerBack = sp->next();
                  return true;
                  }
            prev = sp;
            sp = sp->next();
            }
      return false;
      }

//---------------------------------------------------------
//   addSpannerFor
//---------------------------------------------------------

void ChordRest::addSpannerFor(Spanner* e)
      {
      e->setNext(_spannerFor);
      _spannerFor = e;
      }

bool ChordRest::removeSpannerFor(Spanner* e)
      {
      Spanner* sp = _spannerFor;
      Spanner* prev = 0;
      while (sp) {
            if (sp == e) {
                  if (prev)
                        prev->setNext(sp->next());
                  else
                        _spannerFor = sp->next();
                  return true;
                  }
            prev = sp;
            sp = sp->next();
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* ChordRest::drop(const DropData& data)
      {
      Element* e = data.element;
      Measure* m  = measure();
      switch (e->type()) {
            case BREATH:
                  {
                  Breath* b = static_cast<Breath*>(e);
                  b->setTrack(staffIdx() * VOICES);

                  // TODO: insert automatically in all staves?

                  Segment* seg = m->undoGetSegment(Segment::SegBreath, tick());
                  b->setParent(seg);
                  score()->undoAddElement(b);
                  }
                  return e;

            case BAR_LINE:
                  {
                  BarLine* bl = static_cast<BarLine*>(e);
                  bl->setTrack(staffIdx() * VOICES);

                  if (tick() == m->tick())
                        return m->drop(data);

                  Segment* seg = m->undoGetSegment(Segment::SegBarLine, tick());
                  bl->setParent(seg);
                  score()->undoAddElement(bl);
                  }
                  return e;

            case CLEF:
                  score()->undoChangeClef(staff(), segment(), static_cast<Clef*>(e)->clefType());
                  delete e;
                  break;

            case TEMPO_TEXT:
                  {
                  TempoText* tt = static_cast<TempoText*>(e);
                  tt->setParent(segment());
                  int st = tt->textStyleType();
                  if (st != TEXT_STYLE_UNKNOWN)
                        tt->setTextStyle(score()->textStyle(st));
                  score()->undoAddElement(tt);
                  }
                  return e;

            case DYNAMIC:
            case FRET_DIAGRAM:
            case SYMBOL:
                  e->setTrack(track());
                  e->setParent(segment());
                  score()->undoAddElement(e);
                  return e;

            case NOTE:
                  {
                  Note* note = static_cast<Note*>(e);
                  NoteVal nval;
                  nval.pitch = note->pitch();
                  nval.headGroup = note->headGroup();
                  score()->setNoteRest(segment(), track(), nval, data.duration, MScore::AUTO);
                  delete e;
                  }
                  break;

            case HARMONY:
                  static_cast<Harmony*>(e)->render();
                  // fall through
            case TEXT:
            case STAFF_TEXT:
            case STAFF_STATE:
            case INSTRUMENT_CHANGE:
            case REHEARSAL_MARK:
                  e->setParent(segment());
                  e->setTrack((track() / VOICES) * VOICES);
                  {
                  Text* f = static_cast<Text*>(e);
                  int st = f->textStyleType();
                  if (st != TEXT_STYLE_UNKNOWN)
                        f->setTextStyle(score()->textStyle(st));
                  }
                  score()->undoAddElement(e);
                  return e;

            case FIGURED_BASS:
                  {
                  bool bNew;
                  FiguredBass * fb = static_cast<FiguredBass *>(e);
                  fb->setParent( segment() );
                  fb->setTrack( (track() / VOICES) * VOICES );
                  fb->setTicks( duration().ticks() );
                  fb->setOnNote(true);
                  /* FiguredBass * fbNew =*/ FiguredBass::addFiguredBassToSegment(segment(),
                        fb->track(), fb->ticks(), &bNew);
                  // fbNew = fb;
                  if (bNew)
                        score()->undoAddElement(e);
                  return e;
                  }

            case IMAGE:
                  e->setParent(segment());
                  score()->undoAddElement(e);
                  return e;

            case ICON:
                  {
                  switch(static_cast<Icon*>(e)->subtype()) {
                        case ICON_SBEAM:
                              score()->undoChangeProperty(this, P_BEAM_MODE, BEAM_BEGIN);
                              break;
                        case ICON_MBEAM:
                              score()->undoChangeProperty(this, P_BEAM_MODE, BEAM_MID);
                              break;
                        case ICON_NBEAM:
                              score()->undoChangeProperty(this, P_BEAM_MODE, BEAM_NO);
                              break;
                        case ICON_BEAM32:
                              score()->undoChangeProperty(this, P_BEAM_MODE, BEAM_BEGIN32);
                              break;
                        case ICON_BEAM64:
                              score()->undoChangeProperty(this, P_BEAM_MODE, BEAM_BEGIN64);
                              break;
                        case ICON_AUTOBEAM:
                              score()->undoChangeProperty(this, P_BEAM_MODE, BEAM_AUTO);
                              break;
                        }
                  }
                  delete e;
                  break;

            default:
                  qDebug("cannot drop %s", e->name());
                  delete e;
                  return 0;
            }
      return 0;
      }

//---------------------------------------------------------
//   setBeam
//---------------------------------------------------------

void ChordRest::setBeam(Beam* b)
      {
      _beam = b;
      }

//---------------------------------------------------------
//   setDurationType
//---------------------------------------------------------

void ChordRest::setDurationType(TDuration::DurationType t)
      {
      _durationType.setType(t);
      }

void ChordRest::setDurationType(const QString& s)
      {
      _durationType.setType(s);
      }

void ChordRest::setDurationType(int ticks)
      {
      _durationType.setVal(ticks);
      }

void ChordRest::setDurationType(const TDuration& v)
      {
      _durationType = v;
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void ChordRest::setTrack(int val)
      {
      foreach(Articulation* a, _articulations)
            a->setTrack(val);
      Element::setTrack(val);
      if (type() == CHORD) {
            foreach(Note* n, static_cast<Chord*>(this)->notes())
                  n->setTrack(val);
            }
      if (_beam)
            _beam->setTrack(val);
      foreach(Lyrics* l, _lyricsList) {
            if (l)
                  l->setTrack(val);
            }
      if (tuplet())
            tuplet()->setTrack(val);
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int ChordRest::tick() const
      {
      return segment() ? segment()->tick() : -1;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void ChordRest::add(Element* e)
      {
      e->setParent(this);
      e->setTrack(track());
      switch(e->type()) {
            case ARTICULATION:
                  {
                  Articulation* a = static_cast<Articulation*>(e);
                  _articulations.push_back(a);
                  if (a->timeStretch() > 0.0) {
                        qreal otempo = score()->tempo(tick());
                        qreal ntempo = otempo / a->timeStretch();
                        score()->setTempo(tick(), ntempo);
                        score()->setTempo(tick() + actualTicks(), otempo);
                        }
                  }
                  break;
//            case FIGURED_BASS:
            case LYRICS:
                  {
                  Lyrics* l = static_cast<Lyrics*>(e);
                  int size = _lyricsList.size();
                  if (l->no() >= size) {
                        for (int i = size-1; i < l->no(); ++i)
                              _lyricsList.append(0);
                        }
                  _lyricsList[l->no()] = l;
                  }
                  break;
            default:
                  qDebug("ChordRest::add: unknown element %s", e->name());
                  abort();
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void ChordRest::remove(Element* e)
      {
      switch(e->type()) {
            case ARTICULATION:
                  {
                  Articulation* a = static_cast<Articulation*>(e);
                  if (!_articulations.removeOne(a))
                        qDebug("ChordRest::remove(): articulation not found");
                  if (a->timeStretch() > 0.0) {
                        score()->removeTempo(tick());
                        score()->removeTempo(tick() + actualTicks());
                        }
                  }
                  break;
//            case FIGURED_BASS:
            case LYRICS:
                  {
                  for (int i = 0; i < _lyricsList.size(); ++i) {
                        if (_lyricsList[i] != e)
                              continue;
                        _lyricsList[i] = 0;
                        while (!_lyricsList.isEmpty() && _lyricsList.back() == 0)
                              _lyricsList.takeLast();
                        return;
                        }
                  }
                  qDebug("ChordRest::remove: %s %p not found", e->name(), e);
                  break;
            default:
                  qDebug("ChordRest::remove: unknown element %s", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   removeDeleteBeam
//    remove ChordRest from beam
//    delete beam if empty
//---------------------------------------------------------

void ChordRest::removeDeleteBeam()
      {
      if (_beam) {
            Beam* b = _beam;
            b->remove(this);  // this sets _beam to zero
            if (b->isEmpty())
                  delete b;
            Q_ASSERT(_beam == 0);
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant ChordRest::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SMALL:     return QVariant(small());
            case P_BEAM_MODE: return int(beamMode());
            default:          return DurationElement::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool ChordRest::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_SMALL:     setSmall(v.toBool()); break;
            case P_BEAM_MODE: setBeamMode(BeamMode(v.toInt())); break;
            default:          return DurationElement::setProperty(propertyId, v);
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant ChordRest::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SMALL:     return false;
            case P_BEAM_MODE: return BEAM_AUTO;
            default:          return DurationElement::propertyDefault(propertyId);
            }
      score()->setLayoutAll(true);
      }

