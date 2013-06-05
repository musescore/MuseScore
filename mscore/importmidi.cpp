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
#include "importmidi_chord.h"
#include "importmidi_quant.h"


namespace Ms {

extern Preferences preferences;

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
      void processPendingNotes(QList<MidiChord>& notes, int voice, int ctick, int tick);
      void processMeta(int tick, const MidiEvent& mm);
      };


// remove overlapping notes with the same pitch

void removeOverlappingNotes(QList<MTrack> &tracks)
      {
      for (auto &track: tracks) {
            auto &chords = track.chords;
            for (auto i = chords.begin(); i != chords.end(); ++i) {
                  auto &firstChord = i->second;
                  for (auto &note1: firstChord.notes) {
                        auto ii = i;
                        ++ii;
                        bool overlapFound = false;
                        for (; ii != chords.end(); ++ii) {
                              auto &secondChord = ii->second;
                              for (auto &note2: secondChord.notes) {
                                    if (note2.pitch != note1.pitch)
                                          continue;
                                    if (note2.onTime >= (note1.onTime + note1.len))
                                          continue;
                                    qDebug("Midi import: overlapping events: %d+%d %d+%d",
                                           note1.onTime, note1.len, note2.onTime, note2.len);
                                    note1.len = note2.onTime - note1.onTime;
                                    overlapFound = true;
                                    break;
                                    }
                              if (overlapFound)
                                    break;
                              }
                        if (note1.len <= 0) {
                              qDebug("Midi import: duration <= 0: drop note at %d", note1.onTime);
                              continue;
                              }
                        }
                  } // for note1
            }
      }


// based on quickthresh algorithm
// here are default values for audio, in milliseconds
// for midi there will be another values, in ticks

// all notes received in the left inlet within this time period are collected into a chord
// threshTime = 40 ms

// if there are any incoming values within this amount of time
// at the end of the base thresh time,
// the threshold is extended to allow more notes to be added to the chord
// fudgeTime = 10 ms

// this is an extension value of the base thresh time, which is used if notes arrive
// in the object's inlet in the "fudge" time zone
// threshExtTime = 20 ms

void collectChords(QList<MTrack> &tracks, int minNoteDuration)
      {
      for (auto &track: tracks) {
            auto &chords = track.chords;
            if (chords.empty())
                  continue;

            Drumset* drumset = track.mtrack->drumTrack() ? smDrumset : 0;

            int threshTime = minNoteDuration / 2;
            int fudgeTime = threshTime / 4;
            int threshExtTime = threshTime / 2;

            int startTime = -1;    // invalid
            int curThreshTime = -1;
            bool useDrumset = false;

            // if intersection of note durations is less than min(minNoteDuration, threshTime)
            // then this is not a chord
            int tol = -1; // invalid
            int beg = -1;
            int end = -1;

            // chords here consist of a single note
            // because notes are not united into chords yet
            for (auto i = chords.begin(); i != chords.end(); ) {
                  const auto &note = i->second.notes[0];

                  if (note.onTime <= startTime + curThreshTime) {
                        if (!useDrumset || (drumset->isValid(note.pitch)
                                            && drumset->voice(note.pitch) == i->second.voice)) {
                              beg = std::max(beg, note.onTime);
                              end = std::min(end, note.onTime + note.len);
                              tol = std::min(tol, note.len);
                              if (end - beg >= tol) {
                                    auto prev = i;
                                    --prev;
                                    prev->second.notes.push_back(note);
                                    if (note.onTime >= startTime + curThreshTime - fudgeTime)
                                          curThreshTime += threshExtTime;
                                    i = chords.erase(i);
                                    continue;
                                    }
                              }
                        }
                  else {
                        useDrumset = false;
                        if (drumset) {
                              int pitch = note.pitch;
                              if (drumset->isValid(pitch)) {
                                    useDrumset = true;
                                    i->second.voice = drumset->voice(pitch);
                                    }
                              }
                        startTime = note.onTime;
                        beg = note.onTime;
                        end = note.onTime + note.len;
                        tol = threshTime;
                        if (curThreshTime != threshTime)
                              curThreshTime = threshTime;
                        }
                  ++i;
                  }
            }
      }

