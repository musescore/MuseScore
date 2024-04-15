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
#include "importmidi_tuplet.h"

#include <set>

#include "importmidi_tuplet_detect.h"
#include "importmidi_tuplet_filter.h"
#include "importmidi_tuplet_voice.h"
#include "importmidi_chord.h"
#include "importmidi_quant.h"
#include "importmidi_inner.h"
#include "importmidi_operations.h"
#include "engraving/dom/sig.h"

#include "log.h"

namespace mu::iex::midi {
namespace MidiTuplet {
const std::map<int, TupletLimits>& tupletsLimits()
{
    const static std::map<int, TupletLimits> values = {
        { 2, { { 2, 3 }, 1, 2, 2, 2 } },
        { 3, { { 3, 2 }, 1, 3, 3, 2 } },
        { 4, { { 4, 3 }, 3, 4, 3, 3 } },
        { 5, { { 5, 4 }, 3, 4, 4, 4 } },
        { 7, { { 7, 8 }, 4, 6, 5, 6 } },
        { 9, { { 9, 8 }, 6, 7, 7, 8 } }
    };
    return values;
}

const TupletLimits& tupletLimits(int tupletNumber)
{
    auto it = tupletsLimits().find(tupletNumber);

    Q_ASSERT_X(it != tupletsLimits().end(), "MidiTuplet::tupletValue", "Unknown tuplet");

    return it->second;
}

const TupletInfo& tupletFromId(int id, const std::vector<TupletInfo>& tuplets)
{
    auto it = std::find_if(tuplets.begin(), tuplets.end(),
                           [=](const TupletInfo& t) { return t.id == id; });

    Q_ASSERT_X(it != tuplets.end(), "MidiTuplet::tupletFromId", "Tuplet not found from id");

    return *it;
}

TupletInfo& tupletFromId(int id, std::vector<TupletInfo>& tuplets)
{
    return const_cast<TupletInfo&>(
        tupletFromId(id, const_cast<const std::vector<TupletInfo>&>(tuplets)));
}

bool hasNonTrivialChord(
    const ReducedFraction& chordOnTime,
    const QList<MidiNote>& notes,
    const ReducedFraction& tupletOnTime,
    const ReducedFraction& tupletLen)
{
    if (chordOnTime == tupletOnTime) {
        for (const auto& note: notes) {
            if (note.offTime - chordOnTime < tupletLen) {
                return true;
            }
        }
    } else {
        if (chordOnTime > tupletOnTime && chordOnTime < tupletOnTime + tupletLen) {
            return true;
        }
        if (chordOnTime >= tupletOnTime + tupletLen) {
            return false;
        }

        Q_ASSERT_X(chordOnTime < tupletOnTime, "MidiTuplet::hasNonTrivialChord",
                   "Chord on time was not compared correctly");

        for (const auto& note: notes) {
            if (note.offTime < tupletOnTime + tupletLen) {
                return true;
            }
        }
    }
    return false;
}

bool isTupletUseless(
    int voice,
    const ReducedFraction& onTime,
    const ReducedFraction& len,
    const ReducedFraction& maxChordLength,
    const std::multimap<ReducedFraction, MidiChord>& chords)
{
    bool haveIntersectionWithChord = false;
    const auto foundChords = MChord::findChordsForTimeRange(voice, onTime, onTime + len,
                                                            chords, maxChordLength);
    for (const auto& chordIt: foundChords) {
        // ok, tuplet contains at least one chord
        // check now does it have notes with len < tuplet.len
        if (hasNonTrivialChord(chordIt->first, chordIt->second.notes, onTime, len)) {
            haveIntersectionWithChord = true;
            break;
        }
    }

    return !haveIntersectionWithChord;
}

std::multimap<ReducedFraction, TupletData>::iterator
removeTuplet(
    const std::multimap<ReducedFraction, TupletData>::iterator& tupletIt,
    std::multimap<ReducedFraction, TupletData>& tuplets,
    const ReducedFraction& maxChordLength,
    std::multimap<ReducedFraction, MidiChord>& chords)
{
    // remove references to this tuplet in chords and notes
    auto chordIt = chords.lower_bound(tupletIt->second.onTime + tupletIt->second.len);
    if (chordIt != chords.begin()) {
        --chordIt;
        while (chordIt->first + maxChordLength > tupletIt->first) {
            MidiChord& c = chordIt->second;
            if (c.isInTuplet && c.tuplet == tupletIt) {
                c.isInTuplet = false;
            }
            for (auto& note: c.notes) {
                if (note.isInTuplet && note.tuplet == tupletIt) {
                    note.isInTuplet = false;
                }
            }
            if (chordIt == chords.begin()) {
                break;
            }
            --chordIt;
        }
    }

    return tuplets.erase(tupletIt);
}

std::multimap<ReducedFraction, TupletData>::iterator
removeTupletIfEmpty(
    const std::multimap<ReducedFraction, TupletData>::iterator& tupletIt,
    std::multimap<ReducedFraction, TupletData>& tuplets,
    const ReducedFraction& maxChordLength,
    std::multimap<ReducedFraction, MidiChord>& chords)
{
    auto resultIt = tupletIt;
    const auto& tuplet = tupletIt->second;

    if (isTupletUseless(tuplet.voice, tuplet.onTime, tuplet.len, maxChordLength, chords)) {
        resultIt = removeTuplet(tupletIt, tuplets, maxChordLength, chords);
    }

    return resultIt;
}

// tuplets with no chords are removed
// tuplets with single chord with chord.onTime = tuplet.onTime
//    and chord.len = tuplet.len are removed as well

// better to call this function after quantization

void removeEmptyTuplets(MTrack& track)
{
    auto& tuplets = track.tuplets;
    if (tuplets.empty()) {
        return;
    }
    auto& chords = track.chords;
    const ReducedFraction maxChordLength = MChord::findMaxChordLength(chords);

    for (auto tupletIt = tuplets.begin(); tupletIt != tuplets.end();) {
        const auto it = removeTupletIfEmpty(tupletIt, tuplets, maxChordLength, chords);
        if (it != tupletIt) {
            tupletIt = it;
            continue;
        }
        ++tupletIt;
    }
}

int averagePitch(const std::map<ReducedFraction,
                                std::multimap<ReducedFraction, MidiChord>::iterator>& chords)
{
    if (chords.empty()) {
        return -1;
    }

    int sumPitch = 0;
    int noteCounter = 0;
    for (const auto& chord: chords) {
        const auto& midiNotes = chord.second->second.notes;
        for (const auto& midiNote: midiNotes) {
            sumPitch += midiNote.pitch;
            ++noteCounter;
        }
    }

    return qRound(sumPitch * 1.0 / noteCounter);
}

void sortNotesByPitch(const std::multimap<ReducedFraction, MidiChord>::iterator& startBarChordIt,
                      const std::multimap<ReducedFraction, MidiChord>::iterator& endBarChordIt)
{
    struct {
        bool operator()(const MidiNote& n1, const MidiNote& n2)
        {
            return n1.pitch > n2.pitch;
        }
    } pitchComparator;

    for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
        auto& midiNotes = it->second.notes;
        std::sort(midiNotes.begin(), midiNotes.end(), pitchComparator);
    }
}

void sortTupletsByAveragePitch(std::vector<TupletInfo>& tuplets)
{
    struct {
        bool operator()(const TupletInfo& t1, const TupletInfo& t2)
        {
            return averagePitch(t1.chords) > averagePitch(t2.chords);
        }
    } averagePitchComparator;
    std::sort(tuplets.begin(), tuplets.end(), averagePitchComparator);
}

std::pair<ReducedFraction, ReducedFraction>
tupletInterval(const TupletInfo& tuplet,
               const ReducedFraction& basicQuant)
{
    ReducedFraction tupletEnd = tuplet.onTime + tuplet.len;

    for (const auto& chord: tuplet.chords) {
        const auto offTime = Quantize::findMaxQuantizedOffTime(*chord.second, basicQuant);
        if (offTime > tupletEnd) {
            tupletEnd = offTime;
        }
    }

    Q_ASSERT_X(tupletEnd > tuplet.onTime, "MidiTuplet::tupletInterval", "off time <= on time");

    return std::make_pair(tuplet.onTime, tupletEnd);
}

std::vector<std::pair<ReducedFraction, ReducedFraction> >
findTupletIntervals(const std::vector<TupletInfo>& tuplets,
                    const ReducedFraction& basicQuant)
{
    std::vector<std::pair<ReducedFraction, ReducedFraction> > tupletIntervals;
    for (const auto& tuplet: tuplets) {
        tupletIntervals.push_back(tupletInterval(tuplet, basicQuant));
    }

    return tupletIntervals;
}

// find tuplets over which duration lies

std::vector<TupletData>
findTupletsInBarForDuration(
    int voice,
    const ReducedFraction& barStartTick,
    const ReducedFraction& durationOnTime,
    const ReducedFraction& durationLen,
    const std::multimap<ReducedFraction, TupletData>& tupletEvents)
{
    std::vector<TupletData> tupletsData;
    if (tupletEvents.empty()) {
        return tupletsData;
    }
    auto tupletIt = tupletEvents.lower_bound(barStartTick);

    while (tupletIt != tupletEvents.end()
           && tupletIt->first < durationOnTime + durationLen) {
        if (tupletIt->second.voice == voice
            && durationOnTime < tupletIt->first + tupletIt->second.len) {
            // if tuplet and duration intersect each other
            auto tupletData = tupletIt->second;
            // convert tuplet onTime to local bar ticks
            tupletData.onTime -= barStartTick;
            tupletsData.push_back(tupletData);
        }
        ++tupletIt;
    }
    return tupletsData;
}

std::vector<std::multimap<ReducedFraction, TupletData>::const_iterator>
findTupletsForTimeRange(
    int voice,
    const ReducedFraction& onTime,
    const ReducedFraction& len,
    const std::multimap<ReducedFraction, TupletData>& tupletEvents,
    bool strictComparison)
{
    Q_ASSERT_X(len >= ReducedFraction(0, 1),
               "MidiTuplet::findTupletForTimeRange", "Negative length of the time range");

    std::vector<std::multimap<ReducedFraction, TupletData>::const_iterator> result;

    if (tupletEvents.empty()) {
        return result;
    }

    auto it = tupletEvents.upper_bound(onTime + len);
    if (it == tupletEvents.begin()) {
        return result;
    }
    --it;
    while (true) {
        const auto& tupletData = it->second;
        if (tupletData.voice == voice) {
            const auto interval = std::make_pair(onTime, onTime + len);
            const auto tupletInterval = std::make_pair(
                tupletData.onTime, tupletData.onTime + tupletData.len);
            if (haveIntersection(interval, tupletInterval, strictComparison)) {
                result.push_back(it);
            }
        }
        if (it == tupletEvents.begin()) {
            break;
        }
        --it;
    }
    return result;
}

std::multimap<ReducedFraction, TupletData>::const_iterator
findTupletContainingTime(
    int voice,
    const ReducedFraction& time,
    const std::multimap<ReducedFraction, TupletData>& tupletEvents,
    bool strictComparison)
{
    const auto tuplets = findTupletsForTimeRange(voice, time, ReducedFraction(0, 1),
                                                 tupletEvents, strictComparison);
    if (tuplets.empty()) {
        return tupletEvents.end();
    }

    Q_ASSERT_X(tuplets.size() == 1, "MidiTuplet::findTupletContainsTime",
               "More than one tuplet was found for time moment");

    return tuplets.front();
}

std::set<std::pair<const ReducedFraction, MidiChord>*>
findTupletChords(const std::vector<TupletInfo>& tuplets)
{
    std::set<std::pair<const ReducedFraction, MidiChord>*> tupletChords;
    for (const auto& tupletInfo: tuplets) {
        for (const auto& tupletChord: tupletInfo.chords) {
            auto tupletIt = tupletChord.second;
            tupletChords.insert(&*tupletIt);
        }
    }
    return tupletChords;
}

std::list<std::multimap<ReducedFraction, MidiChord>::iterator>
findNonTupletChords(
    const std::vector<TupletInfo>& tuplets,
    const std::multimap<ReducedFraction, MidiChord>::iterator& startBarChordIt,
    const std::multimap<ReducedFraction, MidiChord>::iterator& endBarChordIt)
{
    const auto tupletChords = findTupletChords(tuplets);
    std::list<std::multimap<ReducedFraction, MidiChord>::iterator> nonTuplets;
    for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
        if (tupletChords.find(&*it) == tupletChords.end()) {
            nonTuplets.push_back(it);
        }
    }

