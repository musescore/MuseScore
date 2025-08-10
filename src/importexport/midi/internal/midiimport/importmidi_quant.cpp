/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "importmidi_quant.h"

#include <set>
#include <deque>

#include "engraving/dom/sig.h"
#include "importmidi_fraction.h"
#include "engraving/dom/mscore.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_tuplet.h"
#include "importmidi_inner.h"
#include "importmidi_beat.h"
#include "importmidi_voice.h"
#include "importmidi_tempo.h"
#include "importmidi_operations.h"

#include "modularity/ioc.h"
#include "importexport/midi/imidiconfiguration.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::midi {
namespace Quantize {
ReducedFraction quantValueToFraction(MidiOperations::QuantValue quantValue)
{
    const auto division = ReducedFraction::fromTicks(Constants::DIVISION);
    ReducedFraction fraction;

    switch (quantValue) {
    case MidiOperations::QuantValue::Q_4:
        fraction = division;
        break;
    case MidiOperations::QuantValue::Q_8:
        fraction = division / 2;
        break;
    case MidiOperations::QuantValue::Q_16:
        fraction = division / 4;
        break;
    case MidiOperations::QuantValue::Q_32:
        fraction = division / 8;
        break;
    case MidiOperations::QuantValue::Q_64:
        fraction = division / 16;
        break;
    case MidiOperations::QuantValue::Q_128:
        fraction = division / 32;
        break;
    case MidiOperations::QuantValue::Q_256:
        fraction = division / 64;
        break;
    case MidiOperations::QuantValue::Q_512:
        fraction = division / 128;
        break;
    case MidiOperations::QuantValue::Q_1024:
        fraction = division / 256;
        break;
    default:
        Q_ASSERT_X(false, "Quantize::quantValueToFraction", "Unknown quant value");
        break;
    }

    return fraction;
}

MidiOperations::QuantValue fractionToQuantValue(const ReducedFraction& fraction)
{
    const auto division = ReducedFraction::fromTicks(Constants::DIVISION);
    MidiOperations::QuantValue quantValue = MidiOperations::QuantValue::Q_4;

    if (fraction == division) {
        quantValue = MidiOperations::QuantValue::Q_4;
    } else if (fraction == division / 2) {
        quantValue = MidiOperations::QuantValue::Q_8;
    } else if (fraction == division / 4) {
        quantValue = MidiOperations::QuantValue::Q_16;
    } else if (fraction == division / 8) {
        quantValue = MidiOperations::QuantValue::Q_32;
    } else if (fraction == division / 16) {
        quantValue = MidiOperations::QuantValue::Q_64;
    } else if (fraction == division / 32) {
        quantValue = MidiOperations::QuantValue::Q_128;
    } else if (fraction == division / 64) {
        quantValue = MidiOperations::QuantValue::Q_256;
    } else if (fraction == division / 128) {
        quantValue = MidiOperations::QuantValue::Q_512;
    } else if (fraction == division / 256) {
        quantValue = MidiOperations::QuantValue::Q_1024;
    } else {
        LOGD("Unknown quant fraction %d/%d in division %d/%d.",
             fraction.numerator(), fraction.denominator(),
             division.numerator(), division.denominator());
        quantValue = MidiOperations::QuantValue::Q_INVALID;
    }

    return quantValue;
}

MidiOperations::QuantValue defaultQuantValueFromPreferences()
{
    auto conf = muse::modularity::globalIoc()->resolve<mu::iex::midi::IMidiImportExportConfiguration>("iex_midi");
    int ticks = conf ? conf->midiShortestNote() : (Constants::DIVISION / 4);
    const auto fraction = ReducedFraction::fromTicks(ticks);
    MidiOperations::QuantValue quantValue = fractionToQuantValue(fraction);
    if (quantValue == MidiOperations::QuantValue::Q_INVALID) {
        LOGD("Unknown shortestNote value %d in preferences, defaulting to 16th.", ticks);
        quantValue = MidiOperations::QuantValue::Q_16;
    }
    return quantValue;
}

ReducedFraction shortestQuantizedNoteInRange(
    const std::multimap<ReducedFraction, MidiChord>::const_iterator& beg,
    const std::multimap<ReducedFraction, MidiChord>::const_iterator& end)
{
    const auto division = ReducedFraction::fromTicks(Constants::DIVISION);
    auto minDuration = division;
    for (auto it = beg; it != end; ++it) {
        for (const auto& note: it->second.notes) {
            if (note.offTime - it->first < minDuration) {
                minDuration = note.offTime - it->first;
            }
        }
    }
    const auto minAllowedDuration = MChord::minAllowedDuration();
    auto shortest = division;
    for (; shortest > minAllowedDuration; shortest /= 2) {
        if (shortest <= minDuration) {
            break;
        }
    }
    return shortest;
}

ReducedFraction reduceQuantIfDottedNote(const ReducedFraction& noteLen,
                                        const ReducedFraction& raster)
{
    auto newRaster = raster;
    const auto div = noteLen / raster;
    const double ratio = div.toDouble();
    if (ratio > 1.45 && ratio < 1.55) {     // 1.5: dotted note that is larger than quantization value
        newRaster /= 2;                     // reduce quantization error for dotted notes
    }
    return newRaster;
}

ReducedFraction quantizeValue(const ReducedFraction& value,
                              const ReducedFraction& quant)
{
    const auto valueReduced = value.reduced();
    const auto rasterReduced = quant.reduced();
    int valNum = valueReduced.numerator() * rasterReduced.denominator();
    const int rastNum = rasterReduced.numerator() * valueReduced.denominator();
    const int commonDen = valueReduced.denominator() * rasterReduced.denominator();
    valNum = ((valNum + rastNum / 2) / rastNum) * rastNum;
    return ReducedFraction(valNum, commonDen).reduced();
}

ReducedFraction quantForLen(const ReducedFraction& noteLen,
                            const ReducedFraction& basicQuant)
{
    auto quant = basicQuant;
    while (quant > noteLen && quant >= MChord::minAllowedDuration() * 2) {
        quant /= 2;
    }

    if (quant >= MChord::minAllowedDuration() * 2) {
        quant = reduceQuantIfDottedNote(noteLen, quant);
    }

    return quant;
}

ReducedFraction quantForTuplet(const ReducedFraction& tupletLen,
                               const ReducedFraction& tupletRatio)
{
    const auto quant = tupletLen / tupletRatio.numerator();

    Q_ASSERT_X(quant >= MChord::minAllowedDuration(),
               "Quantize::quantForTuplet", "Too small quant value");

    if (quant >= MChord::minAllowedDuration() * 2) {
        return quant / 2;
    }

    return quant;
}

ReducedFraction findMinQuant(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const ReducedFraction& basicQuant)
{
    ReducedFraction minQuant(-1, 1);
    for (const auto& note: chord.second.notes) {
        const auto quant = quantForLen(note.offTime - chord.first, basicQuant);
        if (minQuant == ReducedFraction(-1, 1) || quant < minQuant) {
            minQuant = quant;
        }
    }
    return minQuant;
}

ReducedFraction findQuantizedTupletChordOnTime(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const ReducedFraction& tupletLen,
    const ReducedFraction& tupletRatio,
    const ReducedFraction& rangeStart)
{
    if (chord.first <= rangeStart) {
        return rangeStart;
    }
    const auto quant = quantForTuplet(tupletLen, tupletRatio);
    return rangeStart + quantizeValue(chord.first - rangeStart, quant);
}

ReducedFraction findQuantizedChordOnTime(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const ReducedFraction& basicQuant)
{
    const ReducedFraction quant = findMinQuant(chord, basicQuant);
    return quantizeValue(chord.first, quant);
}

std::pair<ReducedFraction, ReducedFraction>
findQuantizedTupletNoteOffTime(
    const ReducedFraction& onTime,
    const ReducedFraction& offTime,
    const ReducedFraction& tupletLen,
    const ReducedFraction& tupletRatio,
    const ReducedFraction& rangeStart)
{
    if (offTime <= rangeStart) {
        return { rangeStart, tupletLen };
    }

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
    return { qOffTime, quant };
}

std::pair<ReducedFraction, ReducedFraction>
findQuantizedNoteOffTime(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const ReducedFraction& offTime,
    const ReducedFraction& basicQuant)
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
    return { qOffTime, quant };
}

