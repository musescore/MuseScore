//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: bb.cpp 5427 2012-03-07 12:41:34Z wschweer $
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/mscore.h"
#include "bb.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/text.h"
#include "libmscore/box.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/drumset.h"
#include "libmscore/utils.h"
#include "libmscore/chordlist.h"
#include "libmscore/harmony.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/key.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/keysig.h"

namespace Ms {

//---------------------------------------------------------
//   BBTrack
//---------------------------------------------------------

BBTrack::BBTrack(BBFile* f)
      {
      bb          = f;
      _outChannel = -1;
      _drumTrack  = false;
      }

BBTrack::~BBTrack()
      {
      }

//---------------------------------------------------------
//   MNote
//	special Midi Note
//---------------------------------------------------------

struct MNote {
	Event mc;
      QList<Tie*> ties;

      MNote(const Event& _mc) : mc(_mc) {
            for (int i = 0; i < mc.notes().size(); ++i)
                  ties.append(0);
            }
      };

//---------------------------------------------------------
//   BBFile
//---------------------------------------------------------

BBFile::BBFile()
      {
      for (int i = 0; i < MAX_BARS; ++i)
            _barType[i]  = 0;
      bbDivision = 120;
      }

//---------------------------------------------------------
//   BBFile
//---------------------------------------------------------

BBFile::~BBFile()
      {
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool BBFile::read(const QString& name)
      {
      _siglist.clear();
      _siglist.add(0, Fraction(4, 4));        // debug

      _path = name;
      QFile f(name);
      if (!f.open(QIODevice::ReadOnly)) {
            return false;
            }
      ba = f.readAll();
      f.close();

      a    = (const unsigned char*) ba.data();
      size = ba.size();

      //---------------------------------------------------
      //    read version
      //---------------------------------------------------

      int idx = 0;
      _version = a[idx++];
      switch(_version) {
            case 0x43 ... 0x49:
                  break;
            default:
                  qDebug("BB: unknown file version %02x", _version);
                  return false;
            }

      qDebug("read <%s> version 0x%02x", qPrintable(name), _version);

      //---------------------------------------------------
      //    read title
      //---------------------------------------------------

      int len = a[idx++];
      _title  = new char[len+1];
      for (int i = 0; i < len; ++i)
            _title[i] = a[idx++];
      _title[len] = 0;

      //---------------------------------------------------
      //    read style(timesig), key and bpm
      //---------------------------------------------------

      ++idx;
      ++idx;
      _style = a[idx++] - 1;
      if (_style < 0 || _style >= int(sizeof(styles)/sizeof(*styles))) {
            qDebug("Import bb: unknown style %d", _style + 1);
            return false;
            }
      _key = a[idx++];

      // map D# G# A#   to Eb Ab Db
      // major   C, Db,  D, Eb,  E,  F, Gb,  G, Ab,  A, Bb,  B, C#, D#, F#, G#, A#
      // minor   C, Db,  D, Eb,  E,  F, Gb,  G, Ab,  A, Bb,  B, C#, D#, F#, G#, A#
      static int kt[] = {
           0,    0, -5,  2, -3,  4, -1, -6,  1, -4,  3, -2,  5,  7, -3,  6, -4, -2,
                -3, 4,  -1, -6,  1, -4,  3, -2,  5,  0, -5,  2,  4,  6,  3,  5, 7
           };
      if (_key >= int (sizeof(kt)/sizeof(*kt))) {
            qDebug("bad key %d", _key);
            return false;
            }
      _key = kt[_key];

      _bpm = a[idx] + (a[idx+1] << 8);
      idx += 2;

      qDebug("Title <%s>", _title);
      qDebug("style %d",   _style);
      qDebug("key   %d",   _key);
      qDebug("sig   %d/%d", timesigZ(), timesigN());
      qDebug("bpm   %d", _bpm);

      //---------------------------------------------------
      //    read bar types
      //---------------------------------------------------

      int bar = a[idx++];           // starting bar number
      while (bar < 255) {
            int val = a[idx++];
            if (val == 0)
                  bar += a[idx++];
            else {
                  qDebug("bar type: bar %d val %d", bar, val);
                  _barType[bar++] = val;
                  }
            }

      //---------------------------------------------------
      //    read chord extensions
      //---------------------------------------------------

      int beat;
      for (beat = 0; beat < MAX_BARS * 4;) {
            int val = a[idx++];
            if (val == 0)
                  beat += a[idx++];
            else {
                  BBChord c;
                  c.extension = val;
                  c.beat      = beat * (timesigZ() / timesigN());
                  ++beat;
                  _chords.append(c);
                  }
            }

      //---------------------------------------------------
      //    read chord root
      //---------------------------------------------------

      int roots = 0;
      int maxbeat = 0;
      for (beat = 0; beat < MAX_BARS * 4;) {
            int val = a[idx++];
            if (val == 0)
                  beat += a[idx++];
            else {
                  int root = val % 18;
                  int bass = (root - 1 + val / 18) % 12 + 1;
                  if (root == bass)
                        bass = 0;
                  int ibeat = beat * (timesigZ() / timesigN());
                  if (ibeat != _chords[roots].beat) {
                        qDebug("import bb: inconsistent chord type and root beat");
                        return false;
                        }
                  _chords[roots].root = root;
                  _chords[roots].bass = bass;
                  if (maxbeat < beat)
                        maxbeat = beat;
                  ++roots;
                  ++beat;
                  }
            }

      _measures = ((maxbeat + timesigZ() - 1) / timesigZ()) + 1;

      if (roots != _chords.size()) {
            qDebug("import bb: roots %d != extensions %d", roots, _chords.size());
            return false;
            }
      qDebug("Measures %d", _measures);

#if 0
      qDebug("================chords=======================");
      foreach(BBChord c, _chords) {
            qDebug("chord beat %3d bass %d root %d extension %d",
               c.beat, c.bass, c.root, c.extension);
            }
      qDebug("================chords=======================");
#endif

      if (a[idx] == 1) {            //??
            qDebug("Skip 0x%02x at 0x%04x", a[idx], idx);
            ++idx;
            }

      _startChorus = a[idx++];
      _endChorus   = a[idx++];
      _repeats     = a[idx++];

      qDebug("start chorus %d  end chorus %d repeats %d, pos now 0x%x",
         _startChorus, _endChorus, _repeats, idx);

      if (_startChorus >= _endChorus) {
            _startChorus = 0;
            _endChorus = 0;
            _repeats = 1;
            }

      //---------------------------------------------------
      //    read style file
      //---------------------------------------------------

      bool found = false;
      for (int i = idx; i < size; ++i) {
            if (a[i] == 0x42) {
                  if (a[i+1] < 16) {
                        for (int k = i+2; k < (i+18); ++k) {
                              if (a[k] == '.' && a[k+1] == 'S' && a[k+2] == 'T' && a[k+3] == 'Y') {
                                    found = true;
                                    break;
                                    }
                              }
                        }
                  if (found) {
                        idx = i + 1;
                        break;
                        }
                  }
            }
      if (!found) {
            qDebug("import bb: style file not found");
            return false;
            }

      qDebug("read styleName at 0x%x", idx);
      len = a[idx++];
      _styleName = new char[len+1];

      for (int i = 0; i < len; ++i)
            _styleName[i] = a[idx++];
      _styleName[len] = 0;

      qDebug("style name <%s>", _styleName);

      // read midi events
      int eventStart = a[size-4] + a[size-3] * 256;
      int eventCount = a[size-2] + a[size-1] * 256;

      int endTick = _measures * bbDivision * 4 * timesigZ() / timesigN();

      if (eventCount == 0) {
            qDebug("no melody");
            return true;
            }
      else {
            idx = eventStart;
            qDebug("melody found at 0x%x", idx);
            int i = 0;
            int lastLen = 0;
            for (i = 0; i < eventCount; ++i, idx+=12) {
                  int type = a[idx + 4] & 0xf0;
                  if (type == 0x90) {
                        int channel = a[idx + 7];
                        BBTrack* track = 0;
                        foreach (BBTrack* t, _tracks) {
                              if (t->outChannel() == channel) {
                                    track = t;
                                    break;
                                    }
                              }
                        if (track == 0) {
                              track = new BBTrack(this);
                              track->setOutChannel(channel);
                              _tracks.append(track);
                              }
                        int tick = a[idx] + (a[idx+1]<<8) + (a[idx+2]<<16) + (a[idx+3]<<24);
                        tick -= 4 * bbDivision;
                        if (tick >= endTick) {
                              qDebug("event tick %d > %d", tick, endTick);
                              continue;
                              }
                        Event note(ME_NOTE);
                        note.setOntime((tick * MScore::division) / bbDivision);
                        note.setPitch(a[idx + 5]);
                        note.setVelo(a[idx + 6]);
                        note.setChannel(channel);
                        int len = a[idx+8] + (a[idx+9]<<8) + (a[idx+10]<<16) + (a[idx+11]<<24);
                        if (len == 0) {
                              if (lastLen == 0) {
                                    qDebug("note event of len 0 at idx %04x", idx);
                                    continue;
                                    }
                              len = lastLen;
                              }
                        lastLen = len;
                        note.setDuration((len * MScore::division) / bbDivision);
                        track->append(note);
                        }
                  else if (type == 0xb0) {
                        // ignore controller
                        }
                  else if (type == 0)
                        break;
                  else {
                        qDebug("unknown event type 0x%02x at x%04x", a[idx + 4], idx);
                        break;
                        }
                  }
            qDebug("Events found x%02x (%d)", i, i);
            }
      return true;
      }

//---------------------------------------------------------
//   importBB
//    return true on success
//---------------------------------------------------------

Score::FileError importBB(Score* score, const QString& name)
      {
      BBFile bb;
      if(!QFileInfo(name).exists())
            return Score::FILE_NOT_FOUND;
      if (!bb.read(name)) {
            qDebug("cannot open file <%s>", qPrintable(name));
            return Score::FILE_OPEN_ERROR;
            }
      score->style()->set(ST_chordsXmlFile, true);
      score->style()->chordList()->read("chords.xml");
      *(score->sigmap()) = bb.siglist();

      QList<BBTrack*>* tracks = bb.tracks();
      int ntracks = tracks->size();
      if (ntracks == 0)             // no events?
            ntracks = 1;
      for (int i = 0; i < ntracks; ++i) {
            Part* part = new Part(score);
            Staff* s   = new Staff(score, part, 0);
            part->insertStaff(s);
            score->staves().append(s);
            score->appendPart(part);
            }

      //---------------------------------------------------
      //  create measures
      //---------------------------------------------------

      for (int i = 0; i < bb.measures(); ++i) {
            Measure* measure  = new Measure(score);
            int tick = score->sigmap()->bar2tick(i, 0);
            measure->setTick(tick);
            Fraction ts = score->sigmap()->timesig(tick).timesig();
            measure->setTimesig(ts);
            measure->setLen(ts);
      	score->add(measure);
            }

      //---------------------------------------------------
      //  create notes
      //---------------------------------------------------

	foreach (BBTrack* track, *tracks)
            track->cleanup();

      if (tracks->isEmpty()) {
            for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
                  if (mb->type() != Element::MEASURE)
                        continue;
                  Measure* measure = (Measure*)mb;
                  Rest* rest = new Rest(score, TDuration(TDuration::V_MEASURE));
                  rest->setDuration(measure->len());
                  rest->setTrack(0);
                  Segment* s = measure->getSegment(rest, measure->tick());
                  s->add(rest);
                  }
            }
      else {
      	int staffIdx = 0;
	      foreach (BBTrack* track, *tracks)
                  bb.convertTrack(score, track, staffIdx++);
            }

      for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (mb->type() != Element::MEASURE)
                  continue;
            Measure* measure = (Measure*)mb;
            Segment* s = measure->findSegment(Segment::SegChordRest, measure->tick());
            if (s == 0) {
                  Rest* rest = new Rest(score, TDuration(TDuration::V_MEASURE));
                  rest->setDuration(measure->len());
                  rest->setTrack(0);
                  Segment* s = measure->getSegment(rest, measure->tick());
                  s->add(rest);
                  }
            }

