#include "importmidi_quant.h"
#include "libmscore/sig.h"
#include "importmidi_fraction.h"
#include "libmscore/mscore.h"
#include "preferences.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_tuplet.h"
#include "importmidi_inner.h"
#include "importmidi_beat.h"

#include <set>


namespace Ms {

extern Preferences preferences;

namespace Quantize {

ReducedFraction userQuantNoteToFraction(MidiOperation::QuantValue quantNote)
      {
      const auto division = ReducedFraction::fromTicks(MScore::division);
      auto userQuantValue = ReducedFraction::fromTicks(preferences.shortestNote);
                  // specified quantization value
      switch (quantNote) {
            case MidiOperation::QuantValue::N_4:
                  userQuantValue = division;
                  break;
            case MidiOperation::QuantValue::N_8:
                  userQuantValue = division / 2;
                  break;
            case MidiOperation::QuantValue::N_16:
                  userQuantValue = division / 4;
                  break;
            case MidiOperation::QuantValue::N_32:
                  userQuantValue = division / 8;
                  break;
            case MidiOperation::QuantValue::N_64:
                  userQuantValue = division / 16;
                  break;
            case MidiOperation::QuantValue::N_128:
                  userQuantValue = division / 32;
                  break;
            case MidiOperation::QuantValue::FROM_PREFERENCES:
            default:
                  break;
            }

      return userQuantValue;
      }

ReducedFraction shortestQuantizedNoteInRange(
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &beg,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &end)
      {
      const auto division = ReducedFraction::fromTicks(MScore::division);
      auto minDuration = division;
      for (auto it = beg; it != end; ++it) {
            for (const auto &note: it->second.notes) {
                  if (note.offTime - it->first < minDuration)
                        minDuration = note.offTime - it->first;
                  }
            }
      const auto minAllowedDuration = MChord::minAllowedDuration();
      auto shortest = division;
      for ( ; shortest > minAllowedDuration; shortest /= 2) {
            if (shortest <= minDuration)
                  break;
            }
      return shortest;
      }

ReducedFraction reduceQuantIfDottedNote(const ReducedFraction &noteLen,
                                        const ReducedFraction &raster)
      {
      auto newRaster = raster;
      const auto div = noteLen / raster;
      const double ratio = div.toDouble();
      if (ratio > 1.45 && ratio < 1.55)     // 1.5: dotted note that is larger than quantization value
            newRaster /= 2;                 // reduce quantization error for dotted notes
      return newRaster;
      }

ReducedFraction quantizeValue(const ReducedFraction &value,
                              const ReducedFraction &quant)
      {
      const auto valueReduced = value.reduced();
      const auto rasterReduced = quant.reduced();
      int valNum = valueReduced.numerator() * rasterReduced.denominator();
      const int rastNum = rasterReduced.numerator() * valueReduced.denominator();
      const int commonDen = valueReduced.denominator() * rasterReduced.denominator();
      valNum = ((valNum + rastNum / 2) / rastNum) * rastNum;
      return ReducedFraction(valNum, commonDen).reduced();
      }

ReducedFraction quantForLen(const ReducedFraction &noteLen,
                            const ReducedFraction &basicQuant)
      {
      auto quant = basicQuant;
      while (quant > noteLen && quant >= MChord::minAllowedDuration() * 2)
            quant /= 2;
      if (quant >= MChord::minAllowedDuration() * 2)
            quant = reduceQuantIfDottedNote(noteLen, quant);
      return quant;
      }

ReducedFraction quantForTuplet(const ReducedFraction &tupletLen,
                               const ReducedFraction &tupletRatio)
      {
      const auto quant = tupletLen / tupletRatio.numerator();

      Q_ASSERT_X(quant >= MChord::minAllowedDuration(),
                 "Quantize::quantForTuplet", "Too small quant value");

      if (quant >= MChord::minAllowedDuration() * 2)
            return quant / 2;
      return quant;
      }

ReducedFraction findMinQuant(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      ReducedFraction minQuant(-1, 1);
      for (const auto &note: chord.second.notes) {
            const auto quant = quantForLen(note.offTime - chord.first, basicQuant);
            if (minQuant == ReducedFraction(-1, 1) || quant < minQuant)
                  minQuant = quant;
            }
      return minQuant;
      }

ReducedFraction findQuantizedTupletChordOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart)
      {
      if (chord.first <= rangeStart)
            return rangeStart;
      const auto quant = quantForTuplet(tupletLen, tupletRatio);
      return rangeStart + quantizeValue(chord.first - rangeStart, quant);
      }

ReducedFraction findQuantizedChordOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      const ReducedFraction quant = findMinQuant(chord, basicQuant);
      return quantizeValue(chord.first, quant);
      }

