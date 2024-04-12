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
#include "importmidi_tuplet_filter.h"
#include "importmidi_tuplet.h"
#include "importmidi_tuplet_voice.h"
#include "importmidi_chord.h"
#include "importmidi_quant.h"
#include "importmidi_inner.h"
#include "engraving/dom/mscore.h"

#include <set>

namespace mu::iex::midi {
namespace MidiTuplet {
bool isMoreTupletVoicesAllowed(int voicesInUse, int availableVoices)
{
    return !(voicesInUse >= availableVoices || voicesInUse >= tupletVoiceLimit());
}

class TupletErrorResult
{
public:
    TupletErrorResult(double t = 0.0,
                      double relPlaces = 0.0,
                      const ReducedFraction& r = ReducedFraction(0, 1),
                      size_t vc = 0,
                      size_t tc = 0)
        : tupletAverageError(t)
        , relativeUsedChordPlaces(relPlaces)
        , sumLengthOfRests(r)
        , voiceCount(vc)
        , tupletCount(tc)
    {}

    bool isInitialized() const { return tupletCount != 0; }

    bool operator<(const TupletErrorResult& er) const
    {
        double value = div(tupletAverageError, er.tupletAverageError)
                       - div(relativeUsedChordPlaces, er.relativeUsedChordPlaces)
                       + div(sumLengthOfRests.numerator() * 1.0 / sumLengthOfRests.denominator(),
                             er.sumLengthOfRests.numerator() * 1.0 / er.sumLengthOfRests.denominator());
        if (value == 0) {
            value = div(voiceCount, er.voiceCount)
                    + div(tupletCount, er.tupletCount);
        }
        return value < 0;
    }

private:
    static double div(double val1, double val2)
    {
        if (val1 == val2) {
            return 0;
        }
        return (val1 - val2) / qMax(val1, val2);
    }