    return nonTuplets;
}

// split first tuplet chord, that belong to 2 tuplets, into 2 chords

void splitTupletChord(const std::vector<TupletInfo>::iterator& lastMatch,
                      std::multimap<ReducedFraction, MidiChord>& chords)
{
    auto& chordEvent = lastMatch->chords.begin()->second;
    MidiChord& prevChord = chordEvent->second;
    const auto onTime = chordEvent->first;
    MidiChord newChord = prevChord;
    // erase all notes except the first one
    auto beg = newChord.notes.begin();
    newChord.notes.erase(++beg, newChord.notes.end());
    // erase the first note
    prevChord.notes.erase(prevChord.notes.begin());
    chordEvent = chords.insert({ onTime, newChord });

    Q_ASSERT_X(!prevChord.notes.isEmpty(),
               "MidiTuplet::splitTupletChord",
               "Tuplets were not filtered correctly: same notes in different tuplets");
}

void splitFirstTupletChords(std::vector<TupletInfo>& tuplets,
                            std::multimap<ReducedFraction, MidiChord>& chords)
{
    for (auto now = tuplets.begin(); now != tuplets.end(); ++now) {
        auto lastMatch = tuplets.end();
        const auto nowChordIt = now->chords.begin();
        for (auto prev = tuplets.begin(); prev != now; ++prev) {
            auto prevChordIt = prev->chords.begin();
            if (now->firstChordIndex == 0
                && prev->firstChordIndex == 0
                && nowChordIt->second == prevChordIt->second) {
                lastMatch = prev;
            }
        }
        if (lastMatch != tuplets.end()) {
            splitTupletChord(lastMatch, chords);
        }
    }
}