void sortNotesByPitch(std::multimap<int, MidiChord> &chords)
      {
      struct {
            bool operator()(const MidiNote &note1, const MidiNote &note2)
                  {
                  return note1.pitch < note2.pitch;
                  }
            } pitchSort;

      for (auto &chordEvent: chords) {
            // in each chord sort notes by pitches
            auto &notes = chordEvent.second.notes;
            qSort(notes.begin(), notes.end(), pitchSort);
            }
      }

void sortNotesByLength(std::multimap<int, MidiChord> &chords)
      {
      struct {
            bool operator()(const MidiNote &note1, const MidiNote &note2)
                  {
                  return note1.len < note2.len;
                  }
            } lenSort;

      for (auto &chordEvent: chords) {
            // in each chord sort notes by pitches
            auto &notes = chordEvent.second.notes;
            qSort(notes.begin(), notes.end(), lenSort);
            }
      }

// find notes of each chord that have different durations
// and separate them into different chords
// so all chords will have notes with equal lengths

void splitUnequalChords(QList<MTrack> &tracks)
      {
      for (auto &track: tracks) {
            std::vector<std::pair<int, MidiChord>> newChordEvents;
            auto &chords = track.chords;
            sortNotesByLength(chords);
            for (auto &chordEvent: chords) {
                  auto &chord = chordEvent.second;
                  auto &notes = chord.notes;
                  int len;
                  for (auto i = notes.begin(); i != notes.end(); ) {
                        if (i == notes.begin())
                              len = i->len;
                        else {
                              int newLen = i->len;
                              if (newLen != len) {
                                    auto newChord = chord;
                                    newChord.notes.clear();
                                    newChord.duration = len;
                                    for (int j = i - notes.begin(); j > 0; --j)
                                          newChord.notes.push_back(notes[j - 1]);
                                    newChordEvents.push_back({chord.onTime, newChord});
                                    i = notes.erase(notes.begin(), i);
                                    continue;
                                    }
                              }
                        ++i;
                        }
                  chord.duration = notes.first().len;
                  }
            for (const auto &event: newChordEvents)
                  chords.insert(event);
            }
      }