ReducedFraction findMinQuantizedOnTime(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const ReducedFraction& basicQuant)
{
    ReducedFraction minOnTime(-1, 1);
    for (const auto& note: chord.second.notes) {
        const auto quant = quantForLen(note.offTime - chord.first, basicQuant);
        const auto onTime = quantizeValue(chord.first, quant);
        if (minOnTime == ReducedFraction(-1, 1) || onTime < minOnTime) {
            minOnTime = onTime;
        }
    }
    return minOnTime;
}

ReducedFraction findMaxQuantizedTupletOffTime(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const ReducedFraction& tupletLen,
    const ReducedFraction& tupletRatio,
    const ReducedFraction& rangeStart)
{
    ReducedFraction maxOffTime(0, 1);
    for (const auto& note: chord.second.notes) {
        if (note.offTime <= rangeStart) {
            continue;
        }
        const auto offTime = findQuantizedTupletNoteOffTime(
            chord.first, note.offTime, tupletLen, tupletRatio, rangeStart).first;
        if (offTime > maxOffTime) {
            maxOffTime = offTime;
        }
    }
    return maxOffTime;
}

ReducedFraction findMaxQuantizedOffTime(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const ReducedFraction& basicQuant)
{
    ReducedFraction maxOffTime(0, 1);
    for (const auto& note: chord.second.notes) {
        const auto offTime = findQuantizedNoteOffTime(chord, note.offTime, basicQuant).first;
        if (offTime > maxOffTime) {
            maxOffTime = offTime;
        }
    }
    return maxOffTime;
}

ReducedFraction findOnTimeTupletQuantError(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const ReducedFraction& tupletLen,
    const ReducedFraction& tupletRatio,
    const ReducedFraction& rangeStart)
{
    const auto qOnTime = findQuantizedTupletChordOnTime(chord, tupletLen,
                                                        tupletRatio, rangeStart);
    return (chord.first - qOnTime).absValue();
}

ReducedFraction findOnTimeQuantError(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const ReducedFraction& basicQuant)
{
    const auto qOnTime = findQuantizedChordOnTime(chord, basicQuant);
    return (chord.first - qOnTime).absValue();
}

ReducedFraction findOffTimeTupletQuantError(
    const ReducedFraction& onTime,
    const ReducedFraction& offTime,
    const ReducedFraction& tupletLen,
    const ReducedFraction& tupletRatio,
    const ReducedFraction& rangeStart)
{
    const auto qOffTime = findQuantizedTupletNoteOffTime(onTime, offTime, tupletLen,
                                                         tupletRatio, rangeStart).first;
    return (offTime - qOffTime).absValue();
}

ReducedFraction findOffTimeQuantError(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const ReducedFraction& offTime,
    const ReducedFraction& basicQuant)
{
    const auto qOffTime = findQuantizedNoteOffTime(chord, offTime, basicQuant).first;
    return (offTime - qOffTime).absValue();
}

ReducedFraction findQuantForRange(
    const std::multimap<ReducedFraction, MidiChord>::const_iterator& beg,
    const std::multimap<ReducedFraction, MidiChord>::const_iterator& end,
    const ReducedFraction& basicQuant)
{
    const auto shortestLen = shortestQuantizedNoteInRange(beg, end);
    return quantForLen(shortestLen, basicQuant);
}

