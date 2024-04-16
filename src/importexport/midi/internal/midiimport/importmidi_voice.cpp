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
#include "importmidi_voice.h"

#include <QSet>

#include "importmidi_tuplet.h"
#include "importmidi_inner.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_operations.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/durationtype.h"

using namespace mu::engraving;

namespace mu::iex::midi {
namespace MidiVoice {
// no more than VOICES

size_t toIntVoiceCount(MidiOperations::VoiceCount value)
{
    switch (value) {
    case MidiOperations::VoiceCount::V_1:
        return 1;
    case MidiOperations::VoiceCount::V_2:
        return 2;
    case MidiOperations::VoiceCount::V_3:
        return 3;
    case MidiOperations::VoiceCount::V_4:
        return 4;
    }
    return VOICES;
}

int voiceLimit()
{
    const auto& opers = midiImportOperations.data()->trackOpers;
    const int currentTrack = midiImportOperations.currentTrack();
    const size_t allowedVoiceCount = toIntVoiceCount(opers.maxVoiceCount.value(currentTrack));

    Q_ASSERT_X(allowedVoiceCount <= VOICES,
               "MidiVoice::voiceLimit",
               "Allowed voice count exceeds MuseScore Studio voice limit");

    return static_cast<int>(allowedVoiceCount);
}

#ifdef QT_DEBUG

bool areNotesSortedByOffTimeInAscOrder(
    const QList<MidiNote>& notes,
    const std::vector<int>& groupOfIndexes)
{
    for (size_t i = 0; i != groupOfIndexes.size() - 1; ++i) {
        if (notes[groupOfIndexes[i]].offTime > notes[groupOfIndexes[i + 1]].offTime) {
            return false;
        }
    }
    return true;
}

bool areNotesSortedByOffTimeInAscOrder(const QList<MidiNote>& notes)
{
    for (int i = 0; i != (int)notes.size() - 1; ++i) {
        if (notes[i].offTime > notes[i + 1].offTime) {
            return false;
        }
    }
    return true;
}

bool doesTupletAlreadyExist(
    const ReducedFraction& tupletOnTime,
    int voice,
    const std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets)
{
    const auto range = tuplets.equal_range(tupletOnTime);
    bool found = false;
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.voice == voice) {
            found = true;
            break;
        }
    }
    return found;
}

bool areVoicesSame(const std::multimap<ReducedFraction, MidiChord>& chords)
{
    for (const auto& chord: chords) {
        const MidiChord& c = chord.second;
        if (c.isInTuplet && c.tuplet->second.voice != c.voice) {
            return false;
        }
        for (const auto& note: c.notes) {
            if (note.isInTuplet && note.tuplet->second.voice != c.voice) {
                return false;
            }
        }
    }
    return true;
}

#endif

bool allNotesHaveEqualLength(const QList<MidiNote>& notes)
{
    const auto& offTime = notes[0].offTime;
    for (int i = 1; i != notes.size(); ++i) {
        if (notes[i].offTime != offTime) {
            return false;
        }
    }
    return true;
}

int findDurationCountInGroup(
    const ReducedFraction& chordOnTime,
    const QList<MidiNote>& notes,
    int voice,
    const std::vector<int>& groupOfIndexes,
    const TimeSigMap* sigmap,
    const std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets)
{
#ifdef QT_DEBUG
    Q_ASSERT_X(areNotesSortedByOffTimeInAscOrder(notes, groupOfIndexes),
               "MidiVoice::findDurationCountInGroup",
               "Notes are not sorted by off time in ascending order");
#endif
    const auto& opers = midiImportOperations.data()->trackOpers;
    const int currentTrack = midiImportOperations.currentTrack();
    const bool useDots = opers.useDots.value(currentTrack);

    int count = 0;
    auto onTime = chordOnTime;
    auto onTimeBarStart = MidiBar::findBarStart(onTime, sigmap);
    auto onTimeBarFraction = ReducedFraction(
        sigmap->timesig(onTimeBarStart.ticks()).timesig());

    for (int i: groupOfIndexes) {
        const auto& offTime = notes[i].offTime;
        if (offTime == onTime) {
            continue;
        }
        const auto offTimeBarStart = MidiBar::findBarStart(offTime, sigmap);

        if (offTimeBarStart != onTimeBarStart) {
            const auto offTimeBarFraction = ReducedFraction(
                sigmap->timesig(offTimeBarStart.ticks()).timesig());

            const auto tupletsForDuration = MidiTuplet::findTupletsInBarForDuration(
                voice, onTimeBarStart, onTime, offTimeBarStart - onTime, tuplets);

            // additional durations on measure boundary
            const auto durations = Meter::toDurationList(
                onTime - onTimeBarStart, offTimeBarStart - onTimeBarStart,
                offTimeBarFraction, tupletsForDuration, Meter::DurationType::NOTE,
                useDots, false);

            count += MidiDuration::durationCount(durations);

            onTime = offTimeBarStart;
            onTimeBarStart = offTimeBarStart;
            onTimeBarFraction = offTimeBarFraction;
        }

        const auto tupletsForDuration = MidiTuplet::findTupletsInBarForDuration(
            voice, onTimeBarStart, onTime, offTime - onTime, tuplets);

        const auto durations = Meter::toDurationList(
            onTime - onTimeBarStart, offTime - onTimeBarStart, onTimeBarFraction,
            tupletsForDuration, Meter::DurationType::NOTE, useDots, false);

        count += MidiDuration::durationCount(durations);

        onTime = offTime;
    }
    return count;
}