    double tupletAverageError;
    double relativeUsedChordPlaces;
    ReducedFraction sumLengthOfRests;
    size_t voiceCount;
    size_t tupletCount;
};

bool haveCommonChords(int i, int j, const std::vector<TupletInfo>& tuplets)
{
    if (tuplets.empty()) {
        return false;
    }
    std::set<std::pair<const ReducedFraction, MidiChord>*> usedChords;
    for (const auto& chord: tuplets[i].chords) {
        usedChords.insert(&*chord.second);
    }
    for (const auto& chord: tuplets[j].chords) {
        if (usedChords.find(&*chord.second) != usedChords.end()) {
            return true;
        }
    }
    return false;
}

// remove overlapping tuplets with the same tuplet number
// when tuplet with bigger length contains the same notes

void removeUselessTuplets(std::vector<TupletInfo>& tuplets)
{
    struct {
        bool operator()(const TupletInfo& t1, const TupletInfo& t2)
        {
            if (t1.tupletNumber != t2.tupletNumber) {
                return t1.tupletNumber < t2.tupletNumber;
            }
            return t1.len < t2.len;
        }
    } comparator;
    std::sort(tuplets.begin(), tuplets.end(), comparator);

    size_t beg = 0;
    while (beg < tuplets.size()) {
        size_t end = beg + 1;
        while (tuplets.size() > end && tuplets[end].tupletNumber == tuplets[beg].tupletNumber) {
            ++end;
        }
        for (size_t i = beg; i < end - 1; ++i) {
            const auto& t1 = tuplets[i];
            for (size_t j = i + 1; j < end; ++j) {
                const auto& t2 = tuplets[j];
                if (t1.onTime >= t2.onTime && t1.onTime + t1.len <= t2.onTime + t2.len) {
                    // check onTimes
                    if (t2.chords.rbegin()->first < t1.onTime + t1.len
                        && t2.chords.begin()->first >= t1.onTime) {
                        // remove larger tuplet
                        tuplets.erase(tuplets.begin() + j);
                        --j;
                        --end;
                    }
                }
            }
        }
        beg = end;
    }
}

std::set<int> findLongestUncommonGroup(
    const std::vector<TupletInfo>& tuplets,
    const ReducedFraction& basicQuant)
{
    struct TInfo
    {
        bool operator<(const TInfo& other) const
        {
            if (offTime < other.offTime) {
                return true;
            } else if (offTime > other.offTime) {
                return false;
            } else {
                return onTime > other.onTime;
            }
        }

        bool operator==(const TInfo& other) const
        {
            return offTime == other.offTime;
        }

        ReducedFraction onTime;
        ReducedFraction offTime;
        int index;
    };

    std::vector<TInfo> info;
    for (size_t i = 0; i != tuplets.size(); ++i) {
        const auto& tuplet = tuplets[i];
        const auto interval = tupletInterval(tuplet, basicQuant);
        info.push_back({ interval.first, interval.second, static_cast<int>(i) });
    }

    std::sort(info.begin(), info.end());
    info.erase(std::unique(info.begin(), info.end()), info.end());

    // check for overlapping tuplets
    // and check for rare case: because of tol when detecting tuplets
    // non-overlapping tuplets can have common chords

    std::set<int> indexes;
    size_t lastSelected = 0;
    for (size_t i = 0; i != info.size(); ++i) {
        if (i > 0 && info[i].onTime < info[lastSelected].offTime) {
            continue;
        }
        if (haveCommonChords(info[lastSelected].index, info[i].index, tuplets)) {
            continue;
        }
        lastSelected = i;
        indexes.insert(info[i].index);
    }

    return indexes;
}

struct TupletCommon
{
    // indexes of tuplets that have common chords with the tuplet with tupletIndex
    std::set<int> commonIndexes;
};

bool areInCommons(const TupletInfo& t1, const TupletInfo& t2)
{
    for (auto it1 = t1.chords.begin(); it1 != t1.chords.end(); ++it1) {
        for (auto it2 = t2.chords.begin(); it2 != t2.chords.end(); ++it2) {
            if (&*it1->second != &*it2->second) {
                continue;
            }
            if (t1.firstChordIndex != 0 || t2.firstChordIndex != 0
                || it1 != t1.chords.begin() || it2 != t2.chords.begin()
                || !isMoreTupletVoicesAllowed(1, it1->second->second.notes.size())) {
                return true;
            }
        }
    }
    return false;
}

std::vector<TupletCommon> findTupletCommons(const std::vector<TupletInfo>& tuplets)
{
    std::vector<TupletCommon> tupletCommons(tuplets.size());

    for (size_t i = 0; i != tuplets.size() - 1; ++i) {
        for (size_t j = i + 1; j != tuplets.size(); ++j) {
            if (areInCommons(tuplets[i], tuplets[j])) {
                tupletCommons[i].commonIndexes.insert(int(j));
            }
        }
    }
    return tupletCommons;
}

bool isInCommonIndexes(
    int indexToCheck,
    const std::vector<int>& selectedTuplets,
    const std::vector<TupletCommon>& tupletCommons)
{
    for (size_t i = 0; i != selectedTuplets.size(); ++i) {
        const int tupletIndex = selectedTuplets[i];

        Q_ASSERT_X(indexToCheck != tupletIndex, "MidiTuplet::isInCommonIndexes",
                   "Checked indexes are the same but they should be different");

        if (indexToCheck > tupletIndex) {
            const auto& indexes = tupletCommons[tupletIndex].commonIndexes;
            if (indexes.find(indexToCheck) != indexes.end()) {
                return true;
            }
        } else {
            const auto& indexes = tupletCommons[indexToCheck].commonIndexes;
            if (indexes.find(tupletIndex) != indexes.end()) {
                return true;
            }
        }
    }
    return false;
}

TupletErrorResult findTupletError(
    const std::vector<int>& tupletIndexes,
    const std::vector<TupletInfo>& tuplets,
    size_t voiceCount,
    const ReducedFraction& basicQuant)
{
    ReducedFraction sumError{ 0, 1 };
    ReducedFraction sumLengthOfRests{ 0, 1 };
    size_t sumChordCount = 0;
    int sumChordPlaces = 0;
    std::set<std::pair<const ReducedFraction, MidiChord>*> usedChords;
    std::vector<char> usedIndexes(tuplets.size(), 0);

    for (int i: tupletIndexes) {
        const auto& tuplet = tuplets[i];

        sumError += tuplet.tupletSumError;
        sumLengthOfRests += tuplet.sumLengthOfRests;
        sumChordCount += tuplet.chords.size();
        sumChordPlaces += tuplet.tupletNumber;

        usedIndexes[i] = 1;
        for (const auto& chord: tuplet.chords) {
            usedChords.insert(&*chord.second);
        }
    }
    // add quant error of all chords excluded from tuplets
    for (size_t i = 0; i != tuplets.size(); ++i) {
        if (usedIndexes[i]) {
            continue;
        }
        const auto& tuplet = tuplets[i];
        for (const auto& chord: tuplet.chords) {
            if (usedChords.find(&*chord.second) != usedChords.end()) {
                continue;
            }
            sumError += Quantize::findOnTimeQuantError(*chord.second, basicQuant);
        }
    }

    return TupletErrorResult{
        sumError.numerator() * 1.0 / (sumError.denominator() * sumChordCount),
        sumChordCount* 1.0 / sumChordPlaces,
        sumLengthOfRests,
        voiceCount,
        tupletIndexes.size()
    };
}

#ifdef QT_DEBUG

bool areCommonsDifferent(const std::vector<int>& selectedCommons)
{
    std::set<int> commons;
    for (int i: selectedCommons) {
        if (commons.find(i) != commons.end()) {
            return false;
        }
        commons.insert(i);
    }
    return true;
}

bool areCommonsUncommon(const std::vector<int>& selectedCommons,
                        const std::vector<TupletCommon>& tupletCommons)
{
    std::set<int> commons;
    for (int i: selectedCommons) {
        for (int j: tupletCommons[i].commonIndexes) {
            commons.insert(j);
        }
    }
    for (int i: selectedCommons) {
        if (commons.find(i) != commons.end()) {
            return false;
        }
    }
    return true;
}

#endif

int findAvailableVoice(
    size_t tupletIndex,
    const std::vector<std::pair<ReducedFraction, ReducedFraction> >& tupletIntervals,
    const std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& voiceIntervals)
{
    int voice = 0;
    while (true) {
        const auto it = voiceIntervals.find(voice);
        if (it != voiceIntervals.end()
            && haveIntersection(tupletIntervals[tupletIndex], it->second)) {
            ++voice;
            continue;
        }
        break;
    }

    return voice;
}

std::map<std::pair<const ReducedFraction, MidiChord>*, int>
prepareUsedFirstChords(const std::vector<int>& selectedTuplets,
                       const std::vector<TupletInfo>& tuplets)
{
    std::map<std::pair<const ReducedFraction, MidiChord>*, int> usedFirstChords;
    for (int i: selectedTuplets) {
        if (tuplets[i].firstChordIndex != 0) {
            continue;
        }
        const auto firstChord = tuplets[i].chords.begin();
        const auto it = usedFirstChords.find(&*firstChord->second);
        if (it != usedFirstChords.end()) {
            ++(it->second);
        } else {
            usedFirstChords.insert({ &*firstChord->second, 1 });
        }
    }

    return usedFirstChords;
}

std::vector<int> findUnusedIndexes(const std::vector<int>& selectedTuplets)
{
    std::vector<int> unusedIndexes;
    int k = 0;
    for (int i = 0; i != selectedTuplets.back(); ++i) {
        if (i == selectedTuplets[k]) {
            ++k;
            continue;
        }
        unusedIndexes.push_back(i);
    }
    return unusedIndexes;
}

bool canUseIndex(
    int indexToCheck,
    const std::vector<TupletInfo>& tuplets,
    const std::vector<std::pair<ReducedFraction, ReducedFraction> >& tupletIntervals,
    const std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& voiceIntervals,
    const std::map<std::pair<const ReducedFraction, MidiChord>*, int>& usedFirstChords)
{
    const auto& tuplet = tuplets[indexToCheck];
    // check tuplets for common 1st chord
    if (tuplet.firstChordIndex == 0) {
        const auto firstChord = tuplet.chords.begin();
        const auto it = usedFirstChords.find(&*firstChord->second);
        if (it != usedFirstChords.end() && !isMoreTupletVoicesAllowed(
                it->second, it->first->second.notes.size())) {
            return false;
        }
    }
    // check tuplets for resulting voice count
    const int voice = findAvailableVoice(indexToCheck, tupletIntervals, voiceIntervals);
    const int voiceCount = qMax((int)voiceIntervals.size(), voice + 1);       // index + 1 = count
    if (voiceCount > 1 && (int)tuplet.chords.size()
        < tupletLimits(tuplet.tupletNumber).minNoteCountAddVoice) {
        return false;
    }
    return true;
}

#ifdef QT_DEBUG

bool areTupletChordsEmpty(const std::vector<TupletInfo>& tuplets)
{
    for (const auto& tuplet: tuplets) {
        if (tuplet.chords.empty()) {
            return true;
        }
    }
    return false;
}

template<typename Iter>
bool validateSelectedTuplets(Iter beginIt,
                             Iter endIt,
                             const std::vector<TupletInfo>& tuplets)
{
    // <chord address, used voices>
    std::map<std::pair<const ReducedFraction, MidiChord>*, size_t> usedChords;
    for (auto indexIt = beginIt; indexIt != endIt; ++indexIt) {
        const auto& tuplet = tuplets[*indexIt];
        const auto& chords = tuplet.chords;
        for (auto it = chords.begin(); it != chords.end(); ++it) {
            bool isFirstChord = (tuplet.firstChordIndex == 0 && it == tuplet.chords.begin());
            const auto fit = usedChords.find(&*(it->second));
            if (fit == usedChords.end()) {
                usedChords.insert({ &*(it->second), isFirstChord ? 1 : engraving::VOICES });
            } else {
                if (!isFirstChord) {
                    return false;
                }
                if (!isMoreTupletVoicesAllowed(static_cast<int>(fit->second), it->second->second.notes.size())) {
                    return false;
                }
                ++(fit->second);
            }
        }
    }
    return true;
}

#endif

void tryUpdateBestIndexes(
    std::vector<int>& bestTupletIndexes,
    TupletErrorResult& minCurrentError,
    const std::vector<int>& selectedTuplets,
    const std::vector<TupletInfo>& tuplets,
    const std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& voiceIntervals,
    const ReducedFraction& basicQuant)
{
    const size_t voiceCount = voiceIntervals.size();
    const auto error = findTupletError(selectedTuplets, tuplets,
                                       voiceCount, basicQuant);
    if (!minCurrentError.isInitialized() || error < minCurrentError) {
        minCurrentError = error;
        bestTupletIndexes = selectedTuplets;
    }
}

std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >
prepareVoiceIntervals(
    const std::vector<int>& selectedTuplets,
    const std::vector<std::pair<ReducedFraction, ReducedFraction> >& tupletIntervals)
{
    // <voice, intervals>
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > > voiceIntervals;
    for (int i: selectedTuplets) {
        int voice = findAvailableVoice(i, tupletIntervals, voiceIntervals);
        voiceIntervals[voice].push_back(tupletIntervals[i]);
    }
    return voiceIntervals;
}

class ValidTuplets
{
public:
    ValidTuplets(int tupletsSize)
        : indexes_(tupletsSize)
        , first_(0)
    {
        for (int i = 0; i != (int)indexes_.size(); ++i) {
            indexes_[i] = { i - 1, i + 1 };
        }
    }