//--------------------------------------------------------------------------------------------

bool isHumanPerformance(
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const TimeSigMap* sigmap)
{
    if (chords.empty()) {
        return false;
    }

    const auto basicQuant = ReducedFraction::fromTicks(Constants::DIVISION) / 4;      // 1/16
    int matches = 0;
    int count = 0;

    std::set<ReducedFraction> usedOnTimes;

    for (const auto& chord: chords) {
        const auto quant = qMin(basicQuant,
                                quantForLen(MChord::maxNoteLen(chord), basicQuant));
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
            if (diff < MChord::minAllowedDuration()) {
                ++matches;
            }
        }
    }

    const double TOL = 0.6;
    const double matched = matches * 1.0 / count;

    return matched < TOL;
}

std::multimap<int, MTrack>
getTrackWithAllChords(const std::multimap<int, MTrack>& tracks)
{
    std::multimap<int, MTrack> singleTrack{ { 0, MTrack() } };
    auto& allChords = singleTrack.begin()->second.chords;
    for (const auto& track: tracks) {
        const MTrack& t = track.second;
        for (const auto& chord: t.chords) {
            allChords.insert(chord);
        }
    }
    return singleTrack;
}

void setIfHumanPerformance(
    const std::multimap<int, MTrack>& tracks,
    TimeSigMap* sigmap)
{
    auto allChordsTrack = getTrackWithAllChords(tracks);
    MChord::collectChords(allChordsTrack, { 2, 1 }, { 1, 2 });
    const MTrack& track = allChordsTrack.begin()->second;
    const auto& allChords = track.chords;
    if (allChords.empty()) {
        return;
    }
    const bool isHuman = isHumanPerformance(allChords, sigmap);
    auto& opers = midiImportOperations.data()->trackOpers;
    if (opers.isHumanPerformance.canRedefineDefaultLater()) {
        opers.isHumanPerformance.setDefaultValue(isHuman);
    }

    if (isHuman) {
        if (opers.quantValue.canRedefineDefaultLater()) {
            opers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_8);
        }
        if (opers.maxVoiceCount.canRedefineDefaultLater()) {
            opers.maxVoiceCount.setDefaultValue(MidiOperations::VoiceCount::V_2);
        }
        const double ticksPerSec = MidiTempo::findBasicTempo(tracks, true) * Constants::DIVISION;
        MidiBeat::findBeatLocations(allChords, sigmap, ticksPerSec);          // and set time sig
    }
}

//--------------------------------------------------------------------------------------------

// remove small intersection with the next chord

void preserveLegato(
    ReducedFraction& offTime,
    bool isOffTimeInTuplet,
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant)
{
    auto it = std::next(chordIt);
    while (it != chords.end() && it->second.voice != chordIt->second.voice) {
        ++it;
    }
    if (it != chords.end()) {
        const auto ioi = it->first - chordIt->first;
        const auto cross = offTime - it->first;

        if (it->second.isInTuplet && isOffTimeInTuplet) {
            // while we don't split tuplet into voices we don't want to have
            // note intersections smaller than tuplet note length
            const auto& tuplet = it->second.tuplet->second;
            const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
            if (cross > ReducedFraction(0, 1) && cross < tupletNoteLen) {
                offTime = it->first;
            }
        } else if (!it->second.isInTuplet && !isOffTimeInTuplet) {
            if (cross > ReducedFraction(0, 1) && cross < ioi / 2 && cross < basicQuant / 2) {
                offTime = it->first;
            }
        }
    }
}

std::pair<ReducedFraction, ReducedFraction>
quantizeOffTimeForTuplet(
    const ReducedFraction& noteOffTime,
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    const MidiTuplet::TupletData& tuplet)
{
    Q_ASSERT_X(chordIt->first < noteOffTime,
               "Quantize::quantizeOffTimeForTuplet", "Negative or zero note length");

    const auto tupletRatio = MidiTuplet::tupletLimits(tuplet.tupletNumber).ratio;
    const auto result = findQuantizedTupletNoteOffTime(
        chordIt->first, noteOffTime, tuplet.len, tupletRatio, tuplet.onTime);
    auto offTime = result.first;
    auto quant = result.second;

    preserveLegato(offTime, true, chordIt, chords, basicQuant);

    // verify that offTime is still inside tuplet
    if (offTime < tuplet.onTime) {
        offTime = tuplet.onTime;
        quant = tuplet.len;
    } else if (offTime > tuplet.onTime + tuplet.len) {
        offTime = tuplet.onTime + tuplet.len;
        quant = tuplet.len;
    }

    return { offTime, quant };
}

std::pair<ReducedFraction, ReducedFraction>
quantizeOffTimeForNonTuplet(
    const ReducedFraction& noteOffTime,
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant)
{
    Q_ASSERT_X(chordIt->first < noteOffTime,
               "Quantize::quantizeOffTimeForNonTuplet", "Negative or zero note length");

    const MidiChord& chord = chordIt->second;
    const auto result = findQuantizedNoteOffTime(*chordIt, noteOffTime, basicQuant);
    auto offTime = result.first;
    auto quant = result.second;
    preserveLegato(offTime, false, chordIt, chords, basicQuant);

    // verify that offTime is still outside tuplets
    if (chord.isInTuplet) {
        const auto& tuplet = chord.tuplet->second;
        if (offTime < tuplet.onTime + tuplet.len) {
            offTime = tuplet.onTime + tuplet.len;
            quant = tuplet.len;
            return { offTime, quant };
        }
    }
    auto next = std::next(chordIt);
    while (true) {
        if (next == chords.end()) {
            break;
        }
        if (!next->second.isInTuplet || next->second.voice != chord.voice) {
            ++next;
            continue;
        }
        const bool isInSameTuplet = chord.isInTuplet && next->second.tuplet == chord.tuplet;
        const auto& tuplet = next->second.tuplet->second;
        if (isInSameTuplet || tuplet.onTime + tuplet.len <= offTime) {
            ++next;
            continue;
        }

        if (offTime > tuplet.onTime) {
            offTime = tuplet.onTime;
            quant = tuplet.len;
        }
        break;
    }

    return { offTime, quant };
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
    std::multimap<ReducedFraction, MidiChord>::const_iterator chord;
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
    const std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator>& chords)
{
    auto it = chords.begin();
    const int voice = (*it)->second.voice;
    for (++it; it != chords.end(); ++it) {
        if ((*it)->second.voice != voice) {
            return false;
        }
    }
    return true;
}