      score->spell();

      //---------------------------------------------------
      //    create title
      //---------------------------------------------------

      Text* text = new Text(score);
//      text->setSubtype(TEXT_TITLE);
      text->setTextStyleType(TEXT_STYLE_TITLE);
      text->setText(bb.title());

      MeasureBase* measure = score->first();
      if (measure->type() != Element::VBOX) {
            measure = new VBox(score);
            measure->setTick(0);
            measure->setNext(score->first());
            score->add(measure);
            }
      measure->add(text);

      //---------------------------------------------------
      //    create chord names
      //---------------------------------------------------

      static const int table[] = {
            14, 9, 16, 11, 18, 13, 8, 15, 10, 17, 12, 19
            };
      foreach(const BBChord& c, bb.chords()) {
            int tick = c.beat * MScore::division;
// qDebug("CHORD %d %d", c.beat, tick);
            Measure* m = score->tick2measure(tick);
            if (m == 0) {
                  qDebug("import BB: measure for tick %d not found", tick);
                  continue;
                  }
            Segment* s = m->getSegment(Segment::SegChordRest, tick);
            Harmony* h = new Harmony(score);
            h->setTrack(0);
            h->setRootTpc(table[c.root-1]);
            if (c.bass > 0)
                  h->setBaseTpc(table[c.bass-1]);
            else
                  h->setBaseTpc(INVALID_TPC);
            h->setId(c.extension);
            h->getDescription();
            h->render();
            s->add(h);
            }

