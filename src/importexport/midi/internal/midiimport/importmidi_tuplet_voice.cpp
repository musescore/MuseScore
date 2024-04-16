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
#include "importmidi_tuplet_voice.h"
#include "importmidi_tuplet.h"
#include "importmidi_chord.h"
#include "importmidi_quant.h"
#include "importmidi_inner.h"
#include "importmidi_voice.h"
#include "importmidi_operations.h"
#include "engraving/dom/mscore.h"

#include <set>

namespace mu::iex::midi {
namespace MidiTuplet {
int tupletVoiceLimit()
{
    const auto& opers = midiImportOperations.data()->trackOpers;
    const int currentTrack = midiImportOperations.currentTrack();
    const size_t allowedVoices = MidiVoice::toIntVoiceCount(opers.maxVoiceCount.value(currentTrack));

    Q_ASSERT_X(allowedVoices <= engraving::VOICES,
               "MidiTuplet::tupletVoiceLimit",
               "Allowed voice count exceeds MuseScore Studio voice limit");

    // for multiple voices: one voice is reserved for non-tuplet chords
    return (allowedVoices == 1) ? 1 : static_cast<int>(allowedVoices) - 1;
}

std::pair<ReducedFraction, ReducedFraction>
chordInterval(const std::pair<const ReducedFraction, MidiChord>& chord,
              const std::multimap<ReducedFraction, MidiChord>& chords,
              const ReducedFraction& basicQuant,
              const ReducedFraction& barStart)
{
    const auto onTime = MidiTuplet::findOnTimeBetweenChords(chord, chords, basicQuant, barStart);
    auto offTime = Quantize::findMaxQuantizedOffTime(chord, basicQuant);
    if (offTime < onTime) {
        offTime = onTime;
    }
    if (offTime == onTime) {
        offTime += Quantize::quantForLen(MChord::minNoteLen(chord), basicQuant);
    }

    Q_ASSERT_X(offTime > onTime, "MidiTuplet::chordInterval", "Off time <= On time");

    return std::make_pair(onTime, offTime);
}

int findTupletWithChord(const MidiChord& midiChord,
                        const std::vector<TupletInfo>& tuplets)
{
    for (size_t i = 0; i != tuplets.size(); ++i) {
        for (const auto& chord: tuplets[i].chords) {
            if (&(chord.second->second) == &midiChord) {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

std::pair<ReducedFraction, ReducedFraction>
backTiedInterval(const TiedTuplet& tiedTuplet,
                 const std::vector<TupletInfo>& tuplets,
                 const std::multimap<ReducedFraction, MidiChord>& chords,
                 const ReducedFraction& basicQuant,
                 const ReducedFraction& barStart)
{
    const TupletInfo& tuplet = tupletFromId(tiedTuplet.tupletId, tuplets);
    const auto end = tupletInterval(tuplet, basicQuant).second;
    const MidiChord& midiChord = tiedTuplet.chord->second;

    const int tupletIndex = findTupletWithChord(midiChord, tuplets);
    const auto beg = (tupletIndex != -1)
                     ? tupletInterval(tuplets[tupletIndex], basicQuant).first
                     : chordInterval(*tiedTuplet.chord, chords, basicQuant, barStart).first;

    return std::make_pair(beg, end);
}

void setTupletVoice(
    std::map<ReducedFraction,
             std::multimap<ReducedFraction, MidiChord>::iterator>& tupletChords,
    int voice)
{
    for (auto& tupletChord: tupletChords) {
        MidiChord& midiChord = tupletChord.second->second;
        midiChord.voice = voice;
    }
}

void setTupletVoices(
    std::vector<TupletInfo>& tuplets,
    std::set<int>& pendingTuplets,
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& tupletIntervals,
    const ReducedFraction& basicQuant)
{
    const int limit = tupletVoiceLimit();
    int voice = 0;
    while (!pendingTuplets.empty() && voice < limit) {
        for (auto it = pendingTuplets.begin(); it != pendingTuplets.end();) {
            TupletInfo& tuplet = tupletFromId(*it, tuplets);
            const auto interval = tupletInterval(tuplet, basicQuant);
            if (!haveIntersection(interval, tupletIntervals[voice])) {
                setTupletVoice(tuplet.chords, voice);
                tupletIntervals[voice].push_back(interval);
                it = pendingTuplets.erase(it);
                continue;
            }
            ++it;
        }
        ++voice;
    }
}

int findPitchDist(
    const QList<MidiNote>& notes,
    const std::vector<TupletInfo>& tuplets,
    int voice)
{
    int pitchDist = std::numeric_limits<int>::max();        // bad value - only for the last choice
    if (tuplets.empty()) {
        return pitchDist;
    }

    int tupletPitch = 0;
    for (const auto& tuplet: tuplets) {
        if (tuplet.chords.begin()->second->second.voice != voice) {
            continue;
        }
        int counter = 0;
        for (const auto& chord: tuplet.chords) {
            const MidiChord& c = chord.second->second;
            tupletPitch += MChord::chordAveragePitch(c.notes);
            ++counter;
        }
        tupletPitch = qRound(tupletPitch * 1.0 / counter);
        break;
    }

    if (tupletPitch == 0) {
        return pitchDist;
    }

    const int chordPitch = MChord::chordAveragePitch(notes);
    pitchDist = qAbs(chordPitch - tupletPitch);

    return pitchDist;
}

void setNonTupletVoices(
    std::set<std::pair<const ReducedFraction, MidiChord>*>& pendingNonTuplets,
    const std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& tupletIntervals,
    const std::vector<TupletInfo>& tuplets,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart)
{
    const int limit = MidiVoice::voiceLimit();
    while (!pendingNonTuplets.empty()) {
        auto chord = *pendingNonTuplets.begin();
        const auto interval = chordInterval(*chord, chords, basicQuant, barStart);
        // pick the voice such that the average pitch difference
        // between the non-tuplet chord and tuplets with this voice
        // is the smallest
        int bestVoice = -1;
        int minPitchDist = -1;
        for (int voice = 0; voice < limit; ++voice) {
            const auto fit = tupletIntervals.find(voice);
            if (fit == tupletIntervals.end() || !haveIntersection(interval, fit->second)) {
                const int pitchDist = findPitchDist(chord->second.notes, tuplets, voice);
                if (minPitchDist == -1 || pitchDist < minPitchDist) {
                    minPitchDist = pitchDist;
                    bestVoice = voice;
                }
            }
        }

        Q_ASSERT_X(bestVoice >= 0,
                   "MidiTuplet::setNonTupletVoices", "Best voice not found");

        chord->second.voice = bestVoice;
        pendingNonTuplets.erase(pendingNonTuplets.begin());
        // don't insert chord interval here
    }
}

#ifdef QT_DEBUG

bool areAllElementsUnique(
    const std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets)
{
    std::set<std::pair<const ReducedFraction, MidiChord>*> chords;
    for (const auto& chord: nonTuplets) {
        if (chords.find(&*chord) == chords.end()) {
            chords.insert(&*chord);
        } else {
            return false;
        }
    }
    return true;
}

bool haveTupletsEmptyChords(const std::vector<TupletInfo>& tuplets)
{
    for (const auto& tuplet: tuplets) {
        if (tuplet.chords.empty()) {
            return true;
        }
    }
    return false;
}

bool doTupletChordsHaveSameVoice(const std::vector<TupletInfo>& tuplets)
{
    for (const auto& tuplet: tuplets) {
        auto it = tuplet.chords.cbegin();
        const int voice = it->second->second.voice;
        ++it;
        for (; it != tuplet.chords.cend(); ++it) {
            if (it->second->second.voice != voice) {
                return false;
            }
        }
    }
    return true;
}

// back tied tuplets are not checked here

bool haveOverlappingVoices(
    const std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets,
    const std::vector<TupletInfo>& tuplets,
    const std::list<TiedTuplet>& backTiedTuplets,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart)
{
    // <voice, intervals>
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > > intervals;

    for (const auto& tuplet: tuplets) {
        const int voice = tuplet.chords.begin()->second->second.voice;
        const auto interval = std::make_pair(tuplet.onTime, tuplet.onTime + tuplet.len);
        if (haveIntersection(interval, intervals[voice])) {
            return true;
        } else {
            intervals[voice].push_back(interval);
        }
    }

    for (const auto& chord: nonTuplets) {
        const int voice = chord->second.voice;
        const auto interval = chordInterval(*chord, chords, basicQuant, barStart);
        if (haveIntersection(interval, intervals[voice])) {
            bool flag = false;
            // if chord is tied then it can intersect tuplet
            for (const TiedTuplet& tiedTuplet: backTiedTuplets) {
                if (tiedTuplet.chord == (&*chord)
                    && (tiedTuplet.voice == -1 || tiedTuplet.voice == voice)) {
                    flag = true;
                    break;
                }
            }
            if (!flag) {
                return true;
            }
        }
    }

    return false;
}

size_t chordCount(
    const std::vector<TupletInfo>& tuplets,
    const std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets)
{
    size_t sum = nonTuplets.size();
    for (const auto& tuplet: tuplets) {
        sum += tuplet.chords.size();
    }
    return sum;
}

bool voiceDontExceedLimit(
    const std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets,
    const std::vector<TupletInfo>& tuplets)
{
    for (const auto& tuplet: tuplets) {
        const int voice = tuplet.chords.begin()->second->second.voice;
        if (voice >= MidiVoice::voiceLimit()) {
            return true;
        }
    }

    for (const auto& chord: nonTuplets) {
        const int voice = chord->second.voice;
        if (voice >= MidiVoice::voiceLimit()) {
            return true;
        }
    }

    return false;
}

#endif

void eraseBackTiedTuplet(
    int tupletId,
    std::list<TiedTuplet>& backTiedTuplets)
{
    for (auto it = backTiedTuplets.begin(); it != backTiedTuplets.end(); ++it) {
        if (it->tupletId == tupletId) {
            backTiedTuplets.erase(it);
            break;
        }
    }
}

// for the case when voice limit = 1

bool excludeExtraVoiceTuplets(
    std::vector<TupletInfo>& tuplets,
    std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets,
    std::list<TiedTuplet>& backTiedTuplets,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart,
    int barIndex)
{
    size_t sz = tuplets.size();
    if (sz == 0) {
        return false;
    }

    std::list<std::multimap<ReducedFraction, MidiChord>::iterator> newNonTuplets;
    size_t addedCount = nonTuplets.size();
    while (addedCount > 0) {
        nonTuplets.splice(nonTuplets.begin(), newNonTuplets);
        // remove tuplets that are overlapped with non-tuplets
        for (size_t i = 0; i < sz;) {
            const auto interval = tupletInterval(tuplets[i], basicQuant);
            bool shift = false;
            size_t counter = 0;
            for (const auto& nonTuplet: nonTuplets) {
                ++counter;
                if (counter > addedCount) {
                    break;
                }
                if (haveIntersection(interval, chordInterval(*nonTuplet, chords,
                                                             basicQuant, barStart))) {
                    bool isTied = false;
                    for (const TiedTuplet& tiedTuplet: backTiedTuplets) {
                        if (tiedTuplet.tupletId == tuplets[i].id
                            && tiedTuplet.chord == &*nonTuplet) {
                            isTied = true;
                            break;
                        }
                    }
                    if (!isTied) {
                        for (const auto& chord: tuplets[i].chords) {
                            if (chord.second->second.barIndex == barIndex) {
                                newNonTuplets.push_back(chord.second);
                            }
                        }
                        eraseBackTiedTuplet(tuplets[i].id, backTiedTuplets);
                        --sz;
                        if (i < sz) {
                            shift = true;
                            tuplets[i] = tuplets[sz];
                        }
                        break;
                    }
                }
            }
            if (shift) {
                continue;
            }
            ++i;
        }
        addedCount = newNonTuplets.size();
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(areAllElementsUnique(nonTuplets),
               "MidiTuplet::excludeExtraVoiceTuplets", "Non unique chords in non-tuplets");
#endif
    bool excluded = (sz != tuplets.size());
    tuplets.resize(sz);

    return excluded;
}

void removeUnusedTuplets(
    std::vector<TupletInfo>& tuplets,
    std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets,
    std::set<int>& pendingTuplets,
    std::list<TiedTuplet>& backTiedTuplets,
    std::set<std::pair<const ReducedFraction, MidiChord>*>& pendingNonTuplets,
    int barIndex)
{
    if (pendingTuplets.empty()) {
        return;
    }

    std::vector<TupletInfo> newTuplets;
    for (size_t i = 0; i != tuplets.size(); ++i) {
        if (pendingTuplets.find(tuplets[i].id) == pendingTuplets.end()) {
            newTuplets.push_back(tuplets[i]);
        } else {
            eraseBackTiedTuplet(tuplets[i].id, backTiedTuplets);
            for (const auto& chord: tuplets[i].chords) {
                if (chord.second->second.barIndex == barIndex) {
                    nonTuplets.push_back(chord.second);
                    pendingNonTuplets.insert(&*chord.second);
                }
            }
        }
    }
    pendingTuplets.clear();
    std::swap(tuplets, newTuplets);
}

std::set<int> findPendingTuplets(const std::vector<TupletInfo>& tuplets)
{
    std::set<int> pendingTuplets;         // tuplet indexes
    for (size_t i = 0; i != tuplets.size(); ++i) {
        pendingTuplets.insert(tuplets[i].id);
    }
    return pendingTuplets;
}

std::set<std::pair<const ReducedFraction, MidiChord>*>
findPendingNonTuplets(
    const std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets)
{
    std::set<std::pair<const ReducedFraction, MidiChord>*> pendingNonTuplets;
    for (const auto& c: nonTuplets) {
        pendingNonTuplets.insert(&*c);
    }
    return pendingNonTuplets;
}

std::list<TiedTuplet>::iterator
eraseBackTiedTuplet(const std::list<TiedTuplet>::iterator& it,
                    std::list<TiedTuplet>& backTiedTuplets,
                    const TupletInfo& tuplet)
{
    for (const auto& chord: tuplet.chords) {
        for (auto it2 = backTiedTuplets.begin(); it2 != backTiedTuplets.end(); ++it2) {
            if (&(chord.second->second) == &(it2->chord->second)) {
                backTiedTuplets.erase(it2);
                break;
            }
        }
    }

    return backTiedTuplets.erase(it);
}

void setVoicesFromPrevBars(
    std::list<TiedTuplet>& backTiedTuplets,
    std::vector<TupletInfo>& tuplets,
    std::set<int>& pendingTuplets,
    const std::set<std::pair<const ReducedFraction, MidiChord>*>& pendingNonTuplets,
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& tupletIntervals,
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& backTupletIntervals,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart)
{
#ifdef NDEBUG
    Q_UNUSED(pendingNonTuplets);
#endif
    bool loopAgain = false;
    do {
        for (auto it = backTiedTuplets.begin(); it != backTiedTuplets.end();) {
            const TiedTuplet& tiedTuplet = *it;
            TupletInfo& tuplet = tupletFromId(tiedTuplet.tupletId, tuplets);
            const auto backInterval = std::make_pair(tuplet.onTime, tuplet.onTime + tuplet.len);

            if (tiedTuplet.voice == -1) {
                ++it;
                continue;
            }
            if (haveIntersection(backInterval, backTupletIntervals[tiedTuplet.voice])) {
                it = backTiedTuplets.erase(it);
                continue;
            }

            for (const auto& chord: tuplet.chords) {
                for (auto it2 = backTiedTuplets.begin();
                     it2 != backTiedTuplets.end(); ++it2) {
                    if (it2 == it) {
                        continue;
                    }
                    if (&(chord.second->second) == &(it2->chord->second)
                        && it2->voice == -1) {
                        it2->voice = tiedTuplet.voice;
                        loopAgain = true;
                        break;
                    }
                }
            }

            setTupletVoice(tuplet.chords, tiedTuplet.voice);
            backTupletIntervals[tiedTuplet.voice].push_back(backInterval);
            tupletIntervals[tiedTuplet.voice].push_back(backInterval);
            // add to intervals chord from previous bar
            tupletIntervals[tiedTuplet.voice].push_back(
                chordInterval(*tiedTuplet.chord, chords, basicQuant, barStart));
            pendingTuplets.erase(tiedTuplet.tupletId);

            Q_ASSERT_X(pendingNonTuplets.find(tiedTuplet.chord) == pendingNonTuplets.end(),
                       "MidiTuplet::setBackTiedVoices",
                       "Tied non-tuplet chord should not be here");

            ++it;
        }
    } while (loopAgain);
}

void setTiedChordVoice(
    std::list<TiedTuplet>& backTiedTuplets,
    std::vector<TupletInfo>& tuplets,
    std::set<int>& pendingTuplets,
    std::set<std::pair<const ReducedFraction, MidiChord>*>& pendingNonTuplets,
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& tupletIntervals,
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& backTupletIntervals,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::list<TiedTuplet>::iterator& backTiedIt,
    bool isNonTupletBackChord,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart)
{
    const TiedTuplet& tiedTuplet = *backTiedIt;

    if (isNonTupletBackChord) {
        // non-tuplet chord tied
        const auto interval = chordInterval(*tiedTuplet.chord, chords, basicQuant, barStart);
        backTupletIntervals[tiedTuplet.voice].push_back(interval);
        tiedTuplet.chord->second.voice = tiedTuplet.voice;
        if (tupletVoiceLimit() > 1) {
            tupletIntervals[tiedTuplet.voice].push_back(interval);
        }
        pendingNonTuplets.erase(tiedTuplet.chord);
    } else {
        // tuplet tied
        const int i = findTupletWithChord(tiedTuplet.chord->second, tuplets);

        Q_ASSERT_X(i != -1, "MidiTuplet::setBackTiedVoices",
                   "Tuplet chord not found in tuplets");

        auto it2 = std::next(backTiedIt);
        for (; it2 != backTiedTuplets.end(); ++it2) {
            if (it2->tupletId == tuplets[i].id) {
                break;
            }
        }
        if (it2 != backTiedTuplets.end()) {
            if (it2->voice == -1) {
                it2->voice = tiedTuplet.voice;
            }
        } else {
            // set voice of not back-tied tuplet that have tied chord
            setTupletVoice(tuplets[i].chords, tiedTuplet.voice);
            const auto interval = tupletInterval(tuplets[i], basicQuant);
            tupletIntervals[tiedTuplet.voice].push_back(interval);
            backTupletIntervals[tiedTuplet.voice].push_back(interval);
            pendingTuplets.erase(tuplets[i].id);
        }
    }
}

void setVoiceOfConnectedBackTied(
    std::list<TiedTuplet>& backTiedTuplets,
    int tiedTupletVoice,
    const std::set<int>& pendingTuplets,
    const TupletInfo& tuplet,
    const std::list<TiedTuplet>::iterator& backTiedIt)
{
    for (const auto& chord: tuplet.chords) {
        for (auto it2 = std::next(backTiedIt); it2 != backTiedTuplets.end(); ++it2) {
            if (pendingTuplets.find(it2->tupletId) == pendingTuplets.end()) {
                continue;
            }
            if (&(chord.second->second) == &(it2->chord->second) && it2->voice == -1) {
                it2->voice = tiedTupletVoice;
                break;
            }
        }
    }
}

int findVoiceForBackTied(
    const std::pair<const ReducedFraction, MidiChord>& tiedTupletChord,
    int voiceLimit,
    const std::pair<ReducedFraction, ReducedFraction>& backInterval,
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& backTupletIntervals,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    bool isNonTupletBackChord,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart)
{
    int voice = 0;
    for (; voice != voiceLimit; ++voice) {
        if (haveIntersection(backInterval, backTupletIntervals[voice])
            || (isNonTupletBackChord && haveIntersection(
                    chordInterval(tiedTupletChord, chords, basicQuant, barStart),
                    backTupletIntervals[voice]))) {
            continue;
        }
        break;
    }

    return voice;
}

std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >
findBackTupletIntervals(
    const std::list<TiedTuplet>& backTiedTuplets,
    const std::vector<TupletInfo>& tuplets)
{
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > > backTupletIntervals;
    for (const auto& t: backTiedTuplets) {
        const auto& tuplet = tupletFromId(t.tupletId, tuplets);
        const auto interval = std::make_pair(tuplet.onTime, tuplet.onTime + tuplet.len);
        backTupletIntervals[t.voice].push_back(interval);
    }
    return backTupletIntervals;
}

void setBackTiedVoices(
    std::list<TiedTuplet>& backTiedTuplets,
    std::vector<TupletInfo>& tuplets,
    std::set<int>& pendingTuplets,
    std::set<std::pair<const ReducedFraction, MidiChord>*>& pendingNonTuplets,
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >& tupletIntervals,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart)
{
    auto backTupletIntervals = findBackTupletIntervals(backTiedTuplets, tuplets);

    // set voices that are already set from one of previous bars
    setVoicesFromPrevBars(backTiedTuplets, tuplets, pendingTuplets, pendingNonTuplets,
                          tupletIntervals, backTupletIntervals, chords, basicQuant, barStart);

    // set yet unset back tied voices
    const int limit = tupletVoiceLimit();

    for (auto it = backTiedTuplets.begin(); it != backTiedTuplets.end();) {
        TiedTuplet& tiedTuplet = *it;
        if (pendingTuplets.find(tiedTuplet.tupletId) == pendingTuplets.end()) {
            ++it;
            continue;
        }

        TupletInfo& tuplet = tupletFromId(tiedTuplet.tupletId, tuplets);
        const auto backInterval = std::make_pair(tuplet.onTime, tuplet.onTime + tuplet.len);
        bool isNonTupletBackChord
            = (pendingNonTuplets.find(tiedTuplet.chord) != pendingNonTuplets.end());

        if (tiedTuplet.voice == -1) {
            const int voice = findVoiceForBackTied(
                *tiedTuplet.chord, limit, backInterval, backTupletIntervals,
                chords, isNonTupletBackChord, basicQuant, barStart);
            if (voice < limit) {
                tiedTuplet.voice = voice;
            } else {        // no available voices
                it = eraseBackTiedTuplet(it, backTiedTuplets, tuplet);
                continue;
            }
        } else {
            if (haveIntersection(backInterval, backTupletIntervals[tiedTuplet.voice])
                || (isNonTupletBackChord
                    && haveIntersection(chordInterval(*tiedTuplet.chord, chords,
                                                      basicQuant, barStart),
                                        backTupletIntervals[tiedTuplet.voice]))) {
                it = eraseBackTiedTuplet(it, backTiedTuplets, tuplet);
                continue;
            }
        }

        setVoiceOfConnectedBackTied(backTiedTuplets, tiedTuplet.voice,
                                    pendingTuplets, tuplet, it);

        // set voices of tied tuplet chords
        setTupletVoice(tuplet.chords, tiedTuplet.voice);
        backTupletIntervals[tiedTuplet.voice].push_back(backInterval);
        tupletIntervals[tiedTuplet.voice].push_back(tupletInterval(tuplet, basicQuant));
        pendingTuplets.erase(tiedTuplet.tupletId);

        setTiedChordVoice(backTiedTuplets, tuplets, pendingTuplets, pendingNonTuplets,
                          tupletIntervals, backTupletIntervals, chords, it,
                          isNonTupletBackChord, basicQuant, barStart);
        ++it;
    }
}

std::map<std::pair<const ReducedFraction, MidiChord>*, size_t>
findMappedTupletChords(const std::vector<TupletInfo>& tuplets)
{
    // <chord address, tupletIndex>
    std::map<std::pair<const ReducedFraction, MidiChord>*, size_t> tupletChords;
    for (size_t i = 0; i != tuplets.size(); ++i) {
        for (const auto& tupletChord: tuplets[i].chords) {
            auto tupletIt = tupletChord.second;
            tupletChords.insert({ &*tupletIt, i });
        }
    }
    return tupletChords;
}

bool areTupletsIntersect(const TupletInfo& t1, const TupletInfo& t2)
{
    const auto onTime1 = t1.onTime;
    const auto endTime1 = onTime1 + t1.len;
    const auto onTime2 = t2.onTime;
    const auto endTime2 = onTime1 + t2.len;
    return endTime1 > onTime2 && onTime1 < endTime2;
}

// result: tied notes indexes

std::vector<int> findTiedNotes(
    const TupletInfo& tuplet,
    const std::multimap<ReducedFraction, MidiChord>::iterator& chordIt,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& startBarTick,
    const ReducedFraction& basicQuant)
{
    std::vector<int> tiedNotes;
    const auto tupletRatio = tupletLimits(tuplet.tupletNumber).ratio;
    const auto firstTupletChordOnTime = Quantize::findQuantizedTupletChordOnTime(
        *tuplet.chords.begin()->second, tuplet.len,
        tupletRatio, startBarTick);

    const auto maxChordOffTime = Quantize::findMaxQuantizedTupletOffTime(
        *chordIt, tuplet.len, tupletRatio, startBarTick);

    if (maxChordOffTime > firstTupletChordOnTime) {
        return tiedNotes;
    }

    const auto onTime = MidiTuplet::findOnTimeBetweenChords(*chordIt, chords,
                                                            basicQuant, startBarTick);
    if (onTime >= tuplet.onTime) {
        return tiedNotes;
    }

    for (int i = 0; i != chordIt->second.notes.size(); ++i) {
        const MidiNote& note = chordIt->second.notes[i];

        const auto offTimeInTuplet = Quantize::findQuantizedTupletNoteOffTime(
            chordIt->first, note.offTime, tuplet.len, tupletRatio, startBarTick).first;

        if (offTimeInTuplet < startBarTick || offTimeInTuplet <= tuplet.onTime) {
            continue;
        }

        const auto regularOffTime = Quantize::findQuantizedNoteOffTime(
            *chordIt, note.offTime, basicQuant).first;
        const auto regularError = (note.offTime - regularOffTime).absValue();
        const auto tupletError = (note.offTime - offTimeInTuplet).absValue();
        if (tupletError > regularError) {
            continue;
        }

        tiedNotes.push_back(i);
    }

    return tiedNotes;
}

// prepare tied tuplets - pairs of tuplet and chord back-tied to it
// voices of back-tied chords from previous bar are set explicitly
// other voices = -1
// tied tuplets with voice != -1 don't intersect each other
// tuplets can be tied only to one chord or tuplet (no 'branches')

std::list<TiedTuplet>
findBackTiedTuplets(
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const std::vector<TupletInfo>& tuplets,
    const ReducedFraction& prevBarStart,
    const ReducedFraction& startBarTick,
    const ReducedFraction& basicQuant,
    int currentBarIndex)
{
    std::list<TiedTuplet> tiedTuplets;
    // <voice, intervals>
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > > backTupletIntervals;
    std::set<int> usedTuplets;
    std::set<std::pair<const ReducedFraction, MidiChord>*> usedChords;
    const auto tupletChords = findMappedTupletChords(tuplets);

    for (size_t i = 0; i != tuplets.size(); ++i) {
        Q_ASSERT_X(!tuplets[i].chords.empty(),
                   "MidiTuplets::findBackTiedTuplets", "Tuplet chords are empty");

        auto chordIt = tuplets[i].chords.begin()->second;
        while (chordIt != chords.begin() && chordIt->first >= prevBarStart) {
            --chordIt;

            const auto tupletIt = tupletChords.find(&*chordIt);
            const bool isInTupletOfThisBar = (tupletIt != tupletChords.end());
            // don't make back tie to the chord in overlapping tuplet
            if (isInTupletOfThisBar
                && areTupletsIntersect(tuplets[tupletIt->second], tuplets[i])) {
                continue;
            }
            // remember voices of tuplets that have tied chords from previous bar
            // and that chords don't belong to the tuplets of this bar
            const int voice = (chordIt->second.barIndex < currentBarIndex)
                              ? chordIt->second.voice : -1;
            const auto interval = std::make_pair(tuplets[i].onTime,
                                                 tuplets[i].onTime + tuplets[i].len);
            // if voice is specified and the new interval have intersection
            // with already found back tuplets with the save voice
            // then discard the interval
            if (voice != -1 && haveIntersection(interval, backTupletIntervals[voice])) {
                continue;
            }

            const auto tiedNotes = findTiedNotes(tuplets[i], chordIt, chords,
                                                 startBarTick, basicQuant);
            if (!tiedNotes.empty()) {
                // don't tie back twice to the same chord or tuplet
                if (usedChords.find(&*chordIt) != usedChords.end()) {
                    continue;
                }
                // don't tie back twice to the same tuplet
                const int tupletIndex = findTupletWithChord(chordIt->second, tuplets);
                if (usedTuplets.find(tupletIndex) != usedTuplets.end()) {
                    continue;
                }
                // we can add back-tied tuplet; voice here can be -1
                tiedTuplets.push_back({ tuplets[i].id, voice, &*chordIt, tiedNotes });
                backTupletIntervals[voice].push_back(interval);
                usedChords.insert(&*chordIt);
                usedTuplets.insert(tupletIndex);
                break;
            }
        }
    }
    return tiedTuplets;
}

// chord notes should not be rearranged here
// because note indexes are stored in tied tuplets

void assignVoices(
    std::vector<TupletInfo>& tuplets,
    std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets,
    std::list<TiedTuplet>& backTiedTuplets,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    const ReducedFraction& barStart,
    int barIndex)
{
#ifdef QT_DEBUG
    Q_ASSERT_X(!haveTupletsEmptyChords(tuplets),
               "MIDI tuplets: assignVoices", "Empty tuplet chords");
#endif
    auto pendingTuplets = findPendingTuplets(tuplets);
    auto pendingNonTuplets = findPendingNonTuplets(nonTuplets);

    // <voice, intervals>
    std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > > tupletIntervals;

    setBackTiedVoices(backTiedTuplets, tuplets, pendingTuplets, pendingNonTuplets,
                      tupletIntervals, chords, basicQuant, barStart);
    setTupletVoices(tuplets, pendingTuplets, tupletIntervals, basicQuant);
    removeUnusedTuplets(tuplets, nonTuplets, pendingTuplets, backTiedTuplets,
                        pendingNonTuplets, barIndex);

    if (tupletVoiceLimit() == 1) {
        bool excluded = excludeExtraVoiceTuplets(tuplets, nonTuplets, backTiedTuplets,
                                                 chords, basicQuant, barStart, barIndex);
        if (excluded) {             // to exclude tuplet intervals - rebuild all intervals
            tupletIntervals.clear();
            for (const auto& tuplet: tuplets) {
                const int voice = tuplet.chords.begin()->second->second.voice;
                tupletIntervals[voice].push_back(tupletInterval(tuplet, basicQuant));
            }
        }
    }

    setNonTupletVoices(pendingNonTuplets, tupletIntervals, tuplets,
                       chords, basicQuant, barStart);
#ifdef QT_DEBUG
    Q_ASSERT_X(pendingNonTuplets.empty(),
               "MIDI tuplets: assignVoices", "Unused non-tuplets");
    Q_ASSERT_X(!haveTupletsEmptyChords(tuplets),
               "MIDI tuplets: assignVoices", "Empty tuplet chords");
    Q_ASSERT_X(doTupletChordsHaveSameVoice(tuplets),
               "MIDI tuplets: assignVoices", "Tuplet chords have different voices");
    Q_ASSERT_X(!haveOverlappingVoices(nonTuplets, tuplets, backTiedTuplets, chords,
                                      basicQuant, barStart),
               "MIDI tuplets: assignVoices", "Overlapping tuplets of the same voice");
    Q_ASSERT_X(!voiceDontExceedLimit(nonTuplets, tuplets),
               "MIDI tuplets: assignVoices", "Voice exceeds the limit");
#endif
}
} // namespace MidiTuplet
} // namespace mu::iex::midi