bool areChordsDifferent(
    const std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator>& chords)
{
    std::set<const std::pair<const ReducedFraction, MidiChord>*> chordSet;

    for (const auto& chord: chords) {
        const auto it = chordSet.find(&*chord);
        if (it == chordSet.end()) {
            chordSet.insert(&*chord);
        } else {
            return false;
        }
    }

    return true;
}

bool notLessThanPrev(
    const std::vector<QuantData>::iterator& it,
    const std::vector<QuantData>& data)
{
    if (it != data.begin()) {
        const auto prev = std::prev(it);
        if (prev->chordRangeStart > it->chordRangeStart) {
            return false;
        }
    }
    return true;
}

bool isTupletRangeCorrect(
    const MidiTuplet::TupletData& tuplet,
    const ReducedFraction& rangeStart,
    const ReducedFraction& rangeEnd)
{
    return rangeStart == tuplet.onTime && rangeEnd == tuplet.onTime + tuplet.len;
}

void checkOffTime(
    const MidiNote& note,
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    const std::multimap<ReducedFraction, MidiChord>& chords)
{
    Q_ASSERT_X(note.offTime - chordIt->first >= MChord::minAllowedDuration(),
               "Quantize::checkOffTime", "Too small note length");

    if (note.isInTuplet) {
        const auto& tuplet = note.tuplet->second;

        Q_ASSERT_X(note.offTime >= tuplet.onTime
                   && note.offTime <= tuplet.onTime + tuplet.len,
                   "Quantize::checkOffTime",
                   "Note off time is outside tuplet but should be inside");
    } else {
        const auto& chord = chordIt->second;

        auto next = std::next(chordIt);
        while (true) {
            if (next == chords.end()) {
                break;
            }
            if (!next->second.isInTuplet || next->second.voice != chord.voice) {
                ++next;
                continue;
            }
            const auto& tuplet = next->second.tuplet->second;
            if ((chord.isInTuplet && next->second.tuplet == chord.tuplet)
                || tuplet.onTime + tuplet.len <= note.offTime) {
                ++next;
                continue;
            }

            Q_ASSERT_X(note.offTime <= tuplet.onTime,
                       "Quantize::checkOffTime",
                       "Note off time is inside next tuplet but it shouldn't");
            break;
        }

        auto prev = chordIt;
        while (true) {
            if (prev->second.barIndex != chord.barIndex) {
                break;
            }
            if (prev->second.voice != chord.voice
                || !prev->second.isInTuplet
                || (chord.isInTuplet && prev->second.tuplet == chord.tuplet)) {
                if (prev != chords.begin()) {
                    --prev;
                    continue;
                }
            } else {
                const auto& tuplet = prev->second.tuplet->second;

                Q_ASSERT_X(note.offTime >= tuplet.onTime + tuplet.len,
                           "Quantize::checkOffTime",
                           "Note off time is inside prev tuplet but it shouldn't");
            }
            break;
        }
    }
}

bool areOnTimeValuesDifferent(const std::multimap<ReducedFraction, MidiChord>& chords)
{
    std::set<std::tuple<ReducedFraction, int, int> > onTimeVoicesBars;
    for (const auto& chord: chords) {
        const auto tuple = std::make_tuple(chord.first,
                                           chord.second.voice,
                                           chord.second.barIndex);
        if (onTimeVoicesBars.find(tuple) == onTimeVoicesBars.end()) {
            onTimeVoicesBars.insert(tuple);
        } else {
            return false;
        }
    }
    return true;
}

bool areTupletChordsConsistent(const std::multimap<ReducedFraction, MidiChord>& chords)
{
    std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator prevTuplet;
    bool prevTupletSet = false;
    bool isInTuplet = false;
    const int limit = MidiVoice::voiceLimit();

    for (int voice = 0; voice != limit; ++voice) {
        for (const auto& chord: chords) {
            const MidiChord& c = chord.second;
            if (c.voice != voice) {
                continue;
            }
            if (c.isInTuplet) {
                if (!isInTuplet && prevTupletSet && c.tuplet == prevTuplet) {
                    LOGD() << "Inconsistent tuplets, bar (from 1):"
                           << (c.barIndex + 1);
                    return false;               // there is a non-tuplet chord inside tuplet
                }
                isInTuplet = true;
                prevTuplet = c.tuplet;
                prevTupletSet = true;
            } else {
                isInTuplet = false;
            }
        }
    }
    return true;
}

bool areTupletChordsConsistent(
    const std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator>& chords)
{
    auto it = chords.begin();
    const bool isInTuplet = (*it)->second.isInTuplet;
    for (it = std::next(it); it != chords.end(); ++it) {
        if (isInTuplet && (!(*it)->second.isInTuplet
                           || (*it)->second.tuplet != (*chords.begin())->second.tuplet)) {
            return false;
        }
        if (!isInTuplet && (*it)->second.isInTuplet) {
            return false;
        }
    }
    return true;
}

