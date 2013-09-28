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
#include "preferences.h"
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
#include "importmidi_meter.h"
#include "importmidi_chord.h"
#include "importmidi_quant.h"
#include "importmidi_tuplet.h"
#include "libmscore/tuplet.h"
#include "importmidi_swing.h"
#include "importmidi_fraction.h"
#include "importmidi_drum.h"
#include "importmidi_inner.h"
#include "importmidi_clef.h"
#include "importmidi_lrhand.h"
#include "importmidi_lyrics.h"

#include <set>


namespace Ms {

extern Preferences preferences;
extern void updateNoteLines(Segment*, int track);


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
                        if ((noteIt->len < MChord::minAllowedDuration())
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
      for (auto &track: tracks) {
            MTrack &mtrack = track.second;
                        // pass current track index through MidiImportOperations
                        // for further usage
            opers.setCurrentTrack(mtrack.indexOfOperation);
            if (mtrack.mtrack->drumTrack())
                  opers.adaptForPercussion(mtrack.indexOfOperation);
            mtrack.tuplets = MidiTuplet::findAllTuplets(mtrack.chords, sigmap, lastTick);
            Quantize::quantizeChords(mtrack.chords, mtrack.tuplets, sigmap);
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
      Score* cs         = staff->score();

      switch (mm.metaType()) {
            case META_TEXT:
            case META_LYRIC: {
                  std::string text = MidiCharset::fromUchar(data);
                  cs->addLyrics(tick, staffIdx, MidiCharset::convertToCharset(text));
                  }
                  break;
            case META_TRACK_NAME:
                  {
                  std::string text = MidiCharset::fromUchar(data);
                  if (name.isEmpty())
                        name = MidiCharset::convertToCharset(text);
                  }
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
      auto barTick = ReducedFraction::fromTicks(measure->tick());
      auto tupletsData = MidiTuplet::findTupletsInBarForDuration(voice, barTick, startTick,
                                                                len, tuplets);
      struct {
            bool operator()(const MidiTuplet::TupletData &d1,
                            const MidiTuplet::TupletData &d2)
                  {
                  return (d1.len > d2.len);
                  }
            } comparator;
                  // sort by tuplet length in desc order
      sort(tupletsData.begin(), tupletsData.end(), comparator);

      const ReducedFraction startTickInBar = startTick - barTick;
      const ReducedFraction endTickInBar = startTickInBar + len;
      return Meter::toDurationList(startTickInBar, endTickInBar,
                                   ReducedFraction(measure->timesig()), tupletsData,
                                   durationType, useDots);
      }

ReducedFraction splitDurationOnBarBoundary(const ReducedFraction &len,
                                           const ReducedFraction &onTime,
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
            Measure* measure = score->tick2measure(startChordTick.ticks());
            if (startChordTick >= ReducedFraction::fromTicks(measure->tick() + measure->ticks())) {
                  qDebug("tick2measure: %d end of score?", startChordTick.ticks());
                  startChordTick += restLen;
                  restLen = ReducedFraction(0, 1);
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
                        restLen = ReducedFraction(0, 1);      // fake
                        break;
                        }
                  for (const auto &durationPair: dl) {
                        const TDuration &duration = durationPair.second;
                        const ReducedFraction &tupletRatio = durationPair.first;
                        len = ReducedFraction(duration.fraction()) / tupletRatio;
                        Rest* rest = new Rest(score, duration);
                        rest->setDuration(duration.fraction());
                        rest->setTrack(track);
                        Segment* s = measure->getSegment(Segment::SegChordRest,
                                                         startChordTick.ticks());
                        s->add(rest);
                        MidiTuplet::addElementToTuplet(voice, startChordTick, len, rest, tuplets);
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
            Note* note = new Note(score);

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

void setTies(Chord *chord,
             Score *score,
             QList<MidiNote> &midiNotes)
      {
      for (int i = 0; i < midiNotes.size(); ++i) {
            const MidiNote &midiNote = midiNotes[i];
            Note *note = chord->findNote(midiNote.pitch);
            midiNotes[i].tie = new Tie(score);
            midiNotes[i].tie->setStartNote(note);
            note->setTieFor(midiNotes[i].tie);
            }
      }


// convert midiChords with the same onTime value to music notation
// and fill the remaining empty duration with rests

void MTrack::processPendingNotes(QList<MidiChord> &midiChords,
                                 int voice,
                                 const ReducedFraction &startChordTickFrac,
                                 const ReducedFraction &nextChordTick)
      {
      Score* score     = staff->score();
      const int track        = staff->idx() * VOICES + voice;
      Drumset* drumset = staff->part()->instr()->drumset();
      const bool useDrumset  = staff->part()->instr()->useDrumset();

                  // all midiChords here should have the same onTime value
                  // and all notes in each midiChord should have the same duration
      ReducedFraction startChordTick = startChordTickFrac;
      while (!midiChords.isEmpty()) {
            const ReducedFraction tick = startChordTick;
            ReducedFraction len = nextChordTick - tick;
            if (len <= ReducedFraction(0, 1))
                  break;
            len = MChord::findMinDuration(midiChords, len);
            Measure* measure = score->tick2measure(tick.ticks());
            len = splitDurationOnBarBoundary(len, tick, measure);

            const auto dl = toDurationList(measure, voice, tick, len, Meter::DurationType::NOTE);
            if (dl.isEmpty())
                  break;
            const TDuration &d = dl[0].second;
            const ReducedFraction &tupletRatio = dl[0].first;
            len = ReducedFraction(d.fraction()) / tupletRatio;

            Chord* chord = new Chord(score);
            chord->setTrack(track);
            chord->setDurationType(d);
            chord->setDuration(d.fraction());
            Segment* s = measure->getSegment(chord, tick.ticks());
            s->add(chord);
            chord->setUserPlayEvents(true);
            MidiTuplet::addElementToTuplet(voice, tick, len, chord, tuplets);

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

void MTrack::createKeys(int accidentalType)
      {
      Score* score = staff->score();
      const int track = staff->idx() * VOICES;

      KeyList* km = staff->keymap();
      if (!hasKey && !mtrack->drumTrack()) {
            KeySigEvent ks;
            ks.setAccidentalType(accidentalType);
            (*km)[0] = ks;
            }
      for (auto it = km->begin(); it != km->end(); ++it) {
            const int tick = it->first;
            const KeySigEvent &key  = it->second;
            KeySig* ks = new KeySig(score);
            ks->setTrack(track);
            ks->setGenerated(false);
            ks->setKeySigEvent(key);
            ks->setMag(staff->mag());
            Measure* m = score->tick2measure(tick);
            Segment* seg = m->getSegment(ks, tick);
            seg->add(ks);
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

      MidiTuplet::createTuplets(staff, tuplets);
      createKeys(key);

      const auto swingType = preferences.midiImportOperations.trackOperations(indexOfOperation).swing;
      Swing::detectSwing(staff, swingType);

      MidiClef::createClefs(staff, indexOfOperation, mtrack->drumTrack());
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
            track.division = mf->division();
            int events = 0;
                        //  - create time signature list from meta events
                        //  - create MidiChord list
                        //  - extract some information from track: program, min/max pitch
            for (const auto &i: t.events()) {
                  const MidiEvent& e = i.second;
                  const auto tick = toMuseScoreTicks(i.first, track.division);
                              // remove time signature events
                  if ((e.type() == ME_META) && (e.metaType() == META_TIME_SIGNATURE)) {
                        sigmap->add(tick.ticks(), metaTimeSignature(e));
                        }
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

bool isGrandStaff(const MTrack &t1, const MTrack &t2)
      {
      const static std::set<int> grandStaffPrograms = {
                  // Piano
              0, 1, 2, 3, 4, 5, 6, 7
                  // Chromatic Percussion
            , 8, 10, 11, 12, 13, 15
                  // Organ
            , 16, 17, 18, 19, 20, 21, 23
                  // Strings
            , 46
                  // Ensemble
            , 50, 51, 54
                  // Brass
            , 62, 63
                  // Synth Lead
            , 80, 81, 82, 83, 84, 85, 86, 87
                  // Synth Pad
            , 88, 89, 90, 91, 92, 93, 94, 95
                  // Synth Effects
            , 96, 97, 98, 99, 100, 101, 102, 103
            };

      return t1.mtrack->outChannel() == t2.mtrack->outChannel()
                  && grandStaffPrograms.find(t1.program) != grandStaffPrograms.end();
      }

bool isSameChannel(const MTrack &t1, const MTrack &t2)
      {
      return (t1.mtrack->outChannel() == t2.mtrack->outChannel());
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
            Part* part   = new Part(score);
            Staff* s     = new Staff(score, part, 0);
            part->insertStaff(s);
            score->staves().push_back(s);
            track.staff = s;

            if (track.mtrack->drumTrack()) {
                              // drum track
                  s->setClef(0, ClefType::PERC);
                  part->instr()->setDrumset(smDrumset);
                  part->instr()->setUseDrumset(true);
                  }
            else {
                  const int avgPitch = MChord::findAveragePitch(track.chords.begin(),
                                                                track.chords.end());
                  s->setClef(0, MidiClef::clefTypeFromAveragePitch(avgPitch));
                  if (idx < (tracks.size() - 1) && idx >= 0
                              && isGrandStaff(tracks[idx], tracks[idx + 1])) {
                                    // assume that the current track and the next track
                                    // form a piano part
                        s->setBracket(0, BRACKET_BRACE);
                        s->setBracketSpan(0, 2);

                        Staff* ss = new Staff(score, part, 1);
                        part->insertStaff(ss);
                        score->staves().push_back(ss);
                        ++idx;
                        const int avgPitch = MChord::findAveragePitch(tracks[idx].chords.begin(),
                                                                      tracks[idx].chords.end());
                        ss->setClef(0, MidiClef::clefTypeFromAveragePitch(avgPitch));
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
            Measure* measure  = new Measure(score);
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
      const Measure *m = score->lastMeasure();
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
            Part *part  = mt.staff->part();
            if (mt.name.isEmpty()) {
                  QString name = instrumentName(midiType, mt.program, mt.mtrack->drumTrack());
                  if (!name.isEmpty()) {
                        mt.name = name;
                        part->setLongName(name);
                        }
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
                  TimeSig* ts = new TimeSig(score);
                  ts->setSig(newTimeSig);
                  ts->setTrack(staffIdx * VOICES);
                  Segment* seg = m->getSegment(ts, tick);
                  seg->add(ts);
                  }
            if (newTimeSig != se.timesig())   // was a pickup measure - skip next timesig
                  ++is;
            }
      }

void processMeta(MTrack &mt, bool isLyric)
      {
      for (const auto &ie : mt.mtrack->events()) {
            const MidiEvent &e = ie.second;
            const auto tick = toMuseScoreTicks(ie.first, mt.division);
            if ((e.type() == ME_META) && ((e.metaType() == META_LYRIC) == isLyric))
                  mt.processMeta(tick.ticks(), e);
            }
      }

void createNotes(const ReducedFraction &lastTick, QList<MTrack> &tracks, MidiType midiType)
      {
      for (int i = 0; i < tracks.size(); ++i) {
            MTrack &mt = tracks[i];
            processMeta(mt, false);
            if (midiType == MT_UNKNOWN)
                  midiType = MT_GM;
            if (i % 2 && isSameChannel(tracks[i - 1], mt)) {
                  mt.program = tracks[i - 1].program;
                  }
                        // if tracks in Grand staff have different names - clear them,
                        // instrument name will be used instead
            if (i % 2 == 0 && i < tracks.size() - 1
                        && isGrandStaff(mt, tracks[i + 1])) {
                  if (mt.name != tracks[i + 1].name) {
                        mt.name = "";
                        tracks[i + 1].name = "";
                        }
                  }
            setTrackInfo(midiType, mt);
                        // pass current track index to the convertTrack function
                        //   through MidiImportOperations
            preferences.midiImportOperations.setCurrentTrack(mt.indexOfOperation);
            mt.convertTrack(lastTick);
            processMeta(mt, true);
            }
      }

QList<TrackMeta> getTracksMeta(const QList<MTrack> &tracks,
                               const MidiFile *mf)
{
      QList<TrackMeta> tracksMeta;
      for (int i = 0; i < tracks.size(); ++i) {
            const MTrack &mt = tracks[i];
            std::string trackName;
            for (const auto &ie: mt.mtrack->events()) {
                  const MidiEvent &e = ie.second;
                  if ((e.type() == ME_META) && (e.metaType() == META_TRACK_NAME)) {
                        trackName = (const char*)e.edata();
                        break;
                        }
                  }
            QString instrName;
            if (i % 2 && isSameChannel(tracks[i - 1], tracks[i])){
                  TrackMeta lastMeta = tracksMeta.back();
                  instrName = lastMeta.instrumentName;
                  }
            else {
                  MidiType midiType = mf->midiType();
                  if (midiType == MT_UNKNOWN)
                        midiType = MT_GM;
                  instrName = instrumentName(midiType, mt.program,
                                             mt.mtrack->drumTrack());
                  }
            tracksMeta.push_back({trackName,
                                  instrName,
                                  mt.mtrack->drumTrack(),
                                  mt.initLyricTrackIndex
                                 });
            }
      return tracksMeta;
      }

void convertMidi(Score *score, const MidiFile *mf)
      {
      ReducedFraction lastTick;
      auto *sigmap = score->sigmap();

      auto tracks = createMTrackList(lastTick, sigmap, mf);
      MChord::collectChords(tracks);
      cleanUpMidiEvents(tracks);
      MChord::removeOverlappingNotes(tracks);
      quantizeAllTracks(tracks, sigmap, lastTick);
      MChord::removeOverlappingNotes(tracks);
      LRHand::splitIntoLeftRightHands(tracks);
      MChord::splitUnequalChords(tracks);
      MidiDrum::splitDrumVoices(tracks);
      MidiDrum::splitDrumTracks(tracks);
                  // no more track insertion/reordering/deletion from now
      QList<MTrack> trackList = prepareTrackList(tracks);
      createInstruments(score, trackList);
      MidiDrum::setStaffBracketForDrums(trackList);
      createMeasures(lastTick, score);
      createNotes(lastTick, trackList, mf->midiType());
      createTimeSignatures(score);
      score->connectTies();
      MidiLyrics::setLyrics(mf, trackList);
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
      const MidiFile *mf = midiData.midiFile(fileName);
      const auto tracks = createMTrackList(lastTick, mockScore.sigmap(), mf);
      QList<MTrack> trackList = prepareTrackList(tracks);
      MidiLyrics::setInitialIndexes(trackList);

      return getTracksMeta(trackList, mf);
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

