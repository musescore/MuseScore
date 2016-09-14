//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "score.h"
#include "slur.h"
#include "staff.h"
#include "excerpt.h"
#include "chord.h"
#include "rest.h"
#include "keysig.h"
#include "volta.h"
#include "measure.h"
#include "beam.h"
#include "page.h"
#include "segment.h"
#include "ottava.h"
#include "stafftype.h"
#include "text.h"
#include "part.h"
#include "sig.h"
#include "box.h"
#include "dynamic.h"
#include "drumset.h"
#include "style.h"
#include "sym.h"
#include "xml.h"
#include "stringdata.h"
#include "tempo.h"
#include "tempotext.h"
#include "clef.h"
#include "barline.h"
#include "timesig.h"
#include "tuplet.h"
#include "spacer.h"
#include "stafftext.h"
#include "repeat.h"
#include "breath.h"
#include "tremolo.h"

namespace Ms {

static int g_guitarStrings[] = {40,45,50,55,59,64};
static int g_bassStrings[]   = {28,33,38,43};
static int g_violinStrings[] = {55,62,69,76};
static int g_violaStrings[]  = {48,55,62,69};
static int g_celloStrings[]  = {36,43,50,57};

//---------------------------------------------------------
//   StyleVal114
//---------------------------------------------------------

struct StyleVal2 {
      StyleIdx idx;
      QVariant val;
      };

static const StyleVal2 style114[] = {
      { StyleIdx::lyricsMinBottomDistance,      Spatium(2) },
      { StyleIdx::frameSystemDistance,          Spatium(1.0) },
      { StyleIdx::minMeasureWidth,              Spatium(4.0) },
      { StyleIdx::endBarDistance,               Spatium(0.30) },

      { StyleIdx::repeatBarTips,                QVariant(false) },
      { StyleIdx::startBarlineSingle,           QVariant(false) },
      { StyleIdx::startBarlineMultiple,         QVariant(true) },
      { StyleIdx::bracketWidth,                 QVariant(0.35) },
      { StyleIdx::bracketDistance,              QVariant(0.25) },
      { StyleIdx::clefLeftMargin,               QVariant(0.5) },
      { StyleIdx::keysigLeftMargin,             QVariant(0.5) },
      { StyleIdx::timesigLeftMargin,            QVariant(0.5) },
      { StyleIdx::clefKeyRightMargin,           QVariant(1.75) },
      { StyleIdx::clefBarlineDistance,          QVariant(0.18) },
      { StyleIdx::stemWidth,                    QVariant(0.13) },
      { StyleIdx::shortenStem,                  QVariant(true) },
      { StyleIdx::shortStemProgression,         QVariant(0.25) },
      { StyleIdx::shortestStem,                 QVariant(2.25) },
      { StyleIdx::beginRepeatLeftMargin,        QVariant(1.0) },
      { StyleIdx::minNoteDistance,              QVariant(0.4) },
      { StyleIdx::barNoteDistance,              QVariant(1.2) },
      { StyleIdx::noteBarDistance,              QVariant(1.0) },
      { StyleIdx::measureSpacing,               QVariant(1.2) },
      { StyleIdx::staffLineWidth,               QVariant(0.08) },
      { StyleIdx::ledgerLineWidth,              QVariant(0.12) },
      { StyleIdx::akkoladeWidth,                QVariant(1.6) },
      { StyleIdx::accidentalDistance,           QVariant(0.22) },
      { StyleIdx::accidentalNoteDistance,       QVariant(0.22) },
      { StyleIdx::beamWidth,                    QVariant(0.48) },
      { StyleIdx::beamDistance,                 QVariant(0.5) },
      { StyleIdx::beamMinLen,                   QVariant(1.25) },
      { StyleIdx::dotNoteDistance,              QVariant(0.35) },
      { StyleIdx::dotRestDistance,              QVariant(0.25) },
      { StyleIdx::dotDotDistance,               QVariant(0.5) },
      { StyleIdx::propertyDistanceHead,         QVariant(1.0) },
      { StyleIdx::propertyDistanceStem,         QVariant(0.5) },
      { StyleIdx::propertyDistance,             QVariant(1.0) },
      { StyleIdx::articulationMag,              QVariant(qreal(1.0)) },
      { StyleIdx::lastSystemFillLimit,          QVariant(0.3) },
      { StyleIdx::hairpinHeight,                QVariant(1.2) },
      { StyleIdx::hairpinContHeight,            QVariant(0.5) },
      { StyleIdx::hairpinLineWidth,             QVariant(0.13) },
      { StyleIdx::showPageNumber,               QVariant(true) },
      { StyleIdx::showPageNumberOne,            QVariant(false) },
      { StyleIdx::pageNumberOddEven,            QVariant(true) },
      { StyleIdx::showMeasureNumber,            QVariant(true) },
      { StyleIdx::showMeasureNumberOne,         QVariant(false) },
      { StyleIdx::measureNumberInterval,        QVariant(5) },
      { StyleIdx::measureNumberSystem,          QVariant(true) },
      { StyleIdx::measureNumberAllStaffs,       QVariant(false) },
      { StyleIdx::smallNoteMag,                 QVariant(qreal(0.7)) },
      { StyleIdx::graceNoteMag,                 QVariant(qreal(0.7)) },
      { StyleIdx::smallStaffMag,                QVariant(qreal(0.7)) },
      { StyleIdx::smallClefMag,                 QVariant(qreal(0.8)) },
      { StyleIdx::genClef,                      QVariant(true) },
      { StyleIdx::genKeysig,                    QVariant(true) },
      { StyleIdx::genCourtesyTimesig,           QVariant(true) },
      { StyleIdx::genCourtesyKeysig,            QVariant(true) },
      { StyleIdx::useStandardNoteNames,         QVariant(true) },
      { StyleIdx::useGermanNoteNames,           QVariant(false) },
      { StyleIdx::useFullGermanNoteNames,       QVariant(false) },
      { StyleIdx::useSolfeggioNoteNames,        QVariant(false) },
      { StyleIdx::useFrenchNoteNames,           QVariant(false) },
      { StyleIdx::chordDescriptionFile,         QVariant(QString("stdchords.xml")) },
      { StyleIdx::chordStyle,                   QVariant(QString("custom")) },
      { StyleIdx::chordsXmlFile,                QVariant(true) },
      { StyleIdx::harmonyY,                     QVariant(0.0) },
      { StyleIdx::concertPitch,                 QVariant(false) },
      { StyleIdx::createMultiMeasureRests,      QVariant(false) },
      { StyleIdx::minEmptyMeasures,             QVariant(2) },
      { StyleIdx::minMMRestWidth,               QVariant(4.0) },
      { StyleIdx::hideEmptyStaves,              QVariant(false) },
      { StyleIdx::gateTime,                     QVariant(100) },
      { StyleIdx::tenutoGateTime,               QVariant(100) },
      { StyleIdx::staccatoGateTime,             QVariant(50) },
      { StyleIdx::slurGateTime,                 QVariant(100) },
      { StyleIdx::ArpeggioNoteDistance,         QVariant(.5) },
      { StyleIdx::ArpeggioLineWidth,            QVariant(.18) },
      { StyleIdx::ArpeggioHookLen,              QVariant(.8) },
      { StyleIdx::keySigNaturals,               QVariant(int(KeySigNatural::BEFORE)) },
      { StyleIdx::tupletMaxSlope,               QVariant(qreal(0.5)) },
      { StyleIdx::tupletOufOfStaff,             QVariant(false) },
      { StyleIdx::tupletVHeadDistance,          QVariant(.5) },
      { StyleIdx::tupletVStemDistance,          QVariant(.25) },
      { StyleIdx::tupletStemLeftDistance,       QVariant(.5) },
      { StyleIdx::tupletStemRightDistance,      QVariant(.5) },
      { StyleIdx::tupletNoteLeftDistance,       QVariant(0.0) },
      { StyleIdx::tupletNoteRightDistance,      QVariant(0.0) },
      { StyleIdx::hideInstrumentNameIfOneInstrument, QVariant(false) },
      };

//---------------------------------------------------------
//   readMeasure
//---------------------------------------------------------

static void readMeasure(Measure* m, int staffIdx, XmlReader& e)
      {
      Segment* segment = 0;
      qreal _spatium = m->spatium();

      QList<Chord*> graceNotes;

      //sort tuplet elements. needed for nested tuplets #22537
      for (Tuplet* t : e.tuplets())
            t->sortElements();
      e.tuplets().clear();
      e.setTrack(staffIdx * VOICES);

      for (int n = m->mstaves().size(); n <= staffIdx; ++n) {
            Staff* staff = m->score()->staff(n);
            MStaff* s    = new MStaff;
            s->lines     = new StaffLines(m->score());
            s->lines->setParent(m);
            s->lines->setTrack(n * VOICES);
            s->lines->setVisible(!staff->invisible());
            m->mstaves().push_back(s);
            }

      // tick is obsolete
      if (e.hasAttribute("tick"))
            e.initTick(m->score()->fileDivision(e.intAttribute("tick")));

      if (e.hasAttribute("len")) {
            QStringList sl = e.attribute("len").split('/');
            if (sl.size() == 2)
                  m->setLen(Fraction(sl[0].toInt(), sl[1].toInt()));
            else
                  qDebug("illegal measure size <%s>", qPrintable(e.attribute("len")));
            m->score()->sigmap()->add(m->tick(), SigEvent(m->len(), m->timesig()));
            m->score()->sigmap()->add(m->endTick(), SigEvent(m->timesig()));
            }

      Staff* staff = m->score()->staff(staffIdx);
      Fraction timeStretch(staff->timeStretch(m->tick()));

      // keep track of tick of previous element
      // this allows markings that need to apply to previous element to do so
      // even though we may have already advanced to next tick position
      int lastTick = e.tick();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "move")
                  e.initTick(e.readFraction().ticks() + m->tick());
            else if (tag == "tick") {
                  e.initTick(m->score()->fileDivision(e.readInt()));
                  lastTick = e.tick();
                  }
            else if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(m->score());
                  barLine->setTrack(e.track());
                  barLine->resetProperty(P_ID::BARLINE_SPAN);
                  barLine->resetProperty(P_ID::BARLINE_SPAN_FROM);
                  barLine->resetProperty(P_ID::BARLINE_SPAN_TO);

                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "subtype") {
                              BarLineType t;
                              switch (e.readInt()) {
                                    default:
                                    case 0:
                                          t = BarLineType::NORMAL;
                                          break;
                                    case 1:
                                          t = BarLineType::DOUBLE;
                                          break;
                                    case 2:
                                          t = BarLineType::START_REPEAT;
                                          break;
                                    case 3:
                                          t = BarLineType::END_REPEAT;
                                          break;
                                    case 4:
                                          t = BarLineType::BROKEN;
                                          break;
                                    case 5:
                                          t = BarLineType::END;
                                          break;
                                    case 6:
                                          t = BarLineType::END_START_REPEAT;
                                          break;
                                    }
                              barLine->setBarLineType(t);
                              }
                        else if (!barLine->Element::readProperties(e))
                              e.unknown();
                        }

                  //
                  //  StartRepeatBarLine: always at the beginning tick of a measure, always BarLineType::START_REPEAT
                  //  BarLine:            in the middle of a measure, has no semantic
                  //  EndBarLine:         at the end tick of a measure
                  //  BeginBarLine:       first segment of a measure

                  Segment::Type st;
                  if ((e.tick() != m->tick()) && (e.tick() != m->endTick()))
                        st = Segment::Type::BarLine;
                  else if (barLine->barLineType() == BarLineType::START_REPEAT && e.tick() == m->tick())
                        st = Segment::Type::StartRepeatBarLine;
                  else if (e.tick() == m->tick() && segment == 0)
                        st = Segment::Type::BeginBarLine;
                  else
                        st = Segment::Type::EndBarLine;
                  segment = m->getSegment(st, e.tick());
                  segment->add(barLine);
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(m->score());
                  chord->setTrack(e.track());
                  chord->read(e);
                  segment = m->getSegment(Segment::Type::ChordRest, e.tick());
                  if (chord->noteType() != NoteType::NORMAL) {
                        graceNotes.push_back(chord);
                        if (chord->tremolo() && chord->tremolo()->tremoloType() < TremoloType::R8) {
                              // old style tremolo found
                              Tremolo* tremolo = chord->tremolo();
                              TremoloType st;
                              switch (tremolo->tremoloType()) {
                                    default:
                                    case TremoloType::OLD_R8:  st = TremoloType::R8;  break;
                                    case TremoloType::OLD_R16: st = TremoloType::R16; break;
                                    case TremoloType::OLD_R32: st = TremoloType::R32; break;
                                    case TremoloType::OLD_C8:  st = TremoloType::C8;  break;
                                    case TremoloType::OLD_C16: st = TremoloType::C16; break;
                                    case TremoloType::OLD_C32: st = TremoloType::C32; break;
                                    }
                              tremolo->setTremoloType(st);
                              }
                        }
                  else {
                        segment->add(chord);
                        Q_ASSERT(segment->segmentType() == Segment::Type::ChordRest);

                        for (int i = 0; i < graceNotes.size(); ++i) {
                              Chord* gc = graceNotes[i];
                              gc->setGraceIndex(i);
                              chord->add(gc);
                              }
                        graceNotes.clear();
                        int crticks = chord->actualTicks();

                        if (chord->tremolo() && chord->tremolo()->tremoloType() < TremoloType::R8) {
                              // old style tremolo found

                              Tremolo* tremolo = chord->tremolo();
                              TremoloType st;
                              switch (tremolo->tremoloType()) {
                                    default:
                                    case TremoloType::OLD_R8:  st = TremoloType::R8;  break;
                                    case TremoloType::OLD_R16: st = TremoloType::R16; break;
                                    case TremoloType::OLD_R32: st = TremoloType::R32; break;
                                    case TremoloType::OLD_C8:  st = TremoloType::C8;  break;
                                    case TremoloType::OLD_C16: st = TremoloType::C16; break;
                                    case TremoloType::OLD_C32: st = TremoloType::C32; break;
                                    }
                              tremolo->setTremoloType(st);
                              if (tremolo->twoNotes()) {
                                    int track = chord->track();
                                    Segment* ss = 0;
                                    for (Segment* ps = m->first(Segment::Type::ChordRest); ps; ps = ps->next(Segment::Type::ChordRest)) {
                                          if (ps->tick() >= e.tick())
                                                break;
                                          if (ps->element(track))
                                                ss = ps;
                                          }
                                    Chord* pch = 0;       // previous chord
                                    if (ss) {
                                          ChordRest* cr = static_cast<ChordRest*>(ss->element(track));
                                          if (cr && cr->type() == Element::Type::CHORD)
                                                pch = static_cast<Chord*>(cr);
                                          }
                                    if (pch) {
                                          tremolo->setParent(pch);
                                          pch->setTremolo(tremolo);
                                          chord->setTremolo(0);
                                          // force duration to half
                                          Fraction pts(timeStretch * pch->globalDuration());
                                          int pcrticks = pts.ticks();
                                          pch->setDuration(Fraction::fromTicks(pcrticks / 2));
                                          chord->setDuration(Fraction::fromTicks(crticks / 2));
                                          }
                                    else {
                                          qDebug("tremolo: first note not found");
                                          }
                                    crticks /= 2;
                                    }
                              else {
                                    tremolo->setParent(chord);
                                    }
                              }
                        lastTick = e.tick();
                        e.incTick(crticks);
                        }
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(m->score());
                  rest->setDurationType(TDuration::DurationType::V_MEASURE);
                  rest->setDuration(m->timesig()/timeStretch);
                  rest->setTrack(e.track());
                  rest->read(e);
                  segment = m->getSegment(rest, e.tick());
                  segment->add(rest);

                  if (!rest->duration().isValid())     // hack
                        rest->setDuration(m->timesig()/timeStretch);

                  lastTick = e.tick();
                  e.incTick(rest->actualTicks());
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(m->score());
                  breath->setTrack(e.track());
                  int tick = e.tick();
                  breath->read(e);
                  // older scores placed the breath segment right after the chord to which it applies
                  // rather than before the next chordrest segment with an element for the staff
                  // result would be layout too far left if there are other segments due to notes in other staves
                  // we need to find tick of chord to which this applies, and add its duration
                  int prevTick;
                  if (e.tick() < tick)
                        prevTick = e.tick();    // use our own tick if we explicitly reset to earlier position
                  else
                        prevTick = lastTick;    // otherwise use tick of previous tick/chord/rest tag
                  // find segment
                  Segment* prev = m->findSegment(Segment::Type::ChordRest, prevTick);
                  if (prev) {
                        // find chordrest
                        ChordRest* lastCR = static_cast<ChordRest*>(prev->element(e.track()));
                        if (lastCR)
                              tick = prevTick + lastCR->actualTicks();
                        }
                  segment = m->getSegment(Segment::Type::Breath, tick);
                  segment->add(breath);
                  }
            else if (tag == "endSpanner") {
                  int id = e.attribute("id").toInt();
                  Spanner* spanner = e.findSpanner(id);
                  if (spanner) {
                        spanner->setTicks(e.tick() - spanner->tick());
                        // if (spanner->track2() == -1)
                              // the absence of a track tag [?] means the
                              // track is the same as the beginning of the slur
                        if (spanner->track2() == -1)
                              spanner->setTrack2(spanner->track() ? spanner->track() : e.track());
                        }
                  else {
                        // remember "endSpanner" values
                        SpannerValues sv;
                        sv.spannerId = id;
                        sv.track2    = e.track();
                        sv.tick2     = e.tick();
                        e.addSpannerValues(sv);
                        }
                  e.readNext();
                  }
            else if (tag == "Slur") {
                  Slur *sl = new Slur(m->score());
                  sl->setTick(e.tick());
                  sl->read(e);
                  //
                  // check if we already saw "endSpanner"
                  //
                  int id = e.spannerId(sl);
                  const SpannerValues* sv = e.spannerValues(id);
                  if (sv) {
                        sl->setTick2(sv->tick2);
                        sl->setTrack2(sv->track2);
                        }
                  m->score()->addSpanner(sl);
                  }
            else if (tag == "HairPin"
               || tag == "Pedal"
               || tag == "Ottava"
               || tag == "Trill"
               || tag == "TextLine"
               || tag == "Volta") {
                  Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, m->score()));
                  sp->setTrack(e.track());
                  sp->setTick(e.tick());
                  // ?? sp->setAnchor(Spanner::Anchor::SEGMENT);
                  sp->read(e);
                  m->score()->addSpanner(sp);
                  //
                  // check if we already saw "endSpanner"
                  //
                  int id = e.spannerId(sp);
                  const SpannerValues* sv = e.spannerValues(id);
                  if (sv) {
                        sp->setTicks(sv->tick2 - sp->tick());
                        sp->setTrack2(sv->track2);
                        }
                  }
            else if (tag == "RepeatMeasure") {
                  RepeatMeasure* rm = new RepeatMeasure(m->score());
                  rm->setTrack(e.track());
                  rm->read(e);
                  segment = m->getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(rm);
                  if (rm->actualDuration().isZero()) { // might happen with 1.3 scores
                        rm->setDuration(m->len());
                        }
                  lastTick = e.tick();
                  e.incTick(m->ticks());
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(m->score());
                  clef->setTrack(e.track());
                  clef->read(e);
                  clef->setGenerated(false);

                  // there may be more than one clef segment for same tick position
                  if (!segment) {
                        // this is the first segment of measure
                        segment = m->getSegment(Segment::Type::Clef, e.tick());
                        }
                  else {
                        bool firstSegment = false;
                        // the first clef may be missing and is added later in layout
                        for (Segment* s = m->segments().first(); s && s->tick() == e.tick(); s = s->next()) {
                              if (s->segmentType() == Segment::Type::Clef
                                    // hack: there may be other segment types which should
                                    // generate a clef at current position
                                 || s->segmentType() == Segment::Type::StartRepeatBarLine
                                 ) {
                                    firstSegment = true;
                                    break;
                                    }
                              }
                        if (firstSegment) {
                              Segment* ns = 0;
                              if (segment->next()) {
                                    ns = segment->next();
                                    while (ns && ns->tick() < e.tick())
                                          ns = ns->next();
                                    }
                              segment = 0;
                              for (Segment* s = ns; s && s->tick() == e.tick(); s = s->next()) {
                                    if (s->segmentType() == Segment::Type::Clef) {
                                          segment = s;
                                          break;
                                          }
                                    }
                              if (!segment) {
                                    segment = new Segment(m, Segment::Type::Clef, e.tick());
                                    m->segments().insert(segment, ns);
                                    }
                              }
                        else {
                              // this is the first clef: move to left
                              segment = m->getSegment(Segment::Type::Clef, e.tick());
                              }
                        }
                  if (e.tick() != m->tick())
                        clef->setSmall(true);         // layout does this ?
                  segment->add(clef);
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(m->score());
                  ts->setTrack(e.track());
                  ts->read(e);
                  // if time sig not at begining of measure => courtesy time sig
                  int currTick = e.tick();
                  bool courtesySig = (currTick > m->tick());
                  if (courtesySig) {
                        // if courtesy sig., just add it without map processing
                        segment = m->getSegment(Segment::Type::TimeSigAnnounce, currTick);
                        segment->add(ts);
                        }
                  else {
                        // if 'real' time sig., do full process
                        segment = m->getSegment(Segment::Type::TimeSig, currTick);
                        segment->add(ts);

                        timeStretch = ts->stretch().reduced();
                        m->setTimesig(ts->sig() / timeStretch);
                        }
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(m->score());
                  ks->setTrack(e.track());
                  ks->read(e);
                  int curTick = e.tick();
                  if (!ks->isCustom() && !ks->isAtonal() && ks->key() == Key::C && curTick == 0) {
                        // ignore empty key signature
                        qDebug("remove keysig c at tick 0");
                        if (ks->links()) {
                              if (ks->links()->size() == 1)
                                    e.linkIds().remove(ks->links()->lid());
                              }
                        }
                  else {
                        // if key sig not at beginning of measure => courtesy key sig
                        bool courtesySig = (curTick == m->endTick());
                        segment = m->getSegment(courtesySig ? Segment::Type::KeySigAnnounce : Segment::Type::KeySig, curTick);
                        segment->add(ks);
                        if (!courtesySig)
                              staff->setKey(curTick, ks->keySigEvent());
                        }
                  }
            else if (tag == "Lyrics") {
                  Element* element = Element::name2Element(tag, m->score());
                  element->setTrack(e.track());
                  element->read(e);
                  segment       = m->getSegment(Segment::Type::ChordRest, e.tick());
                  ChordRest* cr = static_cast<ChordRest*>(segment->element(element->track()));
                  if (!cr)
                        cr = static_cast<ChordRest*>(segment->element(e.track())); // in case lyric itself has bad track info
                  if (!cr)
                        qDebug("Internal error: no chord/rest for lyrics");
                  else
                        cr->add(element);
                  }
            else if (tag == "Text") {
                  Text* t = new StaffText(m->score());
                  t->setTrack(e.track());
                  t->read(e);
                  if (t->empty()) {
                        qDebug("reading empty text: deleted");
                        delete t;
                        }
                  else {
                        segment = m->getSegment(Segment::Type::ChordRest, e.tick());
                        segment->add(t);
                        }
                  }
            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(m->score());
                  dyn->setTrack(e.track());
                  dyn->read(e);
                  dyn->setDynamicType(dyn->xmlText());
                  segment = m->getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(dyn);
                  }
            else if (tag == "Harmony"
               || tag == "FretDiagram"
               || tag == "TremoloBar"
               || tag == "Symbol"
               || tag == "Tempo"
               || tag == "StaffText"
               || tag == "RehearsalMark"
               || tag == "InstrumentChange"
               || tag == "StaffState"
               || tag == "FiguredBass"
               ) {
                  Element* el = Element::name2Element(tag, m->score());
                  // hack - needed because tick tags are unreliable in 1.3 scores
                  // for symbols attached to anything but a measure
                  if (el->type() == Element::Type::SYMBOL)
                        el->setParent(m);    // this will get reset when adding to segment
                  el->setTrack(e.track());
                  el->read(e);
                  segment = m->getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(el);
                  }
            else if (tag == "Marker" || tag == "Jump") {
                  Element* el = Element::name2Element(tag, m->score());
                  el->setTrack(e.track());
                  el->read(e);
                  m->add(el);
                  }
            else if (tag == "Image") {
                  if (MScore::noImages)
                        e.skipCurrentElement();
                  else {
                        Element* el = Element::name2Element(tag, m->score());
                        el->setTrack(e.track());
                        el->read(e);
                        segment = m->getSegment(Segment::Type::ChordRest, e.tick());
                        segment->add(el);
                        }
                  }
            else if (tag == "stretch") {
                  double val = e.readDouble();
                  if (val < 0.0)
                        val = 0;
                  m->setUserStretch(val);
                  }
            else if (tag == "noOffset")
                  m->setNoOffset(e.readInt());
            else if (tag == "measureNumberMode")
                  m->setMeasureNumberMode(MeasureNumberMode(e.readInt()));
            else if (tag == "irregular")
                  m->setIrregular(e.readBool());
            else if (tag == "breakMultiMeasureRest")
                  m->setBreakMultiMeasureRest(e.readBool());
            else if (tag == "sysInitBarLineType") {
                  const QString& val(e.readElementText());
                  BarLine* barLine = new BarLine(m->score());
                  barLine->setTrack(e.track());
                  barLine->setBarLineType(val);
                  segment = m->getSegment(Segment::Type::BeginBarLine, m->tick());
                  segment->add(barLine);
                  }
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(m->score());
                  tuplet->setTrack(e.track());
                  tuplet->setTick(e.tick());
                  tuplet->setParent(m);
                  tuplet->read(e);
                  e.addTuplet(tuplet);
                  }
            else if (tag == "startRepeat") {
                  m->setRepeatStart(true);
                  e.readNext();
                  }
            else if (tag == "endRepeat") {
                  m->setRepeatCount(e.readInt());
                  m->setRepeatEnd(true);
                  }
            else if (tag == "vspacer" || tag == "vspacerDown") {
                  if (m->mstaves()[staffIdx]->_vspacerDown == 0) {
                        Spacer* spacer = new Spacer(m->score());
                        spacer->setSpacerType(SpacerType::DOWN);
                        spacer->setTrack(staffIdx * VOICES);
                        m->add(spacer);
                        }
                  m->mstaves()[staffIdx]->_vspacerDown->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "vspacer" || tag == "vspacerUp") {
                  if (m->mstaves()[staffIdx]->_vspacerUp == 0) {
                        Spacer* spacer = new Spacer(m->score());
                        spacer->setSpacerType(SpacerType::UP);
                        spacer->setTrack(staffIdx * VOICES);
                        m->add(spacer);
                        }
                  m->mstaves()[staffIdx]->_vspacerUp->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "visible")
                  m->mstaves()[staffIdx]->_visible = e.readInt();
            else if (tag == "slashStyle")
                  m->mstaves()[staffIdx]->_slashStyle = e.readInt();
            else if (tag == "Beam") {
                  Beam* beam = new Beam(m->score());
                  beam->setTrack(e.track());
                  beam->read(e);
                  beam->setParent(0);
                  e.addBeam(beam);
                  }
            else if (tag == "Segment")
                  segment->read(e);
            else if (tag == "MeasureNumber") {
                  Text* noText = new Text(m->score());
                  noText->read(e);
                  noText->setFlag(ElementFlag::ON_STAFF, true);
                  // noText->setFlag(ElementFlag::MOVABLE, false); ??
                  noText->setTrack(e.track());
                  noText->setParent(m);
                  m->mstaves()[noText->staffIdx()]->setNoText(noText);
                  }
            else if (tag == "multiMeasureRest") {
                  m->setMMRestCount(e.readInt());
                  // set tick to previous measure
                  m->setTick(e.lastMeasure()->tick());
                  e.initTick(e.lastMeasure()->tick());
                  }
            else if (m->MeasureBase::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      for (Tuplet* tuplet : e.tuplets()) {
            if (tuplet->elements().empty()) {
                  // this should not happen and is a sign of input file corruption
                  qDebug("Measure:read(): empty tuplet id %d (%p), input file corrupted?",
                     tuplet->id(), tuplet);
                  delete tuplet;
                  }
            else
                  tuplet->setParent(m);
            }
      }