// count of resulting durations in music notation

int findDurationCount(
    const QList<MidiNote>& notes,
    int voice,
    int splitPoint,
    const ReducedFraction& chordOnTime,
    const TimeSigMap* sigmap,
    const std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets)
{
    std::vector<int> lowGroup;
    std::vector<int> highGroup;

    for (int i = 0; i != splitPoint; ++i) {
        lowGroup.push_back(i);
    }
    for (int i = splitPoint; i != notes.size(); ++i) {
        highGroup.push_back(i);
    }

    std::sort(lowGroup.begin(), lowGroup.end(),
              [&](int i1, int i2) { return notes[i1].offTime < notes[i2].offTime; });
    std::sort(highGroup.begin(), highGroup.end(),
              [&](int i1, int i2) { return notes[i1].offTime < notes[i2].offTime; });

    return findDurationCountInGroup(chordOnTime, notes, voice, lowGroup, sigmap, tuplets)
           + findDurationCountInGroup(chordOnTime, notes, voice, highGroup, sigmap, tuplets);
}

int findOptimalSplitPoint(
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    const TimeSigMap* sigmap,
    const std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets,
    const std::multimap<ReducedFraction, MidiChord>& chords)
{
    const auto& notes = chordIt->second.notes;

#ifdef QT_DEBUG
    Q_ASSERT_X(!notes.isEmpty(),
               "MidiVoice::findOptimalSplitPoint", "Notes are empty");
    Q_ASSERT_X(areNotesSortedByOffTimeInAscOrder(notes),
               "MidiVoice::findOptimalSplitPoint",
               "Notes are not sorted by length in ascending order");
#endif
    int optSplit = -1;

    if (!allNotesHaveEqualLength(notes)) {
        int minNoteCount = std::numeric_limits<int>::max();

        for (int splitPoint = 1; splitPoint != notes.size(); ++splitPoint) {
            // optimization: don't split notes with equal durations
            if (notes[splitPoint - 1].offTime == notes[splitPoint].offTime) {
                continue;
            }
            int noteCount = findDurationCount(notes, chordIt->second.voice, splitPoint,
                                              chordIt->first, sigmap, tuplets);
            if (noteCount < minNoteCount) {
                minNoteCount = noteCount;
                optSplit = splitPoint;
            }
        }

        Q_ASSERT_X(optSplit != -1,
                   "MidiVoice::findOptimalSplitPoint", "Optimal split point was not defined");
    } else {
        const auto offTime = notes.front().offTime;
        for (auto it = std::next(chordIt); it != chords.end(); ++it) {
            if (it->first >= offTime) {
                break;
            }
            if (it->second.voice != chordIt->second.voice) {
                continue;
            }
            if (it->first < offTime) {
                optSplit = 0;
                break;
            }
        }
    }

    return optSplit;
}

// which part of chord notes, sorted by length - low note indexes or high note indexes
// - should be moved to another voice

enum class MovedVoiceGroup {
    LOW,
    HIGH
};

struct VoiceSplit {
    MovedVoiceGroup group;
    int voice = -1;
};

std::multimap<ReducedFraction,
              std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>::const_iterator
findInsertedTuplet(const ReducedFraction& onTime,
                   int voice,
                   const std::multimap<ReducedFraction,
                                       std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>& insertedTuplets)
{
    const auto range = insertedTuplets.equal_range(onTime);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second->second.voice == voice) {
            return it;
        }
    }
    return insertedTuplets.end();
}

// if new chord intersected with tuplet that already was inserted
// due to some previous chord separation - then it is not an intersection:
// the new chord belongs to this tuplet

