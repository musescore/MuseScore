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
#include "hairpin.h"
#include "figuredbass.h"
#include "icon.h"
#include "utils.h"
#include "keysig.h"
#include "page.h"
#include "hook.h"
#include "rehearsalmark.h"
#include "instrchange.h"

namespace Ms {

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::ChordRest(Score* s)
   : DurationElement(s)
      {
      _staffMove   = 0;
      _beam        = 0;
      _tabDur      = 0;
      _up          = true;
      _beamMode    = Beam::Mode::AUTO;
      m_isSmall    = false;
      _melismaEnd  = false;
      _crossMeasure = CrossMeasure::UNKNOWN;
      }

ChordRest::ChordRest(const ChordRest& cr, bool link)
   : DurationElement(cr)
      {
      _durationType = cr._durationType;
      _staffMove    = cr._staffMove;
      _beam         = 0;
      _tabDur       = 0;  // tab sur. symb. depends upon context: can't be
                          // simply copied from another CR

      _beamMode     = cr._beamMode;
      _up           = cr._up;
      m_isSmall     = cr.m_isSmall;
      _melismaEnd   = cr._melismaEnd;
      _crossMeasure = cr._crossMeasure;

      for (Lyrics* l : cr._lyrics) {        // make deep copy
            Lyrics* nl = new Lyrics(*l);
            if (link)
                  nl->linkTo(l);
            nl->setParent(this);
            nl->setTrack(track());
            _lyrics.push_back(nl);
            }
      }

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void ChordRest::undoUnlink()
      {
      DurationElement::undoUnlink();
      for (Lyrics* l : _lyrics)
            l->undoUnlink();
      }

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::~ChordRest()
      {
      qDeleteAll(_lyrics);
      qDeleteAll(_el);
      delete _tabDur;
      if (_beam && _beam->contains(this))
            delete _beam; // Beam destructor removes references to the deleted object
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void ChordRest::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (_beam && (_beam->elements().front() == this)
       && !measure()->stemless(staffIdx()))
            _beam->scanElements(data, func, all);
      for (Lyrics* l : _lyrics)
            l->scanElements(data, func, all);
      DurationElement* de = this;
      while (de->tuplet() && de->tuplet()->elements().front() == de) {
            de->tuplet()->scanElements(data, func, all);
            de = de->tuplet();
            }
      if (_tabDur)
            func(data, _tabDur);
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void ChordRest::writeProperties(XmlWriter& xml) const
      {
      DurationElement::writeProperties(xml);

      //
      // Beam::Mode default:
      //    REST  - Beam::Mode::NONE
      //    CHORD - Beam::Mode::AUTO
      //
      if ((isRest() && _beamMode != Beam::Mode::NONE) || (isChord() && _beamMode != Beam::Mode::AUTO)) {
            QString s;
            switch(_beamMode) {
                  case Beam::Mode::AUTO:    s = "auto"; break;
                  case Beam::Mode::BEGIN:   s = "begin"; break;
                  case Beam::Mode::MID:     s = "mid"; break;
                  case Beam::Mode::END:     s = "end"; break;
                  case Beam::Mode::NONE:    s = "no"; break;
                  case Beam::Mode::BEGIN32: s = "begin32"; break;
                  case Beam::Mode::BEGIN64: s = "begin64"; break;
                  case Beam::Mode::INVALID: s = "?"; break;
                  }
            xml.tag("BeamMode", s);
            }
      writeProperty(xml, Pid::SMALL);
      if (actualDurationType().dots())
            xml.tag("dots", actualDurationType().dots());
      writeProperty(xml, Pid::STAFF_MOVE);

      if (actualDurationType().isValid())
            xml.tag("durationType", actualDurationType().name());

      if (!ticks().isZero() && (!actualDurationType().fraction().isValid()
         || (actualDurationType().fraction() != ticks()))) {
            xml.tag("duration", ticks());
            //xml.tagE("duration z=\"%d\" n=\"%d\"", ticks().numerator(), ticks().denominator());
            }

      for (Lyrics* lyrics : _lyrics)
            lyrics->write(xml);

      const int curTick = xml.curTick().ticks();

      if (!isGrace()) {
            Fraction t(globalTicks());
            if (staff())
                  t /= staff()->timeStretch(xml.curTick());
            xml.incCurTick(t);
            }

      for (auto i : score()->spannerMap().findOverlapping(curTick - 1, curTick + 1)) {
            Spanner* s = i.value;
            if (s->generated() || !s->isSlur() || toSlur(s)->broken() || !xml.canWrite(s))
                  continue;

            if (s->startElement() == this)
                  s->writeSpannerStart(xml, this, track());
            else if (s->endElement() == this)
                  s->writeSpannerEnd(xml, this, track());
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ChordRest::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "durationType") {
            setDurationType(e.readElementText());
            if (actualDurationType().type() != TDuration::DurationType::V_MEASURE) {
                  if (score()->mscVersion() < 112 && (type() == ElementType::REST) &&
                              // for backward compatibility, convert V_WHOLE rests to V_MEASURE
                              // if long enough to fill a measure.
                              // OTOH, freshly created (un-initialized) rests have numerator == 0 (< 4/4)
                              // (see Fraction() constructor in fraction.h; this happens for instance
                              // when pasting selection from clipboard): they should not be converted
                              ticks().numerator() != 0 &&
                              // rest durations are initialized to full measure duration when
                              // created upon reading the <Rest> tag (see Measure::read() )
                              // so a V_WHOLE rest in a measure of 4/4 or less => V_MEASURE
                              (actualDurationType()==TDuration::DurationType::V_WHOLE && ticks() <= Fraction(4, 4)) ) {
                        // old pre 2.0 scores: convert
                        setDurationType(TDuration::DurationType::V_MEASURE);
                        }
                  else  // not from old score: set duration fraction from duration type
                        setTicks(actualDurationType().fraction());
                  }
            else {
                  if (score()->mscVersion() <= 114) {
                        SigEvent event = score()->sigmap()->timesig(e.tick());
                        setTicks(event.timesig());
                        }
                  }
            }
      else if (tag == "BeamMode") {
            QString val(e.readElementText());
            Beam::Mode bm = Beam::Mode::AUTO;
            if (val == "auto")
                  bm = Beam::Mode::AUTO;
            else if (val == "begin")
                  bm = Beam::Mode::BEGIN;
            else if (val == "mid")
                  bm = Beam::Mode::MID;
            else if (val == "end")
                  bm = Beam::Mode::END;
            else if (val == "no")
                  bm = Beam::Mode::NONE;
            else if (val == "begin32")
                  bm = Beam::Mode::BEGIN32;
            else if (val == "begin64")
                  bm = Beam::Mode::BEGIN64;
            else
                  bm = Beam::Mode(val.toInt());
            _beamMode = Beam::Mode(bm);
            }
      else if (tag == "Articulation" || tag == "Ornament") { // + 4.x compat
            Articulation* atr = new Articulation(score());
            atr->setTrack(track());
            atr->read(e);
            add(atr);
            }
      else if (tag == "leadingSpace" || tag == "trailingSpace") {
            qDebug("ChordRest: %s obsolete", tag.toLocal8Bit().data());
            e.skipCurrentElement();
            }
      else if (tag == "small")
            m_isSmall = e.readInt();
      else if (tag == "duration")
            setTicks(e.readFraction());
      else if (tag == "ticklen") {      // obsolete (version < 1.12)
            int mticks = score()->sigmap()->timesig(e.tick()).timesig().ticks();
            int i = e.readInt();
            if (i == 0)
                  i = mticks;
            if ((type() == ElementType::REST) && (mticks == i)) {
                  setDurationType(TDuration::DurationType::V_MEASURE);
                  setTicks(Fraction::fromTicks(i));
                  }
            else {
                  Fraction f = Fraction::fromTicks(i);
                  setTicks(f);
                  setDurationType(TDuration(f));
                  }
            }
      else if (tag == "dots")
            setDots(e.readInt());
      else if (tag == "staffMove") {
            _staffMove = e.readInt();
            if (vStaffIdx() < part()->staves()->first()->idx() || vStaffIdx() > part()->staves()->last()->idx())
                  _staffMove = 0;
            }
      else if (tag == "Spanner")
            Spanner::readSpanner(e, this, track());
      else if (tag == "Lyrics") {
            Element* element = new Lyrics(score());
            element->setTrack(e.track());
            element->read(e);
            add(element);
            }
      else if (tag == "pos") {
            QPointF pt = e.readPoint();
            setOffset(pt * spatium());
            }
//      else if (tag == "offset")
//            DurationElement::readProperties(e);
      else if (!DurationElement::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   ChordRest::readAddConnector
//---------------------------------------------------------

void ChordRest::readAddConnector(ConnectorInfoReader* info, bool pasteMode)
      {
      const ElementType type = info->type();
      switch (type) {
            case ElementType::SLUR:
                  {
                  Spanner* spanner = toSpanner(info->connector());
                  const Location& l = info->location();

                  if (info->isStart()) {
                        spanner->setTrack(l.track());
                        spanner->setTick(tick());
                        spanner->setStartElement(this);
                        if (pasteMode) {
                              score()->undoAddElement(spanner);
                              for (ScoreElement* ee : spanner->linkList()) {
                                    if (ee == spanner)
                                          continue;
                                    Spanner* ls = toSpanner(ee);
                                    ls->setTick(spanner->tick());
                                    for (ScoreElement* eee : linkList()) {
                                          ChordRest* cr = toChordRest(eee);
                                          if (cr->score() == eee->score() && cr->staffIdx() == ls->staffIdx()) {
                                                ls->setTrack(cr->track());
                                                if (ls->isSlur())
                                                      ls->setStartElement(cr);
                                                break;
                                                }
                                          }
                                    }
                              }
                        else
                              score()->addSpanner(spanner);
                        }
                  else if (info->isEnd()) {
                        spanner->setTrack2(l.track());
                        spanner->setTick2(tick());
                        spanner->setEndElement(this);
                        if (pasteMode) {
                              for (ScoreElement* ee : spanner->linkList()) {
                                    if (ee == spanner)
                                          continue;
                                    Spanner* ls = static_cast<Spanner*>(ee);
                                    ls->setTick2(spanner->tick2());
                                    for (ScoreElement* eee : linkList()) {
                                          ChordRest* cr = toChordRest(eee);
                                          if (cr->score() == eee->score() && cr->staffIdx() == ls->staffIdx()) {
                                                ls->setTrack2(cr->track());
                                                if (ls->type() == ElementType::SLUR)
                                                      ls->setEndElement(cr);
                                                break;
                                                }
                                          }
                                    }
                              }
                        }
                  else
                        qDebug("ChordRest::readAddConnector(): Slur end is neither start nor end");
                  }
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void ChordRest::setSmall(bool val)
      {
      m_isSmall = val;
      }

//---------------------------------------------------------
//   undoSetSmall
//---------------------------------------------------------

void ChordRest::undoSetSmall(bool val)
      {
      undoChangeProperty(Pid::SMALL, val);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* ChordRest::drop(EditData& data)
      {
      Element* e       = data.dropElement;
      Measure* m       = measure();
      bool fromPalette = (e->track() == -1);
      switch (e->type()) {
            case ElementType::BREATH:
                  {
                  Breath* b = toBreath(e);
                  b->setPos(QPointF());
                  // allow breath marks in voice > 1
                  b->setTrack(this->track());
                  b->setPlacement(b->track() & 1 ? Placement::BELOW : Placement::ABOVE);
                  Fraction bt = tick() + actualTicks();    

                  bt = tick() + actualTicks();
                  // TODO: insert automatically in all staves?

                  Segment* seg = m->undoGetSegment(SegmentType::Breath, bt);
                  b->setParent(seg);
                  score()->undoAddElement(b);
                  }
                  return e;

            case ElementType::BAR_LINE:
                  if (data.control())
                        score()->splitMeasure(segment());
                  else {
                        BarLine* bl = toBarLine(e);
                        bl->setPos(QPointF());
                        bl->setTrack(staffIdx() * VOICES);
                        bl->setGenerated(false);

                        if (tick() == m->tick())
                              return m->drop(data);

                        BarLine* obl = 0;
                        for (Staff* st  : staff()->staffList()) {
                              Score* score = st->score();
                              Measure* measure = score->tick2measure(m->tick());
                              Segment* seg = measure->undoGetSegmentR(SegmentType::BarLine, rtick());
                              BarLine* l;
                              if (obl == 0)
                                    obl = l = bl->clone();
                              else
                                    l = toBarLine(obl->linkedClone());
                              l->setTrack(st->idx() * VOICES);
                              l->setScore(score);
                              l->setParent(seg);
                              score->undoAddElement(l);
                              l->layout();
                              }
                        }
                  delete e;
                  return 0;

            case ElementType::CLEF:
                  score()->cmdInsertClef(toClef(e), this);
                  break;

            case ElementType::TIMESIG:
                  if (measure()->system()) {
                        EditData ndd = data;
                        // adding from palette sets pos, but normal paste does not
                        if (!fromPalette)
                              ndd.pos = pagePos();
                        // convert page-relative pos to score-relative
                        ndd.pos += measure()->system()->page()->pos();
                        return measure()->drop(ndd);
                        }
                  else {
                        delete e;
                        return 0;
                        }

            case ElementType::FERMATA:
                  e->setPlacement(track() & 1 ? Placement::BELOW : Placement::ABOVE);
                  for (Element* el: segment()->annotations())
                        if (el->isFermata() && (el->track() == track())) {
                              if (el->subtype() == e->subtype()) {
                                    delete e;
                                    return el;
                                    }
                              else {
                                    e->setPlacement(el->placement());
                                    e->setTrack(track());
                                    e->setParent(segment());
                                    score()->undoChangeElement(el, e);
                                    return e;
                                    }
                              }
                  // fall through
            case ElementType::TEMPO_TEXT:
            case ElementType::DYNAMIC:
            case ElementType::FRET_DIAGRAM:
            case ElementType::TREMOLOBAR:
            case ElementType::SYMBOL:
                  e->setTrack(track());
                  e->setParent(segment());
                  score()->undoAddElement(e);
                  return e;

            case ElementType::NOTE: {
                  Note* note = toNote(e);
                  Segment* seg = segment();
                  score()->undoRemoveElement(this);
                  Chord* chord = new Chord(score());
                  chord->setTrack(track());
                  chord->setDurationType(durationType());
                  chord->setTicks(ticks());
                  chord->setTuplet(tuplet());
                  chord->add(note);
                  score()->undoAddCR(chord, seg->measure(), seg->tick());
                  return note;
                  }

            case ElementType::HARMONY:
                  {
                  // transpose
                  Harmony* harmony = toHarmony(e);
                  Interval interval = staff()->part()->instrument(tick())->transpose();
                  if (!score()->styleB(Sid::concertPitch) && !interval.isZero()) {
                        interval.flip();
                        int rootTpc = transposeTpc(harmony->rootTpc(), interval, true);
                        int baseTpc = transposeTpc(harmony->baseTpc(), interval, true);
                        score()->undoTransposeHarmony(harmony, rootTpc, baseTpc);
                        }
                  // render
                  harmony->render();
                  }
                  // fall through
            case ElementType::TEXT:
            case ElementType::STAFF_TEXT:
            case ElementType::SYSTEM_TEXT:
            case ElementType::STICKING:
            case ElementType::STAFF_STATE:
                  // fall through
            case ElementType::REHEARSAL_MARK:
                  {
                  e->setParent(segment());
                  e->setTrack((track() / VOICES) * VOICES);
                  if (e->isRehearsalMark()) {
                        RehearsalMark* r = toRehearsalMark(e);
                        if (fromPalette)
                              r->setXmlText(score()->createRehearsalMarkText(r));
                        }
                  score()->undoAddElement(e);
                  return e;
                  }
            case ElementType::INSTRUMENT_CHANGE:
                  if (part()->instruments()->find(tick().ticks()) != part()->instruments()->end()) {
                        qDebug() << "InstrumentChange already exists at tick = " << tick().ticks();
                        delete e;
                        return 0;
                        }
                  else {
                        InstrumentChange* ic = toInstrumentChange(e);
                        ic->setParent(segment());
                        ic->setTrack((track() / VOICES) * VOICES);
                        Instrument* instr = ic->instrument();
                        Instrument* prevInstr = part()->instrument(tick());
                        if (instr && instr->isDifferentInstrument(*prevInstr))
                              ic->setupInstrument(instr);
                        score()->undoAddElement(ic);
                        return e;
                        }
            case ElementType::FIGURED_BASS:
                  {
                  bool bNew;
                  FiguredBass * fb = toFiguredBass(e);
                  fb->setParent( segment() );
                  fb->setTrack( (track() / VOICES) * VOICES );
                  fb->setTicks(ticks() );
                  fb->setOnNote(true);
                  FiguredBass::addFiguredBassToSegment(segment(), fb->track(), fb->ticks(), &bNew);
                  if (bNew)
                        score()->undoAddElement(e);
                  return e;
                  }

            case ElementType::IMAGE:
                  e->setParent(segment());
                  score()->undoAddElement(e);
                  return e;

            case ElementType::ICON:
                  {
                  switch (toIcon(e)->iconType()) {
                        case IconType::SBEAM:
                              undoChangeProperty(Pid::BEAM_MODE, int(Beam::Mode::BEGIN));
                              break;
                        case IconType::MBEAM:
                              undoChangeProperty(Pid::BEAM_MODE, int(Beam::Mode::MID));
                              break;
                        case IconType::NBEAM:
                              undoChangeProperty(Pid::BEAM_MODE, int(Beam::Mode::NONE));
                              break;
                        case IconType::BEAM32:
                              undoChangeProperty(Pid::BEAM_MODE, int(Beam::Mode::BEGIN32));
                              break;
                        case IconType::BEAM64:
                              undoChangeProperty(Pid::BEAM_MODE, int(Beam::Mode::BEGIN64));
                              break;
                        case IconType::AUTOBEAM:
                              undoChangeProperty(Pid::BEAM_MODE, int(Beam::Mode::AUTO));
                              break;
                        default:
                              break;
                        }
                  }
                  delete e;
                  break;

            case ElementType::KEYSIG:
                  {
                  KeySig* ks    = toKeySig(e);
                  KeySigEvent k = ks->keySigEvent();
                  delete ks;

                  // apply only to this stave
                  score()->undoChangeKeySig(staff(), tick(), k);
                  }
                  break;

            default:
                  if (e->isSpanner()) {
                        Spanner* spanner = toSpanner(e);
                        spanner->setTick(tick());
                        spanner->setTrack(track());
                        spanner->setTrack2(track());
                        score()->undoAddElement(spanner);
                        return e;
                        }
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
      _crossMeasure = CrossMeasure::UNKNOWN;
      }

void ChordRest::setDurationType(const QString& s)
      {
      _durationType.setType(s);
      _crossMeasure = CrossMeasure::UNKNOWN;
      }

void ChordRest::setDurationType(const Fraction& ticks)
      {
      _durationType.setVal(ticks.ticks());
      _crossMeasure = CrossMeasure::UNKNOWN;
      }

void ChordRest::setDurationType(TDuration v)
      {
      _durationType = v;
      _crossMeasure = CrossMeasure::UNKNOWN;
      }

//---------------------------------------------------------
//   durationUserName
//---------------------------------------------------------

QString ChordRest::durationUserName() const
      {
      QString tupletType = "";
      if (tuplet()) {
              switch (tuplet()->ratio().numerator()) {
                  case 2:
                        tupletType = QObject::tr("Duplet");
                        break;
                  case 3:
                        tupletType = QObject::tr("Triplet");
                        break;
                  case 4:
                        tupletType = QObject::tr("Quadruplet");
                        break;
                  case 5:
                        tupletType = QObject::tr("Quintuplet");
                        break;
                  case 6:
                        tupletType = QObject::tr("Sextuplet");
                        break;
                  case 7:
                        tupletType = QObject::tr("Septuplet");
                        break;
                  case 8:
                        tupletType = QObject::tr("Octuplet");
                        break;
                  case 9:
                        tupletType = QObject::tr("Nonuplet");
                        break;
                  default:
                        //: %1 is tuplet ratio numerator (i.e. the number of notes in the tuplet)
                        tupletType = QObject::tr("%1 note tuplet").arg(tuplet()->ratio().numerator());
                  }
            }
      QString dotString = "";
      if(!tupletType.isEmpty())
          dotString += " ";

      switch (dots()) {
            case 1:
                  dotString += QObject::tr("Dotted %1").arg(durationType().durationTypeUserName()).trimmed();
                  break;
            case 2:
                  dotString += QObject::tr("Double dotted %1").arg(durationType().durationTypeUserName()).trimmed();
                  break;
            case 3:
                  dotString += QObject::tr("Triple dotted %1").arg(durationType().durationTypeUserName()).trimmed();
                  break;
            case 4:
                  dotString += QObject::tr("Quadruple dotted %1").arg(durationType().durationTypeUserName()).trimmed();
                  break;
            default:
                  dotString += durationType().durationTypeUserName();
            }
      return QString("%1%2").arg(tupletType, dotString);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void ChordRest::add(Element* e)
      {
      e->setParent(this);
      e->setTrack(track());
      switch (e->type()) {
            case ElementType::ARTICULATION:     // for backward compatibility
                  qDebug("ChordRest::add: unknown element %s", e->name());
                  break;
            case ElementType::LYRICS:
                  if (e->isStyled(Pid::OFFSET))
                        e->setOffset(e->propertyDefault(Pid::OFFSET).toPointF());
                  _lyrics.push_back(toLyrics(e));
                  break;
            default:
                  qFatal("ChordRest::add: unknown element %s", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void ChordRest::remove(Element* e)
      {
      switch (e->type()) {
            case ElementType::LYRICS: {
                  toLyrics(e)->removeFromScore();
                  auto i = std::find(_lyrics.begin(), _lyrics.end(), toLyrics(e));
                  if (i != _lyrics.end())
                        _lyrics.erase(i);
                  else
                        qDebug("ChordRest::remove: %s %p not found", e->name(), e);
                  }
                  break;
            default:
                  qFatal("ChordRest::remove: unknown element <%s>", e->name());
            }
      }

//---------------------------------------------------------
//   removeDeleteBeam
///   Remove ChordRest from beam, delete beam if empty.
///   \param beamed - if the chordrest is beamed (will get
///                   a (new) beam)
//---------------------------------------------------------

void ChordRest::removeDeleteBeam(bool beamed)
      {
      if (_beam) {
            Beam* b = _beam;
            _beam->remove(this);
            if (b->empty())
                  score()->undoRemoveElement(b);
            else
                  b->layout1();
            }
      if (!beamed && isChord())
            toChord(this)->layoutStem();
      }

//---------------------------------------------------------
//   replaceBeam
//---------------------------------------------------------

void ChordRest::replaceBeam(Beam* newBeam)
      {
      if (_beam == newBeam)
            return;
      removeDeleteBeam(true);
      newBeam->add(this);
      }

//---------------------------------------------------------
//   undoSetBeamMode
//---------------------------------------------------------

void ChordRest::undoSetBeamMode(Beam::Mode mode)
      {
      undoChangeProperty(Pid::BEAM_MODE, int(mode));
      }

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void ChordRest::localSpatiumChanged(qreal oldValue, qreal newValue)
      {
      DurationElement::localSpatiumChanged(oldValue, newValue);
      for (Element* e : lyrics())
            e->localSpatiumChanged(oldValue, newValue);
      for (Element* e : el())
            e->localSpatiumChanged(oldValue, newValue);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant ChordRest::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SMALL:      return QVariant(isSmall());
            case Pid::BEAM_MODE:  return int(beamMode());
            case Pid::STAFF_MOVE: return staffMove();
            case Pid::DURATION_TYPE: return QVariant::fromValue(actualDurationType());
            default:               return DurationElement::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool ChordRest::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::SMALL:
                  setSmall(v.toBool());
                  break;
            case Pid::BEAM_MODE:
                  setBeamMode(Beam::Mode(v.toInt()));
                  break;
            case Pid::STAFF_MOVE:
                  setStaffMove(v.toInt());
                  break;
            case Pid::VISIBLE:
                  setVisible(v.toBool());
                  measure()->checkMultiVoices(staffIdx());
                  break;
            case Pid::DURATION_TYPE:
                  setDurationType(v.value<TDuration>());
                  break;
            default:
                  return DurationElement::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant ChordRest::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SMALL:
                  return false;
            case Pid::BEAM_MODE:
                  return int(Beam::Mode::AUTO);
            case Pid::STAFF_MOVE:
                  return 0;
            default:
                  return DurationElement::propertyDefault(propertyId);
            }
      // Prevent unreachable code warning
      // triggerLayout();
      }

//---------------------------------------------------------
//   isGrace
//---------------------------------------------------------

bool ChordRest::isGrace() const
      {
      return isChord() && toChord(this)->isGrace();
      }

//---------------------------------------------------------
//   isGraceBefore
//---------------------------------------------------------

bool ChordRest::isGraceBefore() const
      {
      return isChord()
         && (toChord(this)->noteType() & (
           NoteType::ACCIACCATURA | NoteType::APPOGGIATURA | NoteType::GRACE4 | NoteType::GRACE16 | NoteType::GRACE32
           ));
      }

//---------------------------------------------------------
//   isGraceAfter
//---------------------------------------------------------

bool ChordRest::isGraceAfter() const
      {
      return isChord()
         && (toChord(this)->noteType() & (NoteType::GRACE8_AFTER | NoteType::GRACE16_AFTER | NoteType::GRACE32_AFTER));
      }

//---------------------------------------------------------
//   hasBreathMark - determine if chordrest has breath-mark
//---------------------------------------------------------
Breath* ChordRest::hasBreathMark() const
      {
      Fraction end = tick() + actualTicks();
      Segment* s = measure()->findSegment(SegmentType::Breath, end);
      return s ? toBreath(s->element(track())) : 0;
      }

//---------------------------------------------------------
//   writeBeam
//---------------------------------------------------------

void ChordRest::writeBeam(XmlWriter& xml) const
      {
      Beam* b = beam();
      if (b && b->elements().front() == this && (MScore::testMode || !b->generated())) {
            b->write(xml);
            }
      }

//---------------------------------------------------------
//   nextSegmentAfterCR
//    returns first segment at tick CR->tick + CR->actualTicks
//    of given types
//---------------------------------------------------------

Segment* ChordRest::nextSegmentAfterCR(SegmentType types) const
      {
      Fraction end = tick() + actualTicks();
      for (Segment* s = segment()->next1MM(types); s; s = s->next1MM(types)) {
            // chordrest ends at afrac+actualFraction
            // we return the segment at or after the end of the chordrest
            // Segment::afrac() is based on ticks; use DurationElement::afrac() if possible
            Element* e = s;
            if (s->isChordRestType()) {
                  // Find the first non-NULL element in the segment
                  for (Element* ee : s->elist()) {
                        if (ee) {
                              e = ee;
                              break;
                              }
                        }
                  }
            if (e->tick() >= end)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void ChordRest::setTrack(int val)
      {
      Element::setTrack(val);
      processSiblings([val] (Element* e) { e->setTrack(val); } );
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ChordRest::setScore(Score* s)
      {
      Element::setScore(s);
      processSiblings([s] (Element* e) { e->setScore(s); } );
      }

//---------------------------------------------------------
//   processSiblings
//---------------------------------------------------------

void ChordRest::processSiblings(std::function<void(Element*)> func)
      {
      if (_beam)
            func(_beam);
      if (_tabDur)
            func(_tabDur);
      for (Lyrics* l : _lyrics)
            func(l);
      if (tuplet())
            func(tuplet());
      }

//---------------------------------------------------------
//   nextArticulationOrLyric
//---------------------------------------------------------

Element* ChordRest::nextArticulationOrLyric(Element* e)
      {
      if (isChord() && e->isArticulation()) {
            Chord* c = toChord(this);
            auto i = std::find(c->articulations().begin(), c->articulations().end(), e);
            if (i != c->articulations().end()) {
                  if (i != c->articulations().end() - 1) {
                        return *(i+1);
                        }
                  else {
                        if (!_lyrics.empty())
                              return _lyrics[0];
                        else
                              return nullptr;
                        }
                  }
            }
      else {
            auto i = std::find(_lyrics.begin(), _lyrics.end(), e);
            if (i != _lyrics.end()) {
                  if (i != _lyrics.end()-1)
                      return *(i+1);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   prevArticulationOrLyric
//---------------------------------------------------------

Element* ChordRest::prevArticulationOrLyric(Element* e)
      {
      auto i = std::find(_lyrics.begin(), _lyrics.end(), e);
      if (i != _lyrics.end()) {
            if (i != _lyrics.begin()) {
                  return *(i-1);
                  }
            else {
                  if (isChord() && !toChord(this)->articulations().empty())
                        return toChord(this)->articulations().back();
                  else
                        return nullptr;
                  }
            }
      else if (isChord() && e->isArticulation()) {
            Chord* c = toChord(this);
            auto j = std::find(c->articulations().begin(), c->articulations().end(), e);
            if (j != c->articulations().end()) {
                  if (j != c->articulations().begin())
                        return *(j-1);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* ChordRest::nextElement()
      {
      Element* e = score()->selection().element();
      if (!e && !score()->selection().elements().isEmpty())
            e = score()->selection().elements().first();
      switch (e->type()) {
            case ElementType::ARTICULATION:
            case ElementType::LYRICS: {
                  Element* next = nextArticulationOrLyric(e);
                  if (next)
                        return next;
                  else
                        break;
                  }
            default: {
                  if (isChord() && !toChord(this)->articulations().empty())
                        return toChord(this)->articulations()[0];
                  else if (!_lyrics.empty())
                        return _lyrics[0];
                  else
                        break;
                  }
            }
      int staffId = e->staffIdx();
      return segment()->nextElement(staffId);
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* ChordRest::prevElement()
      {
      Element* e = score()->selection().element();
      if (!e && !score()->selection().elements().isEmpty())
            e = score()->selection().elements().last();
      switch (e->type()) {
            case ElementType::ARTICULATION:
            case ElementType::LYRICS: {
                  Element* prev = prevArticulationOrLyric(e);
                  if (prev)
                        return prev;
                  else {
                        if (isChord())
                              return toChord(this)->lastElementBeforeSegment();
                        }
                  // fall through
                  }
            default: {
                  break;
                  }
            }
      int staffId = e->staffIdx();
      return segment()->prevElement(staffId);
      }

//---------------------------------------------------------
//   lastElementBeforeSegment
//---------------------------------------------------------

Element* ChordRest::lastElementBeforeSegment()
      {
      if (!_lyrics.empty())
            return _lyrics.back();
//      else if (!_articulations.empty()) {           // TODO:fermata
//            return _articulations.back();
//            }
      else
            return 0;
      }

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* ChordRest::nextSegmentElement()
      {
      return segment()->firstInNextSegments(staffIdx());
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* ChordRest::prevSegmentElement()
      {
      return segment()->lastInPrevSegments(staffIdx());
      }

QString ChordRest::accessibleExtraInfo() const
      {
      QString rez = "";
      for (Element* l : lyrics()) {
            if (!score()->selectionFilter().canSelect(l))
                  continue;
            rez = QString("%1 %2").arg(rez, l->screenReaderInfo());
            }

      if (segment()) {
            for (Element* e : segment()->annotations()) {
                  if (!score()->selectionFilter().canSelect(e))
                        continue;
                  if (e->track() == track())
                        rez = QString("%1 %2").arg(rez, e->screenReaderInfo());
                  }

            SpannerMap& smap = score()->spannerMap();
            auto spanners = smap.findOverlapping(tick().ticks(), tick().ticks());
            for (auto interval : spanners) {
                  Spanner* s = interval.value;
                  if (!score()->selectionFilter().canSelect(s))
                        continue;
                  if (s->type() == ElementType::VOLTA || //voltas are added for barlines
                      s->type() == ElementType::TIE    ) //ties are added in notes
                        continue;

                  if (s->type() == ElementType::SLUR) {
                        if (s->tick() == tick() && s->track() == track())
                              rez = QObject::tr("%1 Start of %2").arg(rez, s->screenReaderInfo());
                        if (s->tick2() == tick() && s->track2() == track())
                              rez = QObject::tr("%1 End of %2").arg(rez, s->screenReaderInfo());
                        }
                  else if (s->staffIdx() == staffIdx()) {
                        bool start = s->tick()  == tick();
                        bool end   = s->tick2() == tick() + ticks();
                        if (start && end)
                              rez = QObject::tr("%1 Start and end of %2").arg(rez, s->screenReaderInfo());
                        else if (start)
                              rez = QObject::tr("%1 Start of %2").arg(rez, s->screenReaderInfo());
                        else if (end)
                              rez = QObject::tr("%1 End of %2").arg(rez, s->screenReaderInfo());
                        }
                  }
            }
      return rez;
      }

//---------------------------------------------------------
//   isMelismaEnd
//    returns true if chordrest represents the end of a melisma
//---------------------------------------------------------

bool ChordRest::isMelismaEnd() const
      {
      return _melismaEnd;
      }

//---------------------------------------------------------
//   setMelismaEnd
//---------------------------------------------------------

void ChordRest::setMelismaEnd(bool v)
      {
      _melismaEnd = v;
      // TODO: don't take "false" at face value
      // check to see if some other melisma ends here,
      // in which case we can leave this set to true
      // for now, rely on the fact that we'll generate the value correctly on layout
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape ChordRest::shape() const
      {
      Shape shape;
      {
      qreal x1 = 1000000.0;
      qreal x2 = -1000000.0;
      bool adjustWidth = false;
      for (Lyrics* l : _lyrics) {
            if (!l || !l->addToSkyline())
                  continue;
            qreal lmargin = score()->styleS(Sid::lyricsMinDistance).val() * spatium() * 0.5;
            qreal rmargin = lmargin;
            Lyrics::Syllabic syl = l->syllabic();
            if ((syl == Lyrics::Syllabic::BEGIN || syl == Lyrics::Syllabic::MIDDLE) && score()->styleB(Sid::lyricsDashForce))
                  rmargin = qMax(rmargin, styleP(Sid::lyricsDashMinLength));
            // for horizontal spacing we only need the lyrics width:
            x1 = qMin(x1, l->bbox().x() - lmargin + l->pos().x());
            x2 = qMax(x2, l->bbox().x() + l->bbox().width() + rmargin + l->pos().x());
            if (l->ticks() == Fraction::fromTicks(Lyrics::TEMP_MELISMA_TICKS))
                  x2 += spatium();
            adjustWidth = true;
            }
      if (adjustWidth)
            shape.addHorizontalSpacing(Shape::SPACING_LYRICS, x1, x2);
      }

      {
      qreal x1 = 1000000.0;
      qreal x2 = -1000000.0;
      bool adjustWidth = false;
      for (Element* e : segment()->annotations()) {
            if (!e || !e->addToSkyline())
                  continue;
            if (e->isHarmony() && e->staffIdx() == staffIdx()) {
                  Harmony* h = toHarmony(e);
                  // calculate bbox only (do not reset position)
                  if (h->bbox().isEmpty()) h->layout1();
                  const qreal margin = styleP(Sid::minHarmonyDistance) * 0.5;
                  x1 = qMin(x1, e->bbox().x() - margin + e->pos().x());
                  x2 = qMax(x2, e->bbox().x() + e->bbox().width() + margin + e->pos().x());
                  adjustWidth = true;
                  }
            else if (e->isFretDiagram()) {
                  FretDiagram* fd = toFretDiagram(e);
                  qreal margin = styleP(Sid::fretMinDistance) * 0.5;
                  bool firstBeat = tick() == measure()->tick();
                  if (fd->pos().x() == 0)
                        fd->layoutHorizontal();
                  else if (fd->bbox().isEmpty())
                        fd->calculateBoundingRect();
                  qreal leftX = firstBeat ? 0 : e->bbox().x() - margin + e->pos().x();
                  qreal rightX = e->bbox().x() + e->bbox().width() + margin + e->pos().x();
                  x1 = qMin(x1, leftX);
                  x2 = qMax(x2, rightX);
                  adjustWidth = true;
                  if (fd->harmony()) {
                        Harmony* h = fd->harmony();
                        margin = styleP(Sid::minHarmonyDistance) * 0.5;
                        if (h->bbox().isEmpty())
                              h->layout1();
                        leftX = firstBeat ? 0 : h->bbox().x() - margin + h->pos().x() + e->pos().x();
                        rightX = h->bbox().x() + h->bbox().width() + margin + h->pos().x() + e->pos().x();
                        x1 = qMin(x1, leftX);
                        x2 = qMax(x2, rightX);
                        adjustWidth = true;
                        }
                  }
            }
      if (adjustWidth)
            shape.addHorizontalSpacing(Shape::SPACING_HARMONY, x1, x2);
      }

      if (isMelismaEnd()) {
            qreal right = rightEdge();
            shape.addHorizontalSpacing(Shape::SPACING_LYRICS, right, right);
            }

      return shape;
      }

//---------------------------------------------------------
//   lyrics
//---------------------------------------------------------

Lyrics* ChordRest::lyrics(int no) const
      {
      for (Lyrics* l : _lyrics) {
            if (l->no() == no)
                  return l;
            }
      return 0;
      }

Lyrics* ChordRest::lyrics(int no, Placement p) const
      {
      for (Lyrics* l : _lyrics) {
            if (l->placement() == p && l->no() == no)
                  return l;
            }
      return 0;
      }

//---------------------------------------------------------
//   lastVerse
//    return last verse number (starting from 0)
//    return -1 if there are no lyrics;
//---------------------------------------------------------

int ChordRest::lastVerse(Placement p) const
      {
      int lastVerse = -1;

      for (Lyrics* l : _lyrics) {
            if (l->placement() == p && l->no() > lastVerse)
                  lastVerse = l->no();
            }

      return lastVerse;
      }

//---------------------------------------------------------
//   removeMarkings
//    - this is normally called after cloning a chord to tie a note over the barline
//    - there is no special undo handling; the assumption is that undo will simply remove the cloned chord
//    - two note tremolos are converted into simple notes
//    - single note tremolos are optionally retained
//---------------------------------------------------------

void ChordRest::removeMarkings(bool /* keepTremolo */)
      {
      qDeleteAll(el());
      el().clear();
      qDeleteAll(lyrics());
      lyrics().clear();
      }

//---------------------------------------------------------
//   isBefore
//---------------------------------------------------------

bool ChordRest::isBefore(const ChordRest* o) const
      {
      if (!o || this == o)
            return false;
      int otick = o->tick().ticks();
      int t     = tick().ticks();
      if (t == otick) { // At least one of the chord is a grace, order the grace notes
            bool oGraceAfter = o->isGraceAfter();
            bool graceAfter  = isGraceAfter();
            bool oGrace      = o->isGrace();
            bool grace       = isGrace();
            // normal note are initialized at graceIndex 0 and graceIndex is 0 based
            int oGraceIndex  = toChord(o)->graceIndex();
            int graceIndex   = toChord(this)->graceIndex();
            // Smaller indexes are further away from the note, and larger indexes are closer to the note.
            // We want to reverse that. Subtracting a 0-based index from the size results in a 1-based index,
            // which is exactly what we want.
            if (oGrace)
                  oGraceIndex = toChord(o->parent())->graceNotes().size() - oGraceIndex;
            if (grace)
                  graceIndex = toChord(parent())->graceNotes().size() - graceIndex;
            otick = otick + (oGraceAfter ? 1 : -1) *  oGraceIndex;
            t     = t + (graceAfter ? 1 : -1) *  graceIndex;
            }
      return t < otick;
      }

//---------------------------------------------------------
//   undoAddAnnotation
//---------------------------------------------------------

void ChordRest::undoAddAnnotation(Element* a)
      {
      Segment* seg = segment();
      Measure* m = measure();
      if (m && m->isMMRest())
            seg = m->mmRestFirst()->findSegmentR(SegmentType::ChordRest, Fraction(0,1));

      a->setTrack(a->systemFlag() ? 0 : track());
      a->setParent(seg);
      score()->undoAddElement(a);
      }

}