ReducedFraction findQuantizedTupletNoteOffTime(
            const ReducedFraction &onTime,
            const ReducedFraction &offTime,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart)
      {
      if (offTime <= rangeStart)
            return rangeStart;

      ReducedFraction qOffTime;
      auto quant = quantForTuplet(tupletLen, tupletRatio);

      while (true) {
            qOffTime = rangeStart + quantizeValue(offTime - rangeStart, quant);
            if (qOffTime <= onTime) {
                  if (quant >= MChord::minAllowedDuration() * 2) {
                        quant /= 2;
                        continue;
                        }
                  qOffTime = onTime + quant;
                  }
            break;
            }
      return qOffTime;
      }

ReducedFraction findQuantizedNoteOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant)
      {
      ReducedFraction qOffTime;
      auto quant = quantForLen(offTime - chord.first, basicQuant);

      while (true) {
            qOffTime = quantizeValue(offTime, quant);
            if (qOffTime <= chord.first) {
                  if (quant >= MChord::minAllowedDuration() * 2) {
                        quant /= 2;
                        continue;
                        }
                  qOffTime = chord.first + quant;
                  }
            break;
            }
      return qOffTime;
      }

ReducedFraction findMinQuantizedOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      ReducedFraction minOnTime(-1, 1);
      for (const auto &note: chord.second.notes) {
            const auto quant = quantForLen(note.offTime - chord.first, basicQuant);
            const auto onTime = quantizeValue(chord.first, quant);
            if (minOnTime == ReducedFraction(-1, 1) || onTime < minOnTime)
                  minOnTime = onTime;
            }
      return minOnTime;
      }

ReducedFraction findMaxQuantizedTupletOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart)
      {
      ReducedFraction maxOffTime(0, 1);
      for (const auto &note: chord.second.notes) {
            if (note.offTime <= rangeStart)
                  continue;
            const auto offTime = findQuantizedTupletNoteOffTime(
                              chord.first, note.offTime, tupletLen, tupletRatio, rangeStart);
            if (offTime > maxOffTime)
                  maxOffTime = offTime;
            }
      return maxOffTime;
      }

ReducedFraction findMaxQuantizedOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      ReducedFraction maxOffTime(0, 1);
      for (const auto &note: chord.second.notes) {
            const auto offTime = findQuantizedNoteOffTime(chord, note.offTime, basicQuant);
            if (offTime > maxOffTime)
                  maxOffTime = offTime;
            }
      return maxOffTime;
      }

ReducedFraction findOnTimeTupletQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart)
      {
      const auto qOnTime = findQuantizedTupletChordOnTime(chord, tupletLen,
                                                          tupletRatio, rangeStart);
      return (chord.first - qOnTime).absValue();
      }

ReducedFraction findOnTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      const auto qOnTime = findQuantizedChordOnTime(chord, basicQuant);
      return (chord.first - qOnTime).absValue();
      }

ReducedFraction findOffTimeTupletQuantError(
            const ReducedFraction &onTime,
            const ReducedFraction &offTime,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart)
      {
      const auto qOffTime = findQuantizedTupletNoteOffTime(onTime, offTime, tupletLen,
                                                           tupletRatio, rangeStart);
      return (offTime - qOffTime).absValue();
      }

ReducedFraction findOffTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant)
      {
      const auto qOffTime = findQuantizedNoteOffTime(chord, offTime, basicQuant);
      return (offTime - qOffTime).absValue();
      }

ReducedFraction findQuantForRange(
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &beg,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &end,
            const ReducedFraction &basicQuant)
      {
      const auto shortestLen = shortestQuantizedNoteInRange(beg, end);
      return quantForLen(shortestLen, basicQuant);
      }

//--------------------------------------------------------------------------------------------

bool isHumanPerformance(
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap)
      {
      if (chords.empty())
            return false;

      const auto quant = ReducedFraction::fromTicks(MScore::division) / 4;    // 1/16
      int matches = 0;
      int count = 0;

      std::set<ReducedFraction> usedOnTimes;

      for (const auto &chord: chords) {
            const auto onTime = quantizeValue(chord.first, quant);
            int barIndex, beat, tick;
            sigmap->tickValues(onTime.ticks(), &barIndex, &beat, &tick);

            const auto barStart = ReducedFraction::fromTicks(sigmap->bar2tick(barIndex, 0));
            const auto barFraction = ReducedFraction(sigmap->timesig(barStart.ticks()).timesig());
            const auto beatLen = Meter::beatLength(barFraction);

            if (((onTime - barStart) / beatLen).reduced().denominator() == 1
                        && usedOnTimes.find(onTime) == usedOnTimes.end()) {
                  usedOnTimes.insert(onTime);
                  ++count;
                  const auto diff = (onTime - chord.first).absValue();
                  if (diff < MChord::minAllowedDuration())
                        ++matches;
                  }
            }

      const double TOL = 0.6;
      const double matched = matches * 1.0 / count;

      return matched < TOL;
      }

