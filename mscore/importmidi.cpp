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
#include "libmscore/tie.h"
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
#include "importmidi_tuplet.h"
#include "libmscore/tuplet.h"
#include "importmidi_swing.h"
#include "importmidi_fraction.h"


namespace Ms {

extern Preferences preferences;
extern void updateNoteLines(Segment*, int track);

const int CLEF_BORDER_PITCH = 60;

//---------------------------------------------------------
//   MTrack
//---------------------------------------------------------

class MTrack {
   public:
      int program = 0;
      Staff* staff = nullptr;
      const MidiTrack* mtrack = nullptr;
      QString name;
      bool hasKey = false;
      int indexOfOperation = 0;

      std::multimap<ReducedFraction, MidiChord> chords;
      std::multimap<ReducedFraction, MidiTuplet::TupletData> tuplets;   // <tupletOnTime, ...>

      void convertTrack(const ReducedFraction &lastTick);
      void processPendingNotes(QList<MidiChord>& midiChords,
                               int voice,
                               const ReducedFraction &startChordTickFrac,
                               const ReducedFraction &nextChordTick);
      void processMeta(int tick, const MidiEvent& mm);
      void fillGapWithRests(Score *score, int voice, const ReducedFraction &startChordTickFrac,
                            const ReducedFraction &restLength, int track);
      QList<std::pair<ReducedFraction, TDuration> >
            toDurationList(const Measure *measure, int voice, const ReducedFraction &startTick,
                           const ReducedFraction &len, Meter::DurationType durationType);
      void addElementToTuplet(int voice, const ReducedFraction &onTime,
                              const ReducedFraction &len, DurationElement *el);
      void createTuplets();
      void createKeys(int accidentalType);
      void createClefs();
      std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator
            findTuplet(int voice, const ReducedFraction &onTime, const ReducedFraction &len);
      };


// remove overlapping notes with the same pitch

void removeOverlappingNotes(std::multimap<int, MTrack> &tracks)
      {
      for (auto &track: tracks) {
            auto &chords = track.second.chords;
            for (auto it = chords.begin(); it != chords.end(); ++it) {
                  auto &firstChord = it->second;
                  const auto &firstOnTime = it->first;
                  for (auto &note1: firstChord.notes) {
                        auto ii = it;
                        ++ii;
                        bool overlapFound = false;
                        for (; ii != chords.end(); ++ii) {
                              auto &secondChord = ii->second;
                              const auto &secondOnTime = ii->first;
                              for (auto &note2: secondChord.notes) {
                                    if (note2.pitch != note1.pitch)
                                          continue;
                                    if (secondOnTime >= (firstOnTime + note1.len))
                                          continue;
                                    qDebug("Midi import: overlapping events: %d+%d %d+%d",
                                           firstOnTime.ticks(), note1.len.ticks(),
                                           secondOnTime.ticks(), note2.len.ticks());
                                    note1.len = secondOnTime - firstOnTime;
                                    overlapFound = true;
                                    break;
                                    }
                              if (overlapFound)
                                    break;
                              }
                        if (note1.len <= ReducedFraction(0)) {
                              qDebug("Midi import: duration <= 0: drop note at %d",
                                     firstOnTime.ticks());
                              continue;
                              }
                        }
                  } // for note1
            }
      }

ReducedFraction minAllowedDuration()
      {
      const static auto minDuration = ReducedFraction::fromTicks(MScore::division) / 32;
      return minDuration;
      }

// based on quickthresh algorithm
//
// http://www.cycling74.com/docs/max5/refpages/max-ref/quickthresh.html
// (link date 9 July 2013)
//
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

void collectChords(std::multimap<int, MTrack> &tracks)
      {
      for (auto &track: tracks) {
            auto &chords = track.second.chords;
            if (chords.empty())
                  continue;

            ReducedFraction threshTime = minAllowedDuration() / 2;
            ReducedFraction fudgeTime = threshTime / 4;
            ReducedFraction threshExtTime = threshTime / 2;

            ReducedFraction startTime(-1, 1);    // invalid
            ReducedFraction curThreshTime(-1, 1);
                        // if intersection of note durations is less than min(minNoteDuration, threshTime)
                        // then this is not a chord
            ReducedFraction tol(-1, 1);       // invalid
            ReducedFraction beg(-1, 1);
            ReducedFraction end(-1, 1);
                        // chords here consist of a single note
                        // because notes are not united into chords yet
            for (auto it = chords.begin(); it != chords.end(); ) {
                  const auto &note = it->second.notes[0];
                              // this should not be executed when it == chords.begin()
                  if (it->first <= startTime + curThreshTime) {
                        if (it->first > beg)
                              beg = it->first;
                        if (it->first + note.len < end)
                              end = it->first + note.len;
                        if (note.len < tol)
                              tol = note.len;
                        if (end - beg >= tol) {
                              // add current note to the previous chord
                              auto prev = it;
                              --prev;
                              prev->second.notes.push_back(note);
                              if (it->first >= startTime + curThreshTime - fudgeTime)
                                    curThreshTime += threshExtTime;
                              it = chords.erase(it);
                              continue;
                              }
                        }
                  else {
                        startTime = it->first;
                        beg = startTime;
                        end = startTime + note.len;
                        tol = threshTime;
                        if (curThreshTime != threshTime)
                              curThreshTime = threshTime;
                        }
                  ++it;
                  }
            }
      }

void sortNotesByPitch(std::multimap<ReducedFraction, MidiChord> &chords)
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

void sortNotesByLength(std::multimap<ReducedFraction, MidiChord> &chords)
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
// so all notes inside every chord will have equal lengths

void splitUnequalChords(std::multimap<int, MTrack> &tracks)
      {
      for (auto &track: tracks) {
            std::vector<std::pair<ReducedFraction, MidiChord>> newChordEvents;
            auto &chords = track.second.chords;
            sortNotesByLength(chords);
            for (auto &chordEvent: chords) {
                  auto &chord = chordEvent.second;
                  auto &notes = chord.notes;
                  ReducedFraction len;
                  for (auto it = notes.begin(); it != notes.end(); ) {
                        if (it == notes.begin())
                              len = it->len;
                        else {
                              ReducedFraction newLen = it->len;
                              if (newLen != len) {
                                    MidiChord newChord;
                                    newChord.voice = chord.voice;
                                    for (int j = it - notes.begin(); j > 0; --j)
                                          newChord.notes.push_back(notes[j - 1]);
                                    newChordEvents.push_back({chordEvent.first, newChord});
                                    it = notes.erase(notes.begin(), it);
                                    continue;
                                    }
                              }
                        ++it;
                        }
                  }
            for (const auto &event: newChordEvents)
                  chords.insert(event);
            }
      }

void removeEmptyTuplets(MTrack &track)
      {
      if (track.tuplets.empty())
            return;
      for (auto it = track.tuplets.begin(); it != track.tuplets.end(); ) {
            const auto &tupletData = it->second;
            bool containsChord = false;
            for (const auto &chord: track.chords) {
                  if (tupletData.voice != chord.second.voice)
                        continue;
                  const ReducedFraction &onTime = chord.first;
                  const ReducedFraction len = maxNoteLen(chord.second.notes);
                  if (onTime + len > tupletData.onTime
                              && onTime + len <= tupletData.onTime + tupletData.len) {
                                    // tuplet contains at least one chord
                        containsChord = true;
                        break;
                        }
                  }
            if (!containsChord) {
                  it = track.tuplets.erase(it);
                  continue;
                  }
            ++it;
            }
      }

void splitDrumVoices(std::multimap<int, MTrack> &tracks)
      {
      for (auto &trackItem: tracks) {
            MTrack &track = trackItem.second;
            std::vector<std::pair<ReducedFraction, MidiChord>> newChordEvents;
            auto &chords = track.chords;
            const Drumset* const drumset = track.mtrack->drumTrack() ? smDrumset : 0;
            if (!drumset)
                  continue;
                              // all chords of drum track have voice == 0
                              // because useMultipleVoices == false (see MidiImportOperations)
            for (auto chordIt = chords.begin(); chordIt != chords.end(); ) {
                  auto &chord = chordIt->second;
                  auto &notes = chord.notes;
                  MidiChord newChord;
                  for (auto it = notes.begin(); it != notes.end(); ) {
                        if (drumset->isValid(it->pitch) && drumset->voice(it->pitch) != 0) {
                              newChord.voice = drumset->voice(it->pitch);
                              newChord.notes.push_back(*it);

                              it = notes.erase(it);
                              continue;
                              }
                        ++it;
                        }
                  if (!newChord.notes.isEmpty()) {
                        newChordEvents.push_back({chordIt->first, newChord});

                        const auto tupletIt = track.findTuplet(chordIt->second.voice, chordIt->first,
                                                               maxNoteLen(newChord.notes));
                        const auto newTupletIt = track.findTuplet(newChord.voice, chordIt->first,
                                                                  maxNoteLen(newChord.notes));
                        if (tupletIt != track.tuplets.end()
                                    && newTupletIt == track.tuplets.end()) {
                              MidiTuplet::TupletData newTupletData = tupletIt->second;
                              newTupletData.voice = newChord.voice;
                              track.tuplets.insert({tupletIt->first, newTupletData});
                              }
                        if (notes.isEmpty()) {
                              removeEmptyTuplets(track);

                              chordIt = chords.erase(chordIt);
                              continue;
                              }
                        }
                  ++chordIt;
                  }
            for (const auto &event: newChordEvents)
                  chords.insert(event);
            }
      }

std::map<int, MTrack> splitDrumTrack(MTrack &drumTrack)
      {
      std::map<int, MTrack> newTracks;         // <percussion note pitch, track>
      if (drumTrack.chords.empty())
            return newTracks;

      while (!drumTrack.chords.empty()) {
            int pitch = -1;
            MTrack *curTrack = nullptr;
            for (auto it = drumTrack.chords.begin(); it != drumTrack.chords.end(); ) {
                  MidiChord &chord = it->second;
                  for (auto noteIt = chord.notes.begin(); noteIt != chord.notes.end(); ) {
                        if (pitch == -1) {
                              pitch = noteIt->pitch;
                              MTrack newTrack = drumTrack;
                              newTrack.chords.clear();
                              newTrack.tuplets.clear();
                              newTrack.name = smDrumset->name(pitch);
                              newTracks.insert({pitch, newTrack});
                              curTrack = &newTracks.find(pitch)->second;
                              }
                        if (noteIt->pitch == pitch) {
                              MidiChord newChord;
                              newChord.voice = chord.voice;
                              newChord.notes.push_back(*noteIt);
                              curTrack->chords.insert({it->first, newChord});

                              auto tupletIt = drumTrack.findTuplet(chord.voice, it->first,
                                                                   noteIt->len);
                              if (tupletIt != drumTrack.tuplets.end()) {
                                    auto newTupletIt = curTrack->findTuplet(newChord.voice, it->first,
                                                                            noteIt->len);
                                    if (newTupletIt == curTrack->tuplets.end()) {
                                          MidiTuplet::TupletData newTupletData = tupletIt->second;
                                          newTupletData.voice = newChord.voice;
                                          curTrack->tuplets.insert({tupletIt->first, newTupletData});
                                          }
                                    }
                              noteIt = chord.notes.erase(noteIt);
                              continue;
                              }
                        ++noteIt;
                        }
                  if (chord.notes.isEmpty()) {
                        it = drumTrack.chords.erase(it);
                        continue;
                        }
                  ++it;
                  }
            }

      return newTracks;
      }

void cleanUpMidiEvents(std::multimap<int, MTrack> &tracks)
      {
      auto &opers = preferences.midiImportOperations;

      for (auto &track: tracks) {
            MTrack &mtrack = track.second;
            opers.setCurrentTrack(mtrack.indexOfOperation);
            const auto raster = Quantize::fixedQuantRaster();
            const bool reduce = opers.currentTrackOperations().quantize.reduceToShorterNotesInBar;

            for (auto chordIt = mtrack.chords.begin(); chordIt != mtrack.chords.end(); ) {
                  MidiChord &ch = chordIt->second;
                  for (auto noteIt = ch.notes.begin(); noteIt != ch.notes.end(); ) {
                        if ((noteIt->len < minAllowedDuration())
                                    || (!reduce && noteIt->len < raster / 2)) {
                              noteIt = ch.notes.erase(noteIt);
                              continue;
                              }
                        ++noteIt;
                        }
                  if (ch.notes.isEmpty()) {
                        chordIt = mtrack.chords.erase(chordIt);
                        continue;
                        }
                  ++chordIt;
                  }
            }
      }

void quantizeAllTracks(std::multimap<int, MTrack> &tracks,
                       TimeSigMap *sigmap,
                       const ReducedFraction &lastTick)
      {
      auto &opers = preferences.midiImportOperations;
      if (opers.count() == 1 && opers.trackOperations(0).quantize.humanPerformance) {
            opers.setCurrentTrack(0);
            Quantize::applyAdaptiveQuant(tracks.begin()->second.chords, sigmap, lastTick);
            Quantize::applyGridQuant(tracks.begin()->second.chords, sigmap, lastTick);
            }
      else {
            for (auto &track: tracks) {
                  MTrack &mtrack = track.second;
                              // pass current track index through MidiImportOperations
                              // for further usage
                  opers.setCurrentTrack(mtrack.indexOfOperation);
                  if (mtrack.mtrack->drumTrack())
                        opers.adaptForPercussion(mtrack.indexOfOperation);
                  Quantize::quantizeChordsAndTuplets(mtrack.tuplets, mtrack.chords, sigmap, lastTick);
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
      const int staffIdx      = staff->idx();
      Score* const cs         = staff->score();

      switch (mm.metaType()) {
            case META_TEXT:
            case META_LYRIC: {
                  const QString s((char*)data);
                  cs->addLyrics(tick, staffIdx, s);
                  }
                  break;

            case META_TRACK_NAME:
                  if (name.isEmpty())
                        name = (const char*)data;
                  break;

            case META_TEMPO:
                  {
                  const unsigned tempo = data[2] + (data[1] << 8) + (data[0] <<16);
                  const double t = 1000000.0 / double(tempo);
                  cs->setTempo(tick, t);
                  // TODO: create TempoText
                  }
                  break;

            case META_KEY_SIGNATURE:
                  {
                  const int key = ((const char*)data)[0];
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

QList<std::pair<ReducedFraction, TDuration> >
MTrack::toDurationList(const Measure *measure,
                       int voice,
                       const ReducedFraction &startTick,
                       const ReducedFraction &len,
                       Meter::DurationType durationType)
      {
      const bool useDots = preferences.midiImportOperations.currentTrackOperations().useDots;
                  // find tuplets over which duration is go
      std::vector<MidiTuplet::TupletData> tupletData
                  = MidiTuplet::findTupletsForDuration(voice, ReducedFraction::fromTicks(measure->tick()),
                                                       startTick, len, tuplets);
      const ReducedFraction startTickInBar = startTick - ReducedFraction::fromTicks(measure->tick());
      const ReducedFraction endTickInBar = startTickInBar + len;
      return Meter::toDurationList(startTickInBar, endTickInBar,
                                   ReducedFraction(measure->timesig()), tupletData,
                                   durationType, useDots);
      }

ReducedFraction splitDurationOnBarBoundary(const ReducedFraction &len, const ReducedFraction &onTime,
                                           const Measure* measure)
      {
      const ReducedFraction barLimit = ReducedFraction::fromTicks(measure->tick() + measure->ticks());
      if (onTime + len > barLimit)
            return barLimit - onTime;
      return len;
      }

// fill the gap between successive chords with rests

void MTrack::fillGapWithRests(Score* score,
                              int voice,
                              const ReducedFraction &startChordTickFrac,
                              const ReducedFraction &restLength,
                              int track)
      {
      ReducedFraction startChordTick = startChordTickFrac;
      ReducedFraction restLen = restLength;
      while (restLen > ReducedFraction(0, 1)) {
            ReducedFraction len = restLen;
            Measure* const measure = score->tick2measure(startChordTick.ticks());
            if (startChordTick >= ReducedFraction::fromTicks(measure->tick() + measure->ticks())) {
                  qDebug("tick2measure: %d end of score?", startChordTick.ticks());
                  startChordTick += restLen;
                  restLen = ReducedFraction(0);
                  break;
                  }
            len = splitDurationOnBarBoundary(len, startChordTick, measure);

            if (len >= ReducedFraction::fromTicks(measure->ticks())) {
                              // rest to the whole measure
                  len = ReducedFraction::fromTicks(measure->ticks());
                  if (voice == 0) {
                        TDuration duration(TDuration::V_MEASURE);
                        Rest* rest = new Rest(score, duration);
                        rest->setDuration(measure->len());
                        rest->setTrack(track);
                        Segment* s = measure->getSegment(rest, startChordTick.ticks());
                        s->add(rest);
                        }
                  restLen -= len;
                  startChordTick += len;
                  }
            else {
                  const auto dl = toDurationList(measure, voice, startChordTick, len,
                                                 Meter::DurationType::REST);
                  if (dl.isEmpty()) {
                        qDebug("cannot create duration list for len %d", len.ticks());
                        restLen = ReducedFraction(0);      // fake
                        break;
                        }
                  for (const auto &durationPair: dl) {
                        const TDuration &duration = durationPair.second;
                        const ReducedFraction &tupletRatio = durationPair.first;
                        len = ReducedFraction(duration.fraction()) / tupletRatio;
                        Rest* const rest = new Rest(score, duration);
                        rest->setDuration(duration.fraction());
                        rest->setTrack(track);
                        Segment* const s = measure->getSegment(Segment::SegChordRest,
                                                               startChordTick.ticks());
                        s->add(rest);
                        addElementToTuplet(voice, startChordTick, len, rest);
                        restLen -= len;
                        startChordTick += len;
                        }
                  }

            }
      }

void setMusicNotesFromMidi(Score *score,
                           const QList<MidiNote> &midiNotes,
                           const ReducedFraction &onTime,
                           const ReducedFraction &len,
                           Chord *chord,
                           const ReducedFraction &tick,
                           const Drumset *drumset,
                           bool useDrumset)
      {
      auto actualFraction = ReducedFraction(chord->actualFraction());

      for (int i = 0; i < midiNotes.size(); ++i) {
            const MidiNote& mn = midiNotes[i];
            Note* const note = new Note(score);

            // TODO - does this need to be key-aware?
            note->setPitch(mn.pitch, pitch2tpc(mn.pitch, KEY_C, PREFER_NEAREST));
            chord->add(note);
            note->setVeloType(MScore::USER_VAL);
            note->setVeloOffset(mn.velo);

            NoteEventList el;
            ReducedFraction f = (onTime - tick) / actualFraction * 1000;
            const int ron = f.numerator() / f.denominator();
            f = len / actualFraction * 1000;
            const int rlen = f.numerator() / f.denominator();

            el.append(NoteEvent(0, ron, rlen));
            note->setPlayEvents(el);

            if (useDrumset) {
                  if (!drumset->isValid(mn.pitch))
                        qDebug("unmapped drum note 0x%02x %d", mn.pitch, mn.pitch);
                  else {
                        MScore::Direction sd = drumset->stemDirection(mn.pitch);
                        chord->setStemDirection(sd);
                        }
                  }

            if (midiNotes[i].tie) {
                  midiNotes[i].tie->setEndNote(note);
                  midiNotes[i].tie->setTrack(note->track());
                  note->setTieBack(midiNotes[i].tie);
                  }
            }
      }

ReducedFraction findMinDuration(const QList<MidiChord> &midiChords,
                                const ReducedFraction &length)
      {
      ReducedFraction len = length;
      for (const auto &chord: midiChords) {
            for (const auto &note: chord.notes) {
                  if ((note.len < len) && (note.len != ReducedFraction(0, 1)))
                        len = note.len;
                  }
            }
      return len;
      }

void setTies(Chord *chord, Score *score, QList<MidiNote> &midiNotes)
      {
      for (int i = 0; i < midiNotes.size(); ++i) {
            const MidiNote &midiNote = midiNotes[i];
            Note *const note = chord->findNote(midiNote.pitch);
            midiNotes[i].tie = new Tie(score);
            midiNotes[i].tie->setStartNote(note);
            note->setTieFor(midiNotes[i].tie);
            }
      }

std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator
MTrack::findTuplet(int voice, const ReducedFraction &onTime, const ReducedFraction &len)
      {
      if (tuplets.empty())
            return tuplets.end();

      auto it = tuplets.lower_bound(onTime);
      if (it == tuplets.end())
            it = tuplets.begin();
      if (it != tuplets.begin())
            --it;
      for ( ; it != tuplets.end(); ++it) {
            const auto &tupletData = it->second;
            if (tupletData.voice != voice)
                  continue;
            if (onTime >= tupletData.onTime
                        && onTime + len <= tupletData.onTime + tupletData.len) {
                  return it;
                  }
            }
      return tuplets.end();
      }

void MTrack::addElementToTuplet(int voice, const ReducedFraction &onTime,
                                const ReducedFraction &len, DurationElement *el)
      {
      const auto it = findTuplet(voice, onTime, len);
      if (it != tuplets.end())
             it->second.elements.push_back(el);       // add chord/rest to the tuplet
      }

// convert midiChords with the same onTime value to music notation
// and fill the remaining empty duration with rests

void MTrack::processPendingNotes(QList<MidiChord> &midiChords,
                                 int voice,
                                 const ReducedFraction &startChordTickFrac,
                                 const ReducedFraction &nextChordTick)
      {
      Score* const score     = staff->score();
      const int track        = staff->idx() * VOICES + voice;
      Drumset* const drumset = staff->part()->instr()->drumset();
      const bool useDrumset  = staff->part()->instr()->useDrumset();

                  // all midiChords here should have the same onTime value
                  // and all notes in each midiChord should have the same duration
      ReducedFraction startChordTick = startChordTickFrac;
      while (!midiChords.isEmpty()) {
            const ReducedFraction tick = startChordTick;
            ReducedFraction len = nextChordTick - tick;
            if (len <= ReducedFraction(0))
                  break;
            len = findMinDuration(midiChords, len);
            Measure* const measure = score->tick2measure(tick.ticks());
            len = splitDurationOnBarBoundary(len, tick, measure);

            const auto dl = toDurationList(measure, voice, tick, len, Meter::DurationType::NOTE);
            if (dl.isEmpty())
                  break;
            const TDuration &d = dl[0].second;
            const ReducedFraction &tupletRatio = dl[0].first;
            len = ReducedFraction(d.fraction()) / tupletRatio;

            Chord* const chord = new Chord(score);
            chord->setTrack(track);
            chord->setDurationType(d);
            chord->setDuration(d.fraction());
            Segment* const s = measure->getSegment(chord, tick.ticks());
            s->add(chord);
            chord->setUserPlayEvents(true);
            addElementToTuplet(voice, tick, len, chord);

            for (int k = 0; k < midiChords.size(); ++k) {
                  MidiChord& midiChord = midiChords[k];
                  setMusicNotesFromMidi(score, midiChord.notes, startChordTick,
                                        len, chord, tick, drumset, useDrumset);
                  if (!midiChord.notes.empty() && midiChord.notes.first().len <= len) {
                        midiChords.removeAt(k);
                        --k;
                        continue;
                        }
                  setTies(chord, score, midiChord.notes);
                  for (auto &midiNote: midiChord.notes)
                        midiNote.len -= len;
                  }
            startChordTick += len;
            }
      fillGapWithRests(score, voice, startChordTick,
                       nextChordTick - startChordTick, track);
      }

void MTrack::createTuplets()
      {
      Score* const score     = staff->score();
      const int track        = staff->idx() * VOICES;

      for (const auto &tupletEvent: tuplets) {
            const auto &tupletData = tupletEvent.second;
            if (tupletData.elements.empty())
                  continue;

            Tuplet* const tuplet = new Tuplet(score);
            const auto ratioIt = MidiTuplet::tupletRatios().find(tupletData.tupletNumber);
            const auto tupletRatio = (ratioIt != MidiTuplet::tupletRatios().end())
                                   ? ratioIt->second : ReducedFraction(2, 2);
            if (ratioIt == MidiTuplet::tupletRatios().end())
                  qDebug("Tuplet ratio not found for tuplet number: %d", tupletData.tupletNumber);
            tuplet->setRatio(tupletRatio.fraction());

            tuplet->setDuration(tupletData.len.fraction());
            const TDuration baseLen = tupletData.len.fraction() / tupletRatio.denominator();
            tuplet->setBaseLen(baseLen);

            tuplet->setTrack(track);
            tuplet->setTick(tupletData.onTime.ticks());
            Measure* const measure = score->tick2measure(tupletData.onTime.ticks());
            tuplet->setParent(measure);

            for (DurationElement *const el: tupletData.elements) {
                  tuplet->add(el);
                  el->setTuplet(tuplet);
                  }
            }
      }

void MTrack::createKeys(int accidentalType)
      {
      Score* const score     = staff->score();
      const int track        = staff->idx() * VOICES;

      KeyList* const km = staff->keymap();
      if (!hasKey && !mtrack->drumTrack()) {
            KeySigEvent ks;
            ks.setAccidentalType(accidentalType);
            (*km)[0] = ks;
            }
      for (auto it = km->begin(); it != km->end(); ++it) {
            const int tick = it->first;
            const KeySigEvent &key  = it->second;
            KeySig* const ks = new KeySig(score);
            ks->setTrack(track);
            ks->setGenerated(false);
            ks->setKeySigEvent(key);
            ks->setMag(staff->mag());
            Measure* const m = score->tick2measure(tick);
            Segment* const seg = m->getSegment(ks, tick);
            seg->add(ks);
            }
      }

ClefType clefTypeFromAveragePitch(int averagePitch)
      {
      return averagePitch < CLEF_BORDER_PITCH ? CLEF_F : CLEF_G;
      }

void createClef(ClefType clefType, Staff* staff, int tick, bool isSmall = false)
      {
      Clef* const clef = new Clef(staff->score());
      clef->setClefType(clefType);
      const int track = staff->idx() * VOICES;
      clef->setTrack(track);
      clef->setGenerated(false);
      clef->setMag(staff->mag());
      clef->setSmall(isSmall);
      Measure* const m = staff->score()->tick2measure(tick);
      Segment* const seg = m->getSegment(clef, tick);
      seg->add(clef);
      }

void createSmallClef(ClefType clefType, int tick, Staff *staff)
      {
      const bool isSmallClef = true;
      createClef(clefType, staff, tick, isSmallClef);
      }

void resetIfNotChanged(int &counter, int &oldCounter)
      {
      if (counter != 0 && counter == oldCounter) {
            counter = 0;
            oldCounter = 0;
            }
      }

bool isTiedBack(const std::multimap<ReducedFraction, MidiChord>::const_iterator &it,
                const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      if (it == chords.begin())
            return false;
      auto i = it;
      --i;
      for (;;) {
            if (i->first + maxNoteLen(i->second.notes) > it->first)
                  return true;
            if (i == chords.begin())
                  break;
            --i;
            }
      return false;
      }

void MTrack::createClefs()
      {
      ClefType currentClef = staff->initialClef()._concertClef;
      createClef(currentClef, staff, 0);

      const auto trackOpers = preferences.midiImportOperations.trackOperations(indexOfOperation);
      if (trackOpers.changeClef) {
            const int hightPitch = 62;          // all notes upper - in treble clef
            const int medPitch = 60;
            const int lowPitch = 57;           // all notes lower - in bass clef

            int oldTrebleCounter = 0;
            int trebleCounter = 0;
            int oldBassCounter = 0;
            int bassCounter = 0;

            const int counterLimit = 3;
                        // N^2 / 2 checks of tied chords in the worst case but fast enough in practice
            for (auto chordIt = chords.begin(); chordIt != chords.end(); ++chordIt) {
                  if (isTiedBack(chordIt, chords))
                        continue;
                  const int tick = chordIt->first.ticks();
                  const int avgPitch = findAveragePitch(chordIt->second.notes);
                  if (currentClef == CLEF_G && avgPitch < lowPitch) {
                        currentClef = CLEF_F;
                        createSmallClef(currentClef, tick, staff);
                        }
                  else if (currentClef == CLEF_F && avgPitch > hightPitch) {
                        currentClef = CLEF_G;
                        createSmallClef(currentClef, tick, staff);
                        }
                  else if (currentClef == CLEF_G && avgPitch >= lowPitch && avgPitch < medPitch) {
                        if (trebleCounter < counterLimit)
                              ++trebleCounter;
                        else {
                              currentClef = CLEF_F;
                              createSmallClef(currentClef, tick, staff);
                              }
                        }
                  else if (currentClef == CLEF_F && avgPitch <= hightPitch && avgPitch >= medPitch) {
                        if (bassCounter < counterLimit)
                              ++bassCounter;
                        else {
                              currentClef = CLEF_G;
                              createSmallClef(currentClef, tick, staff);
                              }
                        }
                  resetIfNotChanged(bassCounter, oldBassCounter);
                  resetIfNotChanged(trebleCounter, oldTrebleCounter);
                  }
            }
      }

void MTrack::convertTrack(const ReducedFraction &lastTick)
      {
      for (int voice = 0; voice < VOICES; ++voice) {
                        // startChordTick is onTime value of all simultaneous notes
                        // chords here are consist of notes with equal durations
                        // several chords may have the same onTime value
            ReducedFraction startChordTick;
            QList<MidiChord> midiChords;

            for (auto it = chords.begin(); it != chords.end();) {
                  const auto &nextChordTick = it->first;
                  const MidiChord& midiChord = it->second;
                  if (midiChord.voice != voice) {
                        ++it;
                        continue;
                        }
                  processPendingNotes(midiChords, voice, startChordTick, nextChordTick);
                              // now 'midiChords' list is empty
                              // so - fill it:
                              // collect all midiChords on current tick position
                  startChordTick = nextChordTick;       // debug
                  for ( ; it != chords.end(); ++it) {
                        const MidiChord& midiChord = it->second;
                        if (it->first != startChordTick)
                              break;
                        if (midiChord.voice != voice)
                              continue;
                        midiChords.append(midiChord);
                        }
                  if (midiChords.isEmpty())
                        break;
                  }
                        // process last chords at the end of the score
            processPendingNotes(midiChords, voice, startChordTick, lastTick);
            }

      const int key = 0;                // TODO-LIB findKey(mtrack, score->sigmap());

      createTuplets();
      createKeys(key);
      createClefs();

      const auto swingType = preferences.midiImportOperations.trackOperations(indexOfOperation).swing;
      Swing::detectSwing(staff, swingType);
      }

Fraction metaTimeSignature(const MidiEvent& e)
      {
      const unsigned char* data = e.edata();
      const int z  = data[0];
      const int nn = data[1];
      int n  = 1;
      for (int i = 0; i < nn; ++i)
            n *= 2;
      return Fraction(z, n);
      }

void insertNewLeftHandTrack(std::multimap<int, MTrack> &tracks,
                            std::multimap<int, MTrack>::iterator &it,
                            const std::multimap<ReducedFraction, MidiChord> &leftHandChords)
      {
      auto leftHandTrack = it->second;
      leftHandTrack.chords = leftHandChords;
      removeEmptyTuplets(leftHandTrack);
      it = tracks.insert({it->first, leftHandTrack});
      }

void addNewLeftHandChord(std::multimap<ReducedFraction, MidiChord> &leftHandChords,
                         const QList<MidiNote> &leftHandNotes,
                         const std::multimap<ReducedFraction, MidiChord>::iterator &it)
      {
      MidiChord leftHandChord = it->second;
      leftHandChord.notes = leftHandNotes;
      leftHandChords.insert({it->first, leftHandChord});
      }

void splitIntoLRHands_FixedPitch(std::multimap<int, MTrack> &tracks,
                                 std::multimap<int, MTrack>::iterator &it)
      {
      auto &srcTrack = it->second;
      const auto trackOpers = preferences.midiImportOperations.trackOperations(srcTrack.indexOfOperation);
      const int splitPitch = 12 * (int)trackOpers.LHRH.splitPitchOctave
                           + (int)trackOpers.LHRH.splitPitchNote;
      std::multimap<ReducedFraction, MidiChord> leftHandChords;

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
            insertNewLeftHandTrack(tracks, it, leftHandChords);
      }

void splitIntoLRHands_HandWidth(std::multimap<int, MTrack> &tracks,
                                std::multimap<int, MTrack>::iterator &it)
      {
      auto &srcTrack = it->second;
      sortNotesByPitch(srcTrack.chords);
      const int octave = 12;
      std::multimap<ReducedFraction, MidiChord> leftHandChords;
                  // chords after MIDI import are sorted by onTime values
      for (auto i = srcTrack.chords.begin(); i != srcTrack.chords.end(); ++i) {
            auto &notes = i->second.notes;
            QList<MidiNote> leftHandNotes;
            const int minPitch = notes.front().pitch;
            const int maxPitch = notes.back().pitch;
            if (maxPitch - minPitch > octave) {
                              // need both hands
                              // assign all chords in range [minPitch .. minPitch + OCTAVE]
                              // to left hand and all other chords - to right hand
                  for (auto j = notes.begin(); j != notes.end(); ) {
                        const auto &note = *j;
                        if (note.pitch <= minPitch + octave) {
                              leftHandNotes.push_back(note);
                              j = notes.erase(j);
                              continue;
                              }
                        ++j;
                        // maybe todo later: if range of right-hand chords > OCTAVE
                        // => assign all bottom right-hand chords to another, third track
                        }
                  }
            else {            // check - use two hands or one hand will be enough (right or left?)
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
            insertNewLeftHandTrack(tracks, it, leftHandChords);
      }

void splitIntoLeftRightHands(std::multimap<int, MTrack> &tracks)
      {
      for (auto it = tracks.begin(); it != tracks.end(); ++it) {
            if (it->second.mtrack->drumTrack())
                  continue;
            const auto operations = preferences.midiImportOperations.trackOperations(
                                                              it->second.indexOfOperation);
            if (!operations.LHRH.doIt)
                  continue;
                        // iterator 'it' will change after track split to ++it
                        // C++11 guarantees that newely inserted item with equal key will go after:
                        //    "The relative ordering of elements with equivalent keys is preserved,
                        //     and newly inserted elements follow those with equivalent keys
                        //     already in the container"
            switch (operations.LHRH.method) {
                  case MidiOperation::LHRHMethod::HAND_WIDTH:
                        splitIntoLRHands_HandWidth(tracks, it);
                        break;
                  case MidiOperation::LHRHMethod::SPECIFIED_PITCH:
                        splitIntoLRHands_FixedPitch(tracks, it);
                        break;
                  }
            }
      }

QList<MTrack> prepareTrackList(const std::multimap<int, MTrack> &tracks)
      {
      QList<MTrack> trackList;
      for (const auto &track: tracks) {
            trackList.push_back(track.second);
            }
      return trackList;
      }

std::multimap<int, MTrack> createMTrackList(ReducedFraction &lastTick,
                                            TimeSigMap *sigmap,
                                            const MidiFile *mf)
      {
      sigmap->clear();
      sigmap->add(0, Fraction(4, 4));   // default time signature

      std::multimap<int, MTrack> tracks;   // <track index, track>
      int trackIndex = -1;
      for (const auto &t: mf->tracks()) {
            MTrack track;
            track.mtrack = &t;
            int events = 0;
                        //  - create time signature list from meta events
                        //  - create MidiChord list
                        //  - extract some information from track: program, min/max pitch
            for (auto i : t.events()) {
                  const MidiEvent& e = i.second;
                              // change division to MScore::division
                  const auto tick = ReducedFraction::fromTicks(
                                   (i.first * MScore::division + mf->division() / 2) / mf->division());
                              // remove time signature events
                  if ((e.type() == ME_META) && (e.metaType() == META_TIME_SIGNATURE))
                        sigmap->add(tick.ticks(), metaTimeSignature(e));
                  else if (e.type() == ME_NOTE) {
                        ++events;
                        const int pitch = e.pitch();
                        const auto len = ReducedFraction::fromTicks(
                                    (e.len() * MScore::division + mf->division() / 2) / mf->division());
                        if (tick + len > lastTick)
                              lastTick = tick + len;

                        MidiNote  n;
                        n.pitch    = pitch;
                        n.velo     = e.velo();
                        n.len      = len;

                        MidiChord c;
                        c.notes.push_back(n);

                        track.chords.insert({tick, c});
                        }
                  else if (e.type() == ME_PROGRAM)
                        track.program = e.dataB();
                  if (tick > lastTick)
                        lastTick = tick;
                  }
            if (events != 0) {
                  ++trackIndex;
                  if (preferences.midiImportOperations.count()) {
                        auto trackOperations
                                    = preferences.midiImportOperations.trackOperations(trackIndex);
                        if (trackOperations.doImport) {
                              track.indexOfOperation = trackIndex;
                              tracks.insert({trackOperations.reorderedIndex, track});
                              }
                        }
                  else {            // if it is an initial track-list query from MIDI import panel
                        track.indexOfOperation = trackIndex;
                        tracks.insert({trackIndex, track});
                        }
                  }
            }

      return tracks;
      }

Measure* barFromIndex(const Score *score, int barIndex)
      {
      const int tick = score->sigmap()->bar2tick(barIndex, 0);
      return score->tick2measure(tick);
      }

int findAveragePitch(const std::map<ReducedFraction, MidiChord>::const_iterator &startChordIt,
                     const std::map<ReducedFraction, MidiChord>::const_iterator &endChordIt)
      {
      int avgPitch = 0;
      int counter = 0;
      for (auto it = startChordIt; it != endChordIt; ++it) {
            avgPitch += findAveragePitch(it->second.notes);
            ++counter;
            }
      if (counter)
            avgPitch /= counter;
      if (avgPitch == 0)
            avgPitch = CLEF_BORDER_PITCH;
      return avgPitch;
      }

void setBracket(Staff *&staff, int &counter)
      {
      if (staff && counter > 1) {
            staff->setBracket(0, BRACKET_NORMAL);
            staff->setBracketSpan(0, counter);
            }
      if (counter)
            counter = 0;
      if (staff)
            staff = nullptr;
      }

void setStaffBracketForDrums(QList<MTrack> &tracks)
      {
      int counter = 0;
      Staff *firstDrumStaff = nullptr;
      int opIndex = -1;
      const auto &opers = preferences.midiImportOperations;

      for (const MTrack &track: tracks) {
            if (track.mtrack->drumTrack()) {
                  if (opIndex != track.indexOfOperation) {
                        setBracket(firstDrumStaff, counter);
                        if (opers.trackOperations(track.indexOfOperation).drums.showStaffBracket) {
                              opIndex = track.indexOfOperation;
                              firstDrumStaff = track.staff;
                              }
                        }
                  ++counter;
                  }
            else
                  setBracket(firstDrumStaff, counter);
            }
      setBracket(firstDrumStaff, counter);
      }

//---------------------------------------------------------
// createInstruments
//   for drum track, if any, set percussion clef
//   for piano 2 tracks, if any, set G and F clefs
//   for other track types set G or F clef
//---------------------------------------------------------

void createInstruments(Score *score, QList<MTrack> &tracks)
      {
      const int ntracks = tracks.size();
      for (int idx = 0; idx < ntracks; ++idx) {
            MTrack& track = tracks[idx];
            Part* const part   = new Part(score);
            Staff* const s     = new Staff(score, part, 0);
            part->insertStaff(s);
            score->staves().push_back(s);
            track.staff = s;

            if (track.mtrack->drumTrack()) {
                              // drum track
                  s->setInitialClef(CLEF_PERC);
                  part->instr()->setDrumset(smDrumset);
                  part->instr()->setUseDrumset(true);
                  }
            else {
                  int avgPitch = findAveragePitch(track.chords.begin(), track.chords.end());
                  s->setInitialClef(clefTypeFromAveragePitch(avgPitch));
                  if ((idx < (ntracks-1))
                              && (tracks.at(idx+1).mtrack->outChannel() == track.mtrack->outChannel())
                              && (track.program == 0)) {
                                    // assume that the current track and the next track
                                    // form a piano part
                        s->setBracket(0, BRACKET_BRACE);
                        s->setBracketSpan(0, 2);

                        Staff* const ss = new Staff(score, part, 1);
                        part->insertStaff(ss);
                        score->staves().push_back(ss);
                        ++idx;
                        avgPitch = findAveragePitch(tracks[idx].chords.begin(), tracks[idx].chords.end());
                        ss->setInitialClef(clefTypeFromAveragePitch(avgPitch));
                        tracks[idx].staff = ss;
                        }
                  }
            score->appendPart(part);
            }
      }

void createMeasures(ReducedFraction &lastTick, Score *score)
      {
      int bars, beat, tick;
      score->sigmap()->tickValues(lastTick.ticks(), &bars, &beat, &tick);
      if (beat > 0 || tick > 0)
            ++bars;           // convert bar index to number of bars

      const bool pickupMeasure = preferences.midiImportOperations.currentTrackOperations().pickupMeasure;

      for (int i = 0; i < bars; ++i) {
            Measure* const measure  = new Measure(score);
            const int tick = score->sigmap()->bar2tick(i, 0);
            measure->setTick(tick);
            const Fraction ts = score->sigmap()->timesig(tick).timesig();
            Fraction nominalTs = ts;

            if (pickupMeasure && i == 0 && bars > 1) {
                  const int secondBarIndex = 1;
                  const int secondBarTick = score->sigmap()->bar2tick(secondBarIndex, 0);
                  Fraction secondTs(score->sigmap()->timesig(secondBarTick).timesig());
                  if (ts < secondTs) {          // the first measure is a pickup measure
                        nominalTs = secondTs;
                        measure->setIrregular(true);
                        }
                  }
            measure->setTimesig(nominalTs);
            measure->setLen(ts);
            score->add(measure);
            }
      const Measure *const m = score->lastMeasure();
      if (m) {
            score->fixTicks();
            lastTick = ReducedFraction::fromTicks(m->endTick());
            }
      }

QString instrumentName(int type, int program, bool isDrumTrack)
      {
      if (isDrumTrack)
            return "Percussion";

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

void setTrackInfo(MidiType midiType, MTrack &mt)
      {
      if (mt.staff->isTop()) {
            Part *const part  = mt.staff->part();
            if (mt.name.isEmpty()) {
                  QString name = instrumentName(midiType, mt.program, mt.mtrack->drumTrack());
                  if (!name.isEmpty())
                        part->setLongName(name);
                  }
            else
                  part->setLongName(mt.name);
            part->setPartName(part->longName().toPlainText());
            part->setMidiChannel(mt.mtrack->outChannel());
            int bank = 0;
            if (mt.mtrack->drumTrack())
                  bank = 128;
            part->setMidiProgram(mt.program & 0x7f, bank);  // only GM
            }
      }

void createTimeSignatures(Score *score)
      {
      for (auto is = score->sigmap()->begin(); is != score->sigmap()->end(); ++is) {
            const SigEvent& se = is->second;
            const int tick = is->first;
            Measure* m = score->tick2measure(tick);
            if (!m)
                  continue;
            Fraction newTimeSig = se.timesig();

            const bool pickupMeasure = preferences.midiImportOperations.currentTrackOperations().pickupMeasure;
            if (pickupMeasure && is == score->sigmap()->begin()) {
                  auto next = is;
                  ++next;
                  if (next != score->sigmap()->end()) {
                        Measure* mm = score->tick2measure(next->first);
                        if (m && mm && m == barFromIndex(score, 0) && mm == barFromIndex(score, 1)
                                    && m->timesig() == mm->timesig() && newTimeSig != mm->timesig())
                              {
                                          // it's a pickup measure - change timesig to nominal value
                                    newTimeSig = mm->timesig();
                              }
                        }
                  }
            for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                  TimeSig* const ts = new TimeSig(score);
                  ts->setSig(newTimeSig);
                  ts->setTrack(staffIdx * VOICES);
                  Segment* const seg = m->getSegment(ts, tick);
                  seg->add(ts);
                  }
            if (newTimeSig != se.timesig())   // was a pickup measure - skip next timesig
                  ++is;
            }
      }

void createNotes(const ReducedFraction &lastTick, QList<MTrack> &tracks, MidiType midiType)
      {
      for (int i = 0; i < tracks.size(); ++i) {
            MTrack &mt = tracks[i];

            for (auto ie : mt.mtrack->events()) {
                  const MidiEvent &e = ie.second;
                  if ((e.type() == ME_META) && (e.metaType() != META_LYRIC))
                        mt.processMeta(ie.first, e);
                  }
            if (midiType == MT_UNKNOWN)
                  midiType = MT_GM;
            setTrackInfo(midiType, mt);
                        // pass current track index to the convertTrack function
                        //   through MidiImportOperations
            preferences.midiImportOperations.setCurrentTrack(mt.indexOfOperation);
            mt.convertTrack(lastTick);

            for (auto ie : mt.mtrack->events()) {
                  const MidiEvent &e = ie.second;
                  if ((e.type() == ME_META) && (e.metaType() == META_LYRIC))
                        mt.processMeta(ie.first, e);
                  }
            }
      }

QList<TrackMeta> getTracksMeta(const std::multimap<int, MTrack> &tracks,
                               const MidiFile *mf)
{
      QList<TrackMeta> tracksMeta;
      for (const auto &track: tracks) {
            const MTrack &mt = track.second;
            const MidiTrack *const midiTrack = mt.mtrack;
            QString trackName;
            for (const auto &ie: midiTrack->events()) {
                  const MidiEvent &e = ie.second;
                  if ((e.type() == ME_META) && (e.metaType() == META_TRACK_NAME))
                        trackName = (const char*)e.edata();
                  }
            MidiType midiType = mf->midiType();
            if (midiType == MT_UNKNOWN)
                  midiType = MT_GM;
            const QString instrName = instrumentName(midiType, mt.program, mt.mtrack->drumTrack());
            const bool isDrumTrack = midiTrack->drumTrack();
            tracksMeta.push_back({trackName, instrName, isDrumTrack});
            }
      return tracksMeta;
      }

void splitDrumTracks(std::multimap<int, MTrack> &tracks)
      {
      for (auto it = tracks.begin(); it != tracks.end(); ++it) {
            if (!it->second.mtrack->drumTrack())
                  continue;
            const auto operations = preferences.midiImportOperations.trackOperations(
                                                            it->second.indexOfOperation);
            if (!operations.drums.doSplit)
                  continue;
            const auto newTracks = splitDrumTrack(it->second);
            const int trackIndex = it->first;
            it = tracks.erase(it);
            for (auto i = newTracks.rbegin(); i != newTracks.rend(); ++i)
                  it = tracks.insert({trackIndex, i->second});
            }
      }

void convertMidi(Score *score, const MidiFile *mf)
      {
      ReducedFraction lastTick;
      auto *const sigmap = score->sigmap();

      auto tracks = createMTrackList(lastTick, sigmap, mf);
      collectChords(tracks);
      cleanUpMidiEvents(tracks);
      quantizeAllTracks(tracks, sigmap, lastTick);
      removeOverlappingNotes(tracks);
      splitIntoLeftRightHands(tracks);
      splitUnequalChords(tracks);
      splitDrumVoices(tracks);
      splitDrumTracks(tracks);
                  // no more track insertion/reordering/deletion from now
      QList<MTrack> trackList = prepareTrackList(tracks);
      createInstruments(score, trackList);
      setStaffBracketForDrums(trackList);
      createMeasures(lastTick, score);
      createNotes(lastTick, trackList, mf->midiType());
      createTimeSignatures(score);
      score->connectTies();
      }

void loadMidiData(MidiFile &mf)
      {
      mf.separateChannel();
      MidiType mt = MT_UNKNOWN;
      for (auto &track: mf.tracks())
            track.mergeNoteOnOffAndFindMidiType(&mt);
      mf.setMidiType(mt);
      }

QList<TrackMeta> extractMidiTracksMeta(const QString &fileName)
      {
      if (fileName.isEmpty())
            return QList<TrackMeta>();

      auto &midiData = preferences.midiImportOperations.midiData();
      if (!midiData.midiFile(fileName)) {
            QFile fp(fileName);
            if (!fp.open(QIODevice::ReadOnly))
                  return QList<TrackMeta>();
            MidiFile mf;
            try {
                  mf.read(&fp);
            }
            catch (...) {
                  fp.close();
                  return QList<TrackMeta>();
            }
            fp.close();

            loadMidiData(mf);
            midiData.setMidiFile(fileName, mf);
            }

      Score mockScore;
      ReducedFraction lastTick;
      const auto tracks = createMTrackList(lastTick, mockScore.sigmap(),
                                           midiData.midiFile(fileName));
      return getTracksMeta(tracks, midiData.midiFile(fileName));
      }

//---------------------------------------------------------
//   importMidi
//---------------------------------------------------------

Score::FileError importMidi(Score *score, const QString &name)
      {
      if (name.isEmpty())
            return Score::FILE_NOT_FOUND;

      auto &midiData = preferences.midiImportOperations.midiData();
      if (!midiData.midiFile(name)) {
            QFile fp(name);
            if (!fp.open(QIODevice::ReadOnly)) {
                  qDebug("importMidi: file open error <%s>", qPrintable(name));
                  return Score::FILE_OPEN_ERROR;
                  }
            MidiFile mf;
            try {
                  mf.read(&fp);
                  }
            catch (QString errorText) {
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

            loadMidiData(mf);
            midiData.setMidiFile(name, mf);
            }

      convertMidi(score, midiData.midiFile(name));

      return Score::FILE_NO_ERROR;
      }
}