bool hasIntersectionWithTuplets(
    int voice,
    const ReducedFraction& onTime,
    const ReducedFraction& offTime,
    const std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets,
    const std::multimap<ReducedFraction,
                        std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>& insertedTuplets,
    const ReducedFraction& tupletOnTime)
{
    const auto foundTuplets = MidiTuplet::findTupletsForTimeRange(
        voice, onTime, offTime - onTime, tuplets, true);
    for (const auto& tupletIt: foundTuplets) {
        const auto ins = findInsertedTuplet(tupletIt->first, voice, insertedTuplets);
        const bool belongsToInserted = (ins != insertedTuplets.end()
                                        && ins->first == tupletOnTime);
        if (!belongsToInserted) {
            return true;
        }
    }

    return false;
}

void addGroupSplits(
    std::vector<VoiceSplit>& splits,
    const ReducedFraction& maxChordLength,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets,
    const std::multimap<ReducedFraction,
                        std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>& insertedTuplets,
    const ReducedFraction& tupletOnTime,
    const ReducedFraction& onTime,
    const ReducedFraction& groupOffTime,
    int origVoice,
    MovedVoiceGroup groupType,
    int maxOccupiedVoice)
{
    const int limit = voiceLimit();
    bool splitAdded = false;

    for (int voice = 0; voice != limit; ++voice) {
        if (voice == origVoice) {
            continue;
        }
        if (voice > maxOccupiedVoice && splitAdded) {
            break;
        }
        if (hasIntersectionWithTuplets(voice, onTime, groupOffTime,
                                       tuplets, insertedTuplets, tupletOnTime)) {
            continue;
        }
        const auto foundChords = MChord::findChordsForTimeRange(
            voice, onTime, groupOffTime, chords, maxChordLength);
        if (!foundChords.empty()) {
            continue;
        }

        splitAdded = true;
        VoiceSplit split;
        split.group = groupType;
        split.voice = voice;
        splits.push_back(split);
    }
}

ReducedFraction maximizeOffTime(const MidiNote& note, const ReducedFraction& offTime)
{
    auto result = offTime;
    if (note.offTime > offTime) {
        result = note.offTime;
    }
    if (note.isInTuplet) {
        const auto& tuplet = note.tuplet->second;
        if (tuplet.onTime + tuplet.len > result) {
            result = tuplet.onTime + tuplet.len;
        }
    }
    return result;
}

std::vector<VoiceSplit> findPossibleVoiceSplits(
    int origVoice,
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    int splitPoint,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets,
    const std::multimap<ReducedFraction,
                        std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>& insertedTuplets,
    int maxOccupiedVoice)
{
    std::vector<VoiceSplit> splits;

    ReducedFraction onTime = chordIt->first;
    ReducedFraction lowGroupOffTime(0, 1);
    ReducedFraction highGroupOffTime(0, 1);

    const auto& notes = chordIt->second.notes;
    for (int i = 0; i != splitPoint; ++i) {
        lowGroupOffTime = maximizeOffTime(notes[i], lowGroupOffTime);
    }
    for (int i = splitPoint; i != notes.size(); ++i) {
        highGroupOffTime = maximizeOffTime(notes[i], highGroupOffTime);
    }

    ReducedFraction tupletOnTime(-1, 1);
    if (chordIt->second.isInTuplet) {
        const auto& tuplet = chordIt->second.tuplet->second;
        tupletOnTime = tuplet.onTime;
        if (tuplet.onTime < onTime) {
            onTime = tuplet.onTime;
        }
        if (tuplet.onTime + tuplet.len > lowGroupOffTime) {
            lowGroupOffTime = tuplet.onTime + tuplet.len;
        }
        if (tuplet.onTime + tuplet.len > highGroupOffTime) {
            highGroupOffTime = tuplet.onTime + tuplet.len;
        }
    }

    const ReducedFraction maxChordLength = MChord::findMaxChordLength(chords);

    if (splitPoint > 0) {
        addGroupSplits(splits, maxChordLength, chords, tuplets, insertedTuplets,
                       tupletOnTime, onTime, lowGroupOffTime, origVoice,
                       MovedVoiceGroup::LOW, maxOccupiedVoice);
    }
    if (splitPoint < notes.size()) {
        addGroupSplits(splits, maxChordLength, chords, tuplets, insertedTuplets,
                       tupletOnTime, onTime, highGroupOffTime, origVoice,
                       MovedVoiceGroup::HIGH, maxOccupiedVoice);
    }

    return splits;
}

const int MAX_PITCH_DIST = 1000;

int findPrevPitchDist(
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    int averagePitch,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    int voice)
{
    auto it = chordIt;
    while (it != chords.begin()) {
        --it;
        if (it->second.voice == voice) {
            return qAbs(MChord::chordAveragePitch(it->second.notes) - averagePitch);
        }
    }
    return MAX_PITCH_DIST;
}

