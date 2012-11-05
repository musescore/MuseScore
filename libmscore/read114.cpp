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
#include "box.h"
#include "dynamic.h"
#include "drumset.h"

//---------------------------------------------------------
//   resolveSymCompatibility
//---------------------------------------------------------

static SymId resolveSymCompatibility(SymId i, QString programVersion)
      {
      if(!programVersion.isEmpty() && programVersion < "1.1")
          i = SymId(i + 5);
      switch(i) {
            case 197:
                  return pedalPedSym;
            case 191:
                  return pedalasteriskSym;
            case 193:
                  return pedaldotSym;
            case 192:
                  return pedaldashSym;
            case 139:
                  return trillSym;
            default:
                  return noSym;
            }
      }

//---------------------------------------------------------
//   Staff::read114
//---------------------------------------------------------

void Staff::read114(const QDomElement& de, ClefList& _clefList)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "type") {
                  StaffType* st = score()->staffType(val.toInt());
                  if (st)
                        _staffType = st;
                  }
            else if (tag == "lines")
                  setLines(val.toInt());
            else if (tag == "small")
                  setSmall(val.toInt());
            else if (tag == "invisible")
                  setInvisible(val.toInt());
            else if (tag == "slashStyle")
                  ;                       // obsolete: setSlashStyle(v);
            else if (tag == "cleflist")
                  _clefList.read(e, _score);
            else if (tag == "keylist")
                  _keymap->read(e, _score);
            else if (tag == "bracket") {
                  BracketItem b;
                  b._bracket = BracketType(e.attribute("type", "-1").toInt());
                  b._bracketSpan = e.attribute("span", "0").toInt();
                  _brackets.append(b);
                  }
            else if (tag == "barLineSpan")
                  _barLineSpan = val.toInt();
            else if (tag == "distOffset")
                  _userDist = e.text().toDouble() * spatium();
            else if (tag == "linkedTo") {
                  int v = val.toInt() - 1;
                  //
                  // if this is an excerpt, link staff to parentScore()
                  //
                  if (score()->parentScore()) {
                        Staff* st = score()->parentScore()->staff(v);
                        if (st)
                              linkTo(st);
                        else {
                              qDebug("staff %d not found in parent", v);
                              }
                        }
                  else {
                        int idx = score()->staffIdx(this);
                        if (v < idx)
                              linkTo(score()->staff(v));
                        }
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   Part::read114
//---------------------------------------------------------

void Part::read114(const QDomElement& de, QList<ClefList*>& clefList)
      {
      int rstaff = 0;
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "Staff") {
                  Staff* staff = new Staff(_score, this, rstaff);
                  _score->staves().push_back(staff);
                  _staves.push_back(staff);
                  ClefList* cl = new ClefList;
                  clefList.append(cl);
                  staff->read114(e, *cl);
                  ++rstaff;
                  }
            else if (tag == "Instrument") {
                  instr(0)->read(e);
                  Drumset* d = instr(0)->drumset();
                  Staff*   st = staff(0);
                  if (d && st && st->lines() != 5) {
                        int n = 0;
                        if (st->lines() == 1)
                              n = 4;
                        for (int  i = 0; i < DRUM_INSTRUMENTS; ++i)
                              d->drum(i).line -= n;
                        }
                  }
            else if (tag == "name") {
                  Text* t = new Text(score());
                  t->read(e);
                  instr(0)->setLongName(t->getFragment());
                  delete t;
                  }
            else if (tag == "shortName") {
                  Text* t = new Text(score());
                  t->read(e);
                  instr(0)->setShortName(t->getFragment());
                  delete t;
                  }
            else if (tag == "trackName")
                  _partName = val;
            else if (tag == "show")
                  _show = val.toInt();
            else
                  domError(e);
            }
      if (_partName.isEmpty())
            _partName = instr(0)->trackName();

      if (instr(0)->useDrumset()) {
            foreach(Staff* staff, _staves) {
                  int lines = staff->lines();
                  staff->setStaffType(score()->staffType(PERCUSSION_STAFF_TYPE));
                  staff->setLines(lines);
                  }
            }
      }


//---------------------------------------------------------
//   read114
//    import old version 1.2 files
//    return false on error
//---------------------------------------------------------

