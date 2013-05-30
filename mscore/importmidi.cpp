//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "midi/midifile.h"
#include "midi/midiinstrument.h"
#include "libmscore/score.h"
#include "libmscore/key.h"
#include "libmscore/clef.h"
#include "libmscore/sig.h"
#include "libmscore/tempo.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/utils.h"
#include "libmscore/text.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/style.h"
#include "libmscore/part.h"
#include "libmscore/timesig.h"
#include "libmscore/barline.h"
#include "libmscore/pedal.h"
#include "libmscore/ottava.h"
#include "libmscore/lyrics.h"
#include "libmscore/bracket.h"
#include "libmscore/drumset.h"
#include "libmscore/box.h"
#include "libmscore/keysig.h"
#include "libmscore/pitchspelling.h"
#include "preferences.h"
#include "importmidi_meter.h"

namespace Ms {

extern Preferences preferences;

//---------------------------------------------------------
//   MidiNote
//---------------------------------------------------------

class MidiNote {
   public:
      int pitch, velo;
      int onTime, len;
      Tie* tie = 0;
      };

//---------------------------------------------------------
//   MidiChord
//---------------------------------------------------------

class MidiChord {
   public:
      int voice = 0;
      int onTime;
      int duration;
      QList<MidiNote> notes;
      };

//---------------------------------------------------------
//   MTrack
//---------------------------------------------------------

class MTrack {
   public:
      int minPitch = 127, maxPitch = 0, medPitch = 0, program = 0;
      Staff* staff = 0;
      MidiTrack* mtrack = 0;
      QString name;
      bool hasKey = false;

      std::multimap<int, MidiChord> chords;

      void convertTrack(int lastTick);
      void findChords();
      void cleanup(int lastTick, TimeSigMap*);
      void quantize(int startBarTick, int endBarTick, std::multimap<int, MidiChord>& dst);
      void processPendingNotes(QList<MidiChord>& notes, int voice, int ctick, int tick);
      void processMeta(int tick, const MidiEvent& mm);