      //---------------------------------------------------
      //    insert layout breaks
      //    add chorus repeat
      //---------------------------------------------------

      int startChorus = bb.startChorus() - 1;
      int endChorus   = bb.endChorus() - 1;

      int n = 0;
      for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (mb->type() != Element::MEASURE)
                  continue;
            Measure* measure = (Measure*)mb;
            if (n && (n % 4) == 0) {
                  LayoutBreak* lb = new LayoutBreak(score);
                  lb->setLayoutBreakType(LayoutBreak::LINE);
                  measure->add(lb);
                  }
            if (startChorus == n)
                  measure->setRepeatFlags(RepeatStart);
            else if (endChorus == n) {
                  measure->setRepeatFlags(RepeatEnd);
                  measure->setRepeatCount(bb.repeats());
                  }
            ++n;
            }

      foreach(Staff* staff, score->staves()) {
            int tick = 0;
            KeySigEvent kse;
            kse.setAccidentalType(bb.key());
            (*staff->keymap())[tick] = kse;
            KeySig* keysig = new KeySig(score);
            keysig->setTrack((score->staffIdx(staff->part()) + staff->rstaff()) * VOICES);
            keysig->setKeySigEvent(kse);
            Measure* mks = score->tick2measure(tick);
            Segment* sks = mks->getSegment(keysig, tick);
            sks->add(keysig);
            }
      score->fixTicks();
      return Score::FILE_NO_ERROR;
      }