int findNextPitchDist(
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    int averagePitch,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    int voice)
{
    auto it = (chordIt == chords.end()) ? chordIt : std::next(chordIt);
    while (it != chords.end()) {
        if (it->second.voice == voice) {
            return qAbs(MChord::chordAveragePitch(it->second.notes) - averagePitch);
        }
        ++it;
    }
    return MAX_PITCH_DIST;
}

int findMinPitchDist(
    int averagePitch,
    const int voice,
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    const std::multimap<ReducedFraction, MidiChord>& chords)
{
    const int OCTAVE = 12;
    const int prevPitchDist = findPrevPitchDist(chordIt, averagePitch, chords, voice);
    const int nextPitchDist = findNextPitchDist(chordIt, averagePitch, chords, voice);

    int pitchDist = MAX_PITCH_DIST;

    if (prevPitchDist < nextPitchDist && prevPitchDist <= OCTAVE) {
        pitchDist = prevPitchDist;
    } else if (nextPitchDist <= prevPitchDist && nextPitchDist <= OCTAVE) {
        pitchDist = nextPitchDist;
    }

    return pitchDist;
}

int findAverageLowPitch(const QList<MidiNote>& notes, int splitPoint)
{
    int averageLowPitch = 0;
    for (int j = 0; j != splitPoint; ++j) {
        averageLowPitch += notes[j].pitch;
    }
    averageLowPitch = qRound(averageLowPitch * 1.0 / splitPoint);

    return averageLowPitch;
}

int findAverageHighPitch(const QList<MidiNote>& notes, int splitPoint)
{
    int averageHighPitch = 0;
    for (int j = splitPoint; j != notes.size(); ++j) {
        averageHighPitch += notes[j].pitch;
    }
    averageHighPitch = qRound(averageHighPitch * 1.0 / (notes.size() - splitPoint));

    return averageHighPitch;
}

VoiceSplit findBestSplit(
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::vector<VoiceSplit>& possibleSplits,
    int splitPoint)
{
    // to minimize <pitch distance, voice>
    std::pair<int, int> minError{ std::numeric_limits<int>::max(),
                                  std::numeric_limits<int>::max() };
    int bestSplit = -1;

    for (size_t i = 0; i != possibleSplits.size(); ++i) {
        const int voice = possibleSplits[i].voice;
        const auto& notes = chordIt->second.notes;
        int totalPitchDist = 0;

        if (splitPoint > 0) {
            const int averageLowPitch = findAverageLowPitch(notes, splitPoint);
            const int lowVoice = (possibleSplits[i].group == MovedVoiceGroup::LOW)
                                 ? voice : chordIt->second.voice;
            totalPitchDist += findMinPitchDist(averageLowPitch, lowVoice, chordIt, chords);
        }

        if (splitPoint < notes.size()) {
            const int averageHighPitch = findAverageHighPitch(notes, splitPoint);
            const int highVoice = (possibleSplits[i].group == MovedVoiceGroup::HIGH)
                                  ? voice : chordIt->second.voice;
            totalPitchDist += findMinPitchDist(averageHighPitch, highVoice, chordIt, chords);
        }

        const std::pair<int, int> error{ totalPitchDist, voice };

        if (error < minError) {
            minError = error;
            bestSplit = static_cast<int>(i);
        }
    }

    Q_ASSERT_X(bestSplit != -1, "MidiVoice::findBestSplit", "Best split was not found");

    return possibleSplits[bestSplit];
}

void insertNewTuplet(
    std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator& tuplet,
    const ReducedFraction& tupletOnTime,
    int newVoice,
    std::multimap<ReducedFraction, MidiChord>& chords,
    std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets,
    std::multimap<ReducedFraction,
                  std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>& insertedTuplets)
{
    MidiTuplet::TupletData newTuplet = tuplet->second;
    newTuplet.voice = newVoice;
#ifdef QT_DEBUG
    Q_ASSERT_X(!doesTupletAlreadyExist(newTuplet.onTime, newVoice, tuplets),
               "MidiVoice::addOrUpdateTuplet", "Tuplet already exists");
#endif
    tuplet = tuplets.insert({ tupletOnTime, newTuplet });
    insertedTuplets.insert({ tupletOnTime, tuplet });
    // maybe impossible due to intersection check but check anyway:
    // if there is a non-tuplet chord with on time = tuplet on time
    // then add that chord to the new tuplet
    const auto range = chords.equal_range(tupletOnTime);
    for (auto it = range.first; it != range.second; ++it) {
        MidiChord& chord = it->second;
        if (chord.voice == newVoice && !chord.isInTuplet) {
            chord.isInTuplet = true;
            chord.tuplet = tuplet;
            break;
        }
    }
}

