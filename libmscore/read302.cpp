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

#include "audio.h"
//#include "barline.h"
#include "excerpt.h"
#include "measurebase.h"
#include "page.h"
#include "part.h"
#include "revisions.h"
#include "score.h"
#include "scoreOrder.h"
#include "sig.h"
#include "spanner.h"
#include "staff.h"
#include "stafftext.h"
#include "style.h"
#include "sym.h"
#include "xml.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif

namespace Ms {

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool Score::read(XmlReader& e)
      {
      // HACK
      // style setting compatibility settings for minor versions
      // this allows new style settings to be added
      // with different default values for older vs newer scores
      // note: older templates get the default values for older scores
      // these can be forced back in MuseScore::getNewFile() if necessary
      QString programVersion = masterScore()->mscoreVersion();
      bool disableHarmonyPlay = MScore::harmonyPlayDisableCompatibility && !MScore::testMode;
      if (!programVersion.isEmpty() && programVersion < "3.5" && disableHarmonyPlay) {
            style().set(Sid::harmonyPlay, false);
            }

      ScoreOrder* order { nullptr };
      while (e.readNextStartElement()) {
            e.setTrack(-1);
            const QStringRef& tag(e.name());
            if (tag == "Staff")
                  readStaff(e);
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
                  const QString& t = e.attribute("tag");
                  QString val(e.readElementText());
                  if (id >= 0 && id < 32) {
                        _layerTags[id] = t;
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
            else if (tag == "page-offset")
                  _pageNumberOffset = e.readInt();
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
            else if (tag == "open") // + Mu4 compat
                  e.skipCurrentElement(); // irgnore, even in Mu4 it doesn't seem to have any usefull meaning
            else if (tag == "markIrregularMeasures")
                  _markIrregularMeasures = e.readInt();
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
                  text->read(e);
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
            else if (tag == "Order") {
                  order = new ScoreOrder(e.attribute("id"));
                  order->read(e);
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
                  s->read(e);
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
                        Score* s       = new Score(m, MScore::baseStyle());
                        int defaultsVersion = m->style().defaultStyleVersion();
                        s->setStyle(*MStyle::resolveStyleDefaults(defaultsVersion));
                        s->style().setDefaultStyleVersion(defaultsVersion);
                        Excerpt* ex    = new Excerpt(m);

                        ex->setPartScore(s);
                        e.setLastMeasure(nullptr);
                        s->read(e);
                        s->linkMeasures(m);
                        ex->setTracks(e.tracks());
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
            else if (tag == "Expression") { // Mu4 compatibility
                  QString s = e.readElementText();
                  TextBase* t = new StaffText(score(), Tid::EXPRESSION);
                  t->setXmlText(s);
                  }
            else
                  e.unknown();
            }
      e.reconnectBrokenConnectors();
      if (e.error() != QXmlStreamReader::NoError) {
            qDebug("%s: xml read error at line %lld col %lld: %s",
               qPrintable(e.getDocName()), e.lineNumber(), e.columnNumber(),
               e.name().toUtf8().data());
            if (e.error() == QXmlStreamReader::CustomError)
                  MScore::lastError = e.errorString();
            else
                  MScore::lastError = QObject::tr("XML read error at line %1, column %2: %3").arg(e.lineNumber()).arg(e.columnNumber()).arg(e.name().toString());
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
      // Make sure every instrument has an instrumentId set.
      for (Part* part : parts()) {
            const InstrumentList* il = part->instruments();
            for (auto it = il->begin(); it != il->end(); it++)
                  static_cast<Instrument*>(it->second)->updateInstrumentId();
            }
      if (order) {
            ScoreOrder* defined = scoreOrders.findByName(order->getName());
            if (defined)
                  {
                  if (defined->isScoreOrder(this)) {
                        // The order in the score file matches a score order
                        // which is already defined so use that order.
                        setScoreOrder(defined);
                        delete order;
                        }
                  else {
                        // The order in the score file is already defined in the score order
                        // but the order is of the instruments is not the same so use the
                        // order as a customized version of the already defined order.
                        scoreOrders.addScoreOrder(order);
                        setScoreOrder(order);
                        }
                  }
            else {
                  defined = scoreOrders.findById(order->getId());
                  if (defined) {
                        // The order in the score file is already available, resuse it.
                        setScoreOrder(defined);
                        delete order;
                        }
                  else {
                        // The order in the score file is new, add it to the score orders.
                        scoreOrders.addScoreOrder(order);
                        setScoreOrder(order);
                        }
                  }
            }

      if (!masterScore()->omr())
            masterScore()->setShowOmr(false);

      fixTicks();

      for (Part* p : qAsConst(_parts)) {
            p->updateHarmonyChannels(false);
            }

      masterScore()->rebuildMidiMapping();
      masterScore()->updateChannel();

//      createPlayEvents();
      return true;
      }

//---------------------------------------------------------
// linkMeasures
//---------------------------------------------------------

void Score::linkMeasures(Score* score)
      {
      MeasureBase *mbMaster = score->first();
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (!mb->isMeasure())
                  continue;
            while (mbMaster && !mbMaster->isMeasure())
                  mbMaster = mbMaster->next();
            if (!mbMaster) {
                  qDebug("Measures in MasterScore and Score are not in sync.");
                  break;
                  }
            mb->linkTo(mbMaster);
            mbMaster = mbMaster->next();
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool MasterScore::read(XmlReader& e)
      {
      if (!Score::read(e))
            return false;
      for (Staff* s : staves())
            s->updateOttava();
      setCreated(false);
      return true;
      }

//---------------------------------------------------------
//   addMovement
//---------------------------------------------------------

void MasterScore::addMovement(MasterScore* score)
      {
      score->_movements = _movements;
      _movements->push_back(score);
      MasterScore* ps = 0;
      for (MasterScore* s : *_movements) {
            s->setPrev(ps);
            if (ps)
                  ps->setNext(s);
            s->setNext(0);
            ps = s;
            }
      }

//---------------------------------------------------------
//   read301
//---------------------------------------------------------

Score::FileError MasterScore::read302(XmlReader& e)
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
            else if (tag == "LastEID")    // Mu4.2+ compatibility
                  e.skipCurrentElement(); // skip, don't log
            else if (tag == "Score") {
                  MasterScore* score;
                  if (top) {
                        score = this;
                        top   = false;
                        }
                  else {
                        score = new MasterScore();
                        score->setMscVersion(mscVersion());
                        addMovement(score);
                        }
                  if (!score->read(e)) {
                        if (e.error() == QXmlStreamReader::CustomError)
                              return FileError::FILE_CRITICALLY_CORRUPTED;
                        return FileError::FILE_BAD_FORMAT;
                        }
                  }
            else if (tag == "Revision") {
                  Revision* revision = new Revision;
                  revision->read(e);
                  revisions()->add(revision);
                  }
            }
      return FileError::FILE_NO_ERROR;
      }

MStyle* styleDefaults301()
      {
      static MStyle* result = nullptr;

      if (result)
            return result;

      result = new MStyle();

      QFile baseDefaults(":/styles/legacy-style-defaults-v3.mss");

      if (!baseDefaults.open(QIODevice::ReadOnly))
            return result;

      result->load(&baseDefaults);

      return result;
      }
}