//---------------------------------------------------------
//   processPendingNotes
//---------------------------------------------------------

int BBFile::processPendingNotes(Score* score, QList<MNote*>* notes, int len, int track)
      {
      Staff* cstaff    = score->staff(track/VOICES);
      Drumset* drumset = cstaff->part()->instr()->drumset();
      bool useDrumset  = cstaff->part()->instr()->useDrumset();
      int tick         = notes->at(0)->mc.ontime();

      //
      // look for len of shortest note
      //
      foreach (const MNote* n, *notes) {
      	if (n->mc.duration() < len)
                  len = n->mc.duration();
            }

      //
      // split notes on measure boundary
      //
      Measure* measure = score->tick2measure(tick);
      if (measure == 0 || (tick >= (measure->tick() + measure->ticks()))) {
            qDebug("no measure found for tick %d", tick);
            notes->clear();
            return len;
            }
      if ((tick + len) > measure->tick() + measure->ticks())
            len = measure->tick() + measure->ticks() - tick;

      Chord* chord = new Chord(score);
      chord->setTrack(track);
      TDuration d;
      d.setVal(len);
      chord->setDurationType(d);
      Segment* s = measure->getSegment(chord, tick);
      s->add(chord);

      foreach (MNote* n, *notes) {
            QList<Event>& nl = n->mc.notes();
            for (int i = 0; i < nl.size(); ++i) {
                  const Event& mn = nl[i];
      		Note* note = new Note(score);
                  note->setPitch(mn.pitch(), mn.tpc());
      		note->setTrack(track);
            	chord->add(note);

                  if (useDrumset) {
                        if (!drumset->isValid(mn.pitch())) {
                              qDebug("unmapped drum note 0x%02x %d", mn.pitch(), mn.pitch());
                              }
                        else {
                              chord->setStemDirection(drumset->stemDirection(mn.pitch()));
                              }
                        }
                  if (n->ties[i]) {
                        n->ties[i]->setEndNote(note);
                        n->ties[i]->setTrack(note->track());
                        note->setTieBack(n->ties[i]);
                        }
                  }
            if (n->mc.duration() <= len) {
                  notes->removeAt(notes->indexOf(n));
                  continue;
                  }
            for (int i = 0; i < nl.size(); ++i) {
                  const Event& mn = nl[i];
                  Note* note = chord->findNote(mn.pitch());
      		n->ties[i] = new Tie(score);
                  n->ties[i]->setStartNote(note);
      		note->setTieFor(n->ties[i]);
                  }
            n->mc.setOntime(n->mc.ontime() + len);
            n->mc.setDuration(n->mc.duration() - len);
            }
      return len;
      }