std::multimap<int, MTrack>
getTrackWithAllChords(const std::multimap<int, MTrack> &tracks)
      {
      std::multimap<int, MTrack> singleTrack{{0, MTrack()}};
      auto &allChords = singleTrack.begin()->second.chords;
      for (const auto &track: tracks) {
            const MTrack &t = track.second;
            for (const auto &chord: t.chords) {
                  allChords.insert(chord);
                  }
            }
      return singleTrack;
      }

void setIfHumanPerformance(
            const std::multimap<int, MTrack> &tracks,
            const TimeSigMap *sigmap)
      {
      auto allChordsTrack = getTrackWithAllChords(tracks);
      MChord::collectChords(allChordsTrack);
      const MTrack &track = allChordsTrack.begin()->second;
      const auto &allChords = track.chords;
      if (allChords.empty())
            return;
      const bool isHuman = isHumanPerformance(allChords, sigmap);
      preferences.midiImportOperations.setHumanPerformance(isHuman);
      if (isHuman) {
            preferences.midiImportOperations.setQuantValue(MidiOperation::QuantValue::N_8);
            const double ticksPerSec = MidiTempo::findBasicTempo(tracks) * MScore::division;
            MidiBeat::findBeatLocations(allChords, sigmap, ticksPerSec);
            }
      }

//--------------------------------------------------------------------------------------------

ReducedFraction quantizeOffTimeForTuplet(
            const ReducedFraction &onTime,
            const ReducedFraction &noteOffTime,
            const MidiTuplet::TupletData &tuplet)
      {
      const auto tupletRatio = MidiTuplet::tupletLimits(tuplet.tupletNumber).ratio;
      auto offTime = findQuantizedTupletNoteOffTime(
                        onTime, noteOffTime, tuplet.len, tupletRatio, tuplet.onTime);
                  // verify that offTime is still inside tuplet
      if (offTime < tuplet.onTime)
            offTime = tuplet.onTime;
      else if (offTime > tuplet.onTime + tuplet.len)
            offTime = tuplet.onTime + tuplet.len;

      return offTime;
      }

ReducedFraction quantizeOffTimeForNonTuplet(
            const ReducedFraction &noteOffTime,
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &basicQuant)
      {
      const MidiChord &chord = chordIt->second;
      auto offTime = findQuantizedNoteOffTime(*chordIt, noteOffTime, basicQuant);

                  // verify that offTime is still outside tuplets
      if (chord.isInTuplet) {
            const auto &tuplet = chord.tuplet->second;
            if (offTime < tuplet.onTime + tuplet.len) {
                  offTime = tuplet.onTime + tuplet.len;
                  return offTime;
                  }
            }
      auto next = std::next(chordIt);
      while (true) {
            if (next == chords.end())
                  break;
            if (!next->second.isInTuplet || next->second.voice != chord.voice) {
                  ++next;
                  continue;
                  }
            const auto &tuplet = next->second.tuplet->second;
            if (next->second.tuplet == chord.tuplet
                        || tuplet.onTime + tuplet.len <= offTime) {
                  ++next;
                  continue;
                  }

            if (offTime > tuplet.onTime)
                  offTime = tuplet.onTime;
            break;
            }

      return offTime;
      }


struct QuantPos
      {
      ReducedFraction time;
      int metricalLevel;
      double penalty;
      int prevPos;
      };

struct QuantData
      {
      std::multimap<ReducedFraction, MidiChord>::iterator chord;
      ReducedFraction quant;
      ReducedFraction quantForLen;
      ReducedFraction chordRangeStart;
      ReducedFraction chordRangeEnd;
                  // if inter on time interval with previous chord
                  // is less than min allowed duration
                  // then chord can be merged with previous chord
      bool canMergeWithPrev = false;
      int metricalLevelForLen;
      std::vector<QuantPos> positions;
      };


#ifdef QT_DEBUG

bool areAllVoicesSame(
            const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chords)
      {
      auto it = chords.begin();
      const int voice = (*it)->second.voice;
      for (++it; it != chords.end(); ++it) {
            if ((*it)->second.voice != voice)
                  return false;
            }
      return true;
      }

bool notLessThanPrev(
            const std::vector<QuantData>::iterator &it,
            const std::vector<QuantData> &data)
      {
      if (it != data.begin()) {
            const auto prev = std::prev(it);
            if (prev->chordRangeStart > it->chordRangeStart)
                  return false;
            }
      return true;
      }