bool areChordsSortedByOnTime(
    const std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator>& chords)
{
    for (size_t i = 0; i != chords.size() - 1; ++i) {
        if (chords[i]->first >= chords[i + 1]->first) {
            return false;
        }
    }
    return true;
}

#endif

ReducedFraction quantizeToLarge(
    const ReducedFraction& time,
    const ReducedFraction& quant)
{
    const auto ratio = time / quant;
    auto quantized = quant * (ratio.numerator() / ratio.denominator());
    if (quantized < time) {
        quantized += quant;
    }
    return quantized;
}

ReducedFraction quantizeToSmall(
    const ReducedFraction& time,
    const ReducedFraction& quant)
{
    const auto ratio = time / quant;
    auto quantized = quant * (ratio.numerator() / ratio.denominator());
    if (quantized >= time) {
        quantized -= quant;
    }
    return quantized;
}

void findMetricalLevels(
    std::vector<QuantData>& data,
    const std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator>& chords,
    const ReducedFraction& tupletQuant,
    const ReducedFraction& barStart,
    const ReducedFraction& barFraction)
{
    const auto divsInfo = (tupletQuant != ReducedFraction(-1, 1))
                          ? Meter::divisionInfo(barFraction, { (*chords.begin())->second.tuplet->second })
                          : Meter::divisionInfo(barFraction, {});

    for (QuantData& d: data) {
        for (auto t = d.chordRangeStart; t <= d.chordRangeEnd; t += d.quant) {
            QuantPos p;
            p.time = t;
            p.metricalLevel = Meter::levelOfTick(t - barStart, divsInfo);
            d.positions.push_back(p);
        }

        int minLevel = std::numeric_limits<int>::max();
        while (true) {
            for (auto t = d.chordRangeStart; t <= d.chordRangeEnd; t += d.quant) {
                if (((t - barStart) / d.quantForLen).reduced().denominator() != 1) {
                    continue;
                }
                int level = Meter::levelOfTick(t - barStart, divsInfo);
                if (level < minLevel) {
                    minLevel = level;
                }
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
    std::vector<QuantData>& data,
    const ReducedFraction& rangeStart,
    const ReducedFraction& rangeEnd,
    const ReducedFraction& barStart,
    const ReducedFraction& beatLen)
{
#ifdef NDEBUG
    Q_UNUSED(rangeStart);
#endif
    for (auto it = data.rbegin(); it != data.rend(); ++it) {
        QuantData& d = *it;
        d.chordRangeEnd = barStart + quantizeToSmall(rangeEnd - barStart, d.quant);

        Q_ASSERT_X(d.chord->first + beatLen >= rangeStart,
                   "Quantize::findChordRangeEnds", "chord on time + beatLen < rangeStart");

        if (d.chord->first + beatLen < rangeEnd) {
            d.chordRangeEnd = barStart + quantizeToSmall(
                d.chord->first + beatLen - barStart, d.quant);
        }
        if (it != data.rbegin()) {
            const auto prev = std::prev(it);          // next in terms of time
            if (prev->chordRangeEnd < d.chordRangeEnd) {
                d.chordRangeEnd = barStart + quantizeToSmall(
                    prev->chordRangeEnd - barStart, d.quant);
            }
            if (!prev->canMergeWithPrev && d.chordRangeEnd == prev->chordRangeEnd) {
                d.chordRangeEnd -= d.quant;
            }
        }
        if (d.chordRangeEnd < d.chordRangeStart) {
            d.chordRangeEnd = d.chordRangeStart;
        }

        Q_ASSERT_X(d.chordRangeEnd < rangeEnd,
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
    std::vector<QuantData>& data,
    const ReducedFraction& rangeStart,
    const ReducedFraction& rangeEnd,
    const ReducedFraction& barStart,
    const ReducedFraction& beatLen)
{
    for (auto it = data.begin(); it != data.end(); ++it) {
        QuantData& d = *it;
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
                if (!d.canMergeWithPrev && d.chordRangeStart == prev->chordRangeStart) {
                    d.chordRangeStart += d.quant;
                }
            }

            if (d.chordRangeStart >= rangeEnd) {
                if (d.quant >= MChord::minAllowedDuration() * 2) {
                    d.quant /= 2;
                } else {
                    d.canMergeWithPrev = true;
                }
                continue;
            }
            break;
        }
#ifdef QT_DEBUG
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
#endif
    }
}

void findQuants(
    std::vector<QuantData>& data,
    const std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator>& chords,
    const ReducedFraction& rangeStart,
    const ReducedFraction& rangeEnd,
    const ReducedFraction& basicQuant,
    const ReducedFraction& tupletQuant,
    const ReducedFraction& barFraction)
{
    for (auto it = chords.begin(); it != chords.end(); ++it) {
        const auto chordIt = *it;
        QuantData d;
        d.chord = chordIt;
        auto len = MChord::minNoteLen(*chordIt);
        if (rangeEnd - rangeStart < len) {
            len = rangeEnd - rangeStart;
        }
        if (it != chords.begin()) {
            const auto prevChordIt = *std::prev(it);
            if (chordIt->first - prevChordIt->first < len) {
                len = chordIt->first - prevChordIt->first;
            }
            if (len < MChord::minAllowedDuration()) {
                d.canMergeWithPrev = true;
            }
        }
        if (tupletQuant != ReducedFraction(-1, 1)) {
            const MidiTuplet::TupletData& tuplet = (*chords.begin())->second.tuplet->second;
            d.quant = tupletQuant;
            d.quantForLen = tuplet.len / tuplet.tupletNumber;
        } else {
            d.quant = quantForLen(len, basicQuant);
            auto maxQuant = basicQuant;
            while (maxQuant < barFraction) {
                maxQuant *= 2;
            }
            d.quantForLen = quantForLen(
                qMin(MChord::minNoteLen(*chordIt), rangeEnd - rangeStart), maxQuant);
        }

        Q_ASSERT_X(d.quant <= rangeEnd - rangeStart,
                   "Quantize::findQuants", "Quant value is larger than range interval");

        data.push_back(d);
    }
}

ReducedFraction findTupletQuant(
    const std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator>& chords)
{
    ReducedFraction tupletQuant(-1, 1);
    if ((*chords.begin())->second.isInTuplet) {
        const MidiTuplet::TupletData& tuplet = (*chords.begin())->second.tuplet->second;
        const auto tupletRatio = MidiTuplet::tupletLimits(tuplet.tupletNumber).ratio;
        tupletQuant = quantForTuplet(tuplet.len, tupletRatio);
    }

    return tupletQuant;
}

std::vector<QuantData> findQuantData(
    const std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator>& chords,
    const ReducedFraction& rangeStart,
    const ReducedFraction& rangeEnd,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart,
    const ReducedFraction& barFraction)
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

struct QuantInfo {
    ReducedFraction onTime;
    double penalty = 0.0;
    std::multimap<ReducedFraction, MidiChord>::const_iterator chord;
};

int findLastChordPosition(const std::vector<QuantData>& quantData)
{
    int posIndex = -1;
    double minPenalty = std::numeric_limits<double>::max();
    const auto& lastPositions = quantData[quantData.size() - 1].positions;
    for (size_t i = 0; i != lastPositions.size(); ++i) {
        if (lastPositions[i].penalty < minPenalty) {
            minPenalty = lastPositions[i].penalty;
            posIndex = static_cast<int>(i);
        }
    }

    Q_ASSERT_X(posIndex != -1,
               "Quantize::findLastChordPosition", "Last index was not found");

    return posIndex;
}

void applyDynamicProgramming(std::vector<QuantData>& quantData)
{
    const auto& opers = midiImportOperations.data()->trackOpers;
    const bool isHuman = opers.isHumanPerformance.value();
    const double MERGE_PENALTY_COEFF = 5.0;

    for (size_t chordIndex = 0; chordIndex != quantData.size(); ++chordIndex) {
        QuantData& d = quantData[chordIndex];
        for (size_t pos = 0; pos != d.positions.size(); ++pos) {
            QuantPos& p = d.positions[pos];

            const auto timePenalty = (d.chord->first - p.time).absValue().toDouble();
            const double levelDiff = qAbs(d.metricalLevelForLen - p.metricalLevel);

            if (isHuman) {
                if (p.metricalLevel <= d.metricalLevelForLen) {
                    p.penalty = timePenalty + levelDiff * d.quantForLen.toDouble();
                } else {
                    p.penalty = timePenalty / (1 + levelDiff);
                }
            } else {
                if (p.metricalLevel <= d.metricalLevelForLen) {
                    p.penalty = timePenalty * (1 + levelDiff);
                } else {
                    p.penalty = timePenalty;
                }
            }

            if (chordIndex == 0) {
                continue;
            }

            const QuantData& dPrev = quantData[chordIndex - 1];
            double minPenalty = std::numeric_limits<double>::max();
            int minPos = -1;

            for (size_t posPrev = 0; posPrev != dPrev.positions.size(); ++posPrev) {
                const QuantPos& pPrev = dPrev.positions[posPrev];
                if (pPrev.time > p.time) {
                    continue;
                }

                double penalty = pPrev.penalty;
                if (pPrev.time == p.time) {
                    if (!d.canMergeWithPrev) {
                        continue;
                    }
                    penalty += d.quant.toDouble() * MERGE_PENALTY_COEFF;
                }

                if (penalty < minPenalty) {
                    minPenalty = penalty;
                    minPos = static_cast<int>(posPrev);
                }
            }

            Q_ASSERT_X(minPos != -1,
                       "Quantize::applyDynamicProgramming", "Min pos was not found");

            p.penalty += minPenalty;
            p.prevPos = minPos;
        }
    }
}

void quantizeOnTimesInRange(
    const std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator>& chords,
    std::map<const std::pair<const ReducedFraction, MidiChord>*, QuantInfo>& foundOnTimes,
    const ReducedFraction& rangeStart,
    const ReducedFraction& rangeEnd,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart,
    const ReducedFraction& barFraction)
{
#ifdef QT_DEBUG
    Q_ASSERT_X(!chords.empty(), "Quantize::quantizeOnTimesInRange", "Empty chords");
    Q_ASSERT_X(areAllVoicesSame(chords),
               "Quantize::quantizeOnTimesInRange", "Chord voices are not the same");
    Q_ASSERT_X(areChordsDifferent(chords),
               "Quantize::quantizeOnTimesInRange", "There are chord duplicates");
    Q_ASSERT_X(areTupletChordsConsistent(chords), "Quantize::quantizeOnTimesInRange",
               "Tuplet and non-tuplet chords mismatch");
    Q_ASSERT_X(areChordsSortedByOnTime(chords),
               "Quantize::quantizeOnTimesInRange", "Chords are not sorted by on time values");

    Q_ASSERT_X(rangeStart != ReducedFraction(-1, 1)
               && rangeEnd != ReducedFraction(-1, 1)
               && rangeStart < rangeEnd,
               "Quantize::quantizeOnTimesInRange",
               "Range start and/or range end are incorrect");

    Q_ASSERT_X((chords.front()->second.isInTuplet) ? isTupletRangeCorrect(
                   chords.front()->second.tuplet->second, rangeStart, rangeEnd) : true,
               "Quantize::quantizeOnTimesInRange", "Tuplet range is incorrect");
#endif
    std::vector<QuantData> quantData = findQuantData(chords, rangeStart, rangeEnd,
                                                     basicQuant, barStart, barFraction);
    applyDynamicProgramming(quantData);

    // backward dynamic programming step - collect optimal chord positions
    int posIndex = findLastChordPosition(quantData);

    Q_ASSERT_X(quantData.size() == chords.size(),
               "Quantize::quantizeOnTimesInRange",
               "Sizes of quant data and chords are not equal");

    for (size_t chordIndex = quantData.size() - 1;; --chordIndex) {
        const QuantPos& p = quantData[chordIndex].positions[posIndex];
        const auto onTime = p.time;
        const auto chordIt = chords[chordIndex];

        const auto found = foundOnTimes.find(&*chordIt);
        if (found == foundOnTimes.end()) {
            QuantInfo info;
            info.chord = chordIt;
            info.onTime = onTime;
            info.penalty = p.penalty;
            foundOnTimes.insert({ &*chordIt, info });
        } else if (p.penalty < found->second.penalty) {
            found->second.onTime = onTime;
            found->second.penalty = p.penalty;
        }

        if (chordIndex == 0) {
            break;
        }
        posIndex = p.prevPos;
    }
}

// input chords - sorted by onTime value

void applyTupletStaccato(std::multimap<ReducedFraction, MidiChord>& chords)
{
    for (auto chordIt = chords.begin(); chordIt != chords.end(); ++chordIt) {
        for (MidiNote& note: chordIt->second.notes) {
            if (note.isInTuplet && note.staccato) {
                const MidiTuplet::TupletData& tuplet = note.tuplet->second;
                // decrease tuplet error by enlarging staccato notes:
                // make note.len = tuplet note length
                const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
                note.offTime = chordIt->first + tupletNoteLen;
            }
        }
    }
}

void quantizeOffTimes(
    std::multimap<ReducedFraction, MidiChord>& quantizedChords,
    const ReducedFraction& basicQuant)
{
    for (auto chordIt = quantizedChords.begin(); chordIt != quantizedChords.end(); ++chordIt) {
        MidiChord& chord = chordIt->second;
        // quantize off times
        for (auto& note: chord.notes) {
            // check if note is not in tuplet anymore
            if (note.isInTuplet) {
                if (chordIt->first >= note.tuplet->first + note.tuplet->second.len) {
                    note.isInTuplet = false;
                }
            }

            const auto result = (note.isInTuplet)
                                ? quantizeOffTimeForTuplet(note.offTime, chordIt, quantizedChords,
                                                           basicQuant, note.tuplet->second)
                                : quantizeOffTimeForNonTuplet(note.offTime, chordIt,
                                                              quantizedChords, basicQuant);
            note.offTime = result.first;
            note.offTimeQuant = result.second;
#ifdef QT_DEBUG
            checkOffTime(note, chordIt, quantizedChords);
#endif
        }
    }
}

void addChordsFromPrevRange(
    std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator>& chordsToQuant,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    bool currentlyInTuplet,
    int currentBarIndex,
    int voice,
    const ReducedFraction& barStart,
    const ReducedFraction& rangeStart,
    const ReducedFraction& basicQuant)
{
    if (currentlyInTuplet) {
        return;
    }

    auto it = chordsToQuant.front();
    if (it == chords.begin()) {
        return;
    }
    --it;

    if (rangeStart == barStart) {         // new bar
        while (it->second.barIndex >= currentBarIndex - 1) {
            if (it->second.voice == voice && it->second.barIndex == currentBarIndex) {
                Q_ASSERT_X(!it->second.isInTuplet, "Quantize::addChordsFromPrevRange",
                           "Tuplet chord from previous bar belongs to the current bar");

                chordsToQuant.push_front(it);
            }
            if (it == chords.begin()) {
                break;
            }
            --it;
        }
    } else {
        const auto tol = basicQuant / 2;
        while (it->first > rangeStart - tol) {
            if (it->second.voice == voice && !it->second.isInTuplet) {
                chordsToQuant.push_front(it);
            }
            if (it == chords.begin()) {
                break;
            }
            --it;
        }
    }
}

void quantizeOnTimes(
    const std::multimap<ReducedFraction, MidiChord>& chords,
    std::map<const std::pair<const ReducedFraction, MidiChord>*, QuantInfo>& foundOnTimes,
    const ReducedFraction& basicQuant,
    const TimeSigMap* sigmap)
{
    int maxVoice = 0;
    for (int voice = 0; voice <= maxVoice; ++voice) {
        int currentBarIndex = -1;
        ReducedFraction rangeStart(-1, 1);
        ReducedFraction rangeEnd(-1, 1);
        ReducedFraction barFraction(-1, 1);
        ReducedFraction barStart(-1, 1);
        bool currentlyInTuplet = false;
        std::deque<std::multimap<ReducedFraction, MidiChord>::const_iterator> chordsToQuant;

        for (auto chordIt = chords.begin(); chordIt != chords.end(); ++chordIt) {
            Q_ASSERT_X(MChord::minNoteLen(*chordIt) >= MChord::minAllowedDuration(),
                       "Quantize::quantizeOnTimes",
                       "Note length is less than min allowed duration");

            if (chordIt->second.voice > maxVoice) {
                maxVoice = chordIt->second.voice;
            }
            if (chordIt->second.voice != voice) {
                continue;
            }

            if (chordsToQuant.empty()) {
                rangeStart = rangeEnd;
                currentlyInTuplet = chordIt->second.isInTuplet;
                if (currentBarIndex != chordIt->second.barIndex) {
                    currentBarIndex = chordIt->second.barIndex;
                    barStart = ReducedFraction::fromTicks(
                        sigmap->bar2tick(currentBarIndex, 0));
                    barFraction = ReducedFraction(sigmap->timesig(barStart.ticks()).timesig());
                    if (!currentlyInTuplet) {
                        rangeStart = barStart;
                    }
                }
                if (currentlyInTuplet) {
                    const auto& tuplet = chordIt->second.tuplet->second;
                    rangeStart = tuplet.onTime;
                    rangeEnd = tuplet.onTime + tuplet.len;
                }
            }

            chordsToQuant.push_back(chordIt);

            auto nextChord = std::next(chordIt);
            while (nextChord != chords.end() && nextChord->second.voice != voice) {
                ++nextChord;
            }
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
                        } else if (nextChord->second.isInTuplet) {
                            rangeEnd = nextChord->second.tuplet->second.onTime;
                        }
                    } else {
                        rangeEnd = ReducedFraction::fromTicks(
                            sigmap->bar2tick(currentBarIndex + 1, 0));
                    }
                }

                addChordsFromPrevRange(chordsToQuant, chords,
                                       currentlyInTuplet, currentBarIndex, voice,
                                       barStart, rangeStart, basicQuant);

                quantizeOnTimesInRange(chordsToQuant, foundOnTimes, rangeStart, rangeEnd,
                                       basicQuant, barStart, barFraction);
                chordsToQuant.clear();
            }
        }
    }
}