//---------------------------------------------------------
//   collectNotes
//---------------------------------------------------------

static ciEvent collectNotes(int tick, int voice, ciEvent i, const EventList* el, QList<MNote*>* notes)
      {
      for (;i != el->end(); ++i) {
            const Event& e = *i;
            if (e.type() != ME_CHORD)
                  continue;
            if (e.voice() != voice)
                  continue;
            if (e.ontime() > tick)
                  break;
            if (e.ontime() < tick)
                  continue;
            MNote* n = new MNote(e);
            notes->append(n);
            }
      return i;
      }

//---------------------------------------------------------
//   convertTrack
//---------------------------------------------------------

void BBFile::convertTrack(Score* score, BBTrack* track, int staffIdx)
	{
      track->findChords();
      int voices         = track->separateVoices(2);
	const EventList el = track->events();

      for (int voice = 0; voice < voices; ++voice) {
            int track = staffIdx * VOICES + voice;
            QList<MNote*> notes;

            int ctick = 0;
            ciEvent i = collectNotes(ctick, voice, el.begin(), &el, &notes);

            for (; i != el.end();) {
                  const Event& e = *i;
                  if (e.type() != ME_CHORD || e.voice() != voice) {
                        ++i;
                        continue;
                        }
                  //
                  // process pending notes
                  //
                  int restLen = e.ontime() - ctick;
// qDebug("ctick %d  rest %d ontick %d size %d", ctick, restLen, e.ontime(), notes.size());

                  if (restLen <= 0) {
                        qDebug("bad restlen ontime %d - ctick %d", e.ontime(), ctick);
                        abort();
                        }

                  while (!notes.isEmpty()) {
                        int len = processPendingNotes(score, &notes, restLen, track);
                        if (len == 0) {
                              qDebug("processPendingNotes returns zero, restlen %d, track %d", restLen, track);
                              ctick += restLen;
                              restLen = 0;
                              break;
                              }
                        ctick += len;
                        restLen -= len;
                        }
// qDebug("  1.ctick %d  rest %d", ctick, restLen);
                  //
                  // check for gap and fill with rest
                  //
                  if (voice == 0) {
                        while (restLen > 0) {
                              int len = restLen;
                  		Measure* measure = score->tick2measure(ctick);
                              if (measure == 0 || (ctick >= (measure->tick() + measure->ticks()))) {       // at end?
                                    ctick += len;
                                    restLen -= len;
                                    break;
                                    }
                              // split rest on measure boundary
                              if ((ctick + len) > measure->tick() + measure->ticks()) {
                                    len = measure->tick() + measure->ticks() - ctick;
                                    if (len <= 0) {
                                          qDebug("bad len %d", len);
                                          break;
                                          }
                                    }
                              TDuration d;
                              d.setVal(len);
                              Rest* rest = new Rest(score, d);
                              rest->setDuration(d.fraction());
                              rest->setTrack(staffIdx * VOICES);
                              Segment* s = measure->getSegment(rest, ctick);
                              s->add(rest);
// qDebug("   add rest %d", len);

                              ctick   += len;
                              restLen -= len;
                              }
                        }
                  else
                        ctick += restLen;

// qDebug("  2.ctick %d  rest %d", ctick, restLen);
                  //
                  // collect all notes at ctick
                  //
                  i = collectNotes(ctick, voice, i, &el, &notes);
                  }

            //
      	// process pending notes
            //
            while (!notes.isEmpty())
                  processPendingNotes(score, &notes, 0x7fffffff, track);
            }
      }

