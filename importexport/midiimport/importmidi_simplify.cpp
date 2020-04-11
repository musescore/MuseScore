#include "importmidi_simplify.h"
#include "importmidi_chord.h"
#include "importmidi_inner.h"
#include "importmidi_meter.h"
#include "importmidi_tuplet.h"
#include "importmidi_quant.h"
#include "importmidi_voice.h"
#include "importmidi_operations.h"
#include "mscore/preferences.h"
#include "libmscore/sig.h"
#include "libmscore/durationtype.h"
#include "importmidi_tuplet_voice.h"
#include "midi/midifile.h"


namespace Ms {
namespace Simplify {

bool hasComplexBeamedDurations(const QList<std::pair<ReducedFraction, TDuration> > &list)
      {
      for (const auto &d: list) {
            if (d.second == TDuration::DurationType::V_16TH
                        || d.second == TDuration::DurationType::V_32ND
                        || d.second == TDuration::DurationType::V_64TH
                        || d.second == TDuration::DurationType::V_128TH) {
                  return true;
                  }
            }
      return false;
      }


#ifdef QT_DEBUG

bool areDurationsEqual(
            const QList<std::pair<ReducedFraction, TDuration> > &durations,
            const ReducedFraction &desiredLen)
      {
      ReducedFraction sum(0, 1);
      for (const auto &d: durations)
            sum += ReducedFraction(d.second.fraction()) / d.first;

      return desiredLen == desiredLen;
      }

#endif


void lengthenNote(
            MidiNote &note,
            int voice,
            const ReducedFraction &noteOnTime,
            const ReducedFraction &durationStart,
            const ReducedFraction &endTime,
            const ReducedFraction &barStart,
            const ReducedFraction &barFraction,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets,
            bool isDrumTrack)
      {
      if (endTime <= note.offTime)
            return;

      const auto &opers = midiImportOperations.data()->trackOpers;
      const int currentTrack = midiImportOperations.currentTrack();

      const bool useDots = opers.useDots.value(currentTrack);
      const auto tupletsForDuration = MidiTuplet::findTupletsInBarForDuration(
                              voice, barStart, note.offTime, endTime - note.offTime, tuplets);

      Q_ASSERT_X(note.offTimeQuant != ReducedFraction(-1, 1),
                 "Simplify::lengthenNote", "Note quant value was not set");

      const auto origNoteDurations = Meter::toDurationList(
                              durationStart - barStart, note.offTime - barStart, barFraction,
                              tupletsForDuration, Meter::DurationType::NOTE, useDots, false);

      const auto origRestDurations = Meter::toDurationList(
                              note.offTime - barStart, endTime - barStart, barFraction,
                              tupletsForDuration, Meter::DurationType::REST, useDots, false);

      Q_ASSERT_X(areDurationsEqual(origNoteDurations, note.offTime - durationStart),
                 "Simplify::lengthenNote", "Too short note durations remaining");
      Q_ASSERT_X(areDurationsEqual(origRestDurations, endTime - note.offTime),
                 "Simplify::lengthenNote", "Too short rest durations remaining");

                  // double - because can be + 0.5 for dots
      double minNoteDurationCount = MidiDuration::durationCount(origNoteDurations);
      double minRestDurationCount = MidiDuration::durationCount(origRestDurations);

      ReducedFraction bestOffTime(-1, 1);

      for (ReducedFraction offTime = note.offTime + note.offTimeQuant;
                           offTime <= endTime; offTime += note.offTimeQuant) {
            double noteDurationCount = 0;
            double restDurationCount = 0;

            const auto noteDurations = Meter::toDurationList(
                              durationStart - barStart, offTime - barStart, barFraction,
                              tupletsForDuration, Meter::DurationType::NOTE, useDots, false);

            Q_ASSERT_X(areDurationsEqual(noteDurations, offTime - durationStart),
                       "Simplify::lengthenNote", "Too short note durations remaining");

            noteDurationCount += MidiDuration::durationCount(noteDurations);

            if (offTime < endTime) {
                  const auto restDurations = Meter::toDurationList(
                              offTime - barStart, endTime - barStart, barFraction,
                              tupletsForDuration, Meter::DurationType::REST, useDots, false);

                  Q_ASSERT_X(areDurationsEqual(restDurations, endTime - offTime),
                             "Simplify::lengthenNote", "Too short rest durations remaining");

                  restDurationCount += MidiDuration::durationCount(restDurations);
                  }

            if (noteDurationCount + restDurationCount
                              < minNoteDurationCount + minRestDurationCount) {
                  if (opers.isHumanPerformance.value()
                              || isDrumTrack
                              || noteDurationCount <= 1.5) {
                        minNoteDurationCount = noteDurationCount;
                        minRestDurationCount = restDurationCount;
                        bestOffTime = offTime;
                        }
                  }
            }

      if (bestOffTime == ReducedFraction(-1, 1))
            return;

      // check for staccato:
      //    don't apply staccato if note is tied
      //    (case noteOnTime != durationStart - another bar, for example)

      bool hasLossOfAccuracy = false;
      if (!isDrumTrack) {
            const double addedPart = ((bestOffTime - note.offTime)
                                      / (bestOffTime - durationStart)).toDouble();
            const double STACCATO_TOL = 0.3;

            if (addedPart >= STACCATO_TOL) {
                  if (noteOnTime == durationStart && minNoteDurationCount <= 1.5)
                        note.staccato = true;
                  else
                        hasLossOfAccuracy = true;
                  }
            }

      // if the difference is only in one note/rest and there is some loss of accuracy -
      // discard change because it silently reduces duration accuracy
      // without significant improvement of readability

      if (!isDrumTrack
                  && !opers.isHumanPerformance.value()
                  && hasLossOfAccuracy
                  && (origNoteDurations.size() + origRestDurations.size())
                       - (minNoteDurationCount + minRestDurationCount) <= 1
                  && !hasComplexBeamedDurations(origNoteDurations)
                  && !hasComplexBeamedDurations(origRestDurations)) {
            return;
            }

      note.offTime = bestOffTime;
      }

// for drum tracks note duration can be arbitrary
// so start with short duration to check different cases,
// to find the most simple one

void shortenDrumNote(
            MidiNote &note,
            const std::multimap<ReducedFraction, MidiChord>::iterator &it,
            const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      if (it->second.isInTuplet) {        // if note on time is in tuplet -
                                          // set note off time to be inside that tuplet
            const auto &tuplet = it->second.tuplet->second;
            const auto len = tuplet.len / tuplet.tupletNumber;
            const auto newOffTime = qMin(it->first + len, tuplet.onTime + tuplet.len);
            if (note.offTime > newOffTime) {
                  note.offTime = newOffTime;
                  note.isInTuplet = true;
                  note.tuplet = it->second.tuplet;
                  note.offTimeQuant = len;
                  }
            }
      else {            // if note on time is outside tuplets -
                        // set note off time to be outside all tuplets
            auto next = std::next(it);
            while (next != chords.end() && next->second.voice != it->second.voice)
                  ++next;
            if (next != chords.end()) {
                  const auto len = ReducedFraction::fromTicks(
                                          MScore::division) / 8;   // 1/32
                  auto newOffTime = it->first + len;
                  if (next->second.isInTuplet) {
                        const auto &tuplet = next->second.tuplet->second;
                        if (tuplet.onTime < newOffTime)
                              newOffTime = tuplet.onTime;
                        }
                  if (newOffTime < note.offTime) {
                        note.offTime = newOffTime;
                        note.isInTuplet = false;
                        note.offTimeQuant = len;
                        }
                  }
            }
      }

void minimizeNumberOfRests(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets,
            bool isDrumTrack)
      {
      for (auto it = chords.begin(); it != chords.end(); ++it) {
            for (MidiNote &note: it->second.notes) {
                  if (isDrumTrack)
                        shortenDrumNote(note, it, chords);

                  const auto barStart = MidiBar::findBarStart(note.offTime, sigmap);
                  const auto barFraction = ReducedFraction(
                                                sigmap->timesig(barStart.ticks()).timesig());
                  auto durationStart = (it->first > barStart) ? it->first : barStart;

                  if (it->second.isInTuplet) {
                        const auto &tuplet = it->second.tuplet->second;
                        if (note.offTime >= tuplet.onTime + tuplet.len)
                              durationStart = tuplet.onTime + tuplet.len;
                        }
                  auto endTime = (barStart == note.offTime)
                                    ? barStart : barStart + barFraction;
                  if (note.isInTuplet) {
                        const auto &tuplet = note.tuplet->second;
                        if (note.offTime == tuplet.onTime + tuplet.len)
                              continue;
                        if (isDrumTrack) {
                              endTime = tuplet.onTime + tuplet.len;
                              }
                        else {
                              endTime = barStart + Quantize::quantizeToLarge(
                                                      note.offTime - barStart,
                                                      tuplet.len / tuplet.tupletNumber);
                              }
                        }

                  const auto beatLen = Meter::beatLength(barFraction);
                  const auto beatTime = barStart + Quantize::quantizeToLarge(
                                                      note.offTime - barStart, beatLen);
                  if (endTime > beatTime)
                        endTime = beatTime;

                  auto next = std::next(it);
                  while (next != chords.end()
                              && (next->second.voice != it->second.voice
                                  || next->first < note.offTime)) {
                        ++next;
                        }
                  if (next != chords.end()) {
                        if (next->first < endTime)
                              endTime = next->first;
                        if (next->second.isInTuplet && !note.isInTuplet) {
                              const auto &tuplet = next->second.tuplet->second;
                              if (tuplet.onTime < endTime)
                                    endTime = tuplet.onTime;
                              }
                        }

                  lengthenNote(note, it->second.voice, it->first, durationStart, endTime,
                               barStart, barFraction, tuplets, isDrumTrack);
                  }
            }
      }

void simplifyDurations(
            std::multimap<int, MTrack> &tracks,
            const TimeSigMap *sigmap,
            bool simplifyDrumTracks)
      {
      auto &opers = midiImportOperations;

      for (auto &track: tracks) {
            MTrack &mtrack = track.second;
            if (mtrack.mtrack->drumTrack() != simplifyDrumTracks)
                  continue;
            auto &chords = track.second.chords;
            if (chords.empty())
                  continue;

            if (opers.data()->trackOpers.simplifyDurations.value(mtrack.indexOfOperation)) {
                  MidiOperations::CurrentTrackSetter setCurrentTrack{opers, mtrack.indexOfOperation};

                  Q_ASSERT_X(MidiTuplet::areTupletRangesOk(chords, mtrack.tuplets),
                             "Simplify::simplifyDurations", "Tuplet chord/note is outside tuplet "
                             "or non-tuplet chord/note is inside tuplet before simplification");

                  minimizeNumberOfRests(chords, sigmap, mtrack.tuplets, mtrack.mtrack->drumTrack());
                        // empty tuplets may appear after simplification
                  MidiTuplet::removeEmptyTuplets(mtrack);

                  Q_ASSERT_X(MidiTuplet::areTupletRangesOk(chords, mtrack.tuplets),
                             "Simplify::simplifyDurations", "Tuplet chord/note is outside tuplet "
                             "or non-tuplet chord/note is inside tuplet after simplification");
                  }
            }
      }

void simplifyDurationsForDrums(std::multimap<int, MTrack> &tracks, const TimeSigMap *sigmap)
      {
      simplifyDurations(tracks, sigmap, true);
      }

void simplifyDurationsNotDrums(std::multimap<int, MTrack> &tracks, const TimeSigMap *sigmap)
      {
      simplifyDurations(tracks, sigmap, false);
      }

} // Simplify
} // Ms