// if on time after quantization become >= note off time
// then shift note off time preserving old note length

void moveOffTimes(
    const ReducedFraction& oldOnTime,
    const ReducedFraction& newOnTime,
    QList<MidiNote>& notes)
{
    for (auto& note: notes) {
        if (newOnTime >= note.offTime) {
            Q_ASSERT_X(newOnTime > oldOnTime,
                       "Quantize::moveOffTimes", "Invalid note on time or off time");
            Q_ASSERT_X(note.offTime > oldOnTime,
                       "Quantize::moveOffTimes", "Invalid old note length");

            note.offTime += newOnTime - oldOnTime;
        }
    }
}

std::multimap<ReducedFraction, MidiChord>
findQuantizedChords(
    const std::map<const std::pair<const ReducedFraction, MidiChord>*, QuantInfo>& foundOnTimes)
{
    std::multimap<ReducedFraction, MidiChord> quantizedChords;
    for (const auto& f: foundOnTimes) {
        const QuantInfo& i = f.second;
        const MidiChord& chord = i.chord->second;

        auto found = quantizedChords.end();
        const auto range = quantizedChords.equal_range(i.onTime);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second.voice == chord.voice) {
                found = it;
                break;
            }
        }
        if (found != quantizedChords.end()) {
            MidiChord& fc = found->second;
            fc.notes.append(chord.notes);           // merge chords with equal on times
            moveOffTimes(i.chord->first, i.onTime, fc.notes);
            if (chord.isInTuplet) {
                if (!fc.isInTuplet) {
                    fc.isInTuplet = true;
                    fc.tuplet = chord.tuplet;
                }
#ifdef QT_DEBUG
                else {
                    Q_ASSERT_X(fc.tuplet == chord.tuplet,
                               "Quantize::findQuantizedChords",
                               "Tuplets of merged chords are different");
                }
#endif
            }
        } else {
            MidiChord midiChord(chord);        // copy chord
            moveOffTimes(i.chord->first, i.onTime, midiChord.notes);
            quantizedChords.insert({ i.onTime, midiChord });
        }
    }

    return quantizedChords;
}