//---------------------------------------------------------
//   quantize
//    process one segment (measure)
//---------------------------------------------------------

void BBTrack::quantize(int startTick, int endTick, EventList* dst)
      {
      int mintick = MScore::division * 64;
      iEvent i = _events.begin();
      for (; i != _events.end(); ++i) {
            if (i->ontime() >= startTick)
                  break;
            }
      iEvent si = i;
      for (; i != _events.end(); ++i) {
            const Event& e = *i;
            if (e.ontime() >= endTick)
                  break;
            if (e.type() == ME_NOTE && (e.duration() < mintick))
                  mintick = e.duration();
            }
      if (mintick <= MScore::division / 16)        // minimum duration is 1/64
            mintick = MScore::division / 16;
      else if (mintick <= MScore::division / 8)
            mintick = MScore::division / 8;
      else if (mintick <= MScore::division / 4)
            mintick = MScore::division / 4;
      else if (mintick <= MScore::division / 2)
            mintick = MScore::division / 2;
      else if (mintick <= MScore::division)
            mintick = MScore::division;
      else if (mintick <= MScore::division * 2)
            mintick = MScore::division * 2;
      else if (mintick <= MScore::division * 4)
            mintick = MScore::division * 4;
      else if (mintick <= MScore::division * 8)
            mintick = MScore::division * 8;
      int raster;
      if (mintick > MScore::division)
            raster = MScore::division;
      else
            raster = mintick;

      //
      //  quantize onset
      //
      for (iEvent i = si; i != _events.end(); ++i) {
            Event e = *i;
            if (e.ontime() >= endTick)
                  break;
            if (e.type() == ME_NOTE) {
                  // prefer moving note to the right
      	      int tick = ((e.ontime() + raster/2) / raster) * raster;
                  int diff = tick - e.ontime();
	            int len  = e.duration() - diff;
	            e.setOntime(tick);
      	      e.setDuration(len);
                  }
            dst->insert(e);
            }
      //
      //  quantize duration
      //
      for (iEvent i = dst->begin(); i != dst->end(); ++i) {
            Event& e = *i;
            if (e.type() != ME_NOTE)
                  continue;
            int tick   = e.ontime();
            int len    = e.duration();
            int ntick  = tick + len;
            int nntick = -1;
            for (iEvent ii = (i+1); ii != dst->end(); ++ii) {
                  if (ii->type() == ME_NOTE) {
                        const Event& ee = *ii;
                        if (ee.ontime() == tick)
                              continue;
                        nntick = ee.ontime();
                        break;
                        }
                  }
            if (nntick == -1)
                  len = quantizeLen(len, raster);
            else {
                  int diff = nntick - ntick;
                  if (diff > 0) {
                        // insert rest?
                        if (diff <= raster)
                              len = nntick - tick;
                        else
                              len = quantizeLen(len, raster);
                        }
                  else {
                        if (diff > -raster)
                              len = nntick - tick;
                        else
                              len = quantizeLen(len, raster);
                        }
                  }
            e.setDuration(len);
            }
      }

