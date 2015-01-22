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
#include "mscore/preferences.h"
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
#include "importmidi_tuplet_tonotes.h"
#include "libmscore/tuplet.h"
#include "libmscore/articulation.h"
#include "importmidi_swing.h"
#include "importmidi_fraction.h"
#include "importmidi_drum.h"
#include "importmidi_inner.h"
#include "importmidi_clef.h"
#include "importmidi_lrhand.h"
#include "importmidi_lyrics.h"
#include "importmidi_tie.h"
#include "importmidi_beat.h"
#include "importmidi_tempo.h"
#include "importmidi_simplify.h"
#include "importmidi_voice.h"
#include "importmidi_operations.h"
#include "importmidi_key.h"
#include "libmscore/instrtemplate.h"

#include <set>


namespace Ms {

extern Preferences preferences;
extern void updateNoteLines(Segment*, int track);
extern QList<InstrumentGroup*> instrumentGroups;


void cleanUpMidiEvents(std::multimap<int, MTrack> &tracks)
      {
      for (auto &track: tracks) {
            MTrack &mtrack = track.second;

            for (auto chordIt = mtrack.chords.begin(); chordIt != mtrack.chords.end(); ) {
                  MidiChord &ch = chordIt->second;
                  for (auto noteIt = ch.notes.begin(); noteIt != ch.notes.end(); ) {
                        if (noteIt->offTime - chordIt->first < MChord::minAllowedDuration()) {
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


#ifdef QT_DEBUG

bool doNotesOverlap(const MTrack &track)
      {
      const auto &chords = track.chords;
      for (auto i1 = chords.begin(); i1 != chords.end(); ++i1) {
            const auto &chord1 = i1->second;
            for (const auto &note1: chord1.notes) {
                  for (auto i2 = std::next(i1); i2 != chords.end(); ++i2) {
                        if (i2->first >= note1.offTime)
                              break;
                        const auto &chord2 = i2->second;
                        if (chord1.voice != chord2.voice)
                              continue;
                        for (const auto &note2: chord2.notes) {
                              if (note2.pitch != note1.pitch)
                                    continue;
                              return true;
                              }
                        }
                  }
            }
      return false;
      }

bool doNotesOverlap(const std::multimap<int, MTrack> &tracks)
      {
      bool result = false;
      for (const auto &track: tracks)
            result = doNotesOverlap(track.second);
      return result;
      }

bool noTooShortNotes(const std::multimap<int, MTrack> &tracks)
      {
      for (const auto &track: tracks) {
            const auto &chords = track.second.chords;
            for (const auto &chord: chords) {
                  for (const auto &note: chord.second.notes) {
                        if (note.offTime - chord.first < MChord::minAllowedDuration())
                              return false;
                        }
                  }
            }
      return true;
      }

#endif

std::vector<std::multimap<ReducedFraction, MidiChord> >
separateDrumChordsTo2Voices(const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      std::vector<std::multimap<ReducedFraction, MidiChord> > separatedChords(2);
      for (const auto &chord: chords) {
            const MidiChord &c = chord.second;

            Q_ASSERT(c.voice == 0 || c.voice == 1);

            separatedChords[c.voice].insert({chord.first, c});
            }
      return separatedChords;
      }

void setChordVoice(MidiChord &chord, int voice)
      {
      chord.voice = voice;
      if (chord.isInTuplet)
            chord.tuplet->second.voice = voice;
      for (auto &note: chord.notes) {
            if (note.isInTuplet)
                  note.tuplet->second.voice = voice;
            }
      }

void findAllTupletsForDrums(
            MTrack &mtrack,
            TimeSigMap *sigmap,
            const ReducedFraction &basicQuant)
      {
      const size_t drumVoiceCount = 2;
            // drum track has 2 voices (stem up and stem down),
            // and tuplet detection is applicable for single voice drum tracks,
            // so split track chords into 2 voice groups,
            // detect tuplets and merge chords (and found tuplets) back;
            // it's a small hack due to the fact that tuplet detection
            // is designed to work before voice setting

      std::vector<std::multimap<ReducedFraction, MidiChord> > chords(drumVoiceCount);
      for (const auto &chord: mtrack.chords) {
            const MidiChord &c = chord.second;

            Q_ASSERT(c.voice == 0 || c.voice == 1);

            chords[c.voice].insert({chord.first, c});
            }

      std::vector<std::multimap<ReducedFraction,
                                MidiTuplet::TupletData> > tuplets(drumVoiceCount);
      for (size_t voice = 0; voice < drumVoiceCount; ++voice) {
            if (!chords[voice].empty())
                  MidiTuplet::findAllTuplets(tuplets[voice], chords[voice], sigmap, basicQuant);
            }
      mtrack.chords.clear();
      for (size_t voice = 0; voice < drumVoiceCount; ++voice) {
            for (auto &chord: chords[voice]) {
                        // correct voice because it can be changed during tuplet detection
                  setChordVoice(chord.second, voice);
                  mtrack.chords.insert({chord.first, chord.second});
                  }
            }
      mtrack.updateTupletsFromChords();
      // note: temporary local tuplets and chords are deleted here
      }

void quantizeAllTracks(std::multimap<int, MTrack> &tracks,
                       TimeSigMap *sigmap,
                       const ReducedFraction &lastTick)
      {
      auto &opers = preferences.midiImportOperations;

      for (auto &track: tracks) {
            MTrack &mtrack = track.second;
            if (mtrack.chords.empty())
                  continue;
                        // pass current track index through MidiImportOperations
                        // for further usage
            MidiOperations::CurrentTrackSetter setCurrentTrack{opers, mtrack.indexOfOperation};

            if (opers.data()->processingsOfOpenedFile == 0) {
                  opers.data()->trackOpers.isDrumTrack.setValue(
                                          opers.currentTrack(), mtrack.mtrack->drumTrack());
                  if (mtrack.mtrack->drumTrack()) {
                        opers.data()->trackOpers.maxVoiceCount.setValue(
                                          opers.currentTrack(), MidiOperations::VoiceCount::V_1);
                        }
                  }
            const auto basicQuant = Quantize::quantValueToFraction(
                        opers.data()->trackOpers.quantValue.value(mtrack.indexOfOperation));

            Q_ASSERT_X(MChord::isLastTickValid(lastTick, mtrack.chords),
                       "quantizeAllTracks", "Last tick is less than max note off time");

            MChord::setBarIndexes(mtrack.chords, basicQuant, lastTick, sigmap);

            if (mtrack.mtrack->drumTrack())
                  findAllTupletsForDrums(mtrack, sigmap, basicQuant);
            else
                  MidiTuplet::findAllTuplets(mtrack.tuplets, mtrack.chords, sigmap, basicQuant);

            Q_ASSERT_X(!doNotesOverlap(track.second),
                       "quantizeAllTracks",
                       "There are overlapping notes of the same voice that is incorrect");

                        // (4/3 of the smallest duration) tol is less sensitive
                        // to on time inaccuracies than 1/2 earlier
            MChord::collectChords(mtrack, {2, 1}, {4, 3});
            Quantize::quantizeChords(mtrack.chords, sigmap, basicQuant);
            MidiTuplet::removeEmptyTuplets(mtrack);

            Q_ASSERT_X(MidiTuplet::areTupletRangesOk(mtrack.chords, mtrack.tuplets),
                       "quantizeAllTracks", "Tuplet chord/note is outside tuplet "
                        "or non-tuplet chord/note is inside tuplet");
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
      Score* cs         = staff->score();

      switch (mm.metaType()) {
            case META_TEXT:
            case META_LYRIC:
                  break;      // lyric and text are added in importmidi_lyrics.cpp
            case META_TRACK_NAME:
                  {
                  const std::string text = MidiCharset::fromUchar(data);

                  auto &opers = preferences.midiImportOperations;
                  if (opers.data()->processingsOfOpenedFile == 0) {
                        const int currentTrack = opers.currentTrack();
                        opers.data()->trackOpers.staffName.setValue(currentTrack, text);
                        }

                  if (name.isEmpty())
                        name = MidiCharset::convertToCharset(text);
                  }
                  break;
            case META_TEMPO:  // add later, after adding of notes
                  break;
            case META_KEY_SIGNATURE:
                  {
                  const int key = ((const char*)data)[0];
                  if (key < -7 || key > 7) {
                        qDebug("ImportMidi: illegal key %d", key);
                        break;
                        }
                  KeySigEvent ke;
                  ke.setKey(Key(key));
                  staff->setKey(tick, ke);
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
                              text->setTextStyleType(TextStyleType::COMPOSER);
                              break;
                        case META_TRANSLATOR:
                              text->setTextStyleType(TextStyleType::TRANSLATOR);
                              break;
                        case META_POET:
                              text->setTextStyleType(TextStyleType::POET);
                              break;
                        case META_SUBTITLE:
                              text->setTextStyleType(TextStyleType::SUBTITLE);
                              break;
                        case META_TITLE:
                              text->setTextStyleType(TextStyleType::TITLE);
                              break;
                        }

                  text->setText((const char*)(mm.edata()));

                  MeasureBase* measure = cs->first();
                  if (measure->type() != Element::Type::VBOX) {
                        measure = new VBox(cs);
                        measure->setTick(0);
                        measure->setNext(cs->first());
                        cs->measures()->add(measure);
                        }
                  measure->add(text);
                  }
                  break;
            case META_COPYRIGHT:
                  cs->setMetaTag("copyright", QString((const char*)(mm.edata())));
                  break;
            case META_TIME_SIGNATURE:
                  break;                  // added earlier
            case META_PORT_CHANGE:
                  // TODO
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
                  // find tuplets over which duration goes
      auto barTick = ReducedFraction::fromTicks(measure->tick());
      auto tupletsData = MidiTuplet::findTupletsInBarForDuration(
                        voice, barTick, startTick, len, tuplets);
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

      const auto &opers = preferences.midiImportOperations;
      const bool useDots = opers.data()->trackOpers.useDots.value(indexOfOperation);
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
                        TDuration duration(TDuration::DurationType::V_MEASURE);
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
                        Segment* s = measure->getSegment(Segment::Type::ChordRest,
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
                           Chord *chord,
                           const Drumset *drumset,
                           DrumsetKind useDrumset)
      {
      for (int i = 0; i < midiNotes.size(); ++i) {
            const MidiNote& mn = midiNotes[i];
            Note* note = new Note(score);
            note->setTrack(chord->track());

            NoteVal nval(mn.pitch);
            note->setNval(nval, chord->tick());
            // TODO - does this need to be key-aware?
            //note->setPitch(mn.pitch);
            //note->setTpcFromPitch();

            chord->add(note);
            note->setVeloType(Note::ValueType::USER_VAL);
            note->setVeloOffset(mn.velo);

            if (useDrumset != DrumsetKind::NONE) {
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
      Score* score = staff->score();
      const int track = staff->idx() * VOICES + voice;
      Drumset* drumset = staff->part()->instr()->drumset();
      const DrumsetKind useDrumset = staff->part()->instr()->useDrumset();

      const auto& opers = preferences.midiImportOperations.data()->trackOpers;
      const int currentTrack = preferences.midiImportOperations.currentTrack();

                  // all midiChords here should have the same onTime value
                  // and all notes in each midiChord should have the same duration
      ReducedFraction startChordTick = startChordTickFrac;
      while (!midiChords.isEmpty()) {
            const ReducedFraction tick = startChordTick;
            ReducedFraction len = nextChordTick - tick;
            if (len <= ReducedFraction(0, 1))
                  break;
            len = MChord::findMinDuration(tick, midiChords, len);
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

            if (opers.showStaccato.value(currentTrack)
                        && startChordTick == startChordTickFrac   // first chord in tied chord sequence
                        && midiChords.begin()->isStaccato()) {
                  Articulation* a = new Articulation(chord->score());
                  a->setArticulationType(ArticulationType::Staccato);
                  chord->add(a);
                  }

            Segment* s = measure->getSegment(chord, tick.ticks());
            s->add(chord);
            MidiTuplet::addElementToTuplet(voice, tick, len, chord, tuplets);

            for (int k = 0; k < midiChords.size(); ++k) {
                  MidiChord& midiChord = midiChords[k];
                  setMusicNotesFromMidi(score, midiChord.notes, chord, drumset, useDrumset);
                  if (!midiChord.notes.empty() && midiChord.notes.first().offTime - tick <= len) {
                        midiChords.removeAt(k);
                        --k;
                        continue;
                        }
                  setTies(chord, score, midiChord.notes);
                  }
            startChordTick += len;
            }
      fillGapWithRests(score, voice, startChordTick,
                       nextChordTick - startChordTick, track);
      }

void MTrack::createKeys(Key k)
      {
      Score* score = staff->score();
      const int track = staff->idx() * VOICES;

      KeyList* km = staff->keyList();
      if (!hasKey && !mtrack->drumTrack()) {
            KeySigEvent ke;
            ke.setKey(k);
            (*km)[0] = ke;
            }
      Key pkey = Key::C;
      for (auto it = km->begin(); it != km->end(); ++it) {
            const int tick = it->first;
            Key key  = it->second.key();
            if ((key == Key::C) && (key == pkey))     // dont insert uneccessary C key
                  continue;
            pkey = key;
            KeySig* ks = new KeySig(score);
            ks->setTrack(track);
            ks->setGenerated(false);
            ks->setKey(key);
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

      const Key key = Key::C;                // TODO-LIB findKey(mtrack, score->sigmap());

      MidiTuplet::createTuplets(staff, tuplets);
      createKeys(key);

      const auto& opers = preferences.midiImportOperations.data()->trackOpers;
      const auto swingType = opers.swing.value(indexOfOperation);
      Swing::detectSwing(staff, swingType);

      Q_ASSERT_X(MidiTie::areTiesConsistent(staff), "MTrack::convertTrack", "Ties are inconsistent");

      Q_ASSERT_X(MidiTuplet::haveTupletsEnoughElements(staff),
                 "MTrack::convertTrack",
                 "Tuplet has less than 2 elements or all elements are rests");

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
                        // show track even if all initial notes were cleaned up
            if (track.second.hadInitialNotes)
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
            track.isDivisionInTps = mf->isDivisionInTps();
            bool hasNotes = false;
                        //  - create time signature list from meta events
                        //  - create MidiChord list
                        //  - extract some information from track: program, min/max pitch
            for (const auto &i: t.events()) {
                  const MidiEvent& e = i.second;
                  const auto tick = toMuseScoreTicks(i.first, track.division,
                                                     track.isDivisionInTps);
                              // remove time signature events
                  if ((e.type() == ME_META) && (e.metaType() == META_TIME_SIGNATURE)) {
                                    // because file can have incorrect data
                                    // like time sig event not at the beginning of bar
                                    // we need to round tick value to integral bar count
                        int bars, beats, ticks;
                        sigmap->tickValues(tick.ticks(), &bars, &beats, &ticks);
                        sigmap->add(sigmap->bar2tick(bars, 0), metaTimeSignature(e));
                        }
                  else if (e.type() == ME_NOTE) {
                        hasNotes = true;
                        const int pitch = e.pitch();
                        const auto len = toMuseScoreTicks(e.len(), track.division,
                                                          track.isDivisionInTps);
                        if (tick + len > lastTick)
                              lastTick = tick + len;

                        MidiNote n;
                        n.pitch           = pitch;
                        n.velo            = e.velo();
                        n.offTime         = tick + len;
                        n.origOnTime      = tick;

                        MidiChord c;
                        c.notes.push_back(n);

                        track.chords.insert({tick, c});
                        }
                  else if (e.type() == ME_PROGRAM)
                        track.program = e.dataB();
                  if (tick > lastTick)
                        lastTick = tick;
                  }
            if (hasNotes) {
                  ++trackIndex;
                  track.hadInitialNotes = true;
                  const auto *data = preferences.midiImportOperations.data();
                  if (data->processingsOfOpenedFile > 0) {
                        if (data->trackOpers.doImport.value(trackIndex)) {
                              track.indexOfOperation = trackIndex;
                              const int reorderedIndex
                                    = data->trackOpers.trackIndexAfterReorder.value(trackIndex);
                              tracks.insert({reorderedIndex, track});
                              }
                        }
                  else {            // if it is an initial track-list query from MIDI import panel
                        track.indexOfOperation = trackIndex;
                        tracks.insert({trackIndex, track});
                        }
                  }
            else {
                  track.hadInitialNotes = false;       // it's a tempo track or something else
                  tracks.insert({-1, track});
                  }
            }

      Q_ASSERT_X(MChord::isLastTickValid(lastTick, tracks),
                 "createMTrackList", "Last tick is less than max note off time");

      return tracks;
      }

Measure* barFromIndex(const Score *score, int barIndex)
      {
      const int tick = score->sigmap()->bar2tick(barIndex, 0);
      return score->tick2measure(tick);
      }

bool isGrandStaff(const MTrack &t1, const MTrack &t2)
      {
      return t1.mtrack->outChannel() == t2.mtrack->outChannel()
                  && MChord::isGrandStaffProgram(t1.program)
                  && !t1.mtrack->drumTrack() && !t2.mtrack->drumTrack();
      }

bool isSameChannel(const MTrack &t1, const MTrack &t2)
      {
      return (t1.mtrack->outChannel() == t2.mtrack->outChannel());
      }

bool is3StaffOrgan(int program)
      {
      return program >= 16 && program <= 20;
      }

std::set<int> findAllPitches(const MTrack &track)
      {
      std::set<int> pitches;
      for (const auto &chord: track.chords) {
            for (const auto &note: chord.second.notes)
                  pitches.insert(note.pitch);
            }
      return pitches;
      }

void findNotEmptyDrumPitches(std::set<int> &drumPitches, const InstrumentTemplate *templ)
      {
      for (int i = 0; i != DRUM_INSTRUMENTS; ++i) {
            if (!templ->drumset->name(i).isEmpty())
                  drumPitches.insert(i);
            }
      }

bool hasNotDefinedDrumPitch(const std::set<int> &trackPitches, const std::set<int> &drumPitches)
      {
      bool hasNotDefinedPitch = false;
      for (const int pitch: trackPitches) {
            if (drumPitches.find(pitch) == drumPitches.end()) {
                  hasNotDefinedPitch = true;
                  break;
                  }
            }

      return hasNotDefinedPitch;
      }

std::vector<InstrumentTemplate *> findInstrumentsForProgram(const MTrack &track)
      {
      std::vector<InstrumentTemplate *> suitableTemplates;
      const int program = track.program;

      std::set<int> trackPitches;
      if (track.mtrack->drumTrack())
            trackPitches = findAllPitches(track);

      for (const auto &group: instrumentGroups) {
            for (const auto &templ: group->instrumentTemplates) {
                  if (templ->staffGroup == StaffGroup::TAB)
                        continue;
                  const bool isDrumTemplate = (templ->useDrumset != DrumsetKind::NONE);
                  if (track.mtrack->drumTrack() != isDrumTemplate)
                        continue;

                  std::set<int> drumPitches;
                  if (isDrumTemplate && templ->drumset)
                        findNotEmptyDrumPitches(drumPitches, templ);

                  for (const auto &channel: templ->channel) {
                        if (channel.program == program) {
                              if (isDrumTemplate && templ->drumset) {
                                    if (hasNotDefinedDrumPitch(trackPitches, drumPitches))
                                          break;
                                    }
                              suitableTemplates.push_back(templ);
                              break;
                              }
                        }
                  }
            }
      return suitableTemplates;
      }

std::pair<int, int> findMinMaxPitch(const MTrack &track)
      {
      int minPitch = std::numeric_limits<int>::max();
      int maxPitch = -1;

      for (const auto &chord: track.chords) {
            for (const auto &note: chord.second.notes) {
                  if (note.pitch < minPitch)
                        minPitch = note.pitch;
                  if (note.pitch > maxPitch)
                        maxPitch = note.pitch;
                  }
            }
      return std::make_pair(minPitch, maxPitch);
      }

int findMaxPitchDiff(const std::pair<int, int> &minMaxPitch, const InstrumentTemplate *templ)
      {
      int diff = 0;
      if (minMaxPitch.first < templ->minPitchP)
            diff = templ->minPitchP - minMaxPitch.first;
      if (minMaxPitch.second > templ->maxPitchP)
            diff = qMax(diff, minMaxPitch.second - templ->maxPitchP);

      Q_ASSERT(diff >= 0);

      return diff;
      }

void sortInstrumentTemplates(
            std::vector<InstrumentTemplate *> &templates,
            const std::pair<int, int> &minMaxPitch)
      {
      std::sort(templates.begin(), templates.end(),
                [minMaxPitch](const InstrumentTemplate *templ1, const InstrumentTemplate *templ2) {
            const int diff1 = findMaxPitchDiff(minMaxPitch, templ1);
            const int diff2 = findMaxPitchDiff(minMaxPitch, templ2);

            if (diff1 != diff2)
                  return diff1 < diff2;
                        // if drumset is not null - it's a particular drum instrument
                        // if drum set is null - it's a common drumset
                        // so prefer particular drum instruments
            if (templ1->drumset && !templ2->drumset)
                  return true;
            if (!templ1->drumset && templ2->drumset)
                  return false;
            return templ1->genres.size() > templ2->genres.size();
            });
      }

std::vector<InstrumentTemplate *> findSuitableInstruments(const MTrack &track)
      {
      std::vector<InstrumentTemplate *> templates = findInstrumentsForProgram(track);
      if (templates.empty())
            return templates;

      const std::pair<int, int> minMaxPitch = findMinMaxPitch(track);
      sortInstrumentTemplates(templates, minMaxPitch);
                  // instruments here are sorted primarily by valid pitch range difference,
                  // so first nonzero difference means that current
                  // and all successive instruments are unsuitable;
                  // but if all instruments are unsuitable then leave them all in list
      for (auto it = templates.begin(); it != templates.end(); ++it) {
            const int diff = findMaxPitchDiff(minMaxPitch, *it);
            if (diff > 0) {
                  if (it != templates.begin())
                        templates.erase(it, templates.end());
                  else              // add empty template option
                        templates.push_back(nullptr);
                  break;
                  }
            }

      return templates;
      }

void findInstrumentsForAllTracks(const QList<MTrack> &tracks)
      {
      auto& opers = preferences.midiImportOperations;
      auto &instrListOption = opers.data()->trackOpers.msInstrList;

      if (opers.data()->processingsOfOpenedFile == 0) {
                        // create instrument list on MIDI file opening
            for (const auto &track: tracks) {
                  instrListOption.setValue(track.indexOfOperation,
                                           findSuitableInstruments(track));
                  if (!instrListOption.value(track.indexOfOperation).empty()) {
                        const int defaultInstrIndex = 0;
                        opers.data()->trackOpers.msInstrIndex.setDefaultValue(defaultInstrIndex);
                        }
                  }
            }
      }

bool areNext2GrandStaff(int currentTrack, const QList<MTrack> &tracks)
      {
      if (currentTrack + 1 >= tracks.size())
            return false;
      return isGrandStaff(tracks[currentTrack], tracks[currentTrack + 1]);
      }

bool areNext3OrganStaff(int currentTrack, const QList<MTrack> &tracks)
      {
      if (currentTrack + 2 >= tracks.size())
            return false;
      if (!is3StaffOrgan(tracks[currentTrack].program))
            return false;

      return isGrandStaff(tracks[currentTrack], tracks[currentTrack + 1])
                  && isSameChannel(tracks[currentTrack + 1], tracks[currentTrack + 2]);
      }

void createInstruments(Score *score, QList<MTrack> &tracks)
      {
      const auto& opers = preferences.midiImportOperations;
      const auto &instrListOption = opers.data()->trackOpers.msInstrList;

      const int ntracks = tracks.size();
      for (int idx = 0; idx < ntracks; ++idx) {
            MTrack& track = tracks[idx];
            Part* part = new Part(score);

            const auto &instrList = instrListOption.value(track.indexOfOperation);
            const InstrumentTemplate *instr = nullptr;
            if (!instrList.empty()) {
                  const int instrIndex = opers.data()->trackOpers.msInstrIndex.value(
                                                                    track.indexOfOperation);
                  instr = instrList[instrIndex];
                  if (instr)
                        part->initFromInstrTemplate(instr);
                  }

            if (areNext3OrganStaff(idx, tracks))
                  part->setStaves(3);
            else if (areNext2GrandStaff(idx, tracks))
                  part->setStaves(2);
            else
                  part->setStaves(1);

            if (part->nstaves() == 1) {
                  if (!instr && track.mtrack->drumTrack()) {
                        part->staff(0)->setStaffType(StaffType::preset(StaffTypes::PERC_DEFAULT));
                        part->instr()->setDrumset(smDrumset);
                        part->instr()->setUseDrumset(DrumsetKind::DEFAULT_DRUMS);
                        }
                  }
            else {
                  if (!instr) {
                        part->staff(0)->setBarLineSpan(2);
                        part->staff(0)->setBracket(0, BracketType::BRACE);
                        }
                  else {
                        part->staff(0)->setBarLineSpan(instr->barlineSpan[0]);
                        part->staff(0)->setBracket(0, instr->bracket[0]);
                        }
                  part->staff(0)->setBracketSpan(0, 2);
                  }

            if (instr) {
                  for (int i = 0; i != part->nstaves(); ++i) {
                        part->staff(i)->setLines(instr->staffLines[i]);
                        part->staff(i)->setSmall(instr->smallStaff[i]);
                        part->staff(i)->setDefaultClefType(instr->clefTypes[i]);
                        }
                  }

            for (int i = 0; i != part->nstaves(); ++i) {
                  if (i > 0)
                        ++idx;
                  tracks[idx].staff = part->staff(i);
                  }

            score->appendPart(part);
            }
      }

bool isPickupWithLessTimeSig(const Fraction &firstBarTimeSig, const Fraction &secondBarTimeSig)
      {
      return firstBarTimeSig < secondBarTimeSig;
      }

bool isPickupWithGreaterTimeSig(
            const Fraction &firstBarTimeSig,
            const Fraction &secondBarTimeSig,
            const ReducedFraction &firstTick)
      {
      return firstBarTimeSig == secondBarTimeSig * 2
                  && firstBarTimeSig.numerator() % 3 != 0
                  && firstTick > ReducedFraction(0, 1);
      }

// search for pickup measure only if next 3 bars have equal time signatures
bool areNextBarsEqual(const Score *score, int barCount)
      {
      const int baseBarTick = score->sigmap()->bar2tick(1, 0);
      const Fraction baseTimeSig = score->sigmap()->timesig(baseBarTick).timesig();

      const int equalTimeSigCount = 3;
      for (int i = 2; i <= equalTimeSigCount - 1 && i < barCount; ++i) {
            const int barTick = score->sigmap()->bar2tick(i, 0);
            const Fraction timeSig = score->sigmap()->timesig(barTick).timesig();
            if (timeSig != baseTimeSig)
                  return false;
            }
      return true;
      }

void tryCreatePickupMeasure(
            const ReducedFraction &firstTick,
            Score *score,
            int *begBarIndex,
            int *barCount)
      {
      const int firstBarTick = score->sigmap()->bar2tick(0, 0);
      const int secondBarTick = score->sigmap()->bar2tick(1, 0);
      const Fraction firstTimeSig = score->sigmap()->timesig(firstBarTick).timesig();
      const Fraction secondTimeSig = score->sigmap()->timesig(secondBarTick).timesig();

      if (isPickupWithLessTimeSig(firstTimeSig, secondTimeSig)) {
            Measure* pickup = new Measure(score);
            pickup->setTick(firstBarTick);
            pickup->setNo(0);
            pickup->setIrregular(true);
            pickup->setTimesig(secondTimeSig);       // nominal time signature
            pickup->setLen(firstTimeSig);            // actual length
            score->measures()->add(pickup);
            *begBarIndex = 1;
            }
      else if (isPickupWithGreaterTimeSig(firstTimeSig, secondTimeSig, firstTick)) {
                        // split measure into 2 equal measures
                        // but for simplicity don't treat first bar as a pickup measure:
                        // leave its actual length equal to nominal length
            ++(*barCount);

            score->sigmap()->add(firstBarTick, secondTimeSig);

            Measure* firstBar = new Measure(score);
            firstBar->setTick(firstBarTick);
            firstBar->setNo(0);
            firstBar->setTimesig(secondTimeSig);
            firstBar->setLen(secondTimeSig);
            score->measures()->add(firstBar);

            Measure* secondBar = new Measure(score);
            secondBar->setTick(firstBarTick + secondTimeSig.ticks());
            secondBar->setNo(1);
            secondBar->setTimesig(secondTimeSig);
            secondBar->setLen(secondTimeSig);
            score->measures()->add(secondBar);

            *begBarIndex = 2;
            }
      }

void createMeasures(const ReducedFraction &firstTick, ReducedFraction &lastTick, Score *score)
      {
      int barCount, beat, tick;
      score->sigmap()->tickValues(lastTick.ticks(), &barCount, &beat, &tick);
      if (beat > 0 || tick > 0)
            ++barCount;           // convert bar index to number of bars

      auto &data = *preferences.midiImportOperations.data();
      if (data.processingsOfOpenedFile == 0) {
            if (!areNextBarsEqual(score, barCount))
                  data.trackOpers.searchPickupMeasure.setValue(false);
            }

      const bool tryDetectPickupMeasure = data.trackOpers.searchPickupMeasure.value();
      int begBarIndex = 0;

      if (tryDetectPickupMeasure && barCount > 1)
            tryCreatePickupMeasure(firstTick, score, &begBarIndex, &barCount);

      for (int i = begBarIndex; i < barCount; ++i) {
            Measure* m = new Measure(score);
            const int tick = score->sigmap()->bar2tick(i, 0);
            m->setTick(tick);
            m->setNo(i);
            const Fraction timeSig = score->sigmap()->timesig(tick).timesig();
            m->setTimesig(timeSig);
            m->setLen(timeSig);
            score->measures()->add(m);
            }

      const Measure *m = score->lastMeasure();
      if (m) {
            score->fixTicks();
            lastTick = ReducedFraction::fromTicks(m->endTick());
            }
      }

QString instrumentName(MidiType type, int program, bool isDrumTrack)
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
      return MidiInstrument::instrName(int(type), hbank, lbank, program);
      }

static QString concatenateWithComma(const QString &left, const QString &right)
      {
      if (left.isEmpty())
            return right;
      if (right.isEmpty())
            return left;
      return left + ", " + right;
      }

void setTrackInfo(MidiType midiType, MTrack &mt)
      {
      auto &opers = preferences.midiImportOperations;
      const QString instrName = instrumentName(midiType, mt.program, mt.mtrack->drumTrack());

      if (opers.data()->processingsOfOpenedFile == 0) {
            const int currentTrack = opers.currentTrack();
            opers.data()->trackOpers.midiInstrName.setValue(currentTrack, instrName);
                        // set channel number (from 1): number = index + 1
            opers.data()->trackOpers.channel.setValue(currentTrack, mt.mtrack->outChannel() + 1);
            }

      if (mt.staff->isTop()) {
            Part *part  = mt.staff->part();
            part->setLongName(concatenateWithComma(instrName, mt.name));
            part->setPartName(part->longName());
            part->setMidiChannel(mt.mtrack->outChannel());
            int bank = 0;
            if (mt.mtrack->drumTrack())
                  bank = 128;
            part->setMidiProgram(mt.program & 0x7f, bank);  // only GM
            }

      if (mt.name.isEmpty() && !instrName.isEmpty())
            mt.name = instrName;
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

            const auto& opers = preferences.midiImportOperations;
            const bool pickupMeasure = opers.data()->trackOpers.searchPickupMeasure.value();

            if (pickupMeasure && is == score->sigmap()->begin()) {
                  auto next = std::next(is);
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
            const auto tick = toMuseScoreTicks(ie.first, mt.division, mt.isDivisionInTps);
            if ((e.type() == ME_META) && ((e.metaType() == META_LYRIC) == isLyric))
                  mt.processMeta(tick.ticks(), e);
            }
      }

// set program equal to all staves, as it should be
// often only first stave in Grand Staff have correct program, other - default (piano)
// also handle track names
void setGrandStaffProgram(QList<MTrack> &tracks)
      {
      for (int i = 0; i < tracks.size(); ++i) {
            MTrack &mt = tracks[i];

            if (areNext3OrganStaff(i, tracks)) {
                  tracks[i + 1].program = mt.program;
                  tracks[i + 2].program = mt.program;

                  mt.name = concatenateWithComma(mt.name, "Manual");
                  tracks[i + 1].name = "";
                  tracks[i + 2].name = concatenateWithComma(tracks[i + 2].name, "Pedal");

                  i += 2;
                  }
            else if (areNext2GrandStaff(i, tracks)) {
                  tracks[i + 1].program = mt.program;
                  if (mt.name != tracks[i + 1].name) {
                        mt.name = "";             // only one name place near bracket is available
                        tracks[i + 1].name = "";  // so instrument name will be used instead
                        }
                  i += 1;
                  }
            }
      }

void createNotes(const ReducedFraction &lastTick, QList<MTrack> &tracks, MidiType midiType)
      {
      for (int i = 0; i < tracks.size(); ++i) {
            MTrack &mt = tracks[i];
                        // pass current track index to the convertTrack function
                        //   through MidiImportOperations
            auto &opers = preferences.midiImportOperations;
            MidiOperations::CurrentTrackSetter setCurrentTrack(opers, mt.indexOfOperation);

            processMeta(mt, false);
            if (midiType == MidiType::UNKNOWN)
                  midiType = MidiType::GM;
            setTrackInfo(midiType, mt);
            mt.convertTrack(lastTick);
            processMeta(mt, true);
            }
      }

void setLeftRightHandSplit(const std::multimap<int, MTrack> &tracks)
      {
      for (auto it = tracks.begin(); it != tracks.end(); ++it) {
            int trackIndex = it->first;
            const MTrack &mtrack = it->second;
            if (mtrack.mtrack->drumTrack() || mtrack.chords.empty())
                  continue;

                        // don't split staff if it is already in Grand Staff
            const auto nextIt = std::next(it);
            if (nextIt != tracks.end()) {
                  if (isGrandStaff(mtrack, nextIt->second)) {
                        ++it;
                        continue;
                        }
                  }

            if (LRHand::needToSplit(mtrack.chords, mtrack.program, mtrack.mtrack->drumTrack())) {
                  preferences.midiImportOperations.data()->trackOpers.doStaffSplit.setValue(
                                                                              trackIndex, true);
                  }
            }
      }

ReducedFraction findFirstChordTick(const QList<MTrack> &tracks)
      {
      ReducedFraction minTick(-1, 1);
      for (const auto &track: tracks) {
            for (const auto &chord: track.chords) {
                  if (minTick == ReducedFraction(-1, 1) || chord.first < minTick)
                        minTick = chord.first;
                  }
            }
      return minTick;
      }

void convertMidi(Score *score, const MidiFile *mf)
      {
      ReducedFraction lastTick;
      auto *sigmap = score->sigmap();

      auto tracks = createMTrackList(lastTick, sigmap, mf);
      cleanUpMidiEvents(tracks);
      auto &opers = preferences.midiImportOperations;

      if (opers.data()->processingsOfOpenedFile == 0) {       // for newly opened MIDI file
            opers.data()->trackCount = 0;
            for (const auto &track: tracks) {
                  if (track.first != -1)
                        ++opers.data()->trackCount;
                  }
            MidiLyrics::extractLyricsToMidiData(mf);
            }
                  // for newly opened MIDI file - detect if it is a human performance
                  // if so - detect beats and set initial time signature
      if (opers.data()->processingsOfOpenedFile == 0)
            Quantize::setIfHumanPerformance(tracks, sigmap);
      else        // user value
            MidiBeat::setTimeSignature(sigmap);

      Q_ASSERT_X((opers.data()->trackOpers.isHumanPerformance.value())
                        ? Meter::userTimeSigToFraction(opers.data()->trackOpers.timeSigNumerator.value(),
                                                       opers.data()->trackOpers.timeSigDenominator.value())
                          != ReducedFraction(0, 1) : true,
                 "convertMidi", "Null time signature for human-performed MIDI file");

      MChord::collectChords(tracks, {2, 1}, {1, 2});
      MidiBeat::adjustChordsToBeats(tracks, lastTick);
      MChord::mergeChordsWithEqualOnTimeAndVoice(tracks);

                  // for newly opened MIDI file
      if (opers.data()->processingsOfOpenedFile == 0
                  && opers.data()->trackOpers.doStaffSplit.canRedefineDefaultLater()) {
            setLeftRightHandSplit(tracks);
            }

      MChord::removeOverlappingNotes(tracks);

      Q_ASSERT_X(!doNotesOverlap(tracks),
                 "convertMidi", "There are overlapping notes of the same voice that is incorrect");

      LRHand::splitIntoLeftRightHands(tracks);
      MidiDrum::splitDrumVoices(tracks);
      quantizeAllTracks(tracks, sigmap, lastTick);
      MChord::removeOverlappingNotes(tracks);

      Q_ASSERT_X(!doNotesOverlap(tracks),
                 "convertMidi", "There are overlapping notes of the same voice that is incorrect");
      Q_ASSERT_X(noTooShortNotes(tracks),
                 "convertMidi", "There are notes of length < min allowed duration");

      MChord::mergeChordsWithEqualOnTimeAndVoice(tracks);
      Simplify::simplifyDurationsNotDrums(tracks, sigmap);
      if (MidiVoice::separateVoices(tracks, sigmap))
            Simplify::simplifyDurationsNotDrums(tracks, sigmap);    // again
      MidiDrum::splitDrumTracks(tracks);
      Simplify::simplifyDurationsForDrums(tracks, sigmap);
      MChord::splitUnequalChords(tracks);
                  // no more track insertion/reordering/deletion from now
      QList<MTrack> trackList = prepareTrackList(tracks);
      setGrandStaffProgram(trackList);
      findInstrumentsForAllTracks(trackList);
      createInstruments(score, trackList);
      MidiDrum::setStaffBracketForDrums(trackList);

      const auto firstTick = findFirstChordTick(trackList);
      createMeasures(firstTick, lastTick, score);
      createNotes(lastTick, trackList, mf->midiType());
      createTimeSignatures(score);
      score->connectTies();
      MidiLyrics::setLyricsToScore(trackList);
      MidiTempo::setTempo(tracks, score);
      MidiKey::setMainKeySig(trackList);
      }

void loadMidiData(MidiFile &mf)
      {
      mf.separateChannel();
      MidiType mt = MidiType::UNKNOWN;
      for (auto &track: mf.tracks())
            track.mergeNoteOnOffAndFindMidiType(&mt);
      mf.setMidiType(mt);
      }

Score::FileError importMidi(Score *score, const QString &name)
      {
      if (name.isEmpty())
            return Score::FileError::FILE_NOT_FOUND;

      auto &opers = preferences.midiImportOperations;

      MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, name);
      if (!opers.hasMidiFile(name))
            opers.addNewMidiFile(name);

      if (opers.data()->processingsOfOpenedFile == 0) {

            QFile fp(name);
            if (!fp.open(QIODevice::ReadOnly)) {
                  qDebug("importMidi: file open error <%s>", qPrintable(name));
                  return Score::FileError::FILE_OPEN_ERROR;
                  }
            MidiFile mf;
            try {
                  mf.read(&fp);
                  }
            catch (QString errorText) {
                  if (!MScore::noGui) {
                        QMessageBox::warning(0,
                           QWidget::tr("MuseScore: Load MIDI"),
                           QWidget::tr("Load failed: ") + errorText,
                           QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
                        }
                  fp.close();
                  qDebug("importMidi: bad file format");
                  return Score::FileError::FILE_BAD_FORMAT;
                  }
            fp.close();

            loadMidiData(mf);
            opers.setMidiFileData(name, mf);
            }

      convertMidi(score, opers.midiFile(name));
      ++opers.data()->processingsOfOpenedFile;

      return Score::FileError::FILE_NO_ERROR;
      }
}