//---------------------------------------------------------
//   readStaffContent
//---------------------------------------------------------

static void readStaffContent(Score* score, XmlReader& e)
      {
      int staff = e.intAttribute("id", 1) - 1;
      e.initTick(0);
      e.setTrack(staff * VOICES);

      Measure* measure = score->firstMeasure();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "Measure") {
                  if (staff == 0) {
                        measure = new Measure(score);
                        measure->setTick(e.tick());
                        const SigEvent& ev = score->sigmap()->timesig(measure->tick());
                        measure->setLen(ev.timesig());
                        measure->setTimesig(ev.nominal());

                        readMeasure(measure, staff, e);
                        measure->checkMeasure(staff);

                        if (!measure->isMMRest()) {
                              score->measures()->add(measure);
                              e.setLastMeasure(measure);
                              e.initTick(measure->tick() + measure->ticks());
                              }
                        else {
                              // this is a multi measure rest
                              // always preceded by the first measure it replaces
                              Measure* m = e.lastMeasure();

                              if (m) {
                                    m->setMMRest(measure);
                                    measure->setTick(m->tick());
                                    }
                              }
                         }
                  else {
                        if (measure == 0) {
                              qDebug("Score::readStaff(): missing measure!");
                              measure = new Measure(score);
                              measure->setTick(e.tick());
                              score->measures()->add(measure);
                              }
                        e.initTick(measure->tick());

                        readMeasure(measure, staff, e);
                        measure->checkMeasure(staff);

                        if (measure->isMMRest())
                              measure = e.lastMeasure()->nextMeasure();
                        else {
                              e.setLastMeasure(measure);
                              if (measure->mmRest())
                                    measure = measure->mmRest();
                              else
                                    measure = measure->nextMeasure();
                              }
                        }
                  }
            else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                  MeasureBase* mb = static_cast<MeasureBase*>(Element::name2Element(tag, score));
                  mb->read(e);
                  mb->setTick(e.tick());
                  score->measures()->add(mb);
                  }
            else if (tag == "tick")
                  e.initTick(score->fileDivision(e.readInt()));
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

