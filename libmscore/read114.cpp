//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

//---------------------------------------------------------
//   read114
//    import old version 1.2 files
//    return false on error
//---------------------------------------------------------

bool Score::read114(const QDomElement& de)
      {
      spanner.clear();

      if (parentScore())
            setMscVersion(parentScore()->mscVersion());
      for (QDomElement ee = de.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
            curTrack = -1;
            const QString& tag(ee.tagName());
            const QString& val(ee.text());
            int i = val.toInt();
            if (tag == "Staff")
                  readStaff(ee);
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(this);
                  ks->read(ee);
                  customKeysigs.append(ks);
                  }
            else if (tag == "StaffType") {
                  int idx        = ee.attribute("idx").toInt();
                  StaffType* ost = _staffTypes.value(idx);
                  StaffType* st;
                  if (ost)
                        st = ost;
                  else {
                        QString group  = ee.attribute("group", "pitched");
                        if (group == "percussion")
                              st  = new StaffTypePercussion();
                        else if (group == "tablature")
                              st  = new StaffTypeTablature();
                        else
                              st  = new StaffTypePitched();
                        }
                  st->read(ee);
                  if (idx < _staffTypes.size())
                        _staffTypes[idx] = st;
                  else
                        _staffTypes.append(st);
                  }
            else if (tag == "siglist")
                  _sigmap->read(ee, _fileDivision);
            else if (tag == "tempolist")        // obsolete
                  ;           // tempomap()->read(ee, _fileDivision);
            else if (tag == "programVersion") {
                  _mscoreVersion = val;
                  parseVersion(val);
                  }
            else if (tag == "programRevision")
                  _mscoreRevision = val.toInt();
            else if (tag == "Mag" || tag == "MagIdx" || tag == "xoff" || tag == "yoff") {
                  // obsolete
                  ;
                  }
            else if (tag == "playMode")
                  _playMode = PlayMode(i);
            else if (tag == "SyntiSettings") {
                  _syntiState.clear();
                  _syntiState.read(ee);
                  }
            else if (tag == "Spatium")
                  _style.setSpatium (val.toDouble() * MScore::DPMM); // obsolete, moved to Style
            else if (tag == "page-offset")            // obsolete, moved to Score
                  setPageNumberOffset(i);
            else if (tag == "Division")
                  _fileDivision = i;
            else if (tag == "showInvisible")
                  _showInvisible = i;
            else if (tag == "showUnprintable")
                  _showUnprintable = i;
            else if (tag == "showFrames")
                  _showFrames = i;
            else if (tag == "showMargins")
                  _showPageborders = i;
            else if (tag == "Style") {
                  qreal sp = _style.spatium();
                  _style.load(ee);
                  if (_layoutMode == LayoutFloat) {
                        // style should not change spatium in
                        // float mode
                        _style.setSpatium(sp);
                        }
                  }
            else if (tag == "TextStyle") {
                  TextStyle s;
                  s.read(ee);
                  // settings for _reloff::x and _reloff::y in old formats
                  // is now included in style; setting them to 0 fixes most
                  // cases of backward compatibility
                  s.setRxoff(0);
                  s.setRyoff(0);
                  _style.setTextStyle(s);
                  }
            else if (tag == "page-layout") {
                  if (_layoutMode != LayoutFloat && _layoutMode != LayoutSystem) {
                        PageFormat pf = *pageFormat();
                        pf.read(ee);
                        setPageFormat(pf);
                        }
                  }
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(this);
                  text->read(ee);
                  setMetaTag("copyright", text->getText());
                  delete text;
                  }
            else if (tag == "movement-number")
                  setMetaTag("movementNumber", val);
            else if (tag == "movement-title")
                  setMetaTag("movementTitle", val);
            else if (tag == "work-number")
                  setMetaTag("workNumber", val);
            else if (tag == "work-title")
                  setMetaTag("workTitle", val);
            else if (tag == "source")
                  setMetaTag("source", val);
            else if (tag == "metaTag") {
                  QString name = ee.attribute("name");
                  setMetaTag(name, val);
                  }
            else if (tag == "Part") {
                  Part* part = new Part(this);
                  part->read(ee);
                  _parts.push_back(part);
                  }
            else if (tag == "Symbols")          // obsolete
                  ;
            else if (tag == "cursorTrack")      // obsolete
                  ;
            else if (tag == "Slur") {
                  Slur* slur = new Slur(this);
                  slur->read(ee);
                  spanner.append(slur);
                  }
            else if ((tag == "HairPin")
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Pedal")) {
                  ;
                  }
            else if (tag == "Excerpt") {
                  Excerpt* e = new Excerpt(this);
                  e->read(ee);
                  _excerpts.append(e);
                  }
            else if (tag == "Beam") {
                  Beam* beam = new Beam(this);
                  beam->read(ee);
                  beam->setParent(0);
                  // _beams.append(beam);
                  }
            else if (tag == "Score") {          // recursion
                  Score* s = new Score(style());
                  s->setParentScore(this);
                  s->read(ee);
                  addExcerpt(s);
                  }
            else if (tag == "PageList") {
                  for (QDomElement e = ee.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
                        if (e.tagName() == "Page") {
                              Page* page = new Page(this);
                              _pages.append(page);
                              page->read(e);
                              }
                        else
                              domError(e);
                        }
                  }
            else if (tag == "name")
                  setName(val);
            else
                  domError(ee);
            }

      for (int idx = 0; idx < _staves.size(); ++idx) {
            Staff* s = _staves[idx];
            int track = idx * VOICES;

            ClefList* cl = s->clefList();
            for (ciClefEvent i = cl->constBegin(); i != cl->constEnd(); ++i) {
                  int tick = i.key();
                  ClefType clefId = i.value()._concertClef;
                  Measure* m = tick2measure(tick);
                  if ((tick == m->tick()) && m->prevMeasure())
                        m = m->prevMeasure();
                  Segment* seg = m->getSegment(SegClef, tick);
                  if (seg->element(track))
                        static_cast<Clef*>(seg->element(track))->setGenerated(false);
                  else {
                        Clef* clef = new Clef(this);
                        clef->setClefType(clefId);
                        clef->setTrack(track);
                        clef->setParent(seg);
                        clef->setGenerated(false);
                        seg->add(clef);
                        }
                  }

            KeyList* km = s->keymap();
            for (ciKeyList i = km->begin(); i != km->end(); ++i) {
                  int tick = i->first;
                  KeySigEvent ke = i->second;
                  Measure* m = tick2measure(tick);
                  Segment* seg = m->getSegment(SegKeySig, tick);
                  if (seg->element(track))
                        static_cast<KeySig*>(seg->element(track))->setGenerated(false);
                  else {
                        KeySig* ks = keySigFactory(ke);
                        if (ks) {
                              ks->setParent(seg);
                              ks->setTrack(track);
                              ks->setGenerated(false);
                              seg->add(ks);
                              }
                        }
                  }
            }

      //
      // scan spanner in a II. pass
      //
      for (QDomElement ee = de.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
            const QString& tag(ee.tagName());
            if ((tag == "HairPin")
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Pedal")) {
                  Spanner* s = static_cast<Spanner*>(Element::name2Element(tag, this));
                  s->setTrack(0);
                  s->read(ee);
                  int tick2 = s->__tick2();
                  Segment* s1 = tick2segment(curTick);
                  Segment* s2 = tick2segment(tick2);
                  if (s1 == 0 || s2 == 0) {
                        qDebug("cannot place %s at tick %d - %d\n",
                           s->name(), s->__tick1(), tick2);
                        continue;
                        }
                  Measure* m = s2->measure();
                  if (s->type() == VOLTA) {
                        Volta* volta = static_cast<Volta*>(s);
                        volta->setStartMeasure(s1->measure());
                        volta->setEndMeasure(s2->measure());
                        volta->setAnchor(ANCHOR_MEASURE);
                        int n = volta->spannerSegments().size();
                        for (int i = 0; i < n; ++i) {
                              LineSegment* seg = volta->segmentAt(i);
                              if (!seg->userOff().isNull())
                                    seg->setUserYoffset(seg->userOff().y() + 0.5 * spatium());
                              }
                        }
                  if (s->anchor() == ANCHOR_MEASURE) {
                        if (tick2 == m->tick()) {
                              // anchor to EndBarLine segment of previous measure:
                              m  = m->prevMeasure();
                              s2 = m->getSegment(SegEndBarLine, tick2);
                              }
                        s1->measure()->add(s);
                        }
                  else {
                        s->setStartElement(s1);
                        s->setEndElement(s2);
                        s1->add(s);
                        }
                  if (s->type() == OTTAVA) {
                        // fix ottava position
                        Ottava* volta = static_cast<Ottava*>(s);
                        int n = volta->spannerSegments().size();
                        for (int i = 0; i < n; ++i) {
                              LineSegment* seg = volta->segmentAt(i);
                              if (!seg->userOff().isNull())
                                    seg->setUserYoffset(seg->userOff().y() - styleP(ST_ottavaY));
                              }
                        }
                  }
            }

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach(Spanner* s, m->spannerFor()) {
                  if (s->type() == VOLTA) {
                        Volta* volta = static_cast<Volta*>(s);
                        // reset user offsets
                        volta->setUserOff(QPointF());
                        volta->setReadPos(QPointF());
                        foreach(SpannerSegment* ss, volta->spannerSegments()) {
                              ss->setUserOff(QPointF());
                              ss->setReadPos(QPointF());
                              }
                        Measure* m2 = volta->endMeasure();
                        m2->removeSpannerBack(volta);
                        if (m2->prevMeasure())
                              m2 = m2->prevMeasure();
                        volta->setEndMeasure(m2);
                        m2->addSpannerBack(volta);
                        }
                  }
            }

      // check slurs
      foreach(Spanner* s, spanner) {
            if (s->type() != SLUR)
                  continue;
            Slur* slur = static_cast<Slur*>(s);

            if (!slur->startElement() || !slur->endElement()) {
                  qDebug("incomplete Slur\n");
                  if (slur->startElement()) {
                        qDebug("  front %d\n", static_cast<ChordRest*>(slur->startElement())->tick());
                        static_cast<ChordRest*>(slur->startElement())->removeSlurFor(slur);
                        }
                  if (slur->endElement()) {
                        qDebug("  back %d\n", static_cast<ChordRest*>(slur->endElement())->tick());
                        static_cast<ChordRest*>(slur->endElement())->removeSlurBack(slur);
                        }
                  }
            else {
                  ChordRest* cr1 = (ChordRest*)(slur->startElement());
                  ChordRest* cr2 = (ChordRest*)(slur->endElement());
                  if (cr1->tick() > cr2->tick()) {
                        qDebug("Slur invalid start-end tick %d-%d\n", cr1->tick(), cr2->tick());
                        slur->setStartElement(cr2);
                        slur->setEndElement(cr1);
                        }
                  int n1 = 0;
                  int n2 = 0;
                  foreach(Spanner* s, cr1->spannerFor()) {
                        if (s == slur)
                              ++n1;
                        }
                  foreach(Spanner* s, cr2->spannerBack()) {
                        if (s == slur)
                              ++n2;
                        }
                  if (n1 != 1 || n2 != 1) {
                        qDebug("Slur references bad: %d %d\n", n1, n2);
                        }
                  }
            }
      spanner.clear();
      connectTies();

      searchSelectedElements();

      _fileDivision = MScore::division;

      //
      //    sanity check for barLineSpan
      //
      foreach(Staff* staff, _staves) {
            int barLineSpan = staff->barLineSpan();
            int idx = staffIdx(staff);
            int n = nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d\n", idx, barLineSpan, n);
                  staff->setBarLineSpan(n - idx);
                  }
            }

      _showOmr = false;

      // create excerpts
      foreach(Excerpt* excerpt, _excerpts) {
            Score* nscore = ::createExcerpt(excerpt->parts());
            if (nscore) {
                  nscore->setParentScore(this);
                  nscore->setName(excerpt->title());
                  nscore->rebuildMidiMapping();
                  nscore->updateChannel();
                  nscore->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
                  nscore->doLayout();
                  excerpt->setScore(nscore);
                  }
            }

      //
      // check for soundfont,
      // add default soundfont if none found
      // (for compatibility with old scores)
      //
      bool hasSoundfont = false;
      foreach(const SyntiParameter& sp, _syntiState) {
            if (sp.name() == "soundfont") {
                  QFileInfo fi(sp.sval());
                  if(fi.exists())
                        hasSoundfont = true;
                  }
            }
      if (!hasSoundfont)
            _syntiState.append(SyntiParameter("soundfont", MScore::soundFont));

      fixTicks();
      renumberMeasures();
      rebuildMidiMapping();
      updateChannel();
      updateNotes();    // only for parts needed?
      return true;
      }