bool canSplitTuplet(
    const std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator& tuplet,
    int newVoice,
    const ReducedFraction& chordOnTime,
    const QList<MidiNote>& notes,
    const std::multimap<ReducedFraction,
                        std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>& insertedTuplets,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& maxChordLength)
{
    const auto tupletOnTime = tuplet->first;
    const auto& t = tuplet->second;
    const auto insertedTuplet = findInsertedTuplet(tupletOnTime, newVoice, insertedTuplets);

    bool needInsertTuplet = false;
    if (insertedTuplet == insertedTuplets.end()
        && MidiTuplet::hasNonTrivialChord(chordOnTime, notes, t.onTime, t.len)) {
        needInsertTuplet = true;
    }

    const bool needDeleteOldTuplet = MidiTuplet::isTupletUseless(t.voice, tupletOnTime,
                                                                 t.len, maxChordLength, chords);
    // insert new tuplet only if old tuplet should be deleted
    // because parallel equal tuplets with different voices aren't pretty
    if (needInsertTuplet && !needDeleteOldTuplet) {
        // need to amend chord split - need but cannot insert tuplet,
        // in that case no changes should be done earlier in this function!
        return false;
    }

    return true;
}

void splitTuplet(
    std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator& tuplet,
    int newVoice,
    const ReducedFraction& chordOnTime,
    const QList<MidiNote>& notes,
    bool& isInTuplet,
    std::multimap<ReducedFraction,
                  std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>& insertedTuplets,
    std::multimap<ReducedFraction, MidiChord>& chords,
    std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets,
    const ReducedFraction& maxChordLength,
    bool allowParallelTuplets,
    bool isThisAChord)
{
    Q_ASSERT_X(isInTuplet, "MidiVoice::splitTuplet",
               "Tuplet chord/note is not actually in tuplet");

    const auto oldTuplet = tuplet;
    const auto tupletOnTime = tuplet->first;
    const auto insertedTuplet = findInsertedTuplet(tupletOnTime, newVoice, insertedTuplets);

    bool needInsertTuplet = false;

    if (insertedTuplet == insertedTuplets.end()) {
        const auto& t = tuplet->second;
        if (MidiTuplet::hasNonTrivialChord(chordOnTime, notes, t.onTime, t.len)) {
            needInsertTuplet = true;
        } else {
            isInTuplet = false;
        }
    } else {
        tuplet = insertedTuplet->second;
    }

    const auto& t = oldTuplet->second;
    const bool needDeleteOldTuplet = MidiTuplet::isTupletUseless(
        t.voice, t.onTime, t.len, maxChordLength, chords);
    // insert new tuplet only if old tuplet was erased
    // because parallel equal tuplets with different voices aren't pretty
    const bool canInsertTuplet = (allowParallelTuplets)
                                 ? needInsertTuplet : (needInsertTuplet && needDeleteOldTuplet);
    if (canInsertTuplet) {
        insertNewTuplet(tuplet, tupletOnTime, newVoice, chords, tuplets, insertedTuplets);
        isInTuplet = true;
    }
    if (needDeleteOldTuplet) {       // delete after insert, because oldTuplet can be used
        bool canRemoveTuplet = true;
        if (isThisAChord) {
            // don't remove tuplet if chord notes have the same tuplet,
            // it will be removed on note split
            for (const auto& note: notes) {
                if (note.isInTuplet && note.tuplet == oldTuplet) {
                    canRemoveTuplet = false;
                    break;
                }
            }
        }
        if (canRemoveTuplet) {
            MidiTuplet::removeTuplet(oldTuplet, tuplets, maxChordLength, chords);
        }
    }

    Q_ASSERT_X(allowParallelTuplets || !needInsertTuplet || needDeleteOldTuplet,
               "MidiVoice::splitTuplet",
               "Tuplet need to be added but the old tuplet was not deleted");
}