static void readStaff(Staff* staff, XmlReader& e)
      {
      Score* _score = staff->score();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "lines") {
                  int lines = e.readInt();
                  staff->setLines(lines);
                  if (lines != 5) {
                        staff->setBarLineFrom(lines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0);
                        staff->setBarLineTo(lines == 1 ? BARLINE_SPAN_1LINESTAFF_TO   : (lines - 1) * 2);
                        }
                  }
            else if (tag == "small")
                  staff->setSmall(e.readInt());
            else if (tag == "invisible")
                  staff->setInvisible(e.readInt());
            else if (tag == "slashStyle")
                  e.skipCurrentElement();
            else if (tag == "cleflist") {
                  // QList<std::pair<int, ClefType>>& cl = e.clefs(idx());
                  staff->clefList().clear();
                  while (e.readNextStartElement()) {
                        if (e.name() == "clef") {
                              int tick    = e.intAttribute("tick", 0);
                              ClefType ct = Clef::clefType(e.attribute("idx", "0"));
                              staff->clefList().insert(std::pair<int,ClefType>(_score->fileDivision(tick), ct));
                              e.readNext();
                              }
                        else
                              e.unknown();
                        }
                  if (staff->clefList().empty())
                        staff->clefList().insert(std::pair<int,ClefType>(0, ClefType::G));
                  }
            else if (tag == "keylist")
                  staff->keyList()->read(e, _score);
            else if (tag == "bracket") {
                  BracketItem b;
                  b._bracket = BracketType(e.intAttribute("type", -1));
                  b._bracketSpan = e.intAttribute("span", 0);
                  staff->brackets().push_back(b);
                  e.readNext();
                  }
            else if (tag == "barLineSpan")
                  staff->setBarLineSpan(e.readInt());
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readPart
//---------------------------------------------------------