bool isTupletRangeCorrect(
            const MidiTuplet::TupletData &tuplet,
            const ReducedFraction &rangeStart,
            const ReducedFraction &rangeEnd)
      {
      return (rangeStart == tuplet.onTime && rangeEnd == tuplet.onTime + tuplet.len);
      }

void checkOffTime(
            const MidiNote &note,
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      Q_ASSERT_X(note.offTime - chordIt->first >= MChord::minAllowedDuration(),
                 "Quantize::quantizeOffTimes", "Too small note length");

      if (note.isInTuplet) {
            const auto &tuplet = note.tuplet->second;

            Q_ASSERT_X(note.offTime >= tuplet.onTime
                       && note.offTime <= tuplet.onTime + tuplet.len,
                       "Quantize::quantizeOffTimes",
                       "Note off time is outside tuplet but should be inside");
            }
      else {
            const auto &chord = chordIt->second;

            auto next = std::next(chordIt);
            while (true) {
                  if (next == chords.end())
                        break;
                  if (!next->second.isInTuplet || next->second.voice != chord.voice) {
                        ++next;
                        continue;
                        }
                  const auto &tuplet = next->second.tuplet->second;
                  if (next->second.tuplet == chord.tuplet
                              || tuplet.onTime + tuplet.len <= note.offTime) {
                        ++next;
                        continue;
                        }

                  Q_ASSERT_X(note.offTime <= tuplet.onTime,
                             "Quantize::quantizeOffTimes",
                             "Note off time is inside next tuplet but it shouldn't");
                  break;
                  }

            auto prev = chordIt;
            while (true) {
                  if (prev->second.barIndex != chord.barIndex)
                        break;
                  if (prev->second.voice != chord.voice
                              || !prev->second.isInTuplet
                              || prev->second.tuplet == chord.tuplet) {
                        if (prev != chords.begin()) {
                              --prev;
                              continue;
                              }
                        }
                  else {
                        const auto &tuplet = prev->second.tuplet->second;

                        Q_ASSERT_X(note.offTime >= tuplet.onTime + tuplet.len,
                                   "Quantize::quantizeOffTimes",
                                   "Note off time is inside prev tuplet but it shouldn't");
                        }
                  break;
                  }
            }
      }

#endif


ReducedFraction quantizeToLarge(
            const ReducedFraction &time,
            const ReducedFraction &quant)
      {
      const auto ratio = time / quant;
      auto quantized = quant * (ratio.numerator() / ratio.denominator());
      if (quantized < time)
            quantized += quant;
      return quantized;
      }

ReducedFraction quantizeToSmall(
            const ReducedFraction &time,
            const ReducedFraction &quant)
      {
      const auto ratio = time / quant;
      auto quantized = quant * (ratio.numerator() / ratio.denominator());
      if (quantized >= time)
            quantized -= quant;
      return quantized;
      }

void findMetricalLevels(
            std::vector<QuantData> &data,
            const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chords,
            const ReducedFraction &tupletQuant,
            const ReducedFraction &barStart,
            const ReducedFraction &barFraction)
      {
      const auto divsInfo = (tupletQuant != ReducedFraction(-1, 1))
                  ? Meter::divisionInfo(barFraction, {(*chords.begin())->second.tuplet->second})
                  : Meter::divisionInfo(barFraction, {});

      for (QuantData &d: data) {
            for (auto t = d.chordRangeStart; t <= d.chordRangeEnd; t += d.quant) {
                  QuantPos p;
                  p.time = t;
                  p.metricalLevel = Meter::levelOfTick(t - barStart, divsInfo);
                  d.positions.push_back(p);
                  }

            int minLevel = std::numeric_limits<int>::max();
            while (true) {
                  for (auto t = d.chordRangeStart; t <= d.chordRangeEnd; t += d.quant) {
                        if (((t - barStart) / d.quantForLen).reduced().denominator() != 1)
                              continue;
                        int level = Meter::levelOfTick(t - barStart, divsInfo);
                        if (level < minLevel)
                              minLevel = level;
                        }
                  if (minLevel == std::numeric_limits<int>::max()) {
                        d.quantForLen /= 2;

                        Q_ASSERT_X(d.quantForLen >= MChord::minAllowedDuration(),
                                   "Quantize::findMetricalLevels",
                                   "quantForLen < min allowed duration");

                        continue;
                        }
                  break;
                  }
            d.metricalLevelForLen = minLevel;
            }
      }

