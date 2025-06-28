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
#include "importmidi_lrhand.h"
#include "importmidi_inner.h"
#include "importmidi_fraction.h"
#include "importmidi_chord.h"
#include "importmidi_operations.h"

namespace mu::iex::midi {
namespace LRHand {
bool needToSplit(const std::multimap<ReducedFraction, MidiChord>& chords,
                 GM1Program midiProgram,
                 bool isDrumTrack)
{
    if (isDrumTrack || !MChord::isGrandStaffProgram(midiProgram)) {
        return false;
    }

    const int octave = 12;
    for (const auto& chord: chords) {
        const MidiChord& c = chord.second;
        int minPitch = std::numeric_limits<int>::max();
        int maxPitch = 0;
        for (const auto& note: c.notes) {
            if (note.pitch < minPitch) {
                minPitch = note.pitch;
            }
            if (note.pitch > maxPitch) {
                maxPitch = note.pitch;
            }
        }
        if (maxPitch - minPitch > octave) {
            return true;
        }
    }

    return false;
}

#ifdef QT_DEBUG

bool areNotesSortedByPitchInAscOrder(const QList<MidiNote>& notes)
{
    for (int i = 0; i != notes.size() - 1; ++i) {
        if (notes[i].pitch > notes[i + 1].pitch) {
            return false;
        }
    }
    return true;
}

#endif

struct SplitTry {
    int penalty = 0;
    // split point - note index, such that: LOW part = [0, split point)
    // and HIGH part = [split point, size)
    // if split point = size then the chord is assigned to the left hand
    int prevSplitPoint = -1;
};

struct ChordSplitData {
    std::multimap<ReducedFraction, MidiChord>::iterator chord;
    std::vector<SplitTry> possibleSplits;      // each index corresponds to the same note index
};

int findLastSplitPoint(const std::vector<ChordSplitData>& splits)
{
    int splitPoint = -1;
    int minPenalty = std::numeric_limits<int>::max();
    const auto& possibleSplits = splits[splits.size() - 1].possibleSplits;

    for (size_t i = 0; i != possibleSplits.size(); ++i) {
        if (possibleSplits[i].penalty < minPenalty) {
            minPenalty = possibleSplits[i].penalty;
            splitPoint = static_cast<int>(i);
        }
    }

    Q_ASSERT_X(splitPoint != -1,
               "LRHand::findLastSplitPoint", "Last split point was not found");

    return splitPoint;
}

// backward dynamic programming step - collect optimal voice separations

void splitChords(
    const std::vector<ChordSplitData>& splits,
    std::multimap<ReducedFraction, MidiChord>& leftHandChords,
    std::multimap<ReducedFraction, MidiChord>& chords)
{
    Q_ASSERT_X(splits.size() == chords.size(),
               "LRHand::collectSolution", "Sizes of split data and chords don't match");

    int splitPoint = findLastSplitPoint(splits);

    for (size_t pos = splits.size() - 1;; --pos) {
        MidiChord& oldChord = splits[pos].chord->second;
        MidiChord newChord(oldChord);

        if (splitPoint > 0 && splitPoint < oldChord.notes.size()) {
            const auto oldNotes = oldChord.notes;

            oldChord.notes.clear();
            for (int i = splitPoint; i != oldNotes.size(); ++i) {
                oldChord.notes.append(oldNotes[i]);
            }

            newChord.notes.clear();
            for (int i = 0; i != splitPoint; ++i) {
                newChord.notes.append(oldNotes[i]);
            }

            leftHandChords.insert({ splits[pos].chord->first, newChord });
        } else if (splitPoint == oldChord.notes.size()) {
            leftHandChords.insert({ splits[pos].chord->first, newChord });
            chords.erase(splits[pos].chord);
        }

        if (pos == 0) {
            break;
        }
        splitPoint = splits[pos].possibleSplits[splitPoint].prevSplitPoint;
    }
}

int findPitchWidthPenalty(const QList<MidiNote>& notes, int splitPoint)
{
    const int octave = 12;
    const int maxPitchWidth = octave + 2;
    int penalty = 0;

    if (splitPoint > 0) {
        const int lowPitchWidth = qAbs(notes[0].pitch
                                       - notes[splitPoint - 1].pitch);
        if (lowPitchWidth <= octave) {
            penalty += 0;
        } else if (lowPitchWidth <= maxPitchWidth) {
            penalty += 20;
        } else {
            penalty += 100;
        }
    }

    if (splitPoint < notes.size()) {
        const int highPitchWidth = qAbs(notes[splitPoint].pitch
                                        - notes[notes.size() - 1].pitch);
        if (highPitchWidth <= octave) {
            penalty += 0;
        } else if (highPitchWidth <= maxPitchWidth) {
            penalty += 20;
        } else {
            penalty += 100;
        }
    }

    return penalty;
}

bool isOctave(const QList<MidiNote>& notes, int beg, int end)
{
    Q_ASSERT_X(end > 0 && beg >= 0 && end > beg, "LRHand::isOctave", "Invalid note indexes");

    const int octave = 12;
    return end - beg == 2 && notes[end - 1].pitch - notes[beg].pitch == octave;
}

int findSimilarityPenalty(
    const QList<MidiNote>& notes,
    const QList<MidiNote>& prevNotes,
    int splitPoint,
    int prevSplitPoint)
{
    int penalty = 0;
    // check for octaves and accompaniment
    if (splitPoint > 0 && prevSplitPoint > 0) {
        const bool isLowOctave = isOctave(notes, 0, splitPoint);
        const bool isPrevLowOctave = isOctave(prevNotes, 0, prevSplitPoint);

        if (isLowOctave && isPrevLowOctave) {               // octaves
            penalty -= 12;
        } else if (splitPoint > 1 && prevSplitPoint > 1) {  // accompaniment
            penalty -= 5;
        }
    }
    if (splitPoint < notes.size() && prevSplitPoint < prevNotes.size()) {
        const bool isHighOctave = isOctave(notes, splitPoint, notes.size());
        const bool isPrevHighOctave = isOctave(prevNotes, prevSplitPoint, prevNotes.size());

        if (isHighOctave && isPrevHighOctave) {
            penalty -= 12;
        } else if (notes.size() - splitPoint > 1 && prevNotes.size() - prevSplitPoint > 1) {
            penalty -= 5;
        }
    }
    // check for one-note melody
    if (splitPoint - 0 == 1 && prevSplitPoint - 0 == 1) {
        penalty -= 12;
    }
    if (notes.size() - splitPoint == 1 && prevNotes.size() - prevSplitPoint == 1) {
        penalty -= 12;
    }

    return penalty;
}

bool areOffTimesEqual(const QList<MidiNote>& notes, int beg, int end)
{
    Q_ASSERT_X(end > 0 && beg >= 0 && end > beg,
               "LRHand::areOffTimesEqual", "Invalid note indexes");

    bool areEqual = true;
    const ReducedFraction firstOffTime = notes[beg].offTime;
    for (int i = beg + 1; i < end; ++i) {
        if (notes[i].offTime != firstOffTime) {
            areEqual = false;
            break;
        }
    }
    return areEqual;
}

int findDurationPenalty(const QList<MidiNote>& notes, int splitPoint)
{
    int penalty = 0;
    if (splitPoint > 0 && areOffTimesEqual(notes, 0, splitPoint)) {
        penalty -= 10;
    }
    if (splitPoint < notes.size() && areOffTimesEqual(notes, splitPoint, notes.size())) {
        penalty -= 10;
    }
    return penalty;
}

int findNoteCountPenalty(const QList<MidiNote>& notes, int splitPoint)
{
    const int leftHandCount = splitPoint;
    const int rightHandCount = notes.size() - splitPoint;
    if (rightHandCount > 0 && leftHandCount > 0 && leftHandCount < rightHandCount) {
        return 5;
    }
    if (rightHandCount == 0 && leftHandCount > 1) {
        return 10;
    }
    return 0;
}

int findIntersectionPenalty(
    const ReducedFraction& currentOnTime,
    int prevPos,
    int prevSplitPoint,
    const ReducedFraction& maxChordLen,
    const std::vector<ChordSplitData>& splits,
    bool hasLowNotes,
    bool hasHighNotes)
{
    int penalty = 0;
    int pos = prevPos;
    int splitPoint = prevSplitPoint;

    while (splits[pos].chord->first + maxChordLen > currentOnTime) {
        const MidiChord& chord = splits[pos].chord->second;

        if (hasLowNotes) {
            ReducedFraction maxNoteOffTime;
            for (int i = 0; i != splitPoint; ++i) {
                if (chord.notes[i].offTime > maxNoteOffTime) {
                    maxNoteOffTime = chord.notes[i].offTime;
                }
            }
            if (maxNoteOffTime > currentOnTime) {
                penalty += 10;
            }
        }
        if (hasHighNotes) {
            ReducedFraction maxNoteOffTime;
            for (int i = splitPoint; i != chord.notes.size(); ++i) {
                if (chord.notes[i].offTime > maxNoteOffTime) {
                    maxNoteOffTime = chord.notes[i].offTime;
                }
            }
            if (maxNoteOffTime > currentOnTime) {
                penalty += 10;
            }
        }

        if (pos == 0) {
            break;
        }
        const SplitTry& splitTry = splits[pos].possibleSplits[splitPoint];
        splitPoint = splitTry.prevSplitPoint;
        --pos;
    }

    return penalty;
}

std::vector<ChordSplitData> findSplits(std::multimap<ReducedFraction, MidiChord>& chords)
{
    std::vector<ChordSplitData> splits;
    int pos = 0;
    ReducedFraction maxChordLen;

    for (auto it = chords.begin(); it != chords.end(); ++it) {
        const auto& notes = it->second.notes;
#ifdef QT_DEBUG
        Q_ASSERT_X(!notes.isEmpty(),
                   "LRHand::findSplits", "Notes are empty");
        Q_ASSERT_X(areNotesSortedByPitchInAscOrder(notes),
                   "LRHand::findSplits",
                   "Notes are not sorted by pitch in ascending order");
#endif
        const auto len = MChord::maxNoteOffTime(notes) - it->first;
        if (len > maxChordLen) {
            maxChordLen = len;
        }

        ChordSplitData split;
        split.chord = it;

        for (int splitPoint = 0; splitPoint <= notes.size(); ++splitPoint) {
            SplitTry splitTry;
            splitTry.penalty = findPitchWidthPenalty(notes, splitPoint)
                               + findDurationPenalty(notes, splitPoint)
                               + findNoteCountPenalty(notes, splitPoint);

            if (pos > 0) {
                int bestPrevSplitPoint = -1;
                int minPenalty = std::numeric_limits<int>::max();
                const auto& prevNotes = std::prev(it)->second.notes;

                for (int prevSplitPoint = 0;
                     prevSplitPoint <= prevNotes.size(); ++prevSplitPoint) {
                    const int prevPenalty
                        = splits[pos - 1].possibleSplits[prevSplitPoint].penalty
                          + findSimilarityPenalty(
                              notes, prevNotes, splitPoint, prevSplitPoint)
                          + findIntersectionPenalty(
                              it->first, pos - 1, prevSplitPoint,
                              maxChordLen, splits,
                              splitPoint > 0, splitPoint < notes.size());

                    if (prevPenalty < minPenalty) {
                        minPenalty = prevPenalty;
                        bestPrevSplitPoint = prevSplitPoint;
                    }
                }

                Q_ASSERT_X(bestPrevSplitPoint != -1,
                           "LRHand::findSplits",
                           "Best previous split point was not found");

                splitTry.penalty += minPenalty;
                splitTry.prevSplitPoint = bestPrevSplitPoint;
            }

            split.possibleSplits.push_back(splitTry);
        }
        splits.push_back(split);

        ++pos;
    }

    return splits;
}

void insertNewLeftHandTrack(std::multimap<int, MTrack>& tracks,
                            std::multimap<int, MTrack>::iterator& it,
                            const std::multimap<ReducedFraction, MidiChord>& leftHandChords)
{
    auto leftHandTrack = it->second;
    leftHandTrack.chords = leftHandChords;
    it = tracks.insert({ it->first, leftHandTrack });
}

// maybe todo later: if range of right-hand chords > OCTAVE
// => assign all bottom right-hand chords to another, third track

void splitStaff(std::multimap<int, MTrack>& tracks, std::multimap<int, MTrack>::iterator& it)
{
    auto& chords = it->second.chords;
    if (chords.empty()) {
        return;
    }
    MChord::sortNotesByPitch(chords);
    std::vector<ChordSplitData> splits = findSplits(chords);

    Q_ASSERT_X(!splits.empty(), "LRHand::splitStaff", "Empty splits array");

    std::multimap<ReducedFraction, MidiChord> leftHandChords;
    splitChords(splits, leftHandChords, chords);

    if (!leftHandChords.empty()) {
        insertNewLeftHandTrack(tracks, it, leftHandChords);
    }
}

void addNewLeftHandChord(std::multimap<ReducedFraction, MidiChord>& leftHandChords,
                         const QList<MidiNote>& leftHandNotes,
                         const std::multimap<ReducedFraction, MidiChord>::iterator& it)
{
    MidiChord leftHandChord = it->second;
    leftHandChord.notes = leftHandNotes;
    leftHandChords.insert({ it->first, leftHandChord });
}

void splitIntoLeftRightHands(std::multimap<int, MTrack>& tracks)
{
    for (auto it = tracks.begin(); it != tracks.end(); ++it) {
        if (it->second.mtrack->drumTrack() || it->second.chords.empty()) {
            continue;
        }
        const auto& opers = midiImportOperations.data()->trackOpers;
        // iterator 'it' will change after track split to ++it
        // C++11 guarantees that newly inserted item with equal key will go after:
        //    "The relative ordering of elements with equivalent keys is preserved,
        //     and newly inserted elements follow those with equivalent keys
        //     already in the container"
        if (opers.doStaffSplit.value(it->second.indexOfOperation)) {
            splitStaff(tracks, it);
        }
    }
}
} // namespace LRHand
} // namespace mu::iex::midi
