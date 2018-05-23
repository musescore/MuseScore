//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "arpeggio.h"
#include "articulation.h"
#include "chord.h"
#include "chordline.h"
#include "glissando.h"
#include "hook.h"
#include "image.h"
#include "measure.h"
#include "rest.h"
#include "stem.h"
#include "stemslash.h"
#include "timesig.h"
#include "tuplet.h"
#include "xml.h"
#include "score.h"
#include "staff.h"
#include "revisions.h"
#include "part.h"
#include "page.h"
#include "style.h"
#include "sym.h"
#include "audio.h"
#include "sig.h"
#include "barline.h"
#include "excerpt.h"
#include "spanner.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif

namespace Ms {

//---------------------------------------------------------
//   ChordRest::readProperties300old
//---------------------------------------------------------

bool ChordRest::readProperties300old(XmlReader& e)
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
                              // created upon reading the <Rest> tag (see Measure::read300old() )
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
            atr->read300old(e);
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
                        qDebug("ChordRest::read300old(): unknown Slur type <%s>", qPrintable(atype));
                  }
            e.readNext();
            }
      else if (tag == "Lyrics" /*|| tag == "FiguredBass"*/) {
            Element* element = Element::name2Element(tag, score());
            element->setTrack(e.track());
            element->read300old(e);
            add(element);
            }
      else if (tag == "pos") {
            QPointF pt = e.readPoint();
            setUserOff(pt * spatium());
            }
      else if (tag == "offset")
            DurationElement::readProperties300old(e);
      else if (!DurationElement::readProperties300old(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   Rest::read300old
//---------------------------------------------------------

void Rest::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->setTrack(track());
                  s->read300old(e);
                  add(s);
                  }
            else if (tag == "Image") {
                  if (MScore::noImages)
                        e.skipCurrentElement();
                  else {
                        Image* image = new Image(score());
                        image->setTrack(track());
                        image->read300old(e);
                        add(image);
                        }
                  }
            else if (ChordRest::readProperties300old(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Chord::read300old
//---------------------------------------------------------

void Chord::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (readProperties300old(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Chord::readProperties300old
//---------------------------------------------------------

bool Chord::readProperties300old(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "Note") {
            Note* note = new Note(score());
            // the note needs to know the properties of the track it belongs to
            note->setTrack(track());
            note->setChord(this);
            note->read300old(e);
            add(note);
            }
      else if (ChordRest::readProperties300old(e))
            ;
      else if (tag == "Stem") {
            Stem* s = new Stem(score());
            s->read300old(e);
            add(s);
            }
      else if (tag == "Hook") {
            _hook = new Hook(score());
            _hook->read300old(e);
            add(_hook);
            }
      else if (tag == "appoggiatura") {
            _noteType = NoteType::APPOGGIATURA;
            e.readNext();
            }
      else if (tag == "acciaccatura") {
            _noteType = NoteType::ACCIACCATURA;
            e.readNext();
            }
      else if (tag == "grace4") {
            _noteType = NoteType::GRACE4;
            e.readNext();
            }
      else if (tag == "grace16") {
            _noteType = NoteType::GRACE16;
            e.readNext();
            }
      else if (tag == "grace32") {
            _noteType = NoteType::GRACE32;
            e.readNext();
            }
      else if (tag == "grace8after") {
            _noteType = NoteType::GRACE8_AFTER;
            e.readNext();
            }
      else if (tag == "grace16after") {
            _noteType = NoteType::GRACE16_AFTER;
            e.readNext();
            }
      else if (tag == "grace32after") {
            _noteType = NoteType::GRACE32_AFTER;
            e.readNext();
            }
      else if (tag == "StemSlash") {
            StemSlash* ss = new StemSlash(score());
            ss->read300old(e);
            add(ss);
            }
      else if (readProperty(tag, e, Pid::STEM_DIRECTION))
            ;
      else if (tag == "noStem")
            _noStem = e.readInt();
      else if (tag == "Arpeggio") {
            _arpeggio = new Arpeggio(score());
            _arpeggio->setTrack(track());
            _arpeggio->read300old(e);
            _arpeggio->setParent(this);
            }
      // old glissando format, chord-to-chord, attached to its final chord
      else if (tag == "Glissando") {
            // the measure we are reading is not inserted in the score yet
            // as well as, possibly, the glissando intended initial chord;
            // then we cannot fully link the glissando right now;
            // temporarily attach the glissando to its final note as a back spanner;
            // after the whole score is read, Score::connectTies() will look for
            // the suitable initial note
            Note* finalNote = upNote();
            Glissando* gliss = new Glissando(score());
            gliss->read300old(e);
            gliss->setAnchor(Spanner::Anchor::NOTE);
            gliss->setStartElement(nullptr);
            gliss->setEndElement(nullptr);
            // in TAB, use straight line with no text
            if (score()->staff(e.track() >> 2)->isTabStaff(tick())) {
                  gliss->setGlissandoType(GlissandoType::STRAIGHT);
                  gliss->setShowText(false);
                  }
            finalNote->addSpannerBack(gliss);
            }
      else if (tag == "Tremolo") {
            _tremolo = new Tremolo(score());
            _tremolo->setTrack(track());
            _tremolo->read300old(e);
            _tremolo->setParent(this);
            }
      else if (tag == "tickOffset")       // obsolete
            ;
      else if (tag == "ChordLine") {
            ChordLine* cl = new ChordLine(score());
            cl->read300old(e);
            add(cl);
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   Score::readStaff300old
//---------------------------------------------------------

void Score::readStaff300old(XmlReader& e)
      {
      int staff = e.intAttribute("id", 1) - 1;
      e.initTick(0);
      e.setTrack(staff * VOICES);

      if (staff == 0) {
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());

                  if (tag == "Measure") {
                        Measure* measure = 0;
                        measure = new Measure(this);
                        measure->setTick(e.tick());
                        //
                        // inherit timesig from previous measure
                        //
                        Measure* m = e.lastMeasure(); // measure->prevMeasure();
                        Fraction f(m ? m->timesig() : Fraction(4,4));
                        measure->setLen(f);
                        measure->setTimesig(f);

                        measure->read300old(e, staff);
                        measure->checkMeasure(staff);
                        if (!measure->isMMRest()) {
                              measures()->add(measure);
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
                  else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                        MeasureBase* mb = toMeasureBase(Element::name2Element(tag, this));
                        mb->read300old(e);
                        mb->setTick(e.tick());
                        measures()->add(mb);
                        }
                  else if (tag == "tick")
                        e.initTick(fileDivision(e.readInt()));
                  else
                        e.unknown();
                  }
            }
      else {
            Measure* measure = firstMeasure();
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());

                  if (tag == "Measure") {
                        if (measure == 0) {
                              qDebug("Score::readStaff300old(): missing measure!");
                              measure = new Measure(this);
                              measure->setTick(e.tick());
                              measures()->add(measure);
                              }
                        e.initTick(measure->tick());
                        measure->read300old(e, staff);
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
                  else if (tag == "tick")
                        e.initTick(fileDivision(e.readInt()));
                  else
                        e.unknown();
                  }
            }
      }

//---------------------------------------------------------
//   read300old
//    return false on error
//---------------------------------------------------------

bool Score::read300old(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            e.setTrack(-1);
            const QStringRef& tag(e.name());
            if (tag == "Staff")
                  readStaff300old(e);
            else if (tag == "Omr") {
#ifdef OMR
                  masterScore()->setOmr(new Omr(this));
                  masterScore()->omr()->read(e);
#else
                  e.skipCurrentElement();
#endif
                  }
            else if (tag == "Audio") {
                  _audio = new Audio;
                  _audio->read(e);
                  }
            else if (tag == "showOmr")
                  masterScore()->setShowOmr(e.readInt());
            else if (tag == "playMode")
                  _playMode = PlayMode(e.readInt());
            else if (tag == "LayerTag") {
                  int id = e.intAttribute("id");
                  const QString& tag = e.attribute("tag");
                  QString val(e.readElementText());
                  if (id >= 0 && id < 32) {
                        _layerTags[id] = tag;
                        _layerTagComments[id] = val;
                        }
                  }
            else if (tag == "Layer") {
                  Layer layer;
                  layer.name = e.attribute("name");
                  layer.tags = e.attribute("mask").toUInt();
                  _layer.append(layer);
                  e.readNext();
                  }
            else if (tag == "currentLayer")
                  _currentLayer = e.readInt();
            else if (tag == "Synthesizer")
                  _synthesizerState.read(e);
            else if (tag == "Division")
                  _fileDivision = e.readInt();
            else if (tag == "showInvisible")
                  _showInvisible = e.readInt();
            else if (tag == "showUnprintable")
                  _showUnprintable = e.readInt();
            else if (tag == "showFrames")
                  _showFrames = e.readInt();
            else if (tag == "showMargins")
                  _showPageborders = e.readInt();
            else if (tag == "Style") {
                  qreal sp = style().value(Sid::spatium).toDouble();
                  style().load(e);
                  // if (_layoutMode == LayoutMode::FLOAT || _layoutMode == LayoutMode::SYSTEM) {
                  if (_layoutMode == LayoutMode::FLOAT) {
                        // style should not change spatium in
                        // float mode
                        style().set(Sid::spatium, sp);
                        }
                  _scoreFont = ScoreFont::fontFactory(style().value(Sid::MusicalSymbolFont).toString());
                  }
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(this);
                  text->read300old(e);
                  setMetaTag("copyright", text->xmlText());
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
                  part->read(e);
                  _parts.push_back(part);
                  }
            else if ((tag == "HairPin")
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Slur")
                || (tag == "Pedal")) {
                  Spanner* s = toSpanner(Element::name2Element(tag, this));
                  s->read300old(e);
                  addSpanner(s);
                  }
            else if (tag == "Excerpt") {
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        if (isMaster()) {
                              Excerpt* ex = new Excerpt(static_cast<MasterScore*>(this));
                              ex->read(e);
                              excerpts().append(ex);
                              }
                        else {
                              qDebug("Score::read(): part cannot have parts");
                              e.skipCurrentElement();
                              }
                        }
                  }
            else if (e.name() == "Tracklist") {
                  int strack = e.intAttribute("sTrack",   -1);
                  int dtrack = e.intAttribute("dstTrack", -1);
                  if (strack != -1 && dtrack != -1)
                        e.tracks().insert(strack, dtrack);
                  e.skipCurrentElement();
                  }
            else if (tag == "Score") {          // recursion
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        e.tracks().clear();     // ???
                        MasterScore* m = masterScore();
                        Score* s       = new Score(m);
                        Excerpt* ex    = new Excerpt(m);

                        ex->setPartScore(s);
                        ex->setTracks(e.tracks());
                        e.setLastMeasure(nullptr);
                        s->read300old(e);
                        m->addExcerpt(ex);
                        }
                  }
            else if (tag == "name") {
                  QString n = e.readElementText();
                  if (!isMaster()) //ignore the name if it's not a child score
                        excerpt()->setTitle(n);
                  }
            else if (tag == "layoutMode") {
                  QString s = e.readElementText();
                  if (s == "line")
                        _layoutMode = LayoutMode::LINE;
                  else if (s == "system")
                        _layoutMode = LayoutMode::SYSTEM;
                  else
                        qDebug("layoutMode: %s", qPrintable(s));
                  }
            else
                  e.unknown();
            }
      if (e.error() != QXmlStreamReader::NoError) {
            qDebug("%s: xml read error at line %lld col %lld: %s",
               qPrintable(e.getDocName()), e.lineNumber(), e.columnNumber(),
               e.name().toUtf8().data());
            MScore::lastError = QObject::tr("XML read error at line %1 column %2: %3").arg(e.lineNumber()).arg(e.columnNumber()).arg(e.name().toString());
            return false;
            }

      connectTies();

      _fileDivision = MScore::division;

#if 0 // TODO:barline
      //
      //    sanity check for barLineSpan
      //
      for (Staff* st : staves()) {
            int barLineSpan = st->barLineSpan();
            int idx = st->idx();
            int n   = nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  // span until last staff
                  barLineSpan = n - idx;
                  st->setBarLineSpan(barLineSpan);
                  }
            else if (idx == 0 && barLineSpan == 0) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  // span from the first staff until the start of the next span
                  barLineSpan = 1;
                  for (int i = 1; i < n; ++i) {
                        if (staff(i)->barLineSpan() == 0)
                              ++barLineSpan;
                        else
                              break;
                        }
                  st->setBarLineSpan(barLineSpan);
                  }
            // check spanFrom
            int minBarLineFrom = st->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
            if (st->barLineFrom() < minBarLineFrom)
                  st->setBarLineFrom(minBarLineFrom);
            if (st->barLineFrom() > st->lines(0) * 2)
                  st->setBarLineFrom(st->lines(0) * 2);
            // check spanTo
            Staff* stTo = st->barLineSpan() <= 1 ? st : staff(idx + st->barLineSpan() - 1);
            // 1-line staves have special bar line spans
            int maxBarLineTo        = stTo->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_TO : stTo->lines(0) * 2;
            if (st->barLineTo() < MIN_BARLINE_SPAN_FROMTO)
                  st->setBarLineTo(MIN_BARLINE_SPAN_FROMTO);
            if (st->barLineTo() > maxBarLineTo)
                  st->setBarLineTo(maxBarLineTo);
            // on single staff span, check spanFrom and spanTo are distant enough
            if (st->barLineSpan() == 1) {
                  if (st->barLineTo() - st->barLineFrom() < MIN_BARLINE_FROMTO_DIST) {
                        st->setBarLineFrom(0);
                        st->setBarLineTo(0);
                        }
                  }
            }