bool updateChordTuplets(
    MidiChord& chord,
    const ReducedFraction& onTime,
    std::multimap<ReducedFraction,
                  std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>& insertedTuplets,
    std::multimap<ReducedFraction, MidiChord>& chords,
    std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets,
    const ReducedFraction& maxChordLength,
    bool allowParallelTuplets)
{
    if (!allowParallelTuplets) {
        bool canDoSplit = true;
        if (chord.isInTuplet && !canSplitTuplet(chord.tuplet, chord.voice, onTime,
                                                chord.notes, insertedTuplets, chords,
                                                maxChordLength)) {
            canDoSplit = false;
        }
        if (canDoSplit) {
            for (auto& note: chord.notes) {
                if (note.isInTuplet && !canSplitTuplet(note.tuplet, chord.voice, onTime,
                                                       { note }, insertedTuplets, chords,
                                                       maxChordLength)) {
                    canDoSplit = false;
                    break;
                }
            }
        }
        if (!canDoSplit) {
            return false;
        }
    }

    if (chord.isInTuplet) {
        splitTuplet(chord.tuplet, chord.voice, onTime,
                    chord.notes, chord.isInTuplet,
                    insertedTuplets, chords,
                    tuplets, maxChordLength, allowParallelTuplets, true);
    }
    for (auto& note: chord.notes) {
        if (note.isInTuplet) {
            splitTuplet(note.tuplet, chord.voice, onTime,
                        { note }, note.isInTuplet,
                        insertedTuplets, chords,
                        tuplets, maxChordLength, allowParallelTuplets, false);
        }
    }

    return true;
}

int findMaxOccupiedVoiceInBar(
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    const std::multimap<ReducedFraction, MidiChord>& chords)
{
    const unsigned int barIndex = chordIt->second.barIndex;
    int maxVoice = 0;
    // look forward
    for (auto it = chordIt; it != chords.end(); ++it) {
        const MidiChord& chord = it->second;
        if ((unsigned int)chord.barIndex > barIndex + 1) {
            break;
        }
        if ((unsigned int)chord.barIndex == barIndex && chord.voice > maxVoice) {
            maxVoice = chord.voice;
        }
    }
    // look backward
    for (auto it = chordIt;;) {
        const MidiChord& chord = it->second;
        if ((unsigned int)chord.barIndex + 1 < barIndex) {
            break;
        }
        if ((unsigned int)chord.barIndex == barIndex && chord.voice > maxVoice) {
            maxVoice = chord.voice;
        }
        if (it == chords.begin()) {
            break;
        }
        --it;
    }

    return maxVoice;
}