void removeUselessTupletReferences(std::multimap<ReducedFraction, MidiChord>& chords)
{
    for (auto& chord: chords) {
        if (chord.second.isInTuplet) {
            const auto& tuplet = chord.second.tuplet->second;
            if (chord.first >= tuplet.onTime + tuplet.len) {
                chord.second.isInTuplet = false;
            }
        }
        for (auto& note: chord.second.notes) {
            if (note.isInTuplet) {
                const auto& tuplet = note.tuplet->second;
                if (note.offTime <= tuplet.onTime) {
                    note.isInTuplet = false;
                }
            }
        }
    }
}

void quantizeChords(
    std::multimap<ReducedFraction, MidiChord>& chords,
    const TimeSigMap* sigmap,
    const ReducedFraction& basicQuant)
{
#ifdef QT_DEBUG
    Q_ASSERT_X(MidiTuplet::areTupletReferencesValid(chords), "Quantize::quantizeChords",
               "Some tuplet references are invalid");
    Q_ASSERT_X(areOnTimeValuesDifferent(chords), "Quantize::quantizeChords",
               "Chords of the same voices have equal on time values");
    Q_ASSERT_X(areTupletChordsConsistent(chords), "Quantize::quantizeChords",
               "There are non-tuplet chords between tuplet chords");
    Q_ASSERT_X(MChord::areNotesLongEnough(chords), "Quantize::quantizeChords",
               "There are too short notes");
    Q_ASSERT_X(MChord::areBarIndexesSuccessive(chords), "Quantize::quantizeChords",
               "Bar indexes are not successive");
#endif
    applyTupletStaccato(chords);       // apply staccato for tuplet off times
    std::map<const std::pair<const ReducedFraction, MidiChord>*, QuantInfo> foundOnTimes;
    quantizeOnTimes(chords, foundOnTimes, basicQuant, sigmap);
    auto quantizedChords = findQuantizedChords(foundOnTimes);
#ifdef QT_DEBUG
    Q_ASSERT_X(MidiTuplet::areTupletReferencesValid(quantizedChords), "Quantize::quantizeChords",
               "Some tuplet references are invalid");
#endif
    quantizeOffTimes(quantizedChords, basicQuant);
    std::swap(chords, quantizedChords);

    removeUselessTupletReferences(chords);
}
} // namespace Quantize
} // namespace mu::iex::midi