#endif

      if (!masterScore()->omr())
            masterScore()->setShowOmr(false);

      fixTicks();
      masterScore()->rebuildMidiMapping();
      masterScore()->updateChannel();
      createPlayEvents();
      return true;
      }

//---------------------------------------------------------
//   read300old
//---------------------------------------------------------

bool MasterScore::read300old(XmlReader& e)
      {
      if (!Score::read300old(e))
            return false;
      int id = 1;
      for (LinkedElements* le : e.linkIds())
            le->setLid(this, id++);
      for (Staff* s : staves())
            s->updateOttava();
      setCreated(false);
      return true;
      }

//---------------------------------------------------------
//   read300old1
//---------------------------------------------------------

Score::FileError MasterScore::read300old1(XmlReader& e)
      {
      bool top = true;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "programVersion") {
                  setMscoreVersion(e.readElementText());
                  parseVersion(mscoreVersion());
                  }
            else if (tag == "programRevision")
                  setMscoreRevision(e.readIntHex());
            else if (tag == "Score") {
                  MasterScore* score;
                  if (top) {
                        score = this;
                        top   = false;
                        }
                  else {
                        score = new MasterScore();
                        addMovement(score);
                        }
                  if (!score->read300old(e))
                        return FileError::FILE_BAD_FORMAT;
                  }
            else if (tag == "Revision") {
                  Revision* revision = new Revision;
                  revision->read(e);
                  revisions()->add(revision);
                  }
            }
      return FileError::FILE_NO_ERROR;
      }

}