void findChordRangeEnds(
            std::vector<QuantData> &data,
            const ReducedFraction &rangeStart,
            const ReducedFraction &rangeEnd,
            const ReducedFraction &barStart,
            const ReducedFraction &beatLen)
      {
      for (auto it = data.rbegin(); it != data.rend(); ++it) {
            QuantData &d = *it;
            d.chordRangeEnd = barStart + quantizeToSmall(rangeEnd - barStart, d.quant);

            Q_ASSERT_X(d.chord->first + beatLen >= rangeStart,
                       "Quantize::findChordRangeEnds", "chord on time + beatLen < rangeStart");

            if (d.chord->first + beatLen < rangeEnd) {
                  d.chordRangeEnd = barStart + quantizeToSmall(
                                          d.chord->first + beatLen - barStart, d.quant);
                  }
            if (it != data.rbegin()) {
                  const auto prev = std::prev(it);    // next in terms of time
                  if (prev->chordRangeEnd < d.chordRangeEnd) {
                        d.chordRangeEnd = barStart + quantizeToSmall(
                                                prev->chordRangeEnd - barStart, d.quant);
                        }
                  if (!prev->canMergeWithPrev && d.chordRangeEnd == prev->chordRangeEnd)
                        d.chordRangeEnd -= d.quant;
                  }
            if (d.chordRangeEnd < d.chordRangeStart)
                  d.chordRangeEnd = d.chordRangeStart;

            Q_ASSERT_X(d.chordRangeEnd <= rangeEnd,
                       "Quantize::findChordRangeEnds", "chordRangeEnd > rangeEnd");
            Q_ASSERT_X(d.chordRangeStart <= d.chordRangeEnd,
                       "Quantize::findChordRangeEnds", "chordRangeStart is greater than chordRangeEnd");
            Q_ASSERT_X(((d.chordRangeEnd - barStart) / d.quant).reduced().denominator() == 1,
                       "Quantize::findChordRangeEnds",
                       "chordRangeEnd - barStart is not dividable by quant");
            Q_ASSERT_X(((d.chordRangeEnd - d.chordRangeStart) / d.quant).reduced().denominator() == 1,
                       "Quantize::findChordRangeEnds",
                       "chordRangeEnd - chordRangeStart is not dividable by quant");
            }
      }

void findChordRangeStarts(
            std::vector<QuantData> &data,
            const ReducedFraction &rangeStart,
            const ReducedFraction &rangeEnd,
            const ReducedFraction &barStart,
            const ReducedFraction &beatLen)
      {
      for (auto it = data.begin(); it != data.end(); ++it) {
            QuantData &d = *it;
            while (true) {
                  d.chordRangeStart = barStart + quantizeToLarge(rangeStart - barStart, d.quant);

                  Q_ASSERT_X(d.chord->first - beatLen <= rangeEnd,
                             "Quantize::findChordRangeStarts", "chord on time - beatLen > rangeEnd");

                  if (d.chord->first - beatLen > rangeStart) {
                        d.chordRangeStart = barStart + quantizeToLarge(
                                                d.chord->first - beatLen - barStart, d.quant);
                        }

                  if (it != data.begin()) {
                        const auto prev = std::prev(it);
                        if (prev->chordRangeStart > d.chordRangeStart) {
                              d.chordRangeStart = barStart + quantizeToLarge(
                                                      prev->chordRangeStart - barStart, d.quant);
                              }
                        if (!d.canMergeWithPrev && d.chordRangeStart == prev->chordRangeStart)
                              d.chordRangeStart += d.quant;
                        }

                  if (d.chordRangeStart >= rangeEnd) {
                        if (d.quant >= MChord::minAllowedDuration() * 2)
                              d.quant /= 2;
                        else
                              d.canMergeWithPrev = true;
                        continue;
                        }
                  break;
                  }

            Q_ASSERT_X(notLessThanPrev(it, data),
                       "Quantize::findChordRangeStarts",
                       "chordRangeStart is less than previous chordRangeStart");
            Q_ASSERT_X(d.chordRangeStart >= rangeStart,
                       "Quantize::findChordRangeStarts", "chordRangeStart < rangeStart");
            Q_ASSERT_X(d.chordRangeStart < rangeEnd,
                       "Quantize::findChordRangeStarts", "chordRangeStart >= rangeEnd");
            Q_ASSERT_X(((d.chordRangeStart - barStart) / d.quant).reduced().denominator() == 1,
                       "Quantize::findChordRangeStarts",
                       "chordRangeStart - barStart is not dividable by quant");
            }
      }