   private:
      std::multimap<int, MidiChord>::iterator findStartChord(int startBarTick);
      int findShortestNoteDurationInBar(const std::multimap<int, MidiChord>::iterator &start, int endBarTick);
      void quantizeToDurationList(const std::multimap<int, MidiChord>::iterator &startChordIter,
                                  int mintick, int endBarTick, std::multimap<int, MidiChord>& dst);
      };


int MTrack::findShortestNoteDurationInBar(const std::multimap<int, MidiChord>::iterator &start, int endBarTick)
      {
      int division = MScore::division;
      int minDuration = division;

      // find shortest note in measure
      //
      for (auto i = start; i != chords.end(); ++i) {
            if (i->first >= endBarTick)
                  break;
            minDuration = qMin(minDuration, i->second.duration);
            }
      //
      // determine suitable quantization value based
      // on shortest note in measure
      //
      int div = division;
      if (minDuration <= division / 16)        // minimum duration is 1/64
            div = division / 16;
      else if (minDuration <= division / 8)
            div = division / 8;
      else if (minDuration <= division / 4)
            div = division / 4;
      else if (minDuration <= division / 2)
            div = division / 2;
      else if (minDuration <= division)
            div = division;
      else if (minDuration <= division * 2)
            div = division * 2;
      else if (minDuration <= division * 4)
            div = division * 4;
      else if (minDuration <= division * 8)
            div = division * 8;
      if (div == (division / 16))
            minDuration = div;
      else
            minDuration = quantizeLen(minDuration, div >> 1);    //closest

      return minDuration;
      }

int userQuantNoteToTicks(MidiOperation::QuantValue quantNote)
      {
      int division = MScore::division;
      int userQuantValue = preferences.shortestNote;
      // specified quantization value
      switch (quantNote) {
            case MidiOperation::QuantValue::N_4:
                  userQuantValue = division;
                  break;
//            case MidiOperation::QuantValue::N_4_triplet:
//                  userQuantValue = division * 2 / 3;
//                  break;
            case MidiOperation::QuantValue::N_8:
                  userQuantValue = division / 2;
                  break;
//            case MidiOperation::QuantValue::N_8_triplet:
//                  userQuantValue = division / 3;
//                  break;
            case MidiOperation::QuantValue::N_16:
                  userQuantValue = division / 4;
                  break;
//            case MidiOperation::QuantValue::N_16_triplet:
//                  userQuantValue = division / 6;
//                  break;
            case MidiOperation::QuantValue::N_32:
                  userQuantValue = division / 8;
                  break;
//            case MidiOperation::QuantValue::N_32_triplet:
//                  userQuantValue = division / 12;
//                  break;
            case MidiOperation::QuantValue::N_64:
                  userQuantValue = division / 16;
                  break;
            case MidiOperation::QuantValue::FROM_PREFERENCES:
            default:
                  userQuantValue = preferences.shortestNote;
                  break;
            }

      return userQuantValue;
      }

std::multimap<int, MidiChord>::iterator MTrack::findStartChord(int startBarTick)
      {
      auto i = chords.begin();
      for (; i != chords.end(); ++i) {
            if (i->first >= startBarTick)
                  break;
            }
      return i;
      }

void MTrack::quantizeToDurationList(const std::multimap<int, MidiChord>::iterator& startChordIter,
                                    int mintick, int endBarTick, std::multimap<int, MidiChord>& dst)
      {
      int raster  = mintick;
      int raster2 = raster >> 1;
      for (auto i = startChordIter; i != chords.end(); ++i) {
            if (i->first >= endBarTick)
                  break;
            MidiChord e = i->second;
            e.onTime    = ((e.onTime + raster2) / raster) * raster;
            e.duration  = quantizeLen(e.duration, raster);
            dst.insert({e.onTime, e});
            }
      }

//---------------------------------------------------------
//   quantize
//---------------------------------------------------------

void MTrack::quantize(int startBarTick, int endBarTick, std::multimap<int, MidiChord>& dst)
      {
      auto startChordIter = findStartChord(startBarTick);
      auto operations = preferences.midiImportOperations.currentTrackOperations();
      int mintick;

      if (operations.quantize.value == MidiOperation::QuantValue::SHORTEST_IN_BAR)
            mintick = findShortestNoteDurationInBar(startChordIter, endBarTick);
      else {
            int userQuantValue = userQuantNoteToTicks(operations.quantize.value);
            // if user value larger than the smallest note in bar
            // then use the smallest note to keep faster events
            if (operations.quantize.reduceToShorterNotesInBar) {
                  mintick = findShortestNoteDurationInBar(startChordIter, endBarTick);
                  mintick = qMin(userQuantValue, mintick);
                  }
            else
                  mintick = userQuantValue;
            }
      quantizeToDurationList(startChordIter, mintick, endBarTick, dst);
      }

//---------------------------------------------------------
//   cleanup
//    - quantize
//    - remove overlaps
//    - replace old chords by quantized ones
//
//   Note that chords here consist of a single note
//     because notes are not united into chords yet
//---------------------------------------------------------

void MTrack::cleanup(int lastTick, TimeSigMap* sigmap)
      {
      std::multimap<int, MidiChord> dl; // duration list

      //    quantize every measure
      //    and fill the duration list with quantized chords
      //
      int startBarTick = 0;
      for (int i = 1;; ++i) { // iterate over all measures by indexes
            int endBarTick = sigmap->bar2tick(i, 0);
            quantize(startBarTick, endBarTick, dl);
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }

      // remove overlapping notes with the same pitch
      //
      for (auto i = dl.begin(); i != dl.end(); ++i) {
            const MidiChord& e = i->second;
            auto ii = i;
            ++ii;
            for (; ii != dl.end(); ++ii) {
                  const MidiChord& ee = ii->second;
                  // check the first and the only note in chord
                  if (ee.notes[0].pitch != e.notes[0].pitch)
                        continue;
                  if (ee.onTime >= (e.onTime + e.duration))
                        break;
                  qDebug("MidiTrack::cleanup: overlapping events: %d+%d %d+%d",
                     e.onTime, e.duration,
                     ee.onTime, ee.duration);
                  i->second.duration = ee.onTime - e.onTime;
                  break;
                  }
            if (e.duration <= 0) {
                  qDebug("MidiTrack::cleanup: duration <= 0: drop note at %d", e.onTime);
                  continue;
                  }
            }
      chords = dl;
      }

//---------------------------------------------------------
//   findChords
//     with some tolerance
//---------------------------------------------------------

void MTrack::findChords()
      {
      Drumset* drumset = mtrack->drumTrack() ? smDrumset : 0;
      const int jitter       = 3;   // tick tolerance for note on/off

      for (auto i = chords.begin(); i != chords.end(); ++i) {
            const MidiChord& e = i->second;
            int ontime   = i->first;
            int offtime  = ontime + e.duration;

            bool useDrumset = false;
            if (drumset) {
                  int pitch = e.notes[0].pitch;
                  if (drumset->isValid(pitch)) {
                        useDrumset = true;
                        i->second.voice = drumset->voice(pitch);
                        }
                  }
            auto k = i;
            ++k;
            for (; k != chords.end();) {
                  if (k->first - jitter > ontime)
                        break;
                  int on2  = k->first;
                  int off2 = k->first + k->second.duration;
                  if (qAbs(on2 - ontime) > jitter || qAbs(off2 - offtime) > jitter) {
                        ++k;
                        continue;
                        }
                  int pitch = k->second.notes[0].pitch;
                  if (!useDrumset
                     || (drumset->isValid(pitch) && drumset->voice(pitch) == e.voice)
                     ) {
                        i->second.notes.append(k->second.notes[0]);
                        k = chords.erase(k);
                        }
                  else
                        ++k;
                  }
            }
      }

//---------------------------------------------------------
//   processMeta
//---------------------------------------------------------

void MTrack::processMeta(int tick, const MidiEvent& mm)
      {
      if (!staff) {
            qDebug("processMeta: no staff");
            return;
            }
      const uchar* data = (uchar*)mm.edata();
      int staffIdx      = staff->idx();
      Score* cs         = staff->score();

      switch (mm.metaType()) {
            case META_TEXT:
            case META_LYRIC: {
                  QString s((char*)data);
                  cs->addLyrics(tick, staffIdx, s);
                  }
                  break;

            case META_TRACK_NAME:
                  name = (const char*)data;
                  break;

            case META_TEMPO:
                  {
                  unsigned tempo = data[2] + (data[1] << 8) + (data[0] <<16);
                  double t = 1000000.0 / double(tempo);
                  cs->setTempo(tick, t);
                  // TODO: create TempoText
                  }
                  break;

            case META_KEY_SIGNATURE:
                  {
                  int key = ((const char*)data)[0];
                  if (key < -7 || key > 7) {
                        qDebug("ImportMidi: illegal key %d", key);
                        break;
                        }
                  KeySigEvent ks;
                  ks.setAccidentalType(key);
                  (*staff->keymap())[tick] = ks;
                  hasKey = true;
                  }
                  break;
            case META_COMPOSER:     // mscore extension
            case META_POET:
            case META_TRANSLATOR:
            case META_SUBTITLE:
            case META_TITLE:
                  {
                  Text* text = new Text(cs);
                  switch(mm.metaType()) {
                        case META_COMPOSER:
                              text->setTextStyleType(TEXT_STYLE_COMPOSER);
                              break;
                        case META_TRANSLATOR:
                              text->setTextStyleType(TEXT_STYLE_TRANSLATOR);
                              break;
                        case META_POET:
                              text->setTextStyleType(TEXT_STYLE_POET);
                              break;
                        case META_SUBTITLE:
                              text->setTextStyleType(TEXT_STYLE_SUBTITLE);
                              break;
                        case META_TITLE:
                              text->setTextStyleType(TEXT_STYLE_TITLE);
                              break;
                        }

                  text->setText((const char*)(mm.edata()));

                  MeasureBase* measure = cs->first();
                  if (measure->type() != Element::VBOX) {
                        measure = new VBox(cs);
                        measure->setTick(0);
                        measure->setNext(cs->first());
                        cs->add(measure);
                        }
                  measure->add(text);
                  }
                  break;

            case META_COPYRIGHT:
                  cs->setMetaTag("Copyright", QString((const char*)(mm.edata())));
                  break;

            case META_TIME_SIGNATURE:
                  qDebug("midi: meta timesig: %d, division %d", tick, MScore::division);
                  cs->sigmap()->add(tick, Fraction(data[0], 1 << data[1]));
                  break;

            default:
                  if (MScore::debugMode)
                        qDebug("unknown meta type 0x%02x", mm.metaType());
                  break;
            }
      }

//---------------------------------------------------------
//   fillGapsWithRests
//    check for gap and fill with rest
//---------------------------------------------------------

void fillGapsWithRests(Score* score, int ctick, int restLen, int track)
      {
      bool useDots = preferences.midiImportOperations.currentTrackOperations().useDots;
      while (restLen > 0) {
            int len = restLen;
            Measure* measure = score->tick2measure(ctick);
            if (ctick >= measure->tick() + measure->ticks()) {
                  qDebug("tick2measure: %d end of score?", ctick);
                  ctick += restLen;
                  restLen = 0;
                  break;
                  }
            // split rest on measure boundary
            if ((ctick + len) > measure->tick() + measure->ticks())
                  len = measure->tick() + measure->ticks() - ctick;
            if (len >= measure->ticks()) {
                  len = measure->ticks();
                  TDuration d(TDuration::V_MEASURE);
                  Rest* rest = new Rest(score, d);
                  rest->setDuration(measure->len());
                  rest->setTrack(track);
                  Segment* s = measure->getSegment(rest, ctick);
                  s->add(rest);
                  restLen -= len;
                  ctick   += len;
                  }
            else {
                  QList<TDuration> dl = Meter::toDurationList(ctick - measure->tick(),
                                                              ctick + len - measure->tick(),
                                                              measure->timesig(),
                                                              Meter::DurationType::REST,
                                                              useDots);
                  if (dl.isEmpty()) {
                        qDebug("cannot create duration list for len %d", len);
                        restLen = 0;      // fake
                        break;
                        }
                  foreach (TDuration d, dl) {
                        Rest* rest = new Rest(score, d);
                        rest->setDuration(d.fraction());
                        rest->setTrack(track);
                        Segment* s = measure->getSegment(Segment::SegChordRest, ctick);
                        s->add(rest);
                        restLen -= d.ticks();
                        ctick   += d.ticks();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   processPendingNotes
//---------------------------------------------------------

void MTrack::processPendingNotes(QList<MidiChord>& notes, int voice, int ctick, int t)
      {
      Score* score     = staff->score();
      int track        = staff->idx() * VOICES + voice;
      Drumset* drumset = staff->part()->instr()->drumset();
      bool useDrumset  = staff->part()->instr()->useDrumset();
      bool useDots = preferences.midiImportOperations.currentTrackOperations().useDots;

      while (!notes.isEmpty()) {
            int tick = notes[0].onTime;
            int len  = t - tick;
            if (len <= 0)
                  break;
            foreach (const MidiChord& c, notes) {
                  if ((c.duration < len) && (c.duration != 0))
                        len = c.duration;
                  }
            Measure* measure = score->tick2measure(tick);
            // split notes on measure boundary
            if ((tick + len) > measure->tick() + measure->ticks())
                  len = measure->tick() + measure->ticks() - tick;

            QList<TDuration> dl = Meter::toDurationList(tick - measure->tick(),
                                                        tick + len - measure->tick(),
                                                        measure->timesig(),
                                                        Meter::DurationType::NOTE,
                                                        useDots);

            if (dl.isEmpty())
                  break;
            TDuration d = dl[0];
            len = d.ticks();

            Chord* chord = new Chord(score);
            chord->setTrack(track);
            chord->setDurationType(d);
            chord->setDuration(d.fraction());
            Segment* s = measure->getSegment(chord, tick);
            s->add(chord);
            chord->setUserPlayEvents(true);

            int actualTicks = chord->actualTicks();
            for (int k = 0; k < notes.size(); ++k) {
                  MidiChord& n = notes[k];
                  const QList<MidiNote>& nl = n.notes;
                  for (int i = 0; i < nl.size(); ++i) {
                        const MidiNote& mn = nl[i];
                        Note* note = new Note(score);

                        // TODO - does this need to be key-aware?
                        note->setPitch(mn.pitch, pitch2tpc(mn.pitch, KEY_C, PREFER_NEAREST));
                        chord->add(note);
                        note->setVeloType(MScore::USER_VAL);
                        note->setVeloOffset(mn.velo);

                        NoteEventList el;
                        int ron  = (mn.onTime - tick) * 1000 / actualTicks;
                        int rlen = mn.len * 1000 / actualTicks;
                        el.append(NoteEvent(0, ron, rlen));
                        note->setPlayEvents(el);

                        if (useDrumset) {
                              if (!drumset->isValid(mn.pitch))
                                    qDebug("unmapped drum note 0x%02x %d", mn.pitch, mn.pitch);
                              else
                                    chord->setStemDirection(drumset->stemDirection(mn.pitch));
                              }

                        if (n.notes[i].tie) {
                              n.notes[i].tie->setEndNote(note);
                              n.notes[i].tie->setTrack(note->track());
                              note->setTieBack(n.notes[i].tie);
                              }
                        }
                  if (n.duration <= len) {
                        notes.removeAt(k);
                        --k;
                        continue;
                        }

                  for (int i = 0; i < nl.size(); ++i) {
                        const MidiNote& mn = nl[i];
                        Note* note = chord->findNote(mn.pitch);
                  n.notes[i].tie = new Tie(score);
                        n.notes[i].tie->setStartNote(note);
                    note->setTieFor(n.notes[i].tie);
                        }

                  n.onTime   = n.onTime + len;
                  n.duration = n.duration - len;
                  }

            ctick += len;
            }
      if (voice == 0)
            fillGapsWithRests(score, ctick, t - ctick, track);
      }

//---------------------------------------------------------
//   convertTrack
//---------------------------------------------------------

void MTrack::convertTrack(int lastTick)
      {
      Score* score     = staff->score();
      int key          = 0;  // TODO-LIB findKey(mtrack, score->sigmap());
      int track        = staff->idx() * VOICES;
      int voices       = 1; // mtrack->separateVoices(2);

      for (int voice = 0; voice < voices; ++voice) {
            QList<MidiChord> notes;
            int ctick = 0;

            for (auto i = chords.begin(); i != chords.end();) {
                  const MidiChord& e = i->second;
                  if (e.voice != voice) {
                        ++i;
                        continue;
                        }
                  processPendingNotes(notes, voice, ctick, i->first);

                  //
                  // collect all notes on current
                  // tick position
                  //
                  ctick = i->first;       // debug
                  for (;i != chords.end(); ++i) {
                        const MidiChord& e = i->second;
                        if (i->first != ctick)
                              break;
                        if (e.voice != voice)
                              continue;
                        notes.append(e);
                        }
                  if (notes.isEmpty())
                        break;
                  }
            processPendingNotes(notes, voice, ctick, lastTick);
            }

      KeyList* km = staff->keymap();
      if (!hasKey && !mtrack->drumTrack()) {
            KeySigEvent ks;
            ks.setAccidentalType(key);
            (*km)[0] = ks;
            }
      for (auto i = km->begin(); i != km->end(); ++i) {
            int tick = i->first;
            KeySigEvent key  = i->second;
            KeySig* ks = new KeySig(score);
            ks->setTrack(track);
            ks->setGenerated(false);
            ks->setKeySigEvent(key);
            ks->setMag(staff->mag());
            Measure* m = score->tick2measure(tick);
            Segment* seg = m->getSegment(ks, tick);
            seg->add(ks);
            }

#if 0  // TODO
      ClefList* cl = staff->clefList();
      for (ciClefEvent i = cl->begin(); i != cl->end(); ++i) {
            int tick = i.key();
            Clef* clef = new Clef(score);
            clef->setClefType(i.value());
            clef->setTrack(track);
            clef->setGenerated(false);
            clef->setMag(staff->mag());
            Measure* m = score->tick2measure(tick);
            Segment* seg = m->getSegment(clef, tick);
            seg->add(clef);
            }
#endif
      }

#if 0
      //---------------------------------------------------
      //  remove empty measures at beginning
      //---------------------------------------------------

      int startBar, endBar, beat, tick;
      score->sigmap()->tickValues(lastTick, &endBar, &beat, &tick);
      if (beat || tick)
            ++endBar;

      for (startBar = 0; startBar < endBar; ++startBar) {
            int tick1 = score->sigmap()->bar2tick(startBar, 0);
            int tick2 = score->sigmap()->bar2tick(startBar + 1, 0);
            int events = 0;
            foreach (MidiTrack* midiTrack, *tracks) {
                  if (midiTrack->staffIdx() == -1)
                        continue;
                  foreach(const Event ev, midiTrack->events()) {
                        int t = ev.ontime();
                        if (t >= tick2)
                              break;
                        if (t < tick1)
                              continue;
                        if (ev.type() == ME_NOTE) {
                              ++events;
                              break;
                              }
                        }
                  }
            if (events)
                  break;
            }
      tick = score->sigmap()->bar2tick(startBar, 0);
      if (tick)
            qDebug("remove empty measures %d ticks, startBar %d", tick, startBar);
      mf->move(-tick);
#endif

//---------------------------------------------------------
//   metaTimeSignature
//---------------------------------------------------------

Fraction metaTimeSignature(const MidiEvent& e)
      {
      const unsigned char* data = e.edata();
      int z  = data[0];
      int nn = data[1];
      int n  = 1;
      for (int i = 0; i < nn; ++i)
            n *= 2;
      return Fraction(z, n);
      }

void splitIntoLRHands_FixedPitch(QList<MTrack> &tracks, int trackIndex)
      {
      auto &srcTrack = tracks[trackIndex];
      auto operations = preferences.midiImportOperations.trackOperations(trackIndex);
      int splitPitch = 12 * (int)operations.LHRH.splitPitchOctave
                  + (int)operations.LHRH.splitPitchNote;
      if (srcTrack.minPitch < splitPitch) {
            MTrack dstTrack;
            dstTrack.mtrack = tracks[0].mtrack;
            for (auto i = srcTrack.chords.begin(); i != srcTrack.chords.end();) {
                  if (i->second.notes[0].pitch < splitPitch) {
                        dstTrack.chords.insert({i->first, i->second});
                        i = srcTrack.chords.erase(i);
                        continue;
                        }
                  ++i;
                  }
            if (!dstTrack.chords.empty())
                  tracks.push_back(dstTrack);
            }
      }

void splitIntoLRHands_HandWidth(QList<MTrack> &tracks, int &trackIndex)
      {
      auto &srcTrack = tracks[trackIndex];
      auto &operations = preferences.midiImportOperations;

      std::multimap<int, MidiChord> leftHandChords;
      std::multimap<int, MidiChord> rightHandChords;
      std::vector<std::multimap<int, MidiChord>::iterator> chordGroup;

      const int OCTAVE = 12;
      int currentTime = 0;

      // chords after MIDI import are sorted by onTime values
      for (auto i = srcTrack.chords.begin(); i != srcTrack.chords.end(); ++i) {
            // find chords with equal onTime values and put then into chordGroup
            if (chordGroup.empty())
                  currentTime = i->second.onTime;
            chordGroup.push_back(i);
            std::multimap<int, MidiChord>::iterator next = i;
            ++next;
            if ((next != srcTrack.chords.end() && next->second.onTime != currentTime)
                        || (next == srcTrack.chords.end())) {
                  // *i is the last element in group - process current group
                  struct {
                        bool operator()(const std::multimap<int, MidiChord>::iterator &iter1,
                                        const std::multimap<int, MidiChord>::iterator &iter2)
                              {
                              return iter1->second.notes[0].pitch < iter2->second.notes[0].pitch;
                              }
                        } lessThan;
                  std::sort(chordGroup.begin(), chordGroup.end(), lessThan);

                  int minPitch = chordGroup.front()->second.notes[0].pitch;
                  int maxPitch = chordGroup.back()->second.notes[0].pitch;
                  if (maxPitch - minPitch > OCTAVE) {
                        // need both hands
                        // assign all chords in range [minPitch .. minPitch + OCTAVE] to left hand
                        // and assign all other chords to right hand
                        for (const auto &chordIter: chordGroup) {
                              if (chordIter->second.notes[0].pitch <= minPitch + OCTAVE)
                                    leftHandChords.insert({chordIter->first, chordIter->second});
                              else
                                    rightHandChords.insert({chordIter->first, chordIter->second});
                              }
                        // maybe todo later: if range of right-hand chords > OCTAVE => assign all bottom right-hand
                        // chords to another, third track
                        }
                  else { // check - use two hands or one hand will be enough (right or left?)
                        // assign top chord for right hand, all the rest - to left hand
                        rightHandChords.insert({chordGroup.back()->first, chordGroup.back()->second});
                        for (auto p = chordGroup.begin(); p != chordGroup.end() - 1; ++p)
                              leftHandChords.insert({(*p)->first, (*p)->second});
                        }
                  // reset group for next iteration
                  chordGroup.clear();
                  }
            }
      if (!rightHandChords.empty())
            srcTrack.chords = rightHandChords;
      if (!leftHandChords.empty()) {
            MTrack leftHandTrack = srcTrack;
            leftHandTrack.chords = leftHandChords;
            tracks.insert(trackIndex + 1, leftHandTrack);
            // synchronize operations length and tracks list length
            operations.duplicateTrackOperations(trackIndex);
            ++trackIndex;
            }
      }

//---------------------------------------------------
//  heuristic to handle a one track piano:
//    split into left hand/right hand
//---------------------------------------------------

void splitIntoLeftRightHands(QList<MTrack> &tracks)
      {
      for (int i = 0; i < tracks.size(); ++i) {
            const auto &operations = preferences.midiImportOperations.trackOperations(i);
            if (!operations.LHRH.doIt)
                  continue;
            switch (operations.LHRH.method) {
                  case MidiOperation::LHRHMethod::HAND_WIDTH:
                        splitIntoLRHands_HandWidth(tracks, i);
                        break;
                  case MidiOperation::LHRHMethod::FIXED_PITCH:
                        splitIntoLRHands_FixedPitch(tracks, i);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   createMTrackList
//---------------------------------------------------------

void createMTrackList(int& lastTick, TimeSigMap* sigmap, QList<MTrack>& tracks, MidiFile* mf)
      {
      sigmap->clear();
      sigmap->add(0, Fraction(4, 4));   // default time signature

      int trackIndex = -1;
      foreach(MidiTrack* t, mf->tracks()) {
            t->mergeNoteOnOff();

            MTrack track;
            track.mtrack = t;
            int events = 0;

            //  - create time signature list from meta events
            //  - create MidiChord list
            //  - extract some information from track: program, min/max pitch

            for (auto i : t->events()) {
                  const MidiEvent& e = i.second;
                  //
                  // change division to MScore::division
                  //
                  int tick = (i.first * MScore::division + mf->division()/2) / mf->division();

                  //
                  // remove time signature events
                  //
                  if ((e.type() == ME_META) && (e.metaType() == META_TIME_SIGNATURE))
                        sigmap->add(tick, metaTimeSignature(e));
                  else if (e.type() == ME_NOTE) {
                        ++events;
                        int pitch = e.pitch();
                        int len = (e.len() * MScore::division + mf->division()/2) / mf->division();
                        track.maxPitch = qMax(pitch, track.maxPitch);
                        track.minPitch = qMin(pitch, track.minPitch);
                        track.medPitch += pitch;
                        lastTick = qMax(lastTick, tick + len);

                        MidiNote  n;
                        n.pitch    = pitch;
                        n.velo     = e.velo();
                        n.onTime   = tick;
                        n.len      = len;

                        MidiChord c;
                        c.onTime = tick;
                        c.duration = len;
                        c.notes.push_back(n);

                        track.chords.insert({tick, c});
                        }
                  else if (e.type() == ME_PROGRAM)
                        track.program = e.dataB();
                  lastTick = qMax(lastTick, tick);
                  }
            if (events != 0) {
                  auto trackOperations
                        = preferences.midiImportOperations.trackOperations(++trackIndex);
                  if (trackOperations.doImport) {
                        track.medPitch /= events;
                        tracks.push_back(track);
                        }
                  else
                        preferences.midiImportOperations.eraseTrackOperations(trackIndex--);
                  }
            }
      }

//---------------------------------------------------------
// createInstruments
//   for drum track, if any, set percussion clef
//   for piano 2 tracks, if any, set G and F clefs
//   for other track types set G or F clef
//---------------------------------------------------------

void createInstruments(Score* score, QList<MTrack>& tracks)
      {
      int ntracks = tracks.size();
      for (int idx = 0; idx < ntracks; ++idx) {
            MTrack& track = tracks[idx];
            Part* part   = new Part(score);
            Staff* s     = new Staff(score, part, 0);
            part->insertStaff(s);
            score->staves().push_back(s);
            track.staff = s;

            if (track.mtrack->drumTrack()) {
                  // drum track
                  s->setInitialClef(CLEF_PERC);
                  part->instr()->setDrumset(smDrumset);
                  }
            else {
                  if ((idx < (ntracks-1))
                              && (tracks.at(idx+1).mtrack->outChannel() == track.mtrack->outChannel())
                              && (track.program == 0)) {
                        // assume that the current track and the next track
                        // form a piano part
                        Staff* ss = new Staff(score, part, 1);
                        part->insertStaff(ss);
                        score->staves().push_back(ss);

                        s->setInitialClef(CLEF_G);
                        s->setBracket(0, BRACKET_BRACE);
                        s->setBracketSpan(0, 2);
                        ss->setInitialClef(CLEF_F);
                        ++idx;
                        tracks[idx].staff = ss;
                        }
                  else {
                        // other track type
                        ClefType ct = track.medPitch < 58 ? CLEF_F : CLEF_G;
                        s->setInitialClef(ct);
                        }
                  }
            score->appendPart(part);
            }
      }

void createMeasures(int& lastTick, Score* score)
      {
      int bars, beat, tick;
      score->sigmap()->tickValues(lastTick, &bars, &beat, &tick);
      if (beat > 0 || tick > 0)
            ++bars; // convert bar index to number of bars

      for (int i = 0; i < bars; ++i) {
            Measure* measure  = new Measure(score);
            int tick = score->sigmap()->bar2tick(i, 0);
            measure->setTick(tick);
            Fraction ts(score->sigmap()->timesig(tick).timesig());
            measure->setTimesig(ts);
            measure->setLen(ts);

            score->add(measure);
            }
      score->fixTicks();
      lastTick = score->lastMeasure()->endTick();
      }

QString instrumentName(int type, int program)
      {
      int hbank = -1, lbank = -1;
      if (program == -1)
            program = 0;
      else {
            hbank = (program >> 16);
            lbank = (program >> 8) & 0xff;
            program = program & 0xff;
            }
      return MidiInstrument::instrName(type, hbank, lbank, program);
      }

void setTrackInfo(MidiFile* mf, MTrack& mt)
      {
      if (mt.staff->isTop()) {
            Part* part  = mt.staff->part();
            if (mt.name.isEmpty()) {
                  QString name = instrumentName(mf->midiType(), mt.program);
                  if (!name.isEmpty())
                        part->setLongName(name);
                  }
            else
                  part->setLongName(mt.name);
            part->setPartName(part->longName().toPlainText());
            part->setMidiChannel(mt.mtrack->outChannel());
            part->setMidiProgram(mt.program & 0x7f);  // only GM
            }
      }

void createTimeSignatures(Score* score)
      {
      for (auto is = score->sigmap()->begin(); is != score->sigmap()->end(); ++is) {
            const SigEvent& se = is->second;
            int tick    = is->first;
            Measure* m  = score->tick2measure(tick);
            if (!m)
                  continue;
            for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                  TimeSig* ts = new TimeSig(score);
                  ts->setSig(se.timesig());
                  ts->setTrack(staffIdx * VOICES);
                  Segment* seg = m->getSegment(ts, tick);
                  seg->add(ts);
                  }
            }
      }

void createNotes(int lastTick, QList<MTrack>& tracks, MidiFile* mf)
      {
      for (int i = 0; i < tracks.size(); ++i) {
            MTrack& mt = tracks[i];

            for (auto ie : mt.mtrack->events()) {
                  const MidiEvent& e = ie.second;
                  if ((e.type() == ME_META) && (e.metaType() != META_LYRIC))
                        mt.processMeta(ie.first, e);
                  }

            setTrackInfo(mf, mt);
            mt.findChords();

            // pass current track index to the convertTrack function
            //   through MidiImportOperations
            preferences.midiImportOperations.setCurrentTrack(i);
            mt.convertTrack(lastTick);

            for (auto ie : mt.mtrack->events()) {
                  const MidiEvent& e = ie.second;
                  if ((e.type() == ME_META) && (e.metaType() == META_LYRIC))
                        mt.processMeta(ie.first, e);
                  }
            }
      }

void quantizeAndCleanup(int lastTick, TimeSigMap* sigmap, QList<MTrack>& tracks)
      {
      for (int i = 0; i < tracks.size(); ++i) {
            // pass current track index to the cleanup function
            //   through MidiImportOperations
            preferences.midiImportOperations.setCurrentTrack(i);
            tracks[i].cleanup(lastTick, sigmap);   // quantize
            }
      }

void convertMidi(Score* score, MidiFile* mf)
      {
      QList<MTrack> tracks;
      int lastTick = 0;
      auto sigmap = score->sigmap();

      mf->separateChannel();
      createMTrackList(lastTick, sigmap, tracks, mf);
      quantizeAndCleanup(lastTick, sigmap, tracks);
      splitIntoLeftRightHands(tracks);
      createInstruments(score, tracks);
      createMeasures(lastTick, score);
      createNotes(lastTick, tracks, mf);
      createTimeSignatures(score);
      score->connectTies();
      }

QList<TrackMeta> getTracksMeta(QList<MTrack>& tracks, MidiFile* mf)
{
      QList<TrackMeta> tracksMeta;
      for (int i = 0; i < tracks.size(); ++i) {
            MTrack& mt = tracks[i];
            MidiTrack* track = mt.mtrack;

            for (auto ie : track->events()) {
                  const MidiEvent& e = ie.second;
                  if ((e.type() == ME_META) && (e.metaType() == META_TRACK_NAME))
                        mt.name = (const char*)e.edata();
                  }
            MidiType midiType = mf->midiType();
            if (midiType == MT_UNKNOWN)
                  midiType = MT_GM;
            QString name = instrumentName(midiType, mt.program);
            tracksMeta.push_back({mt.name, name});
            }
      return tracksMeta;
      }

QList<TrackMeta> extractMidiTracksMeta(const QString& fileName)
      {
      if (fileName.isEmpty())
            return QList<TrackMeta>();
      QFile fp(fileName);
      if (!fp.open(QIODevice::ReadOnly))
            return QList<TrackMeta>();
      MidiFile mf;
      try {
            mf.read(&fp);
            }
      catch(...) {
            fp.close();
            return QList<TrackMeta>();
            }
      fp.close();

      Score mockScore;
      QList<MTrack> tracks;
      int lastTick = 0;

      mf.separateChannel();
      createMTrackList(lastTick, mockScore.sigmap(), tracks, &mf);
      quantizeAndCleanup(lastTick, mockScore.sigmap(), tracks);
      return getTracksMeta(tracks, &mf);
      }

//---------------------------------------------------------
//   importMidi
//    return true on success
//---------------------------------------------------------

Score::FileError importMidi(Score* score, const QString& name)
      {
      if (name.isEmpty())
            return Score::FILE_NOT_FOUND;
      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly)) {
            qDebug("importMidi: file open error <%s>", qPrintable(name));
            return Score::FILE_OPEN_ERROR;
            }
      MidiFile mf;
      try {
            mf.read(&fp);
            }
      catch(QString errorText) {
            if (!noGui) {
                  QMessageBox::warning(0,
                     QWidget::tr("MuseScore: load midi"),
                     QWidget::tr("Load failed: ") + errorText,
                     QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
                  }
            fp.close();
            qDebug("importMidi: bad file format");
            return Score::FILE_BAD_FORMAT;
            }
      fp.close();

      convertMidi(score, &mf);
      return Score::FILE_NO_ERROR;
      }
}