// first tuplet notes with offTime quantization error,
// that is greater for tuplet quant rather than for regular quant,
// are removed from tuplet, except that was the last note

// if noteLen <= tupletLen
//     remove notes with big offTime quant error;
//     and here is a tuning for clearer tuplet processing:
//         if tuplet has only one (first) chord -
//             we can remove all notes and erase tuplet;
//         if tuplet has multiple chords -
//             we should leave at least one note in the first chord

void minimizeOffTimeError(
    std::vector<TupletInfo>& tuplets,
    std::multimap<ReducedFraction, MidiChord>& chords,
    std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets,
    const ReducedFraction& startBarTick,
    const ReducedFraction& basicQuant)
{
    for (auto it = tuplets.begin(); it != tuplets.end();) {
        TupletInfo& tupletInfo = *it;
        const auto firstChord = tupletInfo.chords.begin();
        if (firstChord == tupletInfo.chords.end() || tupletInfo.firstChordIndex != 0) {
            ++it;
            continue;
        }
        auto onTime = firstChord->second->first;
        if (onTime < startBarTick) {
            onTime = startBarTick;
        }
        MidiChord& midiChord = firstChord->second->second;
        auto& notes = midiChord.notes;

        std::vector<int> removedIndexes;
        std::vector<int> leavedIndexes;
        const auto tupletNoteLen = tupletInfo.len / tupletInfo.tupletNumber;

        for (int i = 0; i != notes.size(); ++i) {
            const auto& note = notes[i];
            if (note.offTime - onTime <= tupletInfo.len
                && note.offTime - onTime > tupletNoteLen) {
                // if note is longer than tuplet note length
                // then it's simpler to move it outside the tuplet
                // if the error is not larger than error inside tuplet
                if ((tupletInfo.chords.size() == 1
                     && notes.size() > (int)removedIndexes.size())
                    || (tupletInfo.chords.size() > 1
                        && notes.size() > (int)removedIndexes.size() + 1)) {
                    const auto tupletError = Quantize::findOffTimeTupletQuantError(
                        onTime, note.offTime, tupletInfo.len,
                        tupletLimits(tupletInfo.tupletNumber).ratio, startBarTick);
                    const auto regularError = Quantize::findOffTimeQuantError(
                        *firstChord->second, note.offTime, basicQuant);

                    if (tupletError >= regularError) {
                        removedIndexes.push_back(i);
                        continue;
                    }
                }
            }
            leavedIndexes.push_back(i);
        }
        if (!removedIndexes.empty()) {
            MidiChord newTupletChord = midiChord;
            newTupletChord.notes.clear();
            for (int i: leavedIndexes) {
                newTupletChord.notes.push_back(notes[i]);
            }

            QList<MidiNote> newNotes;
            for (int i: removedIndexes) {
                newNotes.push_back(notes[i]);
            }
            notes = newNotes;
            // force add chord to this bar, even if it has barIndex of another bar
            nonTuplets.push_back(firstChord->second);
            if (!newTupletChord.notes.empty()) {
                firstChord->second = chords.insert({ firstChord->second->first,
                                                     newTupletChord });
            } else {
                tupletInfo.chords.erase(tupletInfo.chords.begin());
                if (tupletInfo.chords.empty()) {
                    it = tuplets.erase(it);             // remove tuplet without chords
                    continue;
                }
            }
        }
        ++it;
    }
}