static void readPart(Part* part, XmlReader& e)
      {
      Score* _score = part->score();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Staff") {
                  Staff* staff = new Staff(_score);
                  staff->setPart(part);
                  _score->staves().push_back(staff);
                  part->staves()->push_back(staff);
                  readStaff(staff, e);
                  }
            else if (tag == "Instrument") {
                  Instrument* i = part->instrument();
                  i->read(e, part);
                  // add string data from MIDI program number, if possible
                  if (i->stringData()->strings() == 0
                     && i->channel().count() > 0
                     && i->drumset() == nullptr) {
                        int program = i->channel(0)->program;
                        if (program >= 24 && program <= 30)       // guitars
                              i->setStringData(StringData(19, 6, g_guitarStrings));
                        else if ( (program >= 32 && program <= 39) || program == 43)      // bass / double-bass
                              i->setStringData(StringData(24, 4, g_bassStrings));
                        else if (program == 40)                   // violin and other treble string instr.
                              i->setStringData(StringData(24, 4, g_violinStrings));
                        else if (program == 41)                   // viola and other alto string instr.
                              i->setStringData(StringData(24, 4, g_violaStrings));
                        else if (program == 42)                   // cello and other bass string instr.
                              i->setStringData(StringData(24, 4, g_celloStrings));
                        }
                  Drumset* d = i->drumset();
                  Staff*   st = part->staff(0);
                  if (d && st && st->lines() != 5) {
                        int n = 0;
                        if (st->lines() == 1)
                              n = 4;
                        for (int  i = 0; i < DRUM_INSTRUMENTS; ++i)
                              d->drum(i).line -= n;
                        }
                  }
            else if (tag == "name") {
                  Text* t = new Text(_score);
                  t->read(e);
                  part->instrument()->setLongName(t->xmlText());
                  delete t;
                  }
            else if (tag == "shortName") {
                  Text* t = new Text(_score);
                  t->read(e);
                  part->instrument()->setShortName(t->xmlText());
                  delete t;
                  }
            else if (tag == "trackName")
                  part->setPartName(e.readElementText());
            else if (tag == "show")
                  part->setShow(e.readInt());
            else
                  e.unknown();
            }
      if (part->partName().isEmpty())
            part->setPartName(part->instrument()->trackName());

      if (part->instrument()->useDrumset()) {
            for (Staff* staff : *part->staves()) {
                  int lines = staff->lines();
                  int bf    = staff->barLineFrom();
                  int bt    = staff->barLineTo();
                  staff->setStaffType(StaffType::getDefaultPreset(StaffGroup::PERCUSSION));

                  // this allows 2/3-line percussion staves to keep the double spacing they had in 1.3

                  if (lines == 2 || lines == 3)
                        staff->staffType()->setLineDistance(Spatium(2.0));

                  staff->setLines(lines);       // this also sets stepOffset
                  staff->setBarLineFrom(bf);
                  staff->setBarLineTo(bt);
                  }
            }
      //set default articulations
      QList<MidiArticulation> articulations;
      articulations.append(MidiArticulation("", "", 100, 100));
      articulations.append(MidiArticulation("staccato", "", 100, 50));
      articulations.append(MidiArticulation("tenuto", "", 100, 100));
      articulations.append(MidiArticulation("sforzato", "", 120, 100));
      part->instrument()->setArticulation(articulations);
      }

