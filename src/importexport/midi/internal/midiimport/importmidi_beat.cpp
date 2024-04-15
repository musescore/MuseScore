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
#include "importmidi_beat.h"

#include <functional>
#include <cmath>

#include "importmidi_chord.h"
#include "importmidi_fraction.h"
#include "importmidi_inner.h"
#include "importmidi_quant.h"
#include "importmidi_meter.h"
#include "importmidi_tempo.h"
#include "importmidi_operations.h"
#include "thirdparty/beatroot/BeatTracker.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/sig.h"

namespace mu::iex::midi {
namespace MidiBeat {
int beatsInBar(const ReducedFraction& barFraction)
{
    const auto beatLen = Meter::beatLength(barFraction);
    const auto div = barFraction / beatLen;
    return div.numerator() / div.denominator();
}

double findChordSalience1(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    double ticksPerSec)
{
    ReducedFraction duration(0, 1);
    int pitch = std::numeric_limits<int>::max();
    int velocity = 0;

    for (const MidiNote& note: chord.second.notes) {
        if (note.offTime - chord.first > duration) {
            duration = note.offTime - chord.first;
        }
        if (note.pitch < pitch) {
            pitch = note.pitch;
        }
        velocity += note.velo;
    }
    const double durationInSeconds = duration.ticks() / ticksPerSec;

    const double c4 = 84;
    const int pmin = 48;
    const int pmax = 72;

    if (pitch < pmin) {
        pitch = pmin;
    } else if (pitch > pmax) {
        pitch = pmax;
    }

    if (velocity <= 0) {
        velocity = 1;
    }

    return durationInSeconds * (c4 - pitch) * log(velocity);
}

double findChordSalience2(
    const std::pair<const ReducedFraction, MidiChord>& chord, double)
{
    int velocity = 0;
    for (const MidiNote& note: chord.second.notes) {
        velocity += note.velo;
    }
    if (velocity <= 0) {
        velocity = 1;
    }

    return velocity;
}

::EventList prepareChordEvents(
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::function<double(const std::pair<const ReducedFraction, MidiChord>&,
                               double)>& findChordSalience,
    double ticksPerSec)
{
    ::EventList events;
    double minSalience = std::numeric_limits<double>::max();
    for (const auto& chord: chords) {
        ::Event e;
        e.time = chord.first.ticks() / ticksPerSec;
        e.salience = findChordSalience(chord, ticksPerSec);
        if (e.salience < minSalience) {
            minSalience = e.salience;
        }
        events.push_back(e);
    }
    // all saliences should be non-negative
    if (minSalience < 0) {
        for (auto& e: events) {
            e.salience -= minSalience;
        }
    }

    return events;
}

ReducedFraction findLastChordTick(const std::multimap<ReducedFraction, MidiChord>& chords)
{
    ReducedFraction lastOffTime(0, 1);
    for (const auto& chord: chords) {
        for (const auto& note: chord.second.notes) {
            if (note.offTime > lastOffTime) {
                lastOffTime = note.offTime;
            }
        }
    }
    return lastOffTime;
}

// first beat time can be larger than first chord onTime
// so insert additional beats at the beginning to cover all chords

void addFirstBeats(
    std::set<ReducedFraction>& beatSet,
    const ReducedFraction& firstTick,
    int beatsInBar,
    int& addedBeatCount)
{
    if (beatSet.empty()) {
        return;
    }

    addedBeatCount = 0;
    auto firstBeat = *beatSet.begin();
    if (firstTick < firstBeat) {
        if (beatSet.size() > 1) {
            const auto beatLen = *std::next(beatSet.begin()) - firstBeat;
            do {
                firstBeat -= beatLen;
                beatSet.insert(firstBeat);
                ++addedBeatCount;
            } while (firstBeat > firstTick || addedBeatCount % beatsInBar);
        }
    }
}

// last beat time can be smaller than the last chord onTime
// so insert additional beats at the end to cover all chords

void addLastBeats(
    std::set<ReducedFraction>& beatSet,
    const ReducedFraction& lastTick,
    int beatsInBar,
    int& addedBeatCount)
{
    if (beatSet.empty()) {
        return;
    }

    addedBeatCount = 0;
    // theoretically it's possible that every chord have off time
    // at the end of the piece - so check all chords for max off time
    auto lastBeat = *(std::prev(beatSet.end()));
    if (lastTick > lastBeat) {
        if (beatSet.size() > 1) {
            const auto beatLen = lastBeat - *std::prev(beatSet.end(), 2);
            do {
                lastBeat += beatLen;
                beatSet.insert(lastBeat);
                ++addedBeatCount;
            } while (lastBeat < lastTick || addedBeatCount % beatsInBar);
        }
    }
}

MidiOperations::HumanBeatData prepareHumanBeatData(
    const std::vector<double>& beatTimes,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    double ticksPerSec,
    int beatsInBar)
{
    MidiOperations::HumanBeatData beatData;
    if (chords.empty()) {
        return beatData;
    }

    for (const auto& beatTime: beatTimes) {
        beatData.beatSet.insert(MidiTempo::time2Tick(beatTime, ticksPerSec));
    }

    beatData.firstChordTick = chords.begin()->first;
    beatData.lastChordTick = findLastChordTick(chords);

    addFirstBeats(beatData.beatSet, beatData.firstChordTick,
                  beatsInBar, beatData.addedFirstBeats);
    addLastBeats(beatData.beatSet, beatData.lastChordTick,
                 beatsInBar, beatData.addedLastBeats);

    return beatData;
}

double findMatchRank(const std::set<ReducedFraction>& beatSet,
                     const ::EventList& events,
                     const std::vector<int>& levels,
                     int beatsInBar,
                     double ticksPerSec)
{
    std::map<ReducedFraction, double> saliences;
    for (const auto& e: events) {
        saliences.insert({ MidiTempo::time2Tick(e.time, ticksPerSec), e.salience });
    }
    std::vector<ReducedFraction> beatsOfBar;
    double matchFrac = 0;
    int matchCount = 0;
    int beatCount = 0;

    for (const auto& beat: beatSet) {
        beatsOfBar.push_back(beat);
        ++beatCount;
        if (beatCount == beatsInBar) {
            beatCount = 0;
            int relationCount = 0;
            int relationMatches = 0;
            for (size_t i = 0; i != beatsOfBar.size() - 1; ++i) {
                const auto s1 = saliences.find(beatsOfBar[i]);
                for (size_t j = i + 1; j != beatsOfBar.size(); ++j) {
                    ++relationCount;              // before s1 search check
                    if (s1 == saliences.end()) {
                        continue;
                    }
                    const auto s2 = saliences.find(beatsOfBar[j]);
                    if (s2 == saliences.end()) {
                        continue;
                    }
                    if ((s1->second < s2->second) == (levels[i] < levels[j])) {
                        ++relationMatches;
                    }
                }
            }
            if (relationCount) {
                matchFrac += relationMatches * 1.0 / relationCount;
                ++matchCount;
            }
            beatsOfBar.clear();
        }
    }
    if (matchCount) {
        matchFrac /= matchCount;
    }

    return matchFrac;
}

void removeEvery2ndBeat(std::set<ReducedFraction>& beatSet)
{
    auto it = beatSet.begin();
    while (it != beatSet.end() && std::next(it) != beatSet.end()) {
        it = beatSet.erase(std::next(it));
    }
    if (it == beatSet.end()) {
        // insert additional beat at the end to cover all chords
        const auto beatLen = *std::prev(it) - *std::prev(it, 2);
        beatSet.insert(*std::prev(it) + beatLen);
    }
}

// we can use ReducedFraction for time signature (bar fraction)
// because it reduces itself only if numerator or denominator
// is greater than some big number (see ReducedFraction class definition)

std::vector<ReducedFraction> findTimeSignatures(const ReducedFraction& timeSigFromMidiFile)
{
    std::vector<ReducedFraction> fractions{ ReducedFraction(4, 4), ReducedFraction(3, 4) };
    bool match = false;
    for (const ReducedFraction& f: fractions) {
        if (f.isIdenticalTo(timeSigFromMidiFile)) {
            match = true;
            break;
        }
    }
    if (!match) {       // some special time sig in MIDI file - use only it
        fractions.clear();
        fractions.push_back(timeSigFromMidiFile);
    }
    return fractions;
}

void setTimeSig(engraving::TimeSigMap* sigmap, const ReducedFraction& timeSig)
{
    sigmap->clear();
    sigmap->add(0, timeSig.fraction());
}

void findBeatLocations(
    const std::multimap<ReducedFraction, MidiChord>& allChords,
    engraving::TimeSigMap* sigmap,
    double ticksPerSec)
{
    const size_t MIN_BEAT_COUNT = 8;
    const auto barFractions = findTimeSignatures(ReducedFraction(sigmap->timesig(0).timesig()));
    const std::vector<
        std::function<double(const std::pair<const ReducedFraction, MidiChord>&, double)>
        >
    salienceFuncs = { findChordSalience1, findChordSalience2 };

    // <match rank, beat data, comparator>
    std::map<double, MidiOperations::HumanBeatData, std::greater<double> > beatResults;

    for (const auto& func: salienceFuncs) {
        const auto events = prepareChordEvents(allChords, func, ticksPerSec);
        const auto beatTimes = BeatTracker::beatTrack(events);
        if (beatTimes.size() <= MIN_BEAT_COUNT) {
            continue;
        }

        for (const ReducedFraction& barFraction: barFractions) {
            const auto beatLen = Meter::beatLength(barFraction);
            const auto div = barFraction / beatLen;
            const int beatsInBar = div.numerator() / div.denominator();

            const std::vector<Meter::DivisionInfo> divsInfo
                = { Meter::metricDivisionsOfBar(barFraction) };
            const auto levels = Meter::metricLevelsOfBar(barFraction, divsInfo, beatLen);

            Q_ASSERT_X((int)levels.size() == beatsInBar,
                       "MidiBeat::findBeatLocations", "Wrong count of bar levels");

            // beat set - first case
            MidiOperations::HumanBeatData beatData = prepareHumanBeatData(
                beatTimes, allChords, ticksPerSec, beatsInBar);
            beatData.timeSig = barFraction;
            const double matchRank = findMatchRank(beatData.beatSet, events,
                                                   levels, beatsInBar, ticksPerSec);
            beatResults.insert({ matchRank, beatData });
        }
    }

    auto* data = midiImportOperations.data();
    if (!beatResults.empty()) {
        const MidiOperations::HumanBeatData& beatData = beatResults.begin()->second;
        setTimeSig(sigmap, beatData.timeSig);
        data->humanBeatData = beatData;
        data->trackOpers.measureCount2xLess.setDefaultValue(beatData.measureCount2xLess);
        data->trackOpers.timeSigNumerator.setDefaultValue(
            Meter::fractionNumeratorToUserValue(beatData.timeSig.numerator()));
        data->trackOpers.timeSigDenominator.setDefaultValue(
            Meter::fractionDenominatorToUserValue(beatData.timeSig.denominator()));
    } else {
        const auto currentTimeSig = ReducedFraction(sigmap->timesig(0).timesig());
        data->trackOpers.timeSigNumerator.setDefaultValue(
            Meter::fractionNumeratorToUserValue(currentTimeSig.numerator()));
        data->trackOpers.timeSigDenominator.setDefaultValue(
            Meter::fractionDenominatorToUserValue(currentTimeSig.denominator()));
    }
}

void scaleOffTimes(
    QList<MidiNote>& notes,
    const std::set<ReducedFraction>& beats,
    const std::set<ReducedFraction>::const_iterator& onTimeBeatEndIt,
    const ReducedFraction& newOnTimeBeatStart,
    const ReducedFraction& newBeatLen)
{
    for (auto& note: notes) {
        int beatCount = 0;          // beat count between note on time and off time

        Q_ASSERT_X(onTimeBeatEndIt != beats.begin(),
                   "MidiBeat::scaleOffTimes",
                   "End beat iterator cannot be the first beat iterator");

        auto bStart = *std::prev(onTimeBeatEndIt);
        for (auto bit = onTimeBeatEndIt; bit != beats.end(); ++bit) {
            const auto& bEnd = *bit;

            Q_ASSERT_X(bEnd > bStart,
                       "MidiBeat::scaleOffTimes",
                       "Beat end <= beat start for note off time that is incorrect");

            if (note.offTime >= bStart && note.offTime < bEnd) {
                const auto scale = newBeatLen / (bEnd - bStart);
                auto newOffTimeInBeat = (note.offTime - bStart) * scale;
                newOffTimeInBeat = Quantize::quantizeValue(
                    newOffTimeInBeat, MChord::minAllowedDuration());
                const auto desiredBeatStart = newOnTimeBeatStart + newBeatLen * beatCount;
                note.offTime = desiredBeatStart + newOffTimeInBeat;
                break;
            }

            bStart = bEnd;
            ++beatCount;
        }
    }
}

void adjustChordsToBeats(std::multimap<int, MTrack>& tracks)
{
    const auto& opers = midiImportOperations;
    std::set<ReducedFraction> beats = opers.data()->humanBeatData.beatSet;    // copy
    if (beats.empty()) {
        return;
    }

    if (opers.data()->trackOpers.isHumanPerformance.value()) {
        if (opers.data()->trackOpers.measureCount2xLess.value()) {
            removeEvery2ndBeat(beats);
        }

        Q_ASSERT_X(beats.size() > 1, "MidiBeat::adjustChordsToBeats", "Human beat count < 2");

        const auto newBeatLen = ReducedFraction::fromTicks(engraving::Constants::DIVISION);

        for (auto trackIt = tracks.begin(); trackIt != tracks.end(); ++trackIt) {
            auto& chords = trackIt->second.chords;
            if (chords.empty()) {
                continue;
            }
            // do chord alignment according to recognized beats
            std::multimap<ReducedFraction, MidiChord> newChords;
            auto chordIt = chords.begin();
            auto it = beats.begin();
            auto beatStart = *it;
            auto newBeatStart = ReducedFraction(0, 1);

            for (++it; it != beats.end(); ++it) {
                const auto& beatEnd = *it;

                Q_ASSERT_X(beatEnd > beatStart, "MidiBeat::adjustChordsToBeats",
                           "Beat end <= beat start that is incorrect");

                const auto scale = newBeatLen / (beatEnd - beatStart);

                for (; chordIt != chords.end() && chordIt->first < beatEnd; ++chordIt) {
                    auto newOnTimeInBeat = (chordIt->first - beatStart) * scale;
                    // quantize to prevent ReducedFraction overflow
                    newOnTimeInBeat = Quantize::quantizeValue(
                        newOnTimeInBeat, MChord::minAllowedDuration());
                    scaleOffTimes(chordIt->second.notes, beats, it,
                                  newBeatStart, newBeatLen);
                    const auto newOnTime = newBeatStart + newOnTimeInBeat;
                    for (auto& note: chordIt->second.notes) {
                        if (note.offTime - newOnTime < MChord::minAllowedDuration()) {
                            note.offTime = newOnTime + MChord::minAllowedDuration();
                        }
                    }
                    newChords.insert({ newOnTime, chordIt->second });
                }

                if (chordIt == chords.end()) {
                    break;
                }

                beatStart = beatEnd;
                newBeatStart += newBeatLen;
            }

            std::swap(chords, newChords);
#ifdef QT_DEBUG
            Q_ASSERT_X(MChord::areNotesLongEnough(chords),
                       "MidiBeat::adjustChordsToBeats", "There are too short notes");
#endif
        }
    }
}

void updateFirstLastBeats(MidiOperations::HumanBeatData& beatData, const ReducedFraction& timeSig)
{
    for (int i = 0; i != beatData.addedFirstBeats; ++i) {
        Q_ASSERT_X(!beatData.beatSet.empty(), "MidiBeat::updateFirstLastBeats",
                   "Empty beat set after first beats deletion");

        beatData.beatSet.erase(beatData.beatSet.begin());
    }
    for (int i = 0; i != beatData.addedLastBeats; ++i) {
        Q_ASSERT_X(!beatData.beatSet.empty(), "MidiBeat::updateFirstLastBeats",
                   "Empty beat set after last beats deletion");

        beatData.beatSet.erase(std::prev(beatData.beatSet.end()));
    }

    const int beatsInBar = MidiBeat::beatsInBar(timeSig);

    MidiBeat::addFirstBeats(beatData.beatSet, beatData.firstChordTick,
                            beatsInBar, beatData.addedFirstBeats);
    MidiBeat::addLastBeats(beatData.beatSet, beatData.lastChordTick,
                           beatsInBar, beatData.addedLastBeats);
}

void setTimeSignature(engraving::TimeSigMap* sigmap)
{
    auto* data = midiImportOperations.data();
    const std::set<ReducedFraction>& beats = data->humanBeatData.beatSet;
    if (beats.empty()) {
        return;               // don't set time sig for non-human performed MIDI files
    }
    const auto timeSig = Meter::userTimeSigToFraction(data->trackOpers.timeSigNumerator.value(),
                                                      data->trackOpers.timeSigDenominator.value());
    setTimeSig(sigmap, timeSig);
    updateFirstLastBeats(data->humanBeatData, timeSig);
}
} // namespace MidiBeat
} // namespace mu::iex::midi