    int first() const
    {
        return first_;
    }

    bool isValid(int index) const
    {
        return index >= first_ && index < (int)indexes_.size();
    }

    int next(int index) const
    {
        return indexes_[index].second;
    }

    bool empty() const
    {
        return first_ >= (int)indexes_.size();
    }

    int exclude(int index)
    {
        if (index < first_) {
            return index;
        }
        int prev = indexes_[index].first;
        int next = indexes_[index].second;
        indexes_[index].first = -1;
        indexes_[index].second = int(indexes_.size());
        if (prev >= first_) {
            indexes_[prev].second = next;
        }
        if (next < (int)indexes_.size()) {
            indexes_[next].first = prev;
        }
        if (index == first_) {
            first_ = next;
        }
        return next;
    }

    std::vector<std::pair<int, int> > save()
    {
        std::vector<std::pair<int, int> > indexes(indexes_.size() - first_);
        for (size_t i = first_; i != indexes_.size(); ++i) {
            indexes[i - first_] = indexes_[i];
        }
        return indexes;
    }

    void restore(const std::vector<std::pair<int, int> >& indexes)
    {
        first_ = int(indexes_.size() - indexes.size());
        for (size_t i = 0; i < indexes.size(); ++i) {
            indexes_[i + first_] = indexes[i];
        }
    }

private:
    std::vector<std::pair<int, int> > indexes_;       // pair<prev, next>
    int first_;
};

void findNextTuplet(
    std::vector<int>& selectedTuplets,
    ValidTuplets& validTuplets,
    std::vector<int>& bestTupletIndexes,
    TupletErrorResult& minCurrentError,
    const std::vector<TupletCommon>& tupletCommons,
    const std::vector<TupletInfo>& tuplets,
    const std::vector<std::pair<ReducedFraction, ReducedFraction> >& tupletIntervals,
    size_t commonsSize,
    const ReducedFraction& basicQuant)
{
    while (!validTuplets.empty()) {
        size_t index = validTuplets.first();

        bool isCommonGroupBegins = (selectedTuplets.empty() && index == commonsSize);
        if (isCommonGroupBegins) {          // first level
            for (size_t i = index; i < tuplets.size(); ++i) {
                selectedTuplets.push_back(int(i));
            }
        } else {
            selectedTuplets.push_back(int(index));
        }
#ifdef QT_DEBUG
        Q_ASSERT_X(validateSelectedTuplets(selectedTuplets.begin(), selectedTuplets.end(), tuplets),
                   "MIDI tuplets::findNextTuplet", "Tuplets have common chords but they shouldn't");
#endif

        const auto voiceIntervals = prepareVoiceIntervals(selectedTuplets, tupletIntervals);
        const auto usedFirstChords = prepareUsedFirstChords(selectedTuplets, tuplets);
#ifdef QT_DEBUG
        Q_ASSERT_X(areCommonsDifferent(selectedTuplets), "MidiTuplet::findNextTuplet",
                   "There are duplicates in selected commons");
        Q_ASSERT_X(areCommonsUncommon(selectedTuplets, tupletCommons),
                   "MidiTuplet::findNextTuplet", "Incompatible selected commons");
#endif

        if (isCommonGroupBegins) {
            bool canAddMoreIndexes = false;
            for (size_t i = 0; i != commonsSize; ++i) {
                if (!isInCommonIndexes(int(i), selectedTuplets, tupletCommons)
                    && canUseIndex(int(i), tuplets, tupletIntervals,
                                   voiceIntervals, usedFirstChords)) {
                    canAddMoreIndexes = true;
                    break;
                }
            }
            if (!canAddMoreIndexes) {
                tryUpdateBestIndexes(bestTupletIndexes, minCurrentError,
                                     selectedTuplets, tuplets, voiceIntervals, basicQuant);
            }
            return;
        }

        validTuplets.exclude(int(index));
        const auto savedTuplets = validTuplets.save();
        // check tuplets for compatibility
        if (!validTuplets.empty()) {
            for (int i: tupletCommons[index].commonIndexes) {
                validTuplets.exclude(i);
                if (validTuplets.empty()) {
                    break;
                }
            }
        }
        for (int i = validTuplets.first(); validTuplets.isValid(i);) {
            if (!canUseIndex(i, tuplets, tupletIntervals,
                             voiceIntervals, usedFirstChords)) {
                i = validTuplets.exclude(i);
                continue;
            }
            i = validTuplets.next(i);
        }
        if (validTuplets.empty()) {
            const auto unusedIndexes = findUnusedIndexes(selectedTuplets);
            bool canAddMoreIndexes = false;
            for (int i: unusedIndexes) {
                if (!isInCommonIndexes(i, selectedTuplets, tupletCommons)
                    && canUseIndex(i, tuplets, tupletIntervals,
                                   voiceIntervals, usedFirstChords)) {
                    canAddMoreIndexes = true;
                    break;
                }
            }
            if (!canAddMoreIndexes) {
                tryUpdateBestIndexes(bestTupletIndexes, minCurrentError,
                                     selectedTuplets, tuplets, voiceIntervals, basicQuant);
            }
        } else {
            findNextTuplet(selectedTuplets, validTuplets, bestTupletIndexes, minCurrentError,
                           tupletCommons, tuplets, tupletIntervals, commonsSize, basicQuant);
        }

        selectedTuplets.pop_back();
        validTuplets.restore(savedTuplets);
    }
}

void moveUncommonTupletsToEnd(std::vector<TupletInfo>& tuplets, std::set<int>& uncommons)
{
    int swapWith = int(tuplets.size()) - 1;
    for (int i = swapWith; i >= 0; --i) {
        auto it = uncommons.find(i);
        if (it != uncommons.end()) {
            if (i != swapWith) {
                std::swap(tuplets[i], tuplets[swapWith]);
            }
            --swapWith;
            uncommons.erase(it);
        }
    }

    Q_ASSERT_X(uncommons.empty(), "MidiTuplet::moveUncommonTupletsToEnd",
               "Untested uncommon tuplets remaining");
}

std::vector<int> findBestTuplets(
    const std::vector<TupletCommon>& tupletCommons,
    const std::vector<TupletInfo>& tuplets,
    size_t commonsSize,
    const ReducedFraction& basicQuant)
{
    std::vector<int> bestTupletIndexes;
    std::vector<int> selectedTuplets;
    TupletErrorResult minCurrentError;
    const auto tupletIntervals = findTupletIntervals(tuplets, basicQuant);

    ValidTuplets validTuplets(int(tuplets.size()));

    findNextTuplet(selectedTuplets, validTuplets, bestTupletIndexes, minCurrentError,
                   tupletCommons, tuplets, tupletIntervals, commonsSize, basicQuant);

    return bestTupletIndexes;
}

void removeExtraTuplets(std::vector<TupletInfo>& tuplets)
{
    const size_t MAX_TUPLETS = 17;           // found empirically

    if (tuplets.size() <= MAX_TUPLETS) {
        return;
    }

    std::map<TupletErrorResult, size_t> errors;
    for (size_t i = 0; i != tuplets.size(); ++i) {
        auto tupletError = TupletErrorResult{
            tuplets[i].tupletSumError.numerator() * 1.0
            / (tuplets[i].tupletSumError.denominator() * tuplets[i].chords.size()),
            tuplets[i].chords.size() * 1.0 / tuplets[i].tupletNumber,
            tuplets[i].sumLengthOfRests,
            1,
            1
        };
        errors.insert({ tupletError, i });
    }
    std::vector<TupletInfo> newTuplets;
    size_t count = 0;
    for (const auto& e: errors) {
        ++count;
        newTuplets.push_back(tuplets[e.second]);
        if (count == MAX_TUPLETS) {
            break;
        }
    }

    std::swap(tuplets, newTuplets);
}

// first chord in tuplet may belong to other tuplet at the same time
// in the case if there are enough notes in this first chord
// to be split into different voices

void filterTuplets(std::vector<TupletInfo>& tuplets,
                   const ReducedFraction& basicQuant)
{
    if (tuplets.empty()) {
        return;
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(!areTupletChordsEmpty(tuplets),
               "MIDI tuplets: filterTuplets", "Tuplet has no chords but it should");
#endif
    removeUselessTuplets(tuplets);
    removeExtraTuplets(tuplets);

    std::set<int> uncommons = findLongestUncommonGroup(tuplets, basicQuant);
#ifdef QT_DEBUG
    Q_ASSERT_X(validateSelectedTuplets(uncommons.begin(), uncommons.end(), tuplets),
               "MIDI tuplets: filterTuplets",
               "Uncommon tuplets have common chords but they shouldn't");
#endif
    size_t commonsSize = tuplets.size();
    if (uncommons.size() > 1) {
        commonsSize -= uncommons.size();
        moveUncommonTupletsToEnd(tuplets, uncommons);
    }
    const auto tupletCommons = findTupletCommons(tuplets);

    const std::vector<int> bestIndexes = findBestTuplets(tupletCommons, tuplets,
                                                         commonsSize, basicQuant);
#ifdef QT_DEBUG
    Q_ASSERT_X(validateSelectedTuplets(bestIndexes.begin(), bestIndexes.end(), tuplets),
               "MIDI tuplets: filterTuplets", "Tuplets have common chords but they shouldn't");
#endif
    std::vector<TupletInfo> newTuplets;
    for (int i: bestIndexes) {
        newTuplets.push_back(tuplets[i]);
    }

    std::swap(tuplets, newTuplets);
}
} // namespace MidiTuplet
} // namespace mu::iex::midi