//---------------------------------------------------------
//   convertOldTextStyleNames
//---------------------------------------------------------

static QString convertOldTextStyleNames(const QString& s)
      {
      QString rs(s);
      // convert 1.2 text styles
      if (s == "Chordname")
            rs = "Chord Symbol";
      else if (s == "Lyrics odd lines")
            rs = "Lyrics Odd Lines";
      else if (s == "Lyrics even lines")
            rs = "Lyrics Even Lines";
      else if (s == "InstrumentsLong")
            rs = "Instrument Name (Long)";
      else if (s == "InstrumentsShort")
            rs = "Instrument Name (Short)";
      else if (s == "InstrumentsExcerpt")
            rs = "Instrument Name (Part)";
      else if (s == "Poet")
            rs = "Lyricist";
      else if (s == "Technik")
            rs = "Technique";
      else if (s == "TextLine")
            rs = "Text Line";
      else if (s == "Tuplets")
            rs = "Tuplet";
      return rs;
      }

//---------------------------------------------------------
//   readPageFormat
//---------------------------------------------------------

static void readPageFormat(PageFormat* pf, XmlReader& e)
      {
      qreal _oddRightMargin  = 0.0;
      qreal _evenRightMargin = 0.0;
      bool landscape         = false;
      QString type;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "landscape")        // obsolete
                  landscape = e.readInt();
            else if (tag == "page-margins") {
                  type = e.attribute("type","both");
                  qreal lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        qreal val = e.readDouble() * 0.5 / PPI;
                        if (tag == "left-margin")
                              lm = val;
                        else if (tag == "right-margin")
                              rm = val;
                        else if (tag == "top-margin")
                              tm = val;
                        else if (tag == "bottom-margin")
                              bm = val;
                        else
                              e.unknown();
                        }
                  pf->setTwosided(type == "odd" || type == "even");
                  if (type == "odd" || type == "both") {
                        pf->setOddLeftMargin(lm);
                        _oddRightMargin = rm;
                        pf->setOddTopMargin(tm);
                        pf->setOddBottomMargin(bm);
                        }
                  if (type == "even" || type == "both") {
                        pf->setEvenLeftMargin(lm);
                        _evenRightMargin = rm;
                        pf->setEvenTopMargin(tm);
                        pf->setEvenBottomMargin(bm);
                        }
                  }
            else if (tag == "page-height")
                  pf->size().rheight() = e.readDouble() * 0.5 / PPI;
            else if (tag == "page-width")
                  pf->size().rwidth() = e.readDouble() * .5 / PPI;
            else if (tag == "pageFormat") {
                  e.readElementText();
                  }
            else if (tag == "page-offset") {
                  e.readInt();
                  }
            else
                  e.unknown();
            }
      if (landscape)
            pf->size().transpose();
      qreal w1 = pf->size().width() - pf->oddLeftMargin() - _oddRightMargin;
      qreal w2 = pf->size().width() - pf->evenLeftMargin() - _evenRightMargin;
      pf->setPrintableWidth(qMin(w1, w2));     // silently adjust right margins
      }