//---------------------------------------------------------
//   cleanup
//    - quantize
//    - remove overlaps
//---------------------------------------------------------

void BBTrack::cleanup()
	{
      EventList dl;

      //
      //	quantize
      //
      int lastTick = 0;
      foreach (const Event& e, _events) {
            if (e.type() != ME_NOTE)
                  continue;
            int offtime  = e.offtime();
            if (offtime > lastTick)
                  lastTick = offtime;
            }
      int startTick = 0;
      for (int i = 1;; ++i) {
            int endTick = bb->siglist().bar2tick(i, 0);
            quantize(startTick, endTick, &dl);
            if (endTick > lastTick)
                  break;
            startTick = endTick;
            }

      //
      //
      //
      _events.clear();

      for(iEvent i = dl.begin(); i != dl.end(); ++i) {
            Event& e = *i;
            if (e.type() == ME_NOTE) {
                  iEvent ii = i;
                  ++ii;
                  for (; ii != dl.end(); ++ii) {
                        const Event& ee = *ii;
                        if (ee.type() != ME_NOTE || ee.pitch() != e.pitch())
                              continue;
                        if (ee.ontime() >= e.ontime() + e.duration())
                              break;
                        e.setDuration(ee.ontime() - e.ontime());
                        break;
                        }
                  if (e.duration() <= 0)
                        continue;
                  }
		_events.insert(e);
            }
      }

//---------------------------------------------------------
//   findChords
//---------------------------------------------------------

void BBTrack::findChords()
      {
      EventList dl;
      int n = _events.size();

      Drumset* drumset;
      if (_drumTrack)
            drumset = smDrumset;
      else
            drumset = 0;
      int jitter = 3;   // tick tolerance for note on/off

      for (int i = 0; i < n; ++i) {
            Event e = _events[i];
            if (e.type() == ME_INVALID)
                  continue;
            if (e.type() != ME_NOTE) {
                  dl.append(e);
                  continue;
                  }

            Event note(e);
            int ontime       = note.ontime();
            int offtime      = note.offtime();
            Event chord(ME_CHORD);
            chord.setOntime(ontime);
            chord.setDuration(note.duration());
            chord.notes().append(note);
            int voice = 0;
            chord.setVoice(voice);
            dl.append(chord);
            _events[i].setType(ME_INVALID);

            bool useDrumset = false;
            if (drumset) {
                  int pitch = note.pitch();
                  if (drumset->isValid(pitch)) {
                        useDrumset = true;
                        voice = drumset->voice(pitch);
                        chord.setVoice(voice);
                        }
                  }
            for (int k = i + 1; k < n; ++k) {
                  if (_events[k].type() != ME_NOTE)
                        continue;
                  Event nn = _events[k];
                  if (nn.ontime() - jitter > ontime)
                        break;
                  if (qAbs(nn.ontime() - ontime) > jitter || qAbs(nn.offtime() - offtime) > jitter)
                        continue;
                  int pitch = nn.pitch();
                  if (useDrumset) {
                        if (drumset->isValid(pitch) && drumset->voice(pitch) == voice) {
                              chord.notes().append(nn);
                              _events[k].setType(ME_INVALID);
                              }
                        }
                  else {
                        chord.notes().append(nn);
                        _events[k].setType(ME_INVALID);
                        }
                  }
            }
      _events = dl;
      }


//---------------------------------------------------------
//   separateVoices
//---------------------------------------------------------

int BBTrack::separateVoices(int /*maxVoices*/)
      {
      return 1;
      }
}

