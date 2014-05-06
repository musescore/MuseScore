#include "importmidi_simplify.h"
#include "importmidi_chord.h"
#include "importmidi_inner.h"
#include "importmidi_meter.h"
#include "importmidi_tuplet.h"
#include "importmidi_quant.h"
#include "preferences.h"
#include "libmscore/sig.h"
#include "libmscore/durationtype.h"


namespace Ms {
namespace Simplify {

const ReducedFraction findBarStart(const ReducedFraction &time, const TimeSigMap *sigmap)
      {
      int barIndex, beat, tick;
      sigmap->tickValues(time.ticks(), &barIndex, &beat, &tick);
      return ReducedFraction::fromTicks(sigmap->bar2tick(barIndex, 0));
      }

double durationCount(const QList<std::pair<ReducedFraction, TDuration> > &durations)
      {
      double count = durations.size();
      for (const auto &d: durations) {
            if (d.second.dots())
                  count += 0.5;
            }
      return count;
      }

void lengthenNote(
            MidiNote &note,
            int voice,
            const ReducedFraction &noteOnTime,
            const ReducedFraction &durationStart,
            const ReducedFraction &endTime,
            const ReducedFraction &barStart,
            const ReducedFraction &barFraction,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets)
      {
      if (endTime <= note.offTime)
            return;

      const auto &opers = preferences.midiImportOperations.currentTrackOperations();
      const bool useDots = opers.useDots;
      const auto tupletsForDuration = MidiTuplet::findTupletsInBarForDuration(
                                           voice, barStart, note.offTime, endTime, tuplets);

      Q_ASSERT_X(note.quant != ReducedFraction(-1, 1),
                 "Simplify::lengthenNote", "Note quant value was not set");

      double minNoteDurationCount = -1;   // double - because can be + 0.5 for dots
      double minRestDurationCount = -1;
      ReducedFraction bestOffTime(-1, 1);

      for (ReducedFraction offTime = note.offTime; offTime <= endTime; offTime += note.quant) {
            double noteDurationCount = 0;
            double restDurationCount = 0;
            const auto noteDurations = Meter::toDurationList(
                                          durationStart - barStart, offTime - barStart, barFraction,
                                          tupletsForDuration, Meter::DurationType::NOTE, useDots);
            noteDurationCount += durationCount(noteDurations);

            if (offTime < endTime) {
                  const auto restDurations = Meter::toDurationList(
                                          offTime - barStart, endTime - barStart, barFraction,
                                          tupletsForDuration, Meter::DurationType::REST, useDots);
                  restDurationCount += durationCount(restDurations);
                  }

            if (offTime == note.offTime) {      // initialization
                  minNoteDurationCount = noteDurationCount;
                  minRestDurationCount = restDurationCount;
                  bestOffTime = offTime;
                  }
            else if (noteDurationCount + restDurationCount
                            < minNoteDurationCount + minRestDurationCount) {
                  if (opers.quantize.humanPerformance || noteDurationCount == 1) {
                        minNoteDurationCount = noteDurationCount;
                        minRestDurationCount = restDurationCount;
                        bestOffTime = offTime;
                        }
                  }
            }

      Q_ASSERT_X(minNoteDurationCount != -1, "Simplify::lengthenNote", "Off time was not found");

                  // check for staccato:
                  //    don't apply staccato if note is tied
                  //    (case noteOnTime != durationStart - another bar, for example)
      if (noteOnTime == durationStart && minNoteDurationCount == 1) {
            const double STACCATO_TOL = 0.3;
            const double addedPart = ((bestOffTime - note.offTime)
                                      / (bestOffTime - durationStart)).toDouble();
            if (addedPart >= STACCATO_TOL)
                  note.staccato = true;
            }

      note.offTime = bestOffTime;
      }

void minimizeNumberOfRests(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets)
      {
      for (auto it = chords.begin(); it != chords.end(); ++it) {
            for (MidiNote &note: it->second.notes) {
                  const auto barStart = findBarStart(note.offTime, sigmap);
                  const auto barFraction = ReducedFraction(
                                                sigmap->timesig(barStart.ticks()).timesig());
                  auto durationStart = (it->first > barStart) ? it->first : barStart;
                  auto endTime = (barStart == note.offTime)
                                    ? barStart : barStart + barFraction;
                  if (note.isInTuplet) {
                        const auto &tuplet = note.tuplet->second;
                        endTime = durationStart + tuplet.len / tuplet.tupletNumber;
                        }

                  const auto beatLen = Meter::beatLength(barFraction);
                  const auto beatTime = barStart + Quantize::quantizeToLarge(
                                                      note.offTime - barStart, beatLen);
                  if (endTime > beatTime)
                        endTime = beatTime;

                  auto next = std::next(it);
                  while (next != chords.end() && next->second.voice != it->second.voice)
                        ++next;
                  if (next != chords.end()) {
                        if (next->first < endTime)
                              endTime = next->first;
                        }
                  lengthenNote(note, it->second.voice, it->first, durationStart, endTime,
                               barStart, barFraction, tuplets);
                  }
            }
      }

void simplifyNotation(std::multimap<int, MTrack> &tracks, const TimeSigMap *sigmap)
      {
      auto &opers = preferences.midiImportOperations;

      for (auto &track: tracks) {
            MTrack &mtrack = track.second;
            if (mtrack.mtrack->drumTrack())
                  continue;
            auto &chords = track.second.chords;
            if (chords.empty())
                  continue;
                        // pass current track index through MidiImportOperations
                        // for further usage
            opers.setCurrentTrack(mtrack.indexOfOperation);

            if (opers.currentTrackOperations().minimizeNumberOfRests)
                  minimizeNumberOfRests(chords, sigmap, mtrack.tuplets);
            }
      }

} // Simplify
} // Ms