void findQuants(
            std::vector<QuantData> &data,
            const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chords,
            const ReducedFraction &rangeStart,
            const ReducedFraction &rangeEnd,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletQuant,
            const ReducedFraction &barFraction)
      {
      for (auto it = chords.begin(); it != chords.end(); ++it) {
            const auto chordIt = *it;
            QuantData d;
            d.chord = chordIt;
            auto len = MChord::minNoteLen(*chordIt);
            if (rangeEnd - rangeStart < len)
                  len = rangeEnd - rangeStart;
            if (it != chords.begin()) {
                  const auto prevChordIt = *std::prev(it);
                  if (chordIt->first - prevChordIt->first < len)
                        len = chordIt->first - prevChordIt->first;
                  if (len < MChord::minAllowedDuration())
                        d.canMergeWithPrev = true;
                  }
            if (tupletQuant != ReducedFraction(-1, 1)) {
                  const MidiTuplet::TupletData &tuplet = (*chords.begin())->second.tuplet->second;
                  d.quant = tupletQuant;
                  d.quantForLen = tuplet.len / tuplet.tupletNumber;
                  }
            else {
                  d.quant = quantForLen(len, basicQuant);
                  auto maxQuant = basicQuant;
                  while (maxQuant < barFraction)
                        maxQuant *= 2;
                  d.quantForLen = quantForLen(
                           qMin(MChord::minNoteLen(*chordIt), rangeEnd - rangeStart), maxQuant);
                  }

            Q_ASSERT_X(d.quant <= rangeEnd - rangeStart,
                       "Quantize::findQuants", "Quant value is larger than range interval");

            data.push_back(d);
            }
      }

ReducedFraction findTupletQuant(
            const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chords)
      {
      ReducedFraction tupletQuant(-1, 1);
      if ((*chords.begin())->second.isInTuplet) {
            const MidiTuplet::TupletData &tuplet = (*chords.begin())->second.tuplet->second;
            const auto tupletRatio = MidiTuplet::tupletLimits(tuplet.tupletNumber).ratio;
            tupletQuant = quantForTuplet(tuplet.len, tupletRatio);
            }

      return tupletQuant;
      }

std::vector<QuantData> findQuantData(
            const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chords,
            const ReducedFraction &rangeStart,
            const ReducedFraction &rangeEnd,
            const ReducedFraction &basicQuant,
            const ReducedFraction &barStart,
            const ReducedFraction &barFraction)
      {

      Q_ASSERT_X(!chords.empty(), "Quantize::findQuantData", "Empty chords");

      std::vector<QuantData> data;
      const auto tupletQuant = findTupletQuant(chords);
      const auto beatLen = Meter::beatLength(barFraction);

      findQuants(data, chords, rangeStart, rangeEnd, basicQuant, tupletQuant, barFraction);
      findChordRangeStarts(data, rangeStart, rangeEnd, barStart, beatLen);
      findChordRangeEnds(data, rangeStart, rangeEnd, barStart, beatLen);
      findMetricalLevels(data, chords, tupletQuant, barStart, barFraction);

      return data;
      }

int findLastChordPosition(const std::vector<QuantData> &quantData)
      {
      int posIndex = -1;
      double minPenalty = std::numeric_limits<double>::max();
      const auto &lastPositions = quantData[quantData.size() - 1].positions;
      for (int i = 0; i != (int)lastPositions.size(); ++i) {
            if (lastPositions[i].penalty < minPenalty) {
                  minPenalty = lastPositions[i].penalty;
                  posIndex = i;
                  }
            }

      Q_ASSERT_X(posIndex != -1,
                 "Quantize::findLastChordPosition", "Last index was not found");

      return posIndex;
      }

double findTempoPenalty(
            const QuantPos &p,
            const QuantPos &pPrev,
            const QuantData &d,
            const QuantData &dPrev,
            const std::vector<QuantData> &quantData,
            int chordIndex)
      {
      double penalty;
      const QuantData &dPrevPrev = quantData[chordIndex - 2];
      const QuantPos &pPrevPrev = dPrevPrev.positions[pPrev.prevPos];
      if (pPrev.time != p.time && pPrevPrev.time != pPrev.time) {

            Q_ASSERT_X(dPrev.chord->first != d.chord->first
                        && dPrevPrev.chord->first != dPrev.chord->first,
                       "Quantize::findTempoPenalty", "Chords have equal on times");

            const double veloc = ((pPrev.time - p.time)
                        / (dPrev.chord->first - d.chord->first)).toDouble();
            const double prevVeloc = ((pPrevPrev.time - pPrev.time)
                        / (dPrevPrev.chord->first - dPrev.chord->first)).toDouble();
            const double timeDiff = ((p.time + pPrev.time) / 2
                           - (pPrevPrev.time + pPrev.time) / 2).toDouble();
            penalty = qAbs((veloc - prevVeloc) * timeDiff);
            }
      else {
            const auto tempoChangePenalty = ((d.chord->first - p.time)
                            - (dPrev.chord->first - pPrev.time)).absValue();
            penalty = tempoChangePenalty.toDouble();
            }
      return penalty;
      }