void addChordsBetweenTupletNotes(
    std::vector<TupletInfo>& tuplets,
    std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& startBarTick,
    const ReducedFraction& basicQuant)
{
    for (TupletInfo& tuplet: tuplets) {
        for (auto it = nonTuplets.begin(); it != nonTuplets.end();) {
            const auto& chordIt = *it;
            const auto& onTime = chordIt->first;
            const auto tupletInterv = std::make_pair(tuplet.onTime,
                                                     tuplet.onTime + tuplet.len);
            const auto chordInterv = chordInterval(*chordIt, chords,
                                                   basicQuant, startBarTick);
            if (onTime > tuplet.onTime && haveIntersection(tupletInterv, chordInterv)) {
                const auto tupletRatio = tupletLimits(tuplet.tupletNumber).ratio;

                auto tupletError = Quantize::findOnTimeTupletQuantError(
                    *chordIt, tuplet.len, tupletRatio, startBarTick);
                auto regularError = Quantize::findOnTimeQuantError(*chordIt, basicQuant);

                const auto offTime = MChord::maxNoteOffTime(chordIt->second.notes);
                if (offTime < tuplet.onTime + tuplet.len) {
                    tupletError += Quantize::findOffTimeTupletQuantError(
                        onTime, offTime, tuplet.len, tupletRatio, startBarTick);
                    regularError += Quantize::findOffTimeQuantError(
                        *chordIt, offTime, basicQuant);
                }
                if (tupletError < regularError) {
                    tuplet.chords.insert({ onTime, chordIt });
                    it = nonTuplets.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}

#ifdef QT_DEBUG

bool doTupletsHaveCommonChords(const std::vector<TupletInfo>& tuplets)
{
    if (tuplets.empty()) {
        return false;
    }
    std::set<std::pair<const ReducedFraction, MidiChord>*> chordsI;
    for (const auto& tuplet: tuplets) {
        for (const auto& chord: tuplet.chords) {
            if (chordsI.find(&*chord.second) != chordsI.end()) {
                return true;
            }
            chordsI.insert(&*chord.second);
        }
    }
    return false;
}

// check - do all tuplets have references from some chords

bool checkTupletsForExistence(
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::multimap<ReducedFraction, TupletData>& tupletEvents)
{
    // check - do all tuplets have references from some chords
    std::map<const std::pair<const ReducedFraction, TupletData>*, bool> tupletMap;
    for (const auto& tuplet: tupletEvents) {
        tupletMap.insert({ &tuplet, false });
    }

    for (const auto& chord: chords) {
        const MidiChord& c = chord.second;
        if (c.isInTuplet) {
            const auto it = tupletMap.find(&*c.tuplet);
            if (it == tupletMap.end()) {
                LOGD() << "Chords have references to non-existing tuplets";
                return false;
            }
        }
        for (const auto& note: c.notes) {
            if (note.isInTuplet) {
                const auto it = tupletMap.find(&*note.tuplet);
                if (it == tupletMap.end()) {
                    LOGD() << "Notes have references to non-existing tuplets";
                    return false;
                }
            }
        }
    }
    return true;
}

// check - do all tuplets have references from some chords

bool checkForDanglingTuplets(
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::multimap<ReducedFraction, TupletData>& tupletEvents)
{
    const ReducedFraction maxChordLength = MChord::findMaxChordLength(chords);

    for (auto tupletIt = tupletEvents.begin(); tupletIt != tupletEvents.end(); ++tupletIt) {
        const auto& tuplet = tupletIt->second;
        bool hasReference = false;
        auto chordIt = chords.lower_bound(tuplet.onTime + tuplet.len);

        if (chordIt != chords.begin()) {
            --chordIt;
            while (chordIt->first + maxChordLength > tupletIt->first) {
                const MidiChord& c = chordIt->second;
                if (c.isInTuplet && c.tuplet == tupletIt) {
                    hasReference = true;
                    break;
                }
                for (const auto& note: c.notes) {
                    if (note.isInTuplet && note.tuplet == tupletIt) {
                        hasReference = true;
                        break;
                    }
                }
                if (hasReference || chordIt == chords.begin()) {
                    break;
                }
                --chordIt;
            }
        }

        if (!hasReference) {
            LOGD() << "Not all tuplets have references in chords - "
                      "there are dangling tuplets";
            return false;
        }
    }
    return true;
}

// check tuplet events for uniqueness

bool checkAreTupletsUnique(const std::multimap<ReducedFraction, TupletData>& tupletEvents)
{
    std::set<std::pair<ReducedFraction, int> > referencedTuplets;       // <onTime, voice>
    for (const auto& tuplet: tupletEvents) {
        const auto& t = tuplet.second;
        const auto result = referencedTuplets.insert({ t.onTime, t.voice });

        if (!result.second) {
            LOGD() << "Not unique tuplets in tupletEvents";
            return false;
        }
    }
    return true;
}

// check is referenced tuplet count equal to tuplet event count

bool checkForEqualTupletCount(
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::multimap<ReducedFraction, TupletData>& tupletEvents)
{
    std::set<std::pair<ReducedFraction, int> > referencedTuplets;       // <onTime, voice>
    for (const auto& chord: chords) {
        if (chord.second.isInTuplet) {
            const auto& tuplet = chord.second.tuplet->second;
            referencedTuplets.insert({ tuplet.onTime, tuplet.voice });
        }
        for (const auto& note: chord.second.notes) {
            if (note.isInTuplet) {
                const auto& tuplet = note.tuplet->second;
                referencedTuplets.insert({ tuplet.onTime, tuplet.voice });
            }
        }
    }

    if (referencedTuplets.size() < tupletEvents.size()) {
        LOGD() << "Referenced tuplets count ("
               << referencedTuplets.size()
               << ") < tuplet events count ("
               << tupletEvents.size() << ")";
    }
    if (referencedTuplets.size() > tupletEvents.size()) {
        LOGD() << "Referenced tuplets count ("
               << referencedTuplets.size()
               << ") > tuplet events count ("
               << tupletEvents.size() << ")";
    }

    return tupletEvents.size() == referencedTuplets.size();
}

bool areAllTupletsReferenced(
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::multimap<ReducedFraction, TupletData>& tupletEvents)
{
    if (!checkAreTupletsUnique(tupletEvents)) {
        return false;
    }
    if (!checkTupletsForExistence(chords, tupletEvents)) {
        return false;
    }
    if (!checkForDanglingTuplets(chords, tupletEvents)) {
        return false;
    }
    if (!checkForEqualTupletCount(chords, tupletEvents)) {
        return false;
    }
    return true;
}

// this check is not full but often if use invalid iterator for comparison
// it causes crash or something

bool areTupletReferencesValid(const std::multimap<ReducedFraction, MidiChord>& chords)
{
    for (const auto& chord: chords) {
        const MidiChord& c = chord.second;
        if (c.isInTuplet) {
            const auto& tuplet = c.tuplet->second;
            if (tuplet.onTime < ReducedFraction(0, 1)
                || tuplet.len < ReducedFraction(0, 1)) {
                return false;
            }
        }
        for (const auto& note: c.notes) {
            if (note.isInTuplet) {
                const auto& tuplet = note.tuplet->second;
                if (tuplet.onTime < ReducedFraction(0, 1)
                    || tuplet.len < ReducedFraction(0, 1)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool areTupletNonTupletChordsDistinct(
    const std::vector<TupletInfo>& tuplets,
    const std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets)
{
    std::set<std::pair<const ReducedFraction, MidiChord>*> chords;
    for (const TupletInfo& tuplet: tuplets) {
        for (const auto& chord: tuplet.chords) {
            chords.insert(&*chord.second);
        }
    }
    for (const auto& nonTuplet: nonTuplets) {
        const auto it = chords.find(&*nonTuplet);
        if (it != chords.end()) {
            return false;
        }
    }
    return true;
}

bool isTupletRangeOk(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets)
{
    const MidiChord& c = chord.second;
    const auto foundTuplets = findTupletsForTimeRange(
        c.voice, chord.first, ReducedFraction(0, 1), tuplets, false);
    if (c.isInTuplet && foundTuplets.empty()) {
        LOGD() << "Tuplet chord is actually outside tuplets, "
                  "bar number (from 1):" << (c.barIndex + 1);
        return false;
    }
    if (!c.isInTuplet && !foundTuplets.empty()) {
        // chord can touch the tuplet at the end and doesn't belong to it
        for (const auto& t: foundTuplets) {
            if (chord.first != t->second.onTime + t->second.len) {
                LOGD() << "Non-tuplet chord is actually inside tuplet, "
                          "bar number (from 1):" << (c.barIndex + 1);
                return false;
            }
        }
    }
    for (const auto& note: c.notes) {
        const auto foundTuplets1 = findTupletsForTimeRange(
            c.voice, note.offTime, ReducedFraction(0, 1), tuplets, false);
        if (note.isInTuplet && foundTuplets1.empty()) {
            LOGD() << "Tuplet note off time is actually outside tuplets, "
                      "bar number (from 1):" << (c.barIndex + 1);
            return false;
        }
        if (!note.isInTuplet && !foundTuplets1.empty()) {
            // note off time can touch the tuplet
            // at the beg/end and doesn't belong to it
            for (const auto& t: foundTuplets1) {
                if (note.offTime != t->second.onTime
                    && note.offTime != t->second.onTime + t->second.len) {
                    LOGD() << "Non-tuplet note off time is actually inside tuplet, "
                              "bar number (from 1):" << (c.barIndex + 1);
                    return false;
                }
            }
        }
    }
    return true;
}

// should be called after quantization

bool areTupletRangesOk(
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::multimap<ReducedFraction, TupletData>& tuplets)
{
    for (const auto& chord: chords) {
        if (!isTupletRangeOk(chord, tuplets)) {
            return false;
        }
    }
    return true;
}

bool areAllTupletsDifferent(const std::multimap<ReducedFraction, TupletData>& tuplets)
{
    std::set<std::pair<ReducedFraction, int> > tupletsSet;        // <on time, voice>
    for (const auto& tuplet: tuplets) {
        const auto result = tupletsSet.insert({ tuplet.first, tuplet.second.voice });
        const bool isAlreadyInSet = !result.second;
        if (isAlreadyInSet) {
            return false;
        }
    }
    return true;
}

#endif

void addTupletEvents(std::multimap<ReducedFraction, TupletData>& tupletEvents,
                     const std::vector<TupletInfo>& tuplets,
                     const std::list<TiedTuplet>& backTiedTuplets)
{
    for (size_t i = 0; i != tuplets.size(); ++i) {
        const auto& tupletInfo = tuplets[i];
        TupletData tupletData = {
            tupletInfo.chords.begin()->second->second.voice,
            tupletInfo.onTime,
            tupletInfo.len,
            tupletInfo.tupletNumber,
            {}
        };

        const auto it = tupletEvents.insert({ tupletData.onTime, tupletData });
        for (auto& chord: tupletInfo.chords) {
            MidiChord& midiChord = chord.second->second;
            midiChord.tuplet = it;

            Q_ASSERT_X(!midiChord.isInTuplet,
                       "MidiTuplet::addTupletEvents",
                       "Chord is already in tuplet but it shouldn't");

            midiChord.isInTuplet = true;
        }

        for (const TiedTuplet& tiedTuplet: backTiedTuplets) {
            if (tiedTuplet.tupletId == tupletInfo.id) {
                MidiChord& midiChord = tiedTuplet.chord->second;

#ifdef QT_DEBUG
                QString message = "Tied tuplet and tied chord have different voices, "
                                  "tuplet voice = ";
                message += QString::number(tiedTuplet.voice) + ", chord voice = ";
                message += QString::number(midiChord.voice) + ", bar number (from 1) = ";
                message += QString::number(midiChord.barIndex + 1);
#endif
                Q_ASSERT_X(tiedTuplet.voice == midiChord.voice,
                           "MidiTuplet::addTupletEvents", message.toLatin1().data());

                for (int j: tiedTuplet.tiedNoteIndexes) {
                    midiChord.notes[j].tuplet = it;
                    midiChord.notes[j].isInTuplet = true;
                }
                break;
            }
        }

        for (auto& chord: tupletInfo.chords) {
            MidiChord& midiChord = chord.second->second;
            for (auto& note: midiChord.notes) {
                if (note.offTime <= tupletInfo.onTime + tupletInfo.len) {
                    Q_ASSERT_X(!note.isInTuplet,
                               "MidiTuplet::addTupletEvents",
                               "Note is already in tuplet but it shouldn't");

                    note.tuplet = it;
                    note.isInTuplet = true;
                }
            }
        }
    }
}

void markStaccatoTupletNotes(std::vector<TupletInfo>& tuplets)
{
    for (auto& tuplet: tuplets) {
        for (const auto& staccato: tuplet.staccatoChords) {
            const auto it = tuplet.chords.find(staccato.first);
            if (it != tuplet.chords.end()) {
                MidiChord& midiChord = it->second->second;
                midiChord.notes[staccato.second].staccato = true;
            }
        }
        tuplet.staccatoChords.clear();
    }
}

// if notes with staccato were in tuplets previously - remove staccato

void cleanStaccatoOfNonTuplets(
    std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets)
{
    for (auto& nonTuplet: nonTuplets) {
        for (auto& note: nonTuplet->second.notes) {
            if (note.staccato) {
                note.staccato = false;
            }
        }
    }
}

// chord on time shouldn't go before previous chord on time or after next chord on time
// this function finds such on time value between previous and next chords

ReducedFraction findOnTimeBetweenChords(
    const std::pair<const ReducedFraction, MidiChord>& chord,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart)
{
    ReducedFraction onTime(-1, 1);
    auto quant = basicQuant;

    const auto range = chords.equal_range(chord.first);
    auto chordIt = chords.end();

    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.voice == chord.second.voice) {
            chordIt = it;
            break;
        }
    }

    Q_ASSERT_X(chordIt != chords.end(),
               "MidiTuplet::findOnTimeBetweenChords", "Chord iterator was not found");

    if (chordIt->first < barStart) {
        return barStart;
    }

    // chords with equal voices here can have equal on time values
    // so skip such chords
    const int voice = chordIt->second.voice;
    while (true) {
        onTime = Quantize::findMinQuantizedOnTime(*chordIt, quant);
        bool changed = false;

        if (chordIt != chords.begin()) {
            auto it = std::prev(chordIt);
            while (true) {
                if (it->first < chord.first && it->second.voice == voice) {
                    const auto prevChordOnTime
                        = Quantize::findMinQuantizedOnTime(*it, quant);
                    if (onTime < prevChordOnTime) {
                        Q_ASSERT_X(quant >= MChord::minAllowedDuration() * 2,
                                   "MidiTuplet::findOnTimeBetweenChords",
                                   "Too small quantization value");

                        quant /= 2;
                        changed = true;
                    }
                    break;
                }
                if (it == chords.begin() || it->first < chord.first - basicQuant * 2) {
                    break;
                }
                --it;
            }
        }

        if (!changed) {
            for (auto it = std::next(chordIt);
                 it != chords.end() && it->first < chord.first + basicQuant * 2; ++it) {
                if (it->first == chord.first || it->second.voice != voice) {
                    continue;
                }
                const auto nextChordOnTime = Quantize::findMinQuantizedOnTime(*it, quant);
                if (onTime > nextChordOnTime) {
                    Q_ASSERT_X(quant >= MChord::minAllowedDuration() * 2,
                               "MidiTuplet::findOnTimeBetweenChords",
                               "Too small quantization value");

                    quant /= 2;
                    changed = true;
                }
                break;
            }
        }

        if (!changed) {
            break;
        }
    }

    Q_ASSERT_X(onTime != ReducedFraction(-1, 1), "MidiTuplet::findOnTimeBetweenChords",
               "On time for chord interval was not found");

    return onTime;
}

ReducedFraction findPrevBarStart(const ReducedFraction& barStart,
                                 const ReducedFraction& barLen)
{
    auto prevBarStart = barStart - barLen;
    if (prevBarStart < ReducedFraction(0, 1)) {
        prevBarStart = ReducedFraction(0, 1);
    }
    return prevBarStart;
}

void setBarIndexesOfNextBarChords(
    std::vector<TupletInfo>& tuplets,
    std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets,
    int barIndex)
{
    for (auto& tuplet: tuplets) {
        for (auto& chord: tuplet.chords) {
            if (chord.second->second.barIndex > barIndex) {
                chord.second->second.barIndex = barIndex;
            }
        }
    }
    for (auto& chord: nonTuplets) {
        if (chord->second.barIndex > barIndex) {
            chord->second.barIndex = barIndex;
        }
    }
}

// indexes of each new bar should be -1 except possibly chords at the end of prev bar

void findTuplets(
    const std::multimap<ReducedFraction, MidiChord>::iterator& startBarChordIt,
    const std::multimap<ReducedFraction, MidiChord>::iterator& endBarChordIt,
    std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    std::multimap<ReducedFraction, TupletData>& tupletEvents,
    const engraving::TimeSigMap* sigmap,
    int barIndex)
{
    if (chords.empty() || startBarChordIt == endBarChordIt) {
        return;
    }

    const auto& opers = midiImportOperations.data()->trackOpers;
    const int currentTrack = midiImportOperations.currentTrack();
    if (!opers.searchTuplets.value(currentTrack)) {
        return;
    }

    const auto startBarTick = ReducedFraction::fromTicks(
        sigmap->bar2tick(startBarChordIt->second.barIndex, 0));
    const auto endBarTick = ReducedFraction::fromTicks(
        sigmap->bar2tick(startBarChordIt->second.barIndex + 1, 0));

    const auto barFraction = ReducedFraction(sigmap->timesig(startBarTick.ticks()).timesig());
    std::vector<TupletInfo> tuplets = detectTuplets(startBarChordIt, endBarChordIt, startBarTick,
                                                    barFraction, chords, basicQuant, barIndex);
    if (tuplets.empty()) {
        return;
    }

    filterTuplets(tuplets, basicQuant);
    // later notes will be sorted and their indexes become invalid
    // so assign staccato information to notes now
    if (opers.simplifyDurations.value(currentTrack)) {
        markStaccatoTupletNotes(tuplets);
    }

    auto nonTuplets = findNonTupletChords(tuplets, startBarChordIt, endBarChordIt);
    addChordsBetweenTupletNotes(tuplets, nonTuplets, chords, startBarTick, basicQuant);
    sortNotesByPitch(startBarChordIt, endBarChordIt);
    sortTupletsByAveragePitch(tuplets);

    if (tupletVoiceLimit() > 1) {
        splitFirstTupletChords(tuplets, chords);
        minimizeOffTimeError(tuplets, chords, nonTuplets, startBarTick, basicQuant);
    }

    if (opers.simplifyDurations.value(currentTrack)) {
        cleanStaccatoOfNonTuplets(nonTuplets);
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(!doTupletsHaveCommonChords(tuplets),
               "MIDI tuplets: findTuplets", "Tuplets have common chords but they shouldn't");
#endif
    const auto prevBarStart = findPrevBarStart(startBarTick, endBarTick - startBarTick);
    auto backTiedTuplets = findBackTiedTuplets(chords, tuplets, prevBarStart, startBarTick,
                                               basicQuant, startBarChordIt->second.barIndex);
    // backTiedTuplets can be changed here (incompatible are removed)
    assignVoices(tuplets, nonTuplets, backTiedTuplets, chords, basicQuant,
                 startBarTick, barIndex);
#ifdef QT_DEBUG
    Q_ASSERT_X(areTupletNonTupletChordsDistinct(tuplets, nonTuplets),
               "MIDI tuplets: findTuplets", "Tuplets have common chords with non-tuplets");
#endif
    addTupletEvents(tupletEvents, tuplets, backTiedTuplets);
    setBarIndexesOfNextBarChords(tuplets, nonTuplets, barIndex);
}

void setAllTupletOffTimes(
    std::multimap<ReducedFraction, TupletData>& tupletEvents,
    std::multimap<ReducedFraction, MidiChord>& chords,
    const engraving::TimeSigMap* sigmap)
{
    for (auto& chordEvent: chords) {
        MidiChord& chord = chordEvent.second;
        for (MidiNote& note: chord.notes) {
            if (note.isInTuplet) {
                continue;
            }
            const auto barEnd = ReducedFraction::fromTicks(
                sigmap->bar2tick(chord.barIndex + 1, 0));
            if (note.offTime > barEnd) {
                const auto it = findTupletContainingTime(
                    chord.voice, note.offTime, tupletEvents, true);
                if (it != tupletEvents.end()) {
                    note.isInTuplet = true;
                    // hack to remove constness of iterator
                    note.tuplet = tupletEvents.erase(it, it);
                }
            }
        }
    }
}

void findAllTuplets(
    std::multimap<ReducedFraction, TupletData>& tuplets,
    std::multimap<ReducedFraction, MidiChord>& chords,
    const engraving::TimeSigMap* sigmap,
    const ReducedFraction& basicQuant)
{
    if (chords.empty()) {
        return;
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(MChord::areNotesLongEnough(chords),
               "MidiTuplet::findAllTuplets", "There are too short notes");
    Q_ASSERT_X(MChord::areBarIndexesSet(chords),
               "MidiTuplet::findAllTuplets", "Not all bar indexes were set");
    Q_ASSERT_X(MChord::areBarIndexesSuccessive(chords),
               "MidiTuplet::findAllTuplets", "Bar indexes are not successive");
#endif
    {
        auto startBarIt = chords.begin();
        for (auto endBarIt = std::next(startBarIt); endBarIt != chords.end(); ++endBarIt) {
            Q_ASSERT_X(endBarIt->second.barIndex >= startBarIt->second.barIndex,
                       "MidiTuplet::findAllTuplets", "Bar indexes are not successive");

            const int currentBarIndex = startBarIt->second.barIndex;
            if (endBarIt->second.barIndex > currentBarIndex) {
                const size_t oldTupletCount = tuplets.size();
                findTuplets(startBarIt, endBarIt, chords, basicQuant,
                            tuplets, sigmap, currentBarIndex);

                Q_ASSERT_X(tuplets.size() >= oldTupletCount, "MidiTuplet::findAllTuplets",
                           "Some old tuplets were deleted that is incorrect");

                if (tuplets.size() > oldTupletCount) {                  // new tuplets were found
                    // chords at the end of the current bar
                    // may have changed bar index - from next bar to the current bar
                    // because they were included in tuplets
#ifdef QT_DEBUG
                    bool nextBarFound = false;
#endif
                    const auto endBarTick = ReducedFraction::fromTicks(
                        sigmap->bar2tick(currentBarIndex + 1, 0));
                    for (auto it = endBarIt; it != chords.end() && it->first < endBarTick; ++it) {
                        if (it->second.barIndex == currentBarIndex) {
#ifdef QT_DEBUG
                            Q_ASSERT_X(!nextBarFound, "MidiTuplet::findAllTuplets",
                                       "Bar indexes become not successive");
#endif
                            Q_ASSERT_X(endBarIt != chords.end(), "MidiTuplet::findAllTuplets",
                                       "End bar iterator cannot be incremented");

                            ++endBarIt;
                        }
#ifdef QT_DEBUG
                        if (it->second.barIndex > currentBarIndex) {
                            nextBarFound = true;
                        }
#endif
                    }
                }

                startBarIt = endBarIt;
                if (endBarIt == chords.end()) {
                    break;
                }
            }
        }
        // handle the last bar containing chords
        findTuplets(startBarIt, chords.end(), chords, basicQuant, tuplets,
                    sigmap, startBarIt->second.barIndex);
    }
    // check if there are not detected off times inside tuplets
    setAllTupletOffTimes(tuplets, chords, sigmap);
#ifdef QT_DEBUG
    Q_ASSERT_X(areAllTupletsReferenced(chords, tuplets),
               "MidiTuplet::findAllTuplets",
               "Not all tuplets are referenced in chords or notes");
    Q_ASSERT_X(MChord::areNotesLongEnough(chords),
               "MidiTuplet::findAllTuplets", "There are too short notes");
    Q_ASSERT(areAllTupletsDifferent(tuplets));
#endif
}
} // namespace MidiTuplet
} // namespace mu::iex::midi