bool splitChordToVoice(
    std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    const QSet<int>& notesToMove,
    int newVoice,
    std::multimap<ReducedFraction, MidiChord>& chords,
    std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets,
    std::multimap<ReducedFraction,
                  std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>& insertedTuplets,
    const ReducedFraction& maxChordLength,
    bool allowParallelTuplets)
{
    if (notesToMove.isEmpty()) {
        return false;
    }

    bool splitDone = true;
    const ReducedFraction onTime = chordIt->first;
    MidiChord& chord = chordIt->second;
    auto& notes = chord.notes;
#ifdef QT_DEBUG
    Q_ASSERT_X(MidiTuplet::isTupletRangeOk(*chordIt, tuplets),
               "MidiVoice::splitChordToVoice, before split",
               "Tuplet chord/note is outside tuplet "
               "or non-tuplet chord/note is inside tuplet before simplification");
#endif
    if (notesToMove.size() == notes.size()) {
        // don't split chord, just move it to another voice
        const int oldVoice = chord.voice;       // remember for possible undo
        chord.voice = newVoice;

        const bool success = updateChordTuplets(chord, onTime, insertedTuplets,
                                                chords, tuplets, maxChordLength,
                                                allowParallelTuplets);
        if (!success) {
            splitDone = false;
            chord.voice = oldVoice;             // rollback
        }
    } else {            // split chord
        MidiChord newChord(chord);
        newChord.notes.clear();
        newChord.voice = newVoice;
        QList<MidiNote> updatedOldNotes;

        for (int i = 0; i != notes.size(); ++i) {
            if (notesToMove.contains(i)) {
                newChord.notes.append(notes[i]);
            } else {
                updatedOldNotes.append(notes[i]);
            }
        }

        const auto rememberedNotes = notes;           // to undo split if necessary
        // update notes before tuplet update because there will be check
        // for empty tuplets
        notes = updatedOldNotes;

        const bool success = updateChordTuplets(newChord, onTime, insertedTuplets,
                                                chords, tuplets, maxChordLength,
                                                allowParallelTuplets);
        if (success) {
            Q_ASSERT_X(!notes.isEmpty(),
                       "MidiVoice::splitChordToVoice", "Old chord notes are empty");
            Q_ASSERT_X(!newChord.notes.isEmpty(),
                       "MidiVoice::splitChordToVoice", "New chord notes are empty");

            chordIt = chords.insert({ onTime, newChord });
        } else {
            notes = rememberedNotes;            // rollback
            splitDone = false;
        }
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(MidiTuplet::isTupletRangeOk(*chordIt, tuplets),
               "MidiVoice::splitChordToVoice, after split",
               "Tuplet chord/note is outside tuplet "
               "or non-tuplet chord/note is inside tuplet before simplification");
#endif
    return splitDone;
}

QSet<int> findNotesToMove(const QList<MidiNote>& notes,
                          int splitPoint,
                          MovedVoiceGroup splitGroup)
{
    QSet<int> notesToMove;
    if (splitPoint == 0 || splitPoint == notes.size()) {
        // don't split chord, just move it to another voice
        for (int i = 0; i != notes.size(); ++i) {
            notesToMove.insert(i);
        }
    } else {
        switch (splitGroup) {
        case MovedVoiceGroup::LOW:
            for (int i = 0; i != splitPoint; ++i) {
                notesToMove.insert(i);
            }
            break;
        case MovedVoiceGroup::HIGH:
            for (int i = splitPoint; i != notes.size(); ++i) {
                notesToMove.insert(i);
            }
            break;
        }
    }
    return notesToMove;
}

bool doVoiceSeparation(
    std::multimap<ReducedFraction, MidiChord>& chords,
    const TimeSigMap* sigmap,
    std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets)
{
#ifdef QT_DEBUG
    Q_ASSERT_X(MidiTuplet::areTupletRangesOk(chords, tuplets),
               "MidiVoice::doVoiceSeparation",
               "Tuplet chord/note is outside tuplet "
               "or non-tuplet chord/note is inside tuplet before voice separation");
#endif
    MChord::sortNotesByLength(chords);
    std::multimap<ReducedFraction,
                  std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator> insertedTuplets;
    const ReducedFraction maxChordLength = MChord::findMaxChordLength(chords);

    bool changed = false;
    std::map<int, int> maxOccupiedVoices;     // <bar index, max occupied voice>

    for (auto it = chords.begin(); it != chords.end(); ++it) {
        MidiChord& chord = it->second;
        auto& notes = chord.notes;

        const int splitPoint = findOptimalSplitPoint(it, sigmap, tuplets, chords);
        if (splitPoint == -1) {
            continue;
        }

        auto maxVoiceIt = maxOccupiedVoices.find(chord.barIndex);
        if (maxVoiceIt == maxOccupiedVoices.end()) {
            const int maxVoice = findMaxOccupiedVoiceInBar(it, chords);
            maxVoiceIt = maxOccupiedVoices.insert({ chord.barIndex, maxVoice }).first;
        }
        const auto possibleSplits = findPossibleVoiceSplits(
            chord.voice, it, splitPoint, chords,
            tuplets, insertedTuplets, maxVoiceIt->second);
        if (possibleSplits.empty()) {
            continue;
        }

        const VoiceSplit bestSplit = findBestSplit(it, chords, possibleSplits, splitPoint);
        changed = true;

        const QSet<int> notesToMove = findNotesToMove(notes, splitPoint, bestSplit.group);
        const bool splitDone = splitChordToVoice(
            it, notesToMove, bestSplit.voice, chords, tuplets,
            insertedTuplets, maxChordLength);
        if (splitDone && bestSplit.voice > maxVoiceIt->second) {
            maxVoiceIt->second = bestSplit.voice;
        }
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(MidiTuplet::areTupletRangesOk(chords, tuplets),
               "MidiVoice::doVoiceSeparation",
               "Tuplet chord/note is outside tuplet "
               "or non-tuplet chord/note is inside tuplet after voice separation");
#endif
    return changed;
}

int findBarIndexForOffTime(const ReducedFraction& offTime, const TimeSigMap* sigmap)
{
    int barIndex, beat, tick;
    sigmap->tickValues(offTime.ticks(), &barIndex, &beat, &tick);
    if (beat == 0 && tick == 0) {
        --barIndex;
    }
    return barIndex;
}

int averagePitchOfChords(
    const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator>& chords)
{
    if (chords.empty()) {
        return -1;
    }

    int sumPitch = 0;
    int noteCounter = 0;
    for (const auto& chord: chords) {
        const auto& midiNotes = chord->second.notes;
        for (const auto& midiNote: midiNotes) {
            sumPitch += midiNote.pitch;
            ++noteCounter;
        }
    }

    return qRound(sumPitch * 1.0 / noteCounter);
}

void sortVoicesByPitch(const std::map<int, std::vector<
                                          std::multimap<ReducedFraction, MidiChord>::iterator> >& voiceChords)
{
    // [newVoice] = <average pitch, old voice>
    std::vector<std::pair<int, int> > pitchVoices;
    for (const auto& v: voiceChords) {
        pitchVoices.push_back({ averagePitchOfChords(v.second), v.first });
    }

    struct {
        bool operator()(const std::pair<int, int>& p1,
                        const std::pair<int, int>& p2) const
        {
            if (p1.first != p2.first) {
                return p1.first > p2.first;
            }
            return p1.second < p2.second;
        }
    } comparator;
    std::sort(pitchVoices.begin(), pitchVoices.end(), comparator);

    for (int newVoice = 0; newVoice != (int)pitchVoices.size(); ++newVoice) {
        const int oldVoice = pitchVoices[newVoice].second;
        if (newVoice == oldVoice) {
            continue;
        }
        const auto it = voiceChords.find(oldVoice);

        Q_ASSERT_X(it != voiceChords.end(),
                   "MidiVoice::sortVoicesByPitch", "Old voice not found");

        for (auto& chord: it->second) {
            MidiChord& c = chord->second;
            c.voice = newVoice;
            if (c.isInTuplet) {
                c.tuplet->second.voice = newVoice;
            }
            for (auto& note: c.notes) {
                if (note.isInTuplet) {
                    note.tuplet->second.voice = newVoice;
                }
            }
        }
    }
}

void sortVoices(
    std::multimap<ReducedFraction, MidiChord>& chords,
    const TimeSigMap* sigmap)
{
    // <voice, chords>
    std::map<int, std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> > voiceChords;
    int maxBarIndex = 0;

    for (auto it = chords.begin(); it != chords.end(); ++it) {
        const auto& chord = it->second;

        // some notes: if chord off time belongs to tuplet
        // then this tuplet belongs to the same bar as the chord off time;
        // same is for chord on time

        Q_ASSERT_X(chord.barIndex != -1,
                   "MidiVoice::sortVoices", "Chord bar index is undefined");
        Q_ASSERT_X((!voiceChords.empty()) ? chord.barIndex <= maxBarIndex : true,
                   "MidiVoice::sortVoices", "Chord bar index is greater than current index");

        voiceChords[chord.voice].push_back(it);
        const int barIndex = findBarIndexForOffTime(
            MChord::maxNoteOffTime(chord.notes), sigmap);
        if (barIndex > maxBarIndex) {
            maxBarIndex = barIndex;
        }

        if (std::next(it) == chords.end() || std::next(it)->second.barIndex > maxBarIndex) {
            sortVoicesByPitch(voiceChords);
            voiceChords.clear();
        }
    }
}

bool separateVoices(std::multimap<int, MTrack>& tracks, const TimeSigMap* sigmap)
{
    auto& opers = midiImportOperations;
    bool changed = false;

    for (auto& track: tracks) {
        MTrack& mtrack = track.second;
        if (mtrack.mtrack->drumTrack()) {
            continue;
        }
        auto& chords = track.second.chords;
        if (chords.empty()) {
            continue;
        }
        const auto userVoiceCount = toIntVoiceCount(
            opers.data()->trackOpers.maxVoiceCount.value(mtrack.indexOfOperation));
        // pass current track index through MidiImportOperations
        // for further usage
        MidiOperations::CurrentTrackSetter setCurrentTrack{ opers, mtrack.indexOfOperation };

        if (userVoiceCount > 1 && static_cast<int>(userVoiceCount) <= voiceLimit()) {
#ifdef QT_DEBUG
            Q_ASSERT_X(MidiTuplet::areAllTupletsReferenced(mtrack.chords, mtrack.tuplets),
                       "MidiVoice::separateVoices",
                       "Not all tuplets are referenced in chords or notes "
                       "before voice separation");
            Q_ASSERT_X(areVoicesSame(mtrack.chords),
                       "MidiVoice::separateVoices", "Different voices of chord and tuplet "
                                                    "before voice separation");
#endif
            if (doVoiceSeparation(mtrack.chords, sigmap, mtrack.tuplets)) {
                changed = true;
            }
#ifdef QT_DEBUG
            Q_ASSERT_X(MidiTuplet::areAllTupletsReferenced(mtrack.chords, mtrack.tuplets),
                       "MidiVoice::separateVoices",
                       "Not all tuplets are referenced in chords or notes "
                       "after voice separation, before voice sort");
            Q_ASSERT_X(areVoicesSame(mtrack.chords),
                       "MidiVoice::separateVoices", "Different voices of chord and tuplet "
                                                    "after voice separation, before voice sort");
#endif
            sortVoices(mtrack.chords, sigmap);
#ifdef QT_DEBUG
            Q_ASSERT_X(MidiTuplet::areAllTupletsReferenced(mtrack.chords, mtrack.tuplets),
                       "MidiVoice::separateVoices",
                       "Not all tuplets are referenced in chords or notes "
                       "after voice sort");
            Q_ASSERT_X(areVoicesSame(mtrack.chords),
                       "MidiVoice::separateVoices", "Different voices of chord and tuplet "
                                                    "after voice sort");
#endif
        }
    }

    return changed;
}
} // namespace MidiVoice
} // namespace mu::iex::midi