void applyDynamicProgramming(std::vector<QuantData> &quantData)
      {
      const auto &opers = preferences.midiImportOperations.currentTrackOperations();
      const bool isHuman = opers.quantize.humanPerformance;
      const double MERGE_PENALTY_COEFF = 5.0;

      for (int chordIndex = 0; chordIndex != (int)quantData.size(); ++chordIndex) {
            QuantData &d = quantData[chordIndex];
            for (int pos = 0; pos != (int)d.positions.size(); ++pos) {
                  QuantPos &p = d.positions[pos];
                  const auto timePenalty = (d.chord->first - p.time).absValue().toDouble();
                  const double levelDiff = 1 + qAbs(d.metricalLevelForLen - p.metricalLevel);

                  if (p.metricalLevel <= d.metricalLevelForLen)
                        p.penalty = timePenalty * levelDiff;
                  else
                        p.penalty = (isHuman) ? timePenalty / levelDiff : timePenalty;

                  if (chordIndex > 0) {
                        const QuantData &dPrev = quantData[chordIndex - 1];
                        double minPenalty = std::numeric_limits<double>::max();
                        int minPos = -1;
                        for (int posPrev = 0; posPrev != (int)dPrev.positions.size(); ++posPrev) {
                              const QuantPos &pPrev = dPrev.positions[posPrev];
                              if (pPrev.time > p.time)
                                    continue;
                              double penalty = pPrev.penalty;
                              if (pPrev.time == p.time) {
                                    if (!d.canMergeWithPrev)
                                          continue;
                                    penalty += d.quant.toDouble() * MERGE_PENALTY_COEFF;
                                    }

                              if (chordIndex > 1) {
                                    penalty += findTempoPenalty(p, pPrev, d, dPrev,
                                                                quantData, chordIndex);
                                    }

                              if (penalty < minPenalty) {
                                    minPenalty = penalty;
                                    minPos = posPrev;
                                    }
                              }

                        Q_ASSERT_X(minPos != -1,
                                   "Quantize::applyDynamicProgramming", "Min pos was not found");

                        p.penalty += minPenalty;
                        p.prevPos = minPos;
                        }
                  }
            }
      }

void quantizeOnTimesInRange(
            const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chords,
            std::multimap<ReducedFraction, MidiChord> &quantizedChords,
            const ReducedFraction &rangeStart,
            const ReducedFraction &rangeEnd,
            const ReducedFraction &basicQuant,
            const ReducedFraction &barStart,
            const ReducedFraction &barFraction)
      {
      Q_ASSERT_X(!chords.empty(), "Quantize::quantizeOnTimesInRange", "Empty chords");
      Q_ASSERT_X(areAllVoicesSame(chords),
                 "Quantize::quantizeOnTimesInRange", "Chord voices are not the same");

      Q_ASSERT_X(rangeStart != ReducedFraction(-1, 1)
                  && rangeEnd != ReducedFraction(-1, 1)
                  && rangeStart < rangeEnd,
                 "Quantize::quantizeOnTimesInRange",
                 "range start and/or range end are incorrect");

      Q_ASSERT_X((chords.front()->second.isInTuplet) ? isTupletRangeCorrect(
                  chords.front()->second.tuplet->second, rangeStart, rangeEnd) : true,
                 "Quantize::quantizeOnTimesInRange", "Tuplet range is incorrect");


      std::vector<QuantData> quantData = findQuantData(chords, rangeStart, rangeEnd,
                                                       basicQuant, barStart, barFraction);
      applyDynamicProgramming(quantData);

                  // backward dynamic programming step - collect optimal chord positions
      int posIndex = findLastChordPosition(quantData);

      Q_ASSERT_X(quantData.size() == chords.size(),
                 "Quantize::quantizeOnTimesInRange",
                 "Sizes of quant data and chords are not equal");

      for (int chordIndex = quantData.size() - 1; ; --chordIndex) {
            const QuantPos &p = quantData[chordIndex].positions[posIndex];
            const auto onTime = p.time;
            quantizedChords.insert({onTime, chords[chordIndex]->second});
            if (chordIndex == 0)
                  break;
            posIndex = p.prevPos;
            }
      }

// input chords - sorted by onTime value

void applyTupletStaccato(std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (auto chordIt = chords.begin(); chordIt != chords.end(); ++chordIt) {
            for (MidiNote &note: chordIt->second.notes) {
                  if (note.isInTuplet && note.staccato) {
                        const MidiTuplet::TupletData &tuplet = note.tuplet->second;
                              // decrease tuplet error by enlarging staccato notes:
                              // make note.len = tuplet note length
                        const auto tupletNoteLen = (tuplet.onTime + tuplet.len)
                                                    / tuplet.tupletNumber;
                        note.offTime = chordIt->first + tupletNoteLen;
                        }
                  }
            }
      }