//---------------------------------------------------------
//   read114
//    import old version <= 1.3 files
//---------------------------------------------------------

Score::FileError MasterScore::read114(XmlReader& e)
      {
      for (unsigned int i = 0; i < sizeof(style114)/sizeof(*style114); ++i)
            style()->set(style114[i].idx, style114[i].val);

      // old text style defaults
      TextStyle ts = style()->textStyle("Chord Symbol");
      ts.setYoff(-4.0);
      style()->setTextStyle(ts);
      TempoMap tm;

      while (e.readNextStartElement()) {
            e.setTrack(-1);
            const QStringRef& tag(e.name());
            if (tag == "Staff")
                  readStaffContent(this, e);
            else if (tag == "KeySig") {               // not supported
                  KeySig* ks = new KeySig(this);
                  ks->read(e);
                  delete ks;
                  }
            else if (tag == "siglist")
                  _sigmap->read(e, _fileDivision);
            else if (tag == "programVersion") {
                  setMscoreVersion(e.readElementText());
                  parseVersion(mscoreVersion());
                  }
            else if (tag == "programRevision")
                  setMscoreRevision(e.readInt());
            else if (tag == "Mag"
               || tag == "MagIdx"
               || tag == "xoff"
               || tag == "Symbols"
               || tag == "cursorTrack"
               || tag == "yoff")
                  e.skipCurrentElement();       // obsolete
            else if (tag == "tempolist") {
                  // store the tempo list to create invisible tempo text later
                  qreal tempo = e.attribute("fix","2.0").toDouble();
                  tm.setRelTempo(tempo);
                  while (e.readNextStartElement()) {
                        if (e.name() == "tempo") {
                              int tick   = e.attribute("tick").toInt();
                              double tmp = e.readElementText().toDouble();
                              tick       = (tick * MScore::division + _fileDivision/2) / _fileDivision;
                              auto pos   = tm.find(tick);
                              if (pos != tm.end())
                                    tm.erase(pos);
                              tm.setTempo(tick, tmp);
                        }
                        else if (e.name() == "relTempo")
                              e.readElementText();
                        else
                              e.unknown();
                  }
            }
            else if (tag == "playMode")
                  setPlayMode(PlayMode(e.readInt()));
            else if (tag == "SyntiSettings")
                  _synthesizerState.read(e);
            else if (tag == "Spatium")
                  setSpatium (e.readDouble() * DPMM);
            else if (tag == "Division")
                  _fileDivision = e.readInt();
            else if (tag == "showInvisible")
                  setShowInvisible(e.readInt());
            else if (tag == "showFrames")
                  setShowFrames(e.readInt());
            else if (tag == "showMargins")
                  setShowPageborders(e.readInt());
            else if (tag == "Style") {
                  qreal sp = spatium();
                  style()->load(e);
                  // adjust this now so chords render properly on read
                  // other style adjustments can wait until reading is finished
                  if (style(StyleIdx::useGermanNoteNames).toBool())
                        style()->set(StyleIdx::useStandardNoteNames, false);
                  if (_layoutMode == LayoutMode::FLOAT) {
                        // style should not change spatium in
                        // float mode
                        setSpatium(sp);
                        }
                  }
            else if (tag == "TextStyle") {
                  TextStyle s;
                  s.read(e);

                  qreal spMM = spatium() / DPMM;
                  if (s.frameWidthMM() != 0.0)
                        s.setFrameWidth(Spatium(s.frameWidthMM() / spMM));
                  if (s.paddingWidthMM() != 0.0)
                        s.setPaddingWidth(Spatium(s.paddingWidthMM() / spMM));

                  // convert 1.2 text styles
                  s.setName(convertOldTextStyleNames(s.name()));

                  if (s.name() == "Lyrics Odd Lines" || s.name() == "Lyrics Even Lines")
                        s.setAlign((s.align() & ~ Align(AlignmentFlags::VMASK)) | AlignmentFlags::BASELINE);

                  style()->setTextStyle(s);
                  }
            else if (tag == "page-layout") {
                  if (_layoutMode != LayoutMode::FLOAT && _layoutMode != LayoutMode::SYSTEM) {
                        PageFormat pf;
                        pf.copy(*pageFormat());
                        readPageFormat(&pf, e);
                        setPageFormat(pf);
                        }
                  else
                        e.skipCurrentElement();
                  }
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(this);
                  text->read(e);
                  text->layout();
                  setMetaTag("copyright", text->plainText());
                  delete text;
                  }
            else if (tag == "movement-number")
                  setMetaTag("movementNumber", e.readElementText());
            else if (tag == "movement-title")
                  setMetaTag("movementTitle", e.readElementText());
            else if (tag == "work-number")
                  setMetaTag("workNumber", e.readElementText());
            else if (tag == "work-title")
                  setMetaTag("workTitle", e.readElementText());
            else if (tag == "source")
                  setMetaTag("source", e.readElementText());
            else if (tag == "metaTag") {
                  QString name = e.attribute("name");
                  setMetaTag(name, e.readElementText());
                  }
            else if (tag == "Part") {
                  Part* part = new Part(this);
                  readPart(part, e);
                  parts().push_back(part);
                  }
            else if (tag == "Slur") {
                  Slur* slur = new Slur(this);
                  slur->read(e);
                  addSpanner(slur);
                  }
            else if ((tag == "HairPin")
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Pedal")) {
                  Spanner* s = static_cast<Spanner*>(Element::name2Element(tag, this));
                  s->read(e);
                  if (s->track() == -1)
                        s->setTrack(e.track());
                  else
                        e.setTrack(s->track());       // update current track
                  if (s->tick() == -1)
                        s->setTick(e.tick());
                  else
                        e.initTick(s->tick());      // update current tick
                  if (s->track2() == -1)
                        s->setTrack2(s->track());
                  if (s->ticks() == 0) {
                        delete s;
                        qDebug("zero spanner %s ticks: %d", s->name(), s->ticks());
                        }
                  else {
                        addSpanner(s);
                        }
                  }
            else if (tag == "Excerpt") {
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        Excerpt* ex = new Excerpt(this);
                        ex->read(e);
                        _excerpts.append(ex);
                        }
                  }
            else if (tag == "Beam") {
                  Beam* beam = new Beam(this);
                  beam->read(e);
                  beam->setParent(0);
                  // _beams.append(beam);
                  }
            else if (tag == "name")
                  setName(e.readElementText());
            else
                  e.unknown();
            }

      if (e.error() != QXmlStreamReader::NoError)
            return FileError::FILE_BAD_FORMAT;

      for (Staff* s : staves()) {
            int idx   = s->idx();
            int track = idx * VOICES;

            // check barLineSpan
            if (s->barLineSpan() > (nstaves() - idx)) {
                  qDebug("read114: invalid barline span %d (max %d)",
                     s->barLineSpan(), nstaves() - idx);
                  s->setBarLineSpan(nstaves() - idx);
                  }
            for (auto i : s->clefList()) {
                  int tick = i.first;
                  ClefType clefId = i.second._concertClef;
                  Measure* m = tick2measure(tick);
                  if (!m)
                        continue;
                  if ((tick == m->tick()) && m->prevMeasure())
                        m = m->prevMeasure();
                  Segment* seg = m->getSegment(Segment::Type::Clef, tick);
                  if (seg->element(track))
                        toClef(seg->element(track))->setGenerated(false);
                  else {
                        Clef* clef = new Clef(this);
                        clef->setClefType(clefId);
                        clef->setTrack(track);
                        clef->setParent(seg);
                        clef->setGenerated(false);
                        seg->add(clef);
                        }
                  }

            // create missing KeySig
            KeyList* km = s->keyList();
            for (auto i = km->begin(); i != km->end(); ++i) {
                  int tick = i->first;
                  if (tick < 0) {
                        qDebug("read114: Key tick %d", tick);
                        continue;
                        }
                  if (tick == 0 && i->second.key() == Key::C)
                        continue;
                  Measure* m = tick2measure(tick);
                  if (!m)           //empty score
                        break;
                  Segment* seg = m->getSegment(Segment::Type::KeySig, tick);
                  if (seg->element(track))
                        toKeySig(seg->element(track))->setGenerated(false);
                  else {
                        KeySigEvent ke = i->second;
                        KeySig* ks = new KeySig(this);
                        ks->setKeySigEvent(ke);
                        ks->setParent(seg);
                        ks->setTrack(track);
                        ks->setGenerated(false);
                        seg->add(ks);
                        }
                  }
            }

      for (std::pair<int,Spanner*> p : spanner()) {
            Spanner* s = p.second;
            if (!s->isSlur()) {
                  if (s->isVolta()) {
                        Volta* volta = toVolta(s);
                        volta->setAnchor(Spanner::Anchor::MEASURE);
                        }
                  }

            if (s->isOttava() || s->isPedal() || s->isTrill() || s->isTextLine()) {
                  qreal yo = 0;
                  if (s->isOttava()) {
                      // fix ottava position
                      yo = styleS(StyleIdx::ottavaY).val() * spatium();
                      if (s->placeBelow())
                            yo = -yo + s->staff()->height();
                      }
                  else if (s->isPedal()) {
                        yo = styleS(StyleIdx::pedalY).val() * spatium();
                        }
                  else if (s->isTrill()) {
                        yo = styleS(StyleIdx::trillY).val() * spatium();
                        }
                  else if (s->isTextLine()) {
                        yo = -5.0 * spatium();
                  }
                  if (!s->spannerSegments().isEmpty()) {
                        for (SpannerSegment* seg : s->spannerSegments()) {
                              if (!seg->userOff().isNull())
                                    seg->setUserYoffset(seg->userOff().y() - yo);
                              }
                        }
                  else {
                        s->setUserYoffset(-yo);
                        }
                  }
            }

      connectTies();

      //
      // remove "middle beam" flags from first ChordRest in
      // measure
      //
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            int tracks = nstaves() * VOICES;
            bool first = true;
            for (int track = 0; track < tracks; ++track) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (s->segmentType() != Segment::Type::ChordRest)
                              continue;
                        ChordRest* cr = toChordRest(s->element(track));
                        if (cr) {
                              if (cr->isRest()) {
                                    Rest* r = toRest(cr);
                                    if (!r->userOff().isNull()) {
                                          int lineOffset = r->computeLineOffset();
                                          qreal lineDist = r->staff() ? r->staff()->staffType()->lineDistance().val() : 1.0;
                                          r->rUserYoffset() -= (lineOffset * .5 * lineDist * r->spatium());
                                          }
                                    }
                              if (!first) {
                                    switch (cr->beamMode()) {
                                          case Beam::Mode::AUTO:
                                          case Beam::Mode::BEGIN:
                                          case Beam::Mode::END:
                                          case Beam::Mode::NONE:
                                                break;
                                          case Beam::Mode::MID:
                                          case Beam::Mode::BEGIN32:
                                          case Beam::Mode::BEGIN64:
                                                cr->setBeamMode(Beam::Mode::BEGIN);
                                                break;
                                          case Beam::Mode::INVALID:
                                                if (cr->isChord())
                                                      cr->setBeamMode(Beam::Mode::AUTO);
                                                else
                                                      cr->setBeamMode(Beam::Mode::NONE);
                                                break;
                                          }
                                    first = false;
                                    }
                              }
                        }
                  }
            }
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() == Element::Type::VBOX) {
                  Box* b  = static_cast<Box*>(mb);
                  qreal y = styleP(StyleIdx::staffUpperBorder);
                  b->setBottomGap(y);
                  }
            }

      _fileDivision = MScore::division;

      //
      //    sanity check for barLineSpan and update ottavas
      //
      for (Staff* staff : staves()) {
            int barLineSpan = staff->barLineSpan();
            int idx = staffIdx(staff);
            int n = nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  staff->setBarLineSpan(n - idx);
                  }
            staff->updateOttava();
            }

      // adjust some styles
      Spatium lmbd = styleS(StyleIdx::lyricsMinBottomDistance);
      style()->set(StyleIdx::lyricsMinBottomDistance, Spatium(lmbd.val() + 4.0));
      if (style(StyleIdx::hideEmptyStaves).toBool())        // http://musescore.org/en/node/16228
            style()->set(StyleIdx::dontHideStavesInFirstSystem, false);
      if (style(StyleIdx::showPageNumberOne).toBool()) {    // http://musescore.org/en/node/21207
            style()->set(StyleIdx::evenFooterL, QString("$P"));
            style()->set(StyleIdx::oddFooterR, QString("$P"));
            }
      if (style(StyleIdx::minEmptyMeasures).toInt() == 0)
            style()->set(StyleIdx::minEmptyMeasures, 1);
      // hack: net overall effect of layout changes has been for things to take slightly more room
      qreal adjustedSpacing = qMax(styleD(StyleIdx::measureSpacing) * 0.95, 1.0);
      style()->set(StyleIdx::measureSpacing, adjustedSpacing);

      _showOmr = false;

      // add invisible tempo text if necessary
      // some 1.3 scores have tempolist but no tempo text
      fixTicks();
      for (auto i : tm) {
            int tick    = i.first;
            qreal tempo = i.second.tempo;
            if (tempomap()->tempo(tick) != tempo) {
                  TempoText* tt = new TempoText(this);
                  tt->setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(qRound(tempo*60)));
                  tt->setTempo(tempo);
                  tt->setTrack(0);
                  tt->setVisible(false);
                  Measure* m = tick2measure(tick);
                  if (m) {
                        Segment* seg = m->getSegment(Segment::Type::ChordRest, tick);
                        seg->add(tt);
                        setTempo(tick, tempo);
                        }
                  else
                        delete tt;
                  }
            }

      // create excerpts

      for (Excerpt* excerpt : _excerpts) {
            if (excerpt->parts().isEmpty()) {         // ignore empty parts
                  _excerpts.removeOne(excerpt);
                  continue;
                  }
            if (!excerpt->parts().isEmpty()) {
                  Score* nscore = new Score(this);
                  excerpt->setPartScore(nscore);
                  nscore->setName(excerpt->title());
                  nscore->style()->set(StyleIdx::createMultiMeasureRests, true);
                  Ms::createExcerpt(excerpt);
                  }
            }

      // volta offsets in older scores are hardcoded to be relative to a voltaY of -2.0sp
      // we'll force this and live with it for the score
      // but we wait until now to do it so parts don't have this issue
      if (style(StyleIdx::voltaY) == MScore::baseStyle()->value(StyleIdx::voltaY))
            style()->set(StyleIdx::voltaY, -2.0f);

      fixTicks();
      rebuildMidiMapping();
      updateChannel();

      // treat reading a 1.14 file as import
      // on save warn if old file will be overwritten
      setCreated(true);

      return FileError::FILE_NO_ERROR;
      }

}