Score::FileError Score::read114(const QDomElement& de)
      {
      spanner.clear();

      QList<ClefList*> clefListList;
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
                  StaffType* ost = staffType(idx);
                  StaffType* st;
                  if (ost)
                        st = ost->clone();
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
                  st->setBuildin(false);
                  addStaffType(idx, st);
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
                        PageFormat pf;
                        pf.copy(*pageFormat());
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
                  part->read114(ee, clefListList);
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

            ClefList* cl = clefListList[idx];
            for (ciClefEvent i = cl->constBegin(); i != cl->constEnd(); ++i) {
                  int tick = i.key();
                  ClefType clefId = i.value()._concertClef;
                  Measure* m = tick2measure(tick);
                  if ((tick == m->tick()) && m->prevMeasure())
                        m = m->prevMeasure();
                  Segment* seg = m->getSegment(Segment::SegClef, tick);
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
                  Segment* seg = m->getSegment(Segment::SegKeySig, tick);
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
      qDeleteAll(clefListList);

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

                 if ((tag == "Ottava")
                  || (tag == "TextLine")
                  || (tag == "Volta")
                  || (tag == "Pedal")) {
                      TextLine* tl = static_cast<TextLine*>(s);
                      tl->setBeginSymbol(resolveSymCompatibility(tl->beginSymbol(), mscoreVersion()));
                      tl->setContinueSymbol(resolveSymCompatibility(tl->continueSymbol(), mscoreVersion()));
                      tl->setEndSymbol(resolveSymCompatibility(tl->endSymbol(), mscoreVersion()));
                      }

                  int tick2 = s->__tick2();
                  Segment* s1 = tick2segment(curTick);
                  Segment* s2 = tick2segment(tick2);
                  if (s1 == 0 || s2 == 0) {
                        qDebug("cannot place %s at tick %d - %d",
                           s->name(), s->__tick1(), tick2);
                        continue;
                        }
                  if (s->type() == Element::VOLTA) {
                        Volta* volta = static_cast<Volta*>(s);
                        volta->setAnchor(Spanner::ANCHOR_MEASURE);
                        volta->setStartMeasure(s1->measure());
                        Measure* m2 = s2->measure();
                        if (s2->tick() == m2->tick())
                              m2 = m2->prevMeasure();
                        volta->setEndMeasure(m2);
                        s1->measure()->add(s);
                        int n = volta->spannerSegments().size();
                        if (n) {
                              // volta->setYoff(-styleS(ST_voltaHook).val());
                              // LineSegment* ls = volta->segmentAt(0);
                              // ls->setReadPos(QPointF());
                              }
                        }
                  else {
                        s->setStartElement(s1);
                        s->setEndElement(s2);
                        s1->add(s);
                        }
                  if (s->type() == Element::OTTAVA) {
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

      // check slurs
      foreach(Spanner* s, spanner) {
            if (s->type() != Element::SLUR)
                  continue;
            Slur* slur = static_cast<Slur*>(s);

            if (!slur->startElement() || !slur->endElement()) {
                  qDebug("incomplete Slur");
                  if (slur->startElement()) {
                        qDebug("  front %d", static_cast<ChordRest*>(slur->startElement())->tick());
                        static_cast<ChordRest*>(slur->startElement())->removeSlurFor(slur);
                        }
                  if (slur->endElement()) {
                        qDebug("  back %d", static_cast<ChordRest*>(slur->endElement())->tick());
                        static_cast<ChordRest*>(slur->endElement())->removeSlurBack(slur);
                        }
                  }
            else {
                  ChordRest* cr1 = (ChordRest*)(slur->startElement());
                  ChordRest* cr2 = (ChordRest*)(slur->endElement());
                  if (cr1->tick() > cr2->tick()) {
                        qDebug("Slur invalid start-end tick %d-%d", cr1->tick(), cr2->tick());
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
                        qDebug("Slur references bad: %d %d", n1, n2);
                        }
                  }
            }
      spanner.clear();
      connectTies();

      //
      // remove "middle beam" flags from first ChordRest in
      // measure
      //
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            int tracks = nstaves() * VOICES;
            for (int track = 0; track < tracks; ++track) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (s->subtype() != Segment::SegChordRest)
                              continue;
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (cr) {
                              switch(cr->beamMode()) {
                                    case BEAM_AUTO:
                                    case BEAM_BEGIN:
                                    case BEAM_END:
                                    case BEAM_NO:
                                          break;
                                    case BEAM_MID:
                                    case BEAM_BEGIN32:
                                    case BEAM_BEGIN64:
                                          cr->setBeamMode(BEAM_BEGIN);
                                          break;
                                    case BEAM_INVALID:
                                          if (cr->type() == Element::CHORD)
                                                cr->setBeamMode(BEAM_AUTO);
                                          else
                                                cr->setBeamMode(BEAM_NO);
                                          break;
                                    }
                              break;
                              }
                        }
                  }
            }
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() == Element::VBOX) {
                  Box* b  = static_cast<Box*>(mb);
                  qreal y = point(styleS(ST_staffUpperBorder));
                  b->setBottomGap(y);
                  }
            }

      _fileDivision = MScore::division;

      //
      //    sanity check for barLineSpan
      //
      foreach(Staff* staff, _staves) {
            int barLineSpan = staff->barLineSpan();
            int idx = staffIdx(staff);
            int n = nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  staff->setBarLineSpan(n - idx);
                  }
            }

      // adjust some styles
      if (styleS(ST_lyricsDistance).val() == MScore::baseStyle()->valueS(ST_lyricsDistance).val())
            style()->set(ST_lyricsDistance, Spatium(2.0));
      if (styleS(ST_voltaY).val() == MScore::baseStyle()->valueS(ST_voltaY).val())
            style()->set(ST_voltaY, Spatium(-2.0));

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
                  if (fi.exists())
                        hasSoundfont = true;
                  }
            }
      if (!hasSoundfont)
            _syntiState.append(SyntiParameter("soundfont", MScore::soundFont));

//      _mscVersion = MSCVERSION;     // for later drag & drop usage
      fixTicks();
      renumberMeasures();
      rebuildMidiMapping();
      updateChannel();
      updateNotes();    // only for parts needed?

      doLayout();

      //
      // move some elements
      //
      for (Segment* s = firstSegment(); s; s = s->next1()) {
            foreach (Element* e, s->annotations()) {
                  if (e->type() == Element::TEMPO_TEXT) {
                        // reparent from measure to segment
                        e->setUserOff(QPointF(e->userOff().x() - s->pos().x(),
                           e->userOff().y()));
                        }
                  }
            }

      return FILE_NO_ERROR;
      }