void quantizeOffTimes(
            std::multimap<ReducedFraction, MidiChord> &quantizedChords,
            const ReducedFraction &basicQuant)
      {
      for (auto chordIt = quantizedChords.begin(); chordIt != quantizedChords.end(); ) {
            auto &chordEvent = *chordIt;
            MidiChord &chord = chordEvent.second;
                        // quantize off times
            for (auto noteIt = chord.notes.begin(); noteIt != chord.notes.end(); ) {
                  MidiNote &note = *noteIt;
                  auto offTime = note.offTime;

                  if (note.isInTuplet) {
                        offTime = quantizeOffTimeForTuplet(
                                          chordEvent.first, offTime, note.tuplet->second);
                        }
                  else {
                        offTime = quantizeOffTimeForNonTuplet(
                                          offTime, chordIt, quantizedChords, basicQuant);
                        }

                  note.offTime = offTime;
#ifdef QT_DEBUG
                  checkOffTime(note, chordIt, quantizedChords);
#endif
                  ++noteIt;
                  }
            if (chord.notes.isEmpty()) {
                  chordIt = quantizedChords.erase(chordIt);
                  continue;
                  }
            ++chordIt;
            }
      }

void quantizeOnTimes(
            std::multimap<ReducedFraction, MidiChord> &chords,
            std::multimap<ReducedFraction, MidiChord> &quantizedChords,
            const ReducedFraction &basicQuant,
            const TimeSigMap *sigmap)
      {
      int maxVoice = 0;
      for (int voice = 0; voice <= maxVoice; ++voice) {
            int currentBarIndex = -1;
            ReducedFraction rangeStart(-1, 1);
            ReducedFraction rangeEnd(-1, 1);
            ReducedFraction barFraction(-1, 1);
            ReducedFraction barStart(-1, 1);
            bool currentlyInTuplet = false;
            std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> chordsToQuant;

            for (auto chordIt = chords.begin(); chordIt != chords.end(); ++chordIt) {
                  if (chordIt->second.voice > maxVoice)
                        maxVoice = chordIt->second.voice;
                  if (chordIt->second.voice != voice)
                        continue;

                  if (chordsToQuant.empty()) {
                        currentlyInTuplet = chordIt->second.isInTuplet;
                        if (currentBarIndex != chordIt->second.barIndex) {
                              currentBarIndex = chordIt->second.barIndex;
                              barStart = ReducedFraction::fromTicks(
                                                sigmap->bar2tick(currentBarIndex, 0));
                              barFraction = ReducedFraction(sigmap->timesig(barStart.ticks()).timesig());
                              if (!currentlyInTuplet)
                                    rangeStart = barStart;
                              }
                        if (currentlyInTuplet) {
                              const auto &tuplet = chordIt->second.tuplet->second;
                              rangeStart = tuplet.onTime;
                              rangeEnd = tuplet.onTime + tuplet.len;
                              }
                        }

                  chordsToQuant.push_back(chordIt);

                  auto nextChord = std::next(chordIt);
                  while (nextChord != chords.end() && nextChord->second.voice != voice)
                        ++nextChord;
                  if (nextChord == chords.end()
                              || nextChord->second.barIndex != currentBarIndex
                              || nextChord->second.isInTuplet != currentlyInTuplet
                              || (nextChord->second.isInTuplet && currentlyInTuplet
                                  && nextChord->second.tuplet != chordIt->second.tuplet)) {

                        if (!currentlyInTuplet) {
                              if (nextChord != chords.end()) {
                                    if (nextChord->second.barIndex != currentBarIndex) {
                                          rangeEnd = ReducedFraction::fromTicks(
                                                       sigmap->bar2tick(currentBarIndex + 1, 0));
                                          }
                                    else if (nextChord->second.isInTuplet) {
                                          rangeEnd = nextChord->second.tuplet->second.onTime;
                                          }
                                    }
                              else {
                                    rangeEnd = ReducedFraction::fromTicks(
                                                      sigmap->bar2tick(currentBarIndex + 1, 0));
                                    }
                              }

                        quantizeOnTimesInRange(chordsToQuant, quantizedChords, rangeStart, rangeEnd,
                                               basicQuant, barStart, barFraction);
                        chordsToQuant.clear();
                        }
                  }
            }
      }

void quantizeChords(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap,
            const ReducedFraction &basicQuant)
      {
      applyTupletStaccato(chords);     // apply staccato for tuplet off times

      std::multimap<ReducedFraction, MidiChord> quantizedChords;
      quantizeOnTimes(chords, quantizedChords, basicQuant, sigmap);
      quantizeOffTimes(quantizedChords, basicQuant);

      std::swap(chords, quantizedChords);
      }

} // namespace Quantize
} // namespace Ms