void quantizeAllTracks(QList<MTrack>& tracks, TimeSigMap* sigmap, int lastTick)
      {
      auto &opers = preferences.midiImportOperations;
      if (tracks.size() == 1 && opers.trackOperations(0).quantize.humanPerformance) {
            opers.setCurrentTrack(0);
            Quantize::applyAdaptiveQuant(tracks[0].chords, sigmap, lastTick);
            Quantize::applyGridQuant(tracks[0].chords, sigmap, lastTick);
            }
      else {
            for (int i = 0; i < tracks.size(); ++i) {
                  // pass current track index through MidiImportOperations
                  // for further usage
                  opers.setCurrentTrack(i);
                  Quantize::applyGridQuant(tracks[i].chords, sigmap, lastTick);
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

void MTrack::processPendingNotes(QList<MidiChord>& chords, int voice, int ctick, int t)
      {
      Score* score     = staff->score();
      int track        = staff->idx() * VOICES + voice;
      Drumset* drumset = staff->part()->instr()->drumset();
      bool useDrumset  = staff->part()->instr()->useDrumset();
      bool useDots = preferences.midiImportOperations.currentTrackOperations().useDots;

      while (!chords.isEmpty()) {
            int tick = chords[0].onTime;
            int len  = t - tick;
            if (len <= 0)
                  break;
            foreach (const MidiChord& c, chords) {
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
            for (int k = 0; k < chords.size(); ++k) {
                  MidiChord& n = chords[k];
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
                        chords.removeAt(k);
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

void insertNewLeftHandTrack(QList<MTrack> &tracks,
                 int &trackIndex,
                 const std::multimap<int, MidiChord> &leftHandChords)
      {
      auto leftHandTrack = tracks[trackIndex];
      leftHandTrack.chords = leftHandChords;
      tracks.insert(trackIndex + 1, leftHandTrack);
      // synchronize operations length and tracks list length
      preferences.midiImportOperations.duplicateTrackOperations(trackIndex);
      ++trackIndex;
      }

void addNewLeftHandChord(std::multimap<int, MidiChord> &leftHandChords,
                         const QList<MidiNote> &leftHandNotes,
                         const std::multimap<int, MidiChord>::iterator &i)
      {
      MidiChord leftHandChord = i->second;
      leftHandChord.notes = leftHandNotes;
      leftHandChords.insert({i->first, leftHandChord});
      }

void splitIntoLRHands_FixedPitch(QList<MTrack> &tracks, int &trackIndex)
      {
      auto &srcTrack = tracks[trackIndex];
      auto trackOpers = preferences.midiImportOperations.trackOperations(trackIndex);
      int splitPitch = 12 * (int)trackOpers.LHRH.splitPitchOctave
                  + (int)trackOpers.LHRH.splitPitchNote;
      std::multimap<int, MidiChord> leftHandChords;

      for (auto i = srcTrack.chords.begin(); i != srcTrack.chords.end(); ++i) {
            auto &notes = i->second.notes;
            QList<MidiNote> leftHandNotes;
            for (auto j = notes.begin(); j != notes.end(); ) {
                  auto &note = *j;
                  if (note.pitch < splitPitch) {
                        leftHandNotes.push_back(note);
                        j = notes.erase(j);
                        continue;
                        }
                  ++j;
                  }
            if (!leftHandNotes.empty())
                  addNewLeftHandChord(leftHandChords, leftHandNotes, i);
            }
      if (!leftHandChords.empty())
            insertNewLeftHandTrack(tracks, trackIndex, leftHandChords);
      }

void splitIntoLRHands_HandWidth(QList<MTrack> &tracks, int &trackIndex)
      {
      auto &srcTrack = tracks[trackIndex];
      sortNotesByPitch(srcTrack.chords);
      const int OCTAVE = 12;
      std::multimap<int, MidiChord> leftHandChords;

      // chords after MIDI import are sorted by onTime values
      for (auto i = srcTrack.chords.begin(); i != srcTrack.chords.end(); ++i) {
            auto &notes = i->second.notes;
            QList<MidiNote> leftHandNotes;
            int minPitch = notes.front().pitch;
            int maxPitch = notes.back().pitch;
            if (maxPitch - minPitch > OCTAVE) {
                  // need both hands
                  // assign all chords in range [minPitch .. minPitch + OCTAVE] to left hand
                  // and assign all other chords to right hand
                  for (auto j = notes.begin(); j != notes.end(); ) {
                        auto &note = *j;
                        if (note.pitch <= minPitch + OCTAVE) {
                              leftHandNotes.push_back(note);
                              j = notes.erase(j);
                              continue;
                              }
                        ++j;
                        // maybe todo later: if range of right-hand chords > OCTAVE => assign all bottom right-hand
                        // chords to another, third track
                        }
                  }
            else { // check - use two hands or one hand will be enough (right or left?)
                  // assign top chord for right hand, all the rest - to left hand
                  while (notes.size() > 1) {
                        leftHandNotes.push_back(notes.front());
                        notes.erase(notes.begin());
                        }
                  }
            if (!leftHandNotes.empty())
                  addNewLeftHandChord(leftHandChords, leftHandNotes, i);
            }
      if (!leftHandChords.empty())
            insertNewLeftHandTrack(tracks, trackIndex, leftHandChords);
      }

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

void convertMidi(Score* score, MidiFile* mf)
      {
      QList<MTrack> tracks;
      int lastTick = 0;
      auto sigmap = score->sigmap();

      mf->separateChannel();
      createMTrackList(lastTick, sigmap, tracks, mf);
      collectChords(tracks, MScore::division / 32); // tol = 1/128 note
      quantizeAllTracks(tracks, sigmap, lastTick);
      removeOverlappingNotes(tracks);
      splitIntoLeftRightHands(tracks);
      splitUnequalChords(tracks);

      createInstruments(score, tracks);
      createMeasures(lastTick, score);
      createNotes(lastTick, tracks, mf);
      createTimeSignatures(score);
      score->connectTies();
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

