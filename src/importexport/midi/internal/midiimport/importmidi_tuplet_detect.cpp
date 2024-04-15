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
#include "importmidi_tuplet_detect.h"
#include "importmidi_tuplet.h"
#include "importmidi_meter.h"
#include "importmidi_chord.h"
#include "importmidi_quant.h"
#include "importmidi_inner.h"
#include "importmidi_operations.h"

#include <set>

namespace mu::iex::midi {
namespace MidiTuplet {
bool isTupletAllowed(const TupletInfo& tupletInfo)
{
    {
        // special check for duplets and triplets
        const std::vector<int> nums = { 2, 3 };
        // for duplet: if note first and single - only 1/2*tupletLen duration is allowed
        // for triplet: if note first and single - only 1/3*tupletLen duration is allowed
        for (int num: nums) {
            if (tupletInfo.tupletNumber == num
                && tupletInfo.chords.size() == 1
                && tupletInfo.firstChordIndex == 0) {
                const auto& chordEventIt = tupletInfo.chords.begin()->second;
                const auto tupletNoteLen = tupletInfo.len / num;
                for (const auto& note: chordEventIt->second.notes) {
                    if ((note.offTime - chordEventIt->first - tupletNoteLen).absValue()
                        > tupletNoteLen / 2) {
                        return false;
                    }
                }
            }
        }
    }
    // for all tuplets
    const auto& opers = midiImportOperations.data()->trackOpers;
    const int minAllowedNoteCount = (opers.isHumanPerformance.value())
                                    ? tupletLimits(tupletInfo.tupletNumber).minNoteCountHuman
                                    : tupletLimits(tupletInfo.tupletNumber).minNoteCount;
    if ((int)tupletInfo.chords.size() < minAllowedNoteCount) {
        return false;
    }
    // allow duplets and quadruplets with error == regular error
    // because tuplet notation is simpler in that case
    if (tupletInfo.tupletNumber == 2 || tupletInfo.tupletNumber == 4) {
        if (tupletInfo.tupletSumError > tupletInfo.regularSumError) {
            return false;
        }
    } else {
        if (tupletInfo.tupletSumError >= tupletInfo.regularSumError) {
            return false;
        }
    }
    // at least one note has to have len >= (half tuplet note len)
    const auto tupletNoteLen = tupletInfo.len / tupletInfo.tupletNumber;
    for (const auto& tupletChord: tupletInfo.chords) {
        for (const auto& note: tupletChord.second->second.notes) {
            if (note.offTime - tupletChord.first >= tupletNoteLen / 2) {
                return true;
            }
        }
    }
    return false;
}

std::vector<int> findTupletNumbers(const ReducedFraction& divLen,
                                   const ReducedFraction& barFraction)
{
    const auto& opers = midiImportOperations.data()->trackOpers;
    const int currentTrack = midiImportOperations.currentTrack();
    std::vector<int> tupletNumbers;

    if (Meter::isCompound(barFraction) && divLen == Meter::beatLength(barFraction)) {
        if (opers.search2plets.value(currentTrack)) {
            tupletNumbers.push_back(2);
        }
        if (opers.search4plets.value(currentTrack)) {
            tupletNumbers.push_back(4);
        }
    } else {
        if (opers.search3plets.value(currentTrack)) {
            tupletNumbers.push_back(3);
        }
        if (opers.search5plets.value(currentTrack)) {
            tupletNumbers.push_back(5);
        }
        if (opers.search7plets.value(currentTrack)) {
            tupletNumbers.push_back(7);
        }
        if (opers.search9plets.value(currentTrack)) {
            tupletNumbers.push_back(9);
        }
    }

    return tupletNumbers;
}

// find sum length of gaps between successive chords
// less is better

ReducedFraction findSumLengthOfRests(
    const TupletInfo& tupletInfo,
    const ReducedFraction& startBarTick)
{
    auto beg = tupletInfo.onTime;
    const auto tupletEndTime = tupletInfo.onTime + tupletInfo.len;
    const auto tupletNoteLen = tupletInfo.len / tupletInfo.tupletNumber;
    ReducedFraction sumLen = { 0, 1 };

    const auto& opers = midiImportOperations.data()->trackOpers;
    const int currentTrack = midiImportOperations.currentTrack();

    for (const auto& chord: tupletInfo.chords) {
        const auto staccatoIt = (opers.simplifyDurations.value(currentTrack))
                                ? tupletInfo.staccatoChords.find(chord.first)
                                : tupletInfo.staccatoChords.end();

        const MidiChord& midiChord = chord.second->second;
        const auto& chordOnTime = (chord.second->first < startBarTick)
                                  ? startBarTick
                                  : Quantize::findQuantizedTupletChordOnTime(*chord.second, tupletInfo.len,
                                                                             tupletLimits(tupletInfo.tupletNumber).ratio, startBarTick);
        if (beg < chordOnTime) {
            sumLen += (chordOnTime - beg);
        }
        ReducedFraction maxOffTime(0, 1);
        for (int i = 0; i != midiChord.notes.size(); ++i) {
            auto noteOffTime = midiChord.notes[i].offTime;
            if (staccatoIt != tupletInfo.staccatoChords.end() && i == staccatoIt->second) {
                noteOffTime = chordOnTime + tupletNoteLen;
            }
            if (noteOffTime > maxOffTime) {
                maxOffTime = noteOffTime;
            }
        }
        beg = Quantize::findQuantizedTupletNoteOffTime(chord.first, maxOffTime, tupletInfo.len,
                                                       tupletLimits(tupletInfo.tupletNumber).ratio, startBarTick).first;
        if (beg >= tupletEndTime) {
            break;
        }
    }
    if (beg < tupletEndTime) {
        sumLen += (tupletEndTime - beg);
    }
    return sumLen;
}

TupletInfo findTupletApproximation(
    const ReducedFraction& tupletLen,
    int tupletNumber,
    const ReducedFraction& basicQuant,
    const ReducedFraction& startTupletTime,
    const std::multimap<ReducedFraction, MidiChord>::iterator& startChordIt,
    const std::multimap<ReducedFraction, MidiChord>::iterator& endChordIt)
{
    TupletInfo tupletInfo;
    tupletInfo.tupletNumber = tupletNumber;
    tupletInfo.onTime = startTupletTime;
    tupletInfo.len = tupletLen;
    const auto tupletNoteLen = tupletLen / tupletNumber;

    struct Error
    {
        bool operator<(const Error& e) const
        {
            if (tupletError < e.tupletError) {
                return true;
            }
            if (tupletError > e.tupletError) {
                return false;
            }
            return tupletRegularDiff < e.tupletRegularDiff;
        }

        ReducedFraction tupletError;
        ReducedFraction tupletRegularDiff;
    };

    struct Candidate
    {
        int posIndex;
        std::multimap<ReducedFraction, MidiChord>::iterator chord;
        ReducedFraction regularError;
    };

    std::multimap<Error, Candidate> chordCandidates;

    for (int posIndex = 0; posIndex != tupletNumber; ++posIndex) {
        const auto tupletNotePos = startTupletTime + tupletNoteLen * posIndex;
        for (auto it = startChordIt; it != endChordIt; ++it) {
            if (it->first < tupletNotePos - tupletNoteLen / 2) {
                continue;
            }
            if (it->first > tupletNotePos + tupletNoteLen / 2) {
                break;
            }

            const auto tupletError = (it->first - tupletNotePos).absValue();
            const auto regularError = Quantize::findOnTimeQuantError(*it, basicQuant);
            const auto diff = tupletError - regularError;
            chordCandidates.insert({ { tupletError, diff }, { posIndex, it, regularError } });
        }
    }

    std::set<std::pair<const ReducedFraction, MidiChord>*> usedChords;
    std::set<int> usedPosIndexes;
    ReducedFraction diffSum;
    int firstChordIndex = 0;

    for (const auto& candidate: chordCandidates) {
        if (diffSum + candidate.first.tupletRegularDiff > ReducedFraction(0, 1)) {
            break;
        }
        const Candidate& c = candidate.second;
        if (usedPosIndexes.find(c.posIndex) != usedPosIndexes.end()) {
            continue;
        }
        if (usedChords.find(&*c.chord) != usedChords.end()) {
            continue;
        }

        usedChords.insert(&*c.chord);
        usedPosIndexes.insert(c.posIndex);
        diffSum += candidate.first.tupletRegularDiff;
        tupletInfo.chords.insert({ c.chord->first, c.chord });
        tupletInfo.tupletSumError += candidate.first.tupletError;
        tupletInfo.regularSumError += c.regularError;
        // if chord was inserted to the beginning - remember its pos index
        if (c.chord->first == tupletInfo.chords.begin()->first) {
            firstChordIndex = c.posIndex;
        }
    }

    tupletInfo.firstChordIndex = firstChordIndex;

    return tupletInfo;
}

// detect staccato notes; later sum length of rests of this notes
// will be reduced by enlarging the length of notes to the tuplet note length

void detectStaccato(TupletInfo& tuplet)
{
    if ((int)tuplet.chords.size() >= tupletLimits(tuplet.tupletNumber).minNoteCountStaccato) {
        const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
        for (auto& chord: tuplet.chords) {
            MidiChord& midiChord = chord.second->second;
            for (int i = 0; i != midiChord.notes.size(); ++i) {
                if (midiChord.notes[i].offTime - chord.first < tupletNoteLen / 2) {
                    // later if chord have one or more notes
                    // with staccato -> entire chord is staccato

                    // don't mark note as staccato here, only remember it
                    // because different tuplets may contain this note,
                    // it will be resolved after tuplet filtering
                    tuplet.staccatoChords.insert({ chord.first, i });
                }
            }
        }
    }
}

// this function is needed because if there are additional chords
// that can be in the middle between tuplet chords,
// and tuplet chords are staccato, i.e. have short length,
// then tied staccato chords would be not pretty-looked converted to notation

bool haveChordsInTheMiddleBetweenTupletChords(
    const std::multimap<ReducedFraction, MidiChord>::iterator startDivChordIt,
    const std::multimap<ReducedFraction, MidiChord>::iterator endDivChordIt,
    const TupletInfo& tuplet)
{
    std::set<std::pair<const ReducedFraction, MidiChord>*> tupletChords;
    for (const auto& chord: tuplet.chords) {
        tupletChords.insert(&*chord.second);
    }

    const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
    for (auto it = startDivChordIt; it != endDivChordIt; ++it) {
        if (tupletChords.find(&*it) != tupletChords.end()) {
            continue;
        }
        for (int i = 0; i != tuplet.tupletNumber; ++i) {
            const auto pos = tuplet.onTime + tupletNoteLen * i + tupletNoteLen / 2;
            if ((pos - it->first).absValue() < tupletNoteLen / 2) {
                return true;
            }
        }
    }
    return false;
}

bool isTupletLenAllowed(
    const ReducedFraction& tupletLen,
    int tupletNumber,
    const std::multimap<ReducedFraction, MidiChord>::const_iterator beg,
    const std::multimap<ReducedFraction, MidiChord>::const_iterator end,
    const ReducedFraction& basicQuant)
{
    const auto tupletNoteLen = tupletLen / tupletNumber;
    const auto regularQuant = Quantize::findQuantForRange(beg, end, basicQuant);
    return tupletNoteLen >= regularQuant;
}

// only one chord from the next bar can be used to search tuplets
// and this chord should be the last in the current bar

bool isNextBarOwnershipOk(
    const std::multimap<ReducedFraction, MidiChord>::iterator& startIt,
    const std::multimap<ReducedFraction, MidiChord>::iterator& endIt,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    int barIndex)
{
    if (endIt == chords.begin() || endIt == startIt) {
        return false;
    }
    if (std::prev(endIt)->second.barIndex == barIndex) {
        return true;
    }

    int nextBarCounter = 0;
    for (auto it = std::prev(endIt);; --it) {
        if (it->second.barIndex > barIndex) {
            ++nextBarCounter;
        }
        if (it == startIt) {
            break;
        }
    }
    return nextBarCounter == 1;
}

std::vector<TupletInfo> detectTuplets(
    const std::multimap<ReducedFraction, MidiChord>::iterator& startBarChordIt,
    const std::multimap<ReducedFraction, MidiChord>::iterator& endBarChordIt,
    const ReducedFraction& startBarTick,
    const ReducedFraction& barFraction,
    std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    int barIndex)
{
    const auto divLengths = Meter::divisionsOfBarForTuplets(barFraction);

    std::vector<TupletInfo> tuplets;
    int id = 0;
    const auto tol = basicQuant / 2;

    for (const auto& divLen: divLengths) {
        const auto tupletNumbers = findTupletNumbers(divLen, barFraction);
        const auto div = barFraction / divLen;
        const int divCount = div.numerator() / div.denominator();

        for (int i = 0; i != divCount; ++i) {
            const auto startDivTime = startBarTick + divLen * i;
            const auto endDivTime = startBarTick + divLen * (i + 1);
            // check which chords can be inside tuplet period
            // [startDivTime - tol, endDivTime]
            const auto startDivTimeWithTol = qMax(startBarTick, startDivTime - tol);
            auto startDivChordIt = MChord::findFirstChordInRange(startDivTimeWithTol, endDivTime,
                                                                 startBarChordIt, endBarChordIt);
            if (startDivChordIt == endBarChordIt) {
                continue;
            }

            Q_ASSERT_X(!startDivChordIt->second.isInTuplet,
                       "MIDI tuplets: findTuplets", "Tuplet chord has been already used");

            // end iterator, as usual, point to the next - invalid chord
            auto endDivChordIt = chords.lower_bound(endDivTime);
            if (!isNextBarOwnershipOk(startDivChordIt, endDivChordIt, chords, barIndex)) {
                continue;
            }
            // try different tuplets, nested tuplets are not allowed
            // here chords from next bar can be captured
            // if their on time < next bar start
            for (const auto& tupletNumber: tupletNumbers) {
                if (!isTupletLenAllowed(divLen, tupletNumber, startDivChordIt, endDivChordIt,
                                        basicQuant)) {
                    continue;
                }
                auto tupletInfo = findTupletApproximation(divLen, tupletNumber,
                                                          basicQuant, startDivTime, startDivChordIt, endDivChordIt);

                const auto& opers = midiImportOperations.data()->trackOpers;
                const int currentTrack = midiImportOperations.currentTrack();

                if (opers.simplifyDurations.value(currentTrack)) {
                    if (!haveChordsInTheMiddleBetweenTupletChords(
                            startDivChordIt, endDivChordIt, tupletInfo)) {
                        detectStaccato(tupletInfo);
                    }
                }
                tupletInfo.sumLengthOfRests = findSumLengthOfRests(tupletInfo, startBarTick);

                if (!isTupletAllowed(tupletInfo)) {
                    continue;
                }
                tupletInfo.id = id++;
                tuplets.push_back(tupletInfo);           // tuplet found
            }                  // next tuplet type
        }
    }

    return tuplets;
}
} // namespace MidiTuplet
} // namespace mu::iex::midi
