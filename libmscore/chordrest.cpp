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
#include "figuredbass.h"
#include "icon.h"
#include "utils.h"
#include "keysig.h"
#include "page.h"
#include "hook.h"

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
      _small       = false;
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
      _small        = cr._small;
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
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void ChordRest::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (_beam && (_beam->elements().front() == this)
       && !measure()->slashStyle(staffIdx()))
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

      if (!duration().isZero() && (!actualDurationType().fraction().isValid()
         || (actualDurationType().fraction() != duration()))) {
            xml.tag("duration", duration());
            //xml.tagE("duration z=\"%d\" n=\"%d\"", duration().numerator(), duration().denominator());
            }

#ifndef NDEBUG
      if (_beam && (MScore::testMode || !_beam->generated()))
            xml.tag("Beam", _beam->id());
#else
      if (_beam && !_beam->generated())
            xml.tag("Beam", _beam->id());
#endif
      for (Lyrics* lyrics : _lyrics)
            lyrics->write(xml);
      if (!isGrace()) {
            Fraction t(globalDuration());
            if (staff())
                  t /= staff()->timeStretch(xml.curTick());
            xml.incCurTick(t.ticks());
            }
      for (auto i : score()->spanner()) {     // TODO: dont search whole list
            Spanner* s = i.second;
            if (s->generated() || !s->isSlur() || toSlur(s)->broken() || !xml.canWrite(s))
                  continue;

            if (s->startElement() == this) {
                  int id = xml.spannerId(s);
                  xml.tagE(QString("Slur type=\"start\" id=\"%1\"").arg(id));
                  }
            else if (s->endElement() == this) {
                  int id = xml.spannerId(s);
                  xml.tagE(QString("Slur type=\"stop\" id=\"%1\"").arg(id));
                  }
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
                              duration().numerator() != 0 &&
                              // rest durations are initialized to full measure duration when
                              // created upon reading the <Rest> tag (see Measure::read() )
                              // so a V_WHOLE rest in a measure of 4/4 or less => V_MEASURE
                              (actualDurationType()==TDuration::DurationType::V_WHOLE && duration() <= Fraction(4, 4)) ) {
                        // old pre 2.0 scores: convert
                        setDurationType(TDuration::DurationType::V_MEASURE);
                        }
                  else  // not from old score: set duration fraction from duration type
                        setDuration(actualDurationType().fraction());
                  }
            else {
                  if (score()->mscVersion() <= 114) {
                        SigEvent event = score()->sigmap()->timesig(e.tick());
                        setDuration(event.timesig());
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
      else if (tag == "Articulation") {
            Articulation* atr = new Articulation(score());
            atr->setTrack(track());
            atr->read(e);
            add(atr);
            }
      else if (tag == "leadingSpace" || tag == "trailingSpace") {
            qDebug("ChordRest: %s obsolete", tag.toLocal8Bit().data());
            e.skipCurrentElement();
            }
      else if (tag == "Beam") {
            int id = e.readInt();
            Beam* beam = e.findBeam(id);
            if (beam)
                  beam->add(this);        // also calls this->setBeam(beam)
            else
                  qDebug("Beam id %d not found", id);
            }
      else if (tag == "small")
            _small = e.readInt();
      else if (tag == "duration")
            setDuration(e.readFraction());
      else if (tag == "ticklen") {      // obsolete (version < 1.12)
            int mticks = score()->sigmap()->timesig(e.tick()).timesig().ticks();
            int i = e.readInt();
            if (i == 0)
                  i = mticks;
            if ((type() == ElementType::REST) && (mticks == i)) {
                  setDurationType(TDuration::DurationType::V_MEASURE);
                  setDuration(Fraction::fromTicks(i));
                  }
            else {
                  Fraction f = Fraction::fromTicks(i);
                  setDuration(f);
                  setDurationType(TDuration(f));
                  }
            }
      else if (tag == "dots")
            setDots(e.readInt());
      else if (tag == "move")
            _staffMove = e.readInt();
      else if (tag == "Slur") {
            int id = e.intAttribute("id");
            if (id == 0)
                  id = e.intAttribute("number");                  // obsolete
            Spanner* spanner = e.findSpanner(id);
            QString atype(e.attribute("type"));

            if (!spanner) {
                  if (atype == "stop") {
                        SpannerValues sv;
                        sv.spannerId = id;
                        sv.track2    = track();
                        sv.tick2     = e.tick();
                        e.addSpannerValues(sv);
                        }
                  else if (atype == "start")
                        qDebug("spanner: start without spanner");
                  }
            else {
                  if (atype == "start") {
                        if (spanner->ticks() > 0 && spanner->tick() == -1) // stop has been read first
                              spanner->setTicks(spanner->ticks() - e.tick() - 1);
                        spanner->setTick(e.tick());
                        spanner->setTrack(track());
                        if (spanner->type() == ElementType::SLUR)
                              spanner->setStartElement(this);
                        if (e.pasteMode()) {
                              for (ScoreElement* e : spanner->linkList()) {
                                    if (e == spanner)
                                          continue;
                                    Spanner* ls = static_cast<Spanner*>(e);
                                    ls->setTick(spanner->tick());
                                    for (ScoreElement* ee : linkList()) {
                                          ChordRest* cr = toChordRest(ee);
                                          if (cr->score() == ee->score() && cr->staffIdx() == ls->staffIdx()) {
                                                ls->setTrack(cr->track());
                                                if (ls->type() == ElementType::SLUR)
                                                      ls->setStartElement(cr);
                                                break;
                                                }
                                          }
                                    }
                              }
                        }
                  else if (atype == "stop") {
                        spanner->setTick2(e.tick());
                        spanner->setTrack2(track());
                        if (spanner->isSlur())
                              spanner->setEndElement(this);
                        ChordRest* start = toChordRest(spanner->startElement());
                        if (start)
                              spanner->setTrack(start->track());
                        if (e.pasteMode()) {
                              for (ScoreElement* e : spanner->linkList()) {
                                    if (e == spanner)
                                          continue;
                                    Spanner* ls = static_cast<Spanner*>(e);
                                    ls->setTick2(spanner->tick2());
                                    for (ScoreElement* ee : linkList()) {
                                          ChordRest* cr = toChordRest(ee);
                                          if (cr->score() == ee->score() && cr->staffIdx() == ls->staffIdx()) {
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
                        qDebug("ChordRest::read(): unknown Slur type <%s>", qPrintable(atype));
                  }
            e.readNext();
            }
      else if (tag == "Lyrics" /*|| tag == "FiguredBass"*/) {
            Element* element = Element::name2Element(tag, score());
            element->setTrack(e.track());
            element->read(e);
            add(element);
            }
      else if (tag == "pos") {
            QPointF pt = e.readPoint();
            setUserOff(pt * spatium());
            }
      else if (tag == "offset")
            DurationElement::readProperties(e);
      else if (!DurationElement::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void ChordRest::setSmall(bool val)
      {
      _small = val;
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
      Element* e       = data.element;
      Measure* m       = measure();
      bool fromPalette = (e->track() == -1);
      switch (e->type()) {
            case ElementType::BREATH:
                  {
                  Breath* b = toBreath(e);
                  b->setPos(QPointF());
                  int track = staffIdx() * VOICES;
                  b->setTrack(track);

                  // find start tick of next note in staff
#if 0
                  int bt = tick() + actualTicks();    // this could make sense if we allowed breath marks in voice > 1
#else
                  Segment* next = segment()->nextCR(track);
                  int bt = next ? next->tick() : score()->lastSegment()->tick();
#endif

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

            case ElementType::TEMPO_TEXT:
            case ElementType::DYNAMIC:
            case ElementType::FRET_DIAGRAM:
            case ElementType::TREMOLOBAR:
            case ElementType::SYMBOL:
            case ElementType::FERMATA:
                  e->setTrack(track());
                  e->setParent(segment());
                  score()->undoAddElement(e);
                  return e;

            case ElementType::NOTE: {
                  Note* note = toNote(e);
                  NoteVal nval;
                  nval.pitch = note->pitch();
                  nval.tpc1 = note->tpc1();
                  nval.headGroup = note->headGroup();
                  nval.fret = note->fret();
                  nval.string = note->string();
                  score()->setNoteRest(segment(), track(), nval, duration(), Direction::AUTO);
                  delete e;
                  }
                  break;

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
            case ElementType::STAFF_STATE:
            case ElementType::INSTRUMENT_CHANGE:
                  if (e->isInstrumentChange() && part()->instruments()->find(tick()) != part()->instruments()->end()) {
                        qDebug()<<"InstrumentChange already exists at tick = "<<tick();
                        delete e;
                        return 0;
                        }
                  // fall through
            case ElementType::REHEARSAL_MARK:
                  e->setParent(segment());
                  e->setTrack((track() / VOICES) * VOICES);
                  score()->undoAddElement(e);
                  return e;

            case ElementType::FIGURED_BASS:
                  {
                  bool bNew;
                  FiguredBass * fb = toFiguredBass(e);
                  fb->setParent( segment() );
                  fb->setTrack( (track() / VOICES) * VOICES );
                  fb->setTicks( duration().ticks() );
                  fb->setOnNote(true);
                  FiguredBass::addFiguredBassToSegment(segment(),
                        fb->track(), fb->ticks(), &bNew);
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

void ChordRest::setDurationType(int ticks)
      {
      _durationType.setVal(ticks);
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
                        tupletType = QObject::tr("Custom Tuplet");
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
      return QString("%1%2").arg(tupletType).arg(dotString);
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
//      beamed - the chordrest is beamed (will get a (new) beam)
//          remove ChordRest from beam
//          delete beam if empty
//---------------------------------------------------------

void ChordRest::removeDeleteBeam(bool beamed)
      {
      if (_beam) {
            Beam* b = _beam;
            _beam->remove(this);
            if (b->empty())
                  score()->undoRemoveElement(b);
            }
      if (!beamed && isChord())
            toChord(this)->layoutStem();
      }

//---------------------------------------------------------
//   undoSetBeamMode
//---------------------------------------------------------

void ChordRest::undoSetBeamMode(Beam::Mode mode)
      {
      undoChangeProperty(Pid::BEAM_MODE, int(mode));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant ChordRest::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SMALL:      return QVariant(small());
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
      triggerLayout();
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
//   writeBeam
//---------------------------------------------------------

void ChordRest::writeBeam(XmlWriter& xml)
      {
      Beam* b = beam();
      if (b && b->elements().front() == this && (MScore::testMode || !b->generated())) {
            b->setId(xml.nextBeamId());
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
      for (Segment* s = segment()->next1MM(types); s; s = s->next1MM(types)) {
            // chordrest ends at tick+actualTicks
            // we return the segment at or after the end of the chordrest
            if (s->tick() >= tick() + actualTicks())
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

Element* ChordRest::nextArticulationOrLyric(Element* /*e*/)
      {
#if 0 // TODO:fermata
      auto i = std::find(_articulations.begin(), _articulations.end(), e);
      if (i != _articulations.end()) {
            if (i != _articulations.end()-1) {
                  return *(i+1);
                  }
            else {
                  if (!_lyrics.empty())
                        return _lyrics[0];
                  else
                        return nullptr;
                  }
            }
      else {
            auto i = std::find(_lyrics.begin(), _lyrics.end(), e);
            if (i != _lyrics.end()) {
                  if (i != _lyrics.end()-1)
                      return *(i+1);
                  }
            }
#endif
      return 0;
      }

//---------------------------------------------------------
//   prevArticulationOrLyric
//---------------------------------------------------------

Element* ChordRest::prevArticulationOrLyric(Element* /*e*/)
      {
#if 0 // TODO:fermata
      auto i = std::find(_lyrics.begin(), _lyrics.end(), e);
      if (i != _lyrics.end()) {
            if (i != _lyrics.begin()) {
                  return *(i-1);
                  }
            else {
                  if (!_articulations.empty())
                        return _articulations.back();
                  else
                        return nullptr;
                  }
            }
      else {
            auto i = std::find(_articulations.begin(), _articulations.end(), e);
            if (i != _articulations.end()) {
                  if (i != _articulations.begin())
                        return *(i-1);
                  }
            }
#endif
      return 0;
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* ChordRest::nextElement()
      {
      Element* e = score()->selection().element();
#if 0 // TODO:fermata
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
                  if (!_articulations.empty())
                        return _articulations[0];
                  else if (!_lyrics.empty())
                        return _lyrics[0];
                  else
                        break;
                  }
            }
#endif
      int staffId = e->staffIdx();
      return segment()->nextElement(staffId);
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* ChordRest::prevElement()
      {
      Element* e = score()->selection().element();
#if 0 // TODO:fermata
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
#endif
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
#if 0       // TODO:fermata
      for (Articulation* a : articulations()) {
            if (!score()->selectionFilter().canSelect(a))
                  continue;
            rez = QString("%1 %2").arg(rez).arg(a->screenReaderInfo());
            }
#endif
      for (Element* l : lyrics()) {
            if (!score()->selectionFilter().canSelect(l))
                  continue;
            rez = QString("%1 %2").arg(rez).arg(l->screenReaderInfo());
            }

      if (segment()) {
            for (Element* e : segment()->annotations()) {
                  if (!score()->selectionFilter().canSelect(e))
                        continue;
                  if (e->staffIdx() == staffIdx() )
                        rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
                  }

            SpannerMap& smap = score()->spannerMap();
            auto spanners = smap.findOverlapping(tick(), tick());
            for (auto interval : spanners) {
                  Spanner* s = interval.value;
                  if (!score()->selectionFilter().canSelect(s))
                        continue;
                  if (s->type() == ElementType::VOLTA || //voltas are added for barlines
                      s->type() == ElementType::TIE    ) //ties are added in notes
                        continue;

                  Segment* seg = 0;
                  if (s->type() == ElementType::SLUR) {
                        if (s->tick() == tick() && s->track() == track())
                              rez = QObject::tr("%1 Start of %2").arg(rez).arg(s->screenReaderInfo());
                        if (s->tick2() == tick() && s->track2() == track())
                              rez = QObject::tr("%1 End of %2").arg(rez).arg(s->screenReaderInfo());
                        }
                  else  {
                        if (s->tick() == tick() && s->staffIdx() == staffIdx())
                              rez = QObject::tr("%1 Start of %2").arg(rez).arg(s->screenReaderInfo());
                        seg = segment()->next1MM(SegmentType::ChordRest);
                        if (!seg)
                              continue;
                        if (s->tick2() == seg->tick() && s->staffIdx() == staffIdx())
                              rez = QObject::tr("%1 End of %2").arg(rez).arg(s->screenReaderInfo());
                        }
                  }
            }
      return rez;
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape ChordRest::shape() const
      {
      Shape shape;
      qreal margin = spatium() * .5;
      qreal x1 = 1000000.0;
      qreal x2 = -1000000.0;
      for (Lyrics* l : _lyrics) {
            if (l->autoplace())
                  l->rUserYoffset() = 0.0;
            // for horizontal spacing we only need the lyrics width:
            x1 = qMin(x1, l->bbox().x() - margin + l->pos().x());
            x2 = qMax(x2, x1 + l->bbox().width() + margin);
            }
      if (x2 > x1)
            shape.add(QRectF(x1, 1.0, x2-x1, 0.0));
      return shape;
      }

//---------------------------------------------------------
//   lyrics
//---------------------------------------------------------

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
//   flipLyrics
//---------------------------------------------------------

void ChordRest::flipLyrics(Lyrics* l)
      {
      Placement p = l->placement();
      if (p == Placement::ABOVE)
            p = Placement::BELOW;
      else
            p = Placement::ABOVE;
      int verses = lastVerse(p);
      l->undoChangeProperty(Pid::VERSE, verses + 1);
      l->undoChangeProperty(Pid::AUTOPLACE, true);
      l->undoChangeProperty(Pid::PLACEMENT, int(p));
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
      if (isChord())
            qDeleteAll(toChord(this)->articulations());
      qDeleteAll(lyrics());
      }

//---------------------------------------------------------
//   isBefore
//---------------------------------------------------------

bool ChordRest::isBefore(ChordRest* o)
      {
      if (!o)
            return true;
      if (this == o)
            return true;
      int otick = o->tick();
      int t = tick();
      if (t == otick) { // At least one of the chord is a grace, order the grace notes
            bool oGraceAfter = o->isGraceAfter();
            bool graceAfter = isGraceAfter();
            bool oGrace = o->isGrace();
            bool grace = isGrace();
            // normal note are initialized at graceIndex 0 and graceIndex is 0 based
            int oGraceIndex = oGrace ? toChord(o)->graceIndex() +  1 : 0;
            int graceIndex = grace ? toChord(this)->graceIndex() + 1 : 0;
            if (oGrace)
                  oGraceIndex = toChord(o->parent())->graceNotes().size() - oGraceIndex;
            if (grace)
                  graceIndex = toChord(parent())->graceNotes().size() - graceIndex;
            otick = otick + (oGraceAfter ? 1 : -1) *  oGraceIndex;
            t = t + (graceAfter ? 1 : -1) *  graceIndex;
            }
      return t < otick;
      }

}

