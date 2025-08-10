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
#include "importmidi_chord.h"

#include <array>

#include "global/containers.h"

#include "importmidi_inner.h"
#include "importmidi_chord.h"
#include "importmidi_operations.h"
#include "importmidi_quant.h"
#include "engraving/dom/sig.h"

#include "log.h"

namespace mu::iex::midi {
namespace MChord {
bool isGrandStaffProgram(GM1Program program)
{
    static constexpr std::array grandStaffPrograms = {
        GM1Program::AcousticGrandPiano, GM1Program::BrightAcousticPiano,
        GM1Program::ElectricGrandPiano, GM1Program::HonkyTonkPiano,
        GM1Program::ElectricPiano1,     GM1Program::ElectricPiano2,
        GM1Program::Harpsichord,        GM1Program::Clavi,

        GM1Program::Celesta,    GM1Program::MusicBox,
        GM1Program::Vibraphone, GM1Program::Marimba,
        GM1Program::Xylophone,  GM1Program::Dulcimer,

        GM1Program::DrawbarOrgan,   GM1Program::PercussiveOrgan,
        GM1Program::RockOrgan,      GM1Program::ChurchOrgan,
        GM1Program::ReedOrgan,      GM1Program::Accordion,
        GM1Program::TangoAccordion,

        GM1Program::OrchestralHarp,

        GM1Program::SynthStrings1, GM1Program::SynthStrings2,
        GM1Program::SynthVoice,

        GM1Program::SynthBrass1, GM1Program::SynthBrass2,

        GM1Program::Lead1Square,   GM1Program::Lead2Sawtooth,
        GM1Program::Lead3Calliope, GM1Program::Lead4Chiff,
        GM1Program::Lead5Charang,  GM1Program::Lead6Voice,
        GM1Program::Lead7Fifths,   GM1Program::Lead8BassAndLead,

        GM1Program::Pad1NewAge,    GM1Program::Pad2Warm,
        GM1Program::Pad3Polysynth, GM1Program::Pad4Choir,
        GM1Program::Pad5Bowed,     GM1Program::Pad6Metallic,
        GM1Program::Pad7Halo,      GM1Program::Pad8Sweep,

        GM1Program::FX1Rain,       GM1Program::FX2Soundtrack,
        GM1Program::FX3Crystal,    GM1Program::FX4Atmosphere,
        GM1Program::FX5Brightness, GM1Program::FX6Goblins,
        GM1Program::FX7Echoes,     GM1Program::FX8SciFi,
    };

    return muse::contains(grandStaffPrograms, program);
}

std::multimap<ReducedFraction, MidiChord>::iterator
findFirstChordInRange(std::multimap<ReducedFraction, MidiChord>& chords,
                      const ReducedFraction& startRangeTick,
                      const ReducedFraction& endRangeTick)
{
    auto iter = chords.lower_bound(startRangeTick);
    if (iter != chords.end() && iter->first >= endRangeTick) {
        iter = chords.end();
    }
    return iter;
}

std::multimap<ReducedFraction, MidiChord>::const_iterator
findFirstChordInRange(const std::multimap<ReducedFraction, MidiChord>& chords,
                      const ReducedFraction& startRangeTick,
                      const ReducedFraction& endRangeTick)
{
    auto iter = chords.lower_bound(startRangeTick);
    if (iter != chords.end() && iter->first >= endRangeTick) {
        iter = chords.end();
    }
    return iter;
}

const ReducedFraction& minAllowedDuration()
{
    const static auto minDuration = ReducedFraction::fromTicks(engraving::Constants::DIVISION) / 32;
    return minDuration;
}

ReducedFraction minNoteOffTime(const QList<MidiNote>& notes)
{
    if (notes.isEmpty()) {
        return { 0, 1 };
    }
    auto it = notes.begin();
    ReducedFraction minOffTime = it->offTime;
    for (++it; it != notes.end(); ++it) {
        if (it->offTime < minOffTime) {
            minOffTime = it->offTime;
        }
    }
    return minOffTime;
}

ReducedFraction maxNoteOffTime(const QList<MidiNote>& notes)
{
    ReducedFraction maxOffTime(0, 1);
    for (const auto& note: notes) {
        if (note.offTime > maxOffTime) {
            maxOffTime = note.offTime;
        }
    }
    return maxOffTime;
}

ReducedFraction minNoteLen(const std::pair<const ReducedFraction, MidiChord>& chord)
{
    const auto minOffTime = minNoteOffTime(chord.second.notes);
    return minOffTime - chord.first;
}

ReducedFraction maxNoteLen(const std::pair<const ReducedFraction, MidiChord>& chord)
{
    const auto maxOffTime = maxNoteOffTime(chord.second.notes);
    return maxOffTime - chord.first;
}

void removeOverlappingNotes(QList<MidiNote>& notes)
{
    std::list<MidiNote> tempNotes;
    for (const auto& note: notes) {
        tempNotes.push_back(note);
    }

    for (auto noteIt1 = tempNotes.begin(); noteIt1 != tempNotes.end(); ++noteIt1) {
        for (auto noteIt2 = std::next(noteIt1); noteIt2 != tempNotes.end();) {
            if (noteIt2->pitch == noteIt1->pitch) {
                // overlapping notes found
                if (noteIt2->offTime > noteIt1->offTime) {            // set max len before erase
                    noteIt1->offTime = noteIt2->offTime;
                }
                noteIt2 = tempNotes.erase(noteIt2);
                LOGD() << "MIDI import: removeOverlappingNotes: note was removed";
                continue;
            }
            ++noteIt2;
        }
    }
    notes.clear();
    for (const auto& note: tempNotes) {
        notes.append(note);
    }
}

// remove overlapping notes with the same pitch

void removeOverlappingNotes(std::multimap<int, MTrack>& tracks)
{
    for (auto& track: tracks) {
        auto& chords = track.second.chords;
        if (chords.empty()) {
            continue;
        }
#ifdef QT_DEBUG
        Q_ASSERT_X(MidiTuplet::areTupletRangesOk(chords, track.second.tuplets),
                   "MChord::removeOverlappingNotes", "Tuplet chord/note is outside tuplet "
                                                     "or non-tuplet chord/note is inside tuplet before overlaps remove");
#endif
        for (auto i1 = chords.begin(); i1 != chords.end();) {
            const auto& onTime1 = i1->first;
            auto& chord1 = i1->second;
            removeOverlappingNotes(chord1.notes);

            for (auto note1It = chord1.notes.begin(); note1It != chord1.notes.end();) {
                auto& note1 = *note1It;

                for (auto i2 = std::next(i1); i2 != chords.end(); ++i2) {
                    const auto& onTime2 = i2->first;
                    if (onTime2 >= note1.offTime) {
                        break;
                    }
                    auto& chord2 = i2->second;
                    if (chord1.voice != chord2.voice) {
                        continue;
                    }
                    for (auto& note2: chord2.notes) {
                        if (note2.pitch != note1.pitch) {
                            continue;
                        }
                        // overlapping notes found
                        note1.offTime = onTime2;
                        if (!note1.isInTuplet && chord2.isInTuplet) {
                            if (note1.offTime > chord2.tuplet->second.onTime) {
                                note1.isInTuplet = true;
                                note1.tuplet = chord2.tuplet;
                            }
                        } else if (note1.isInTuplet && !chord2.isInTuplet) {
                            note1.isInTuplet = false;
                        }

                        i2 = std::prev(chords.end());
                        break;
                    }
                }
                if (note1.offTime - onTime1 < MChord::minAllowedDuration()) {
                    note1It = chord1.notes.erase(note1It);
                    LOGD("MIDI import: removeOverlappingNotes: note was removed");
                    continue;
                }
                ++note1It;
            }
            if (chord1.notes.isEmpty()) {
                i1 = chords.erase(i1);
                continue;
            }
            ++i1;
        }

        MidiTuplet::removeEmptyTuplets(track.second);
#ifdef QT_DEBUG
        Q_ASSERT_X(MidiTuplet::areTupletRangesOk(chords, track.second.tuplets),
                   "MChord::removeOverlappingNotes", "Tuplet chord/note is outside tuplet "
                                                     "or non-tuplet chord/note is inside tuplet after overlaps remove");
#endif
    }
}

#ifdef QT_DEBUG

// check for equal on time values with the same voice that is invalid
bool areOnTimeValuesDifferent(const std::multimap<ReducedFraction, MidiChord>& chords)
{
    std::map<ReducedFraction, int> onTimeVoices;
    for (const auto& chordEvent: chords) {
        const auto it = onTimeVoices.find(chordEvent.first);
        if (it == onTimeVoices.end()) {
            onTimeVoices.insert({ chordEvent.first, chordEvent.second.voice });
        } else if (chordEvent.second.voice == it->second) {
            return false;
        }
    }
    return true;
}

bool areNotesLongEnough(const std::multimap<ReducedFraction, MidiChord>& chords)
{
    for (const auto& chord: chords) {
        if (minNoteLen(chord) < minAllowedDuration()) {
            return false;
        }
    }
    return true;
}

bool areBarIndexesSuccessive(const std::multimap<ReducedFraction, MidiChord>& chords)
{
    int barIndex = 0;
    for (const auto& chord: chords) {
        const MidiChord& c = chord.second;
        if (c.barIndex < 0) {
            return false;
        }
        if (c.barIndex < barIndex) {
            return false;
        }
        barIndex = c.barIndex;
    }
    return true;
}

bool isLastTickValid(const ReducedFraction& lastTick,
                     const std::multimap<ReducedFraction, MidiChord>& chords)
{
    for (const auto& chord: chords) {
        if (maxNoteOffTime(chord.second.notes) > lastTick) {
            return false;
        }
    }
    return true;
}

bool isLastTickValid(const ReducedFraction& lastTick,
                     const std::multimap<int, MTrack>& tracks)
{
    for (const auto& track: tracks) {
        if (!(isLastTickValid(lastTick, track.second.chords))) {
            return false;
        }
    }
    return true;
}

bool areBarIndexesSet(const std::multimap<ReducedFraction, MidiChord>& chords)
{
    for (const auto& chord: chords) {
        if (chord.second.barIndex == -1) {
            return false;
        }
    }
    return true;
}

#endif

void setToNegative(ReducedFraction& v1, ReducedFraction& v2, ReducedFraction& v3)
{
    v1 = ReducedFraction(-1, 1);
    v2 = ReducedFraction(-1, 1);
    v3 = ReducedFraction(-1, 1);
}

bool hasNotesWithEqualPitch(const MidiChord& chord1, const MidiChord& chord2)
{
    std::set<int> notes1;
    for (const auto& note: chord1.notes) {
        notes1.insert(note.pitch);
    }
    for (const auto& note: chord2.notes) {
        if (notes1.find(note.pitch) != notes1.end()) {
            return true;
        }
    }
    return false;
}

void collectChords(
    std::multimap<int, MTrack>& tracks,
    const ReducedFraction& humanTolCoeff,
    const ReducedFraction& nonHumanTolCoeff)
{
    for (auto& track: tracks) {
        collectChords(track.second, humanTolCoeff, nonHumanTolCoeff);
    }
}

// based on quickthresh algorithm
//
// http://www.cycling74.com/docs/max5/refpages/max-ref/quickthresh.html
// (link date 9 July 2013)
//
// here are default values for audio, in milliseconds
// for midi there will be another values, in ticks

// all notes received in the left inlet within this time period are collected into a chord
// threshTime = 40 ms

// if there are any incoming values within this amount of time
// at the end of the base thresh time,
// the threshold is extended to allow more notes to be added to the chord
// fudgeTime = 10 ms

// this is an extension value of the base thresh time, which is used if notes arrive
// in the object's inlet in the "fudge" time zone
// threshExtTime = 20 ms

//     chord                             |<--fudge time-->|
// ------x-------------------------------|----------------|---------------------|------
//       |<-----------------thresh time------------------>|<--thresh ext time-->|
//
void collectChords(
    MTrack& track,
    const ReducedFraction& humanTolCoeff,
    const ReducedFraction& nonHumanTolCoeff)
{
    auto& chords = track.chords;
    if (chords.empty()) {
        return;
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(areNotesLongEnough(chords),
               "MChord::collectChords", "There are too short notes");
#endif

    const auto& opers = midiImportOperations.data()->trackOpers;
    const auto minAllowedDur = minAllowedDuration();

    const auto threshTime = (opers.isHumanPerformance.value())
                            ? minAllowedDur * humanTolCoeff
                            : minAllowedDur * nonHumanTolCoeff;
    const auto fudgeTime = threshTime / 4;
    const auto threshExtTime = threshTime / 2;

    ReducedFraction currentChordStart;
    ReducedFraction curThreshTime;
    // if note onTime goes after max chord offTime
    // then this is not a chord but arpeggio
    ReducedFraction maxOffTime;

    setToNegative(currentChordStart, curThreshTime, maxOffTime);   // invalidate

    for (auto it = chords.begin(); it != chords.end();) {
        if (it->second.isInTuplet) {
            setToNegative(currentChordStart, curThreshTime, maxOffTime);
            ++it;
            continue;
        }

        const auto maxNoteOffTime = MChord::maxNoteOffTime(it->second.notes);
        if (it->first < currentChordStart + curThreshTime) {
            // this branch should not be executed when it == chords.begin()
            Q_ASSERT_X(it != chords.begin(),
                       "MChord: collectChords", "it == chords.begin()");

            if (it->first <= maxOffTime - minAllowedDur) {
                // add current note to the previous chord
                auto chordAddTo = std::prev(it);
                if (it->second.voice != chordAddTo->second.voice) {
                    setToNegative(currentChordStart, curThreshTime, maxOffTime);
                    ++it;
                    continue;
                }

                if (!hasNotesWithEqualPitch(chordAddTo->second, it->second)) {
                    for (const auto& note: std::as_const(it->second.notes)) {
                        chordAddTo->second.notes.push_back(note);
                    }
                    if (maxNoteOffTime > maxOffTime) {
                        maxOffTime = maxNoteOffTime;
                    }
                }
                if (it->first >= currentChordStart + curThreshTime - fudgeTime
                    && curThreshTime == threshTime) {
                    curThreshTime += threshExtTime;
                }

                it = chords.erase(it);
                continue;
            }
        }

        currentChordStart = it->first;
        maxOffTime = maxNoteOffTime;
        curThreshTime = threshTime;
        ++it;
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(areOnTimeValuesDifferent(chords),
               "MChord: collectChords",
               "onTime values of chords are equal but should be different");
#endif
}

void sortNotesByPitch(std::multimap<ReducedFraction, MidiChord>& chords)
{
    struct {
        bool operator()(const MidiNote& note1, const MidiNote& note2)
        {
            return note1.pitch < note2.pitch;
        }
    } pitchSort;

    for (auto& chordEvent: chords) {
        // in each chord sort notes by pitches
        auto& notes = chordEvent.second.notes;
        std::sort(notes.begin(), notes.end(), pitchSort);
    }
}

void sortNotesByLength(std::multimap<ReducedFraction, MidiChord>& chords)
{
    struct {
        bool operator()(const MidiNote& note1, const MidiNote& note2)
        {
            return note1.offTime < note2.offTime;
        }
    } lenSort;

    for (auto& chordEvent: chords) {
        // in each chord sort notes by lengths
        auto& notes = chordEvent.second.notes;
        std::sort(notes.begin(), notes.end(), lenSort);
    }
}

// find notes of each chord that have different durations
// and separate them into different chords
// so all notes inside every chord will have equal lengths

void splitUnequalChords(std::multimap<int, MTrack>& tracks)
{
    for (auto& track: tracks) {
        std::vector<std::pair<ReducedFraction, MidiChord> > newChordEvents;
        auto& chords = track.second.chords;
        if (chords.empty()) {
            continue;
        }
        sortNotesByLength(chords);
        for (auto& chordEvent: chords) {
            auto& chord = chordEvent.second;
            auto& notes = chord.notes;
            ReducedFraction offTime;
            for (auto it = notes.begin(); it != notes.end();) {
                if (it == notes.begin()) {
                    offTime = it->offTime;
                } else {
                    ReducedFraction newOffTime = it->offTime;
                    if (newOffTime != offTime) {
                        MidiChord newChord(chord);
                        newChord.notes.clear();
                        for (int j = it - notes.begin(); j > 0; --j) {
                            newChord.notes.push_back(notes[j - 1]);
                        }
                        newChordEvents.push_back({ chordEvent.first, newChord });
                        it = notes.erase(notes.begin(), it);
                        continue;
                    }
                }
                ++it;
            }
        }
        for (const auto& event: newChordEvents) {
            chords.insert(event);
        }
    }
}

ReducedFraction findMinDuration(const ReducedFraction& onTime,
                                const QList<MidiChord>& midiChords,
                                const ReducedFraction& length)
{
    ReducedFraction len = length;
    for (const auto& chord: midiChords) {
        for (const auto& note: chord.notes) {
            if ((note.offTime - onTime < len)
                && (note.offTime - onTime != ReducedFraction(0, 1))) {
                len = note.offTime - onTime;
            }
        }
    }
    return len;
}

void mergeChordsWithEqualOnTimeAndVoice(std::multimap<int, MTrack>& tracks)
{
    for (auto& track: tracks) {
        auto& chords = track.second.chords;
        if (chords.empty()) {
            continue;
        }
        // the key is pair<onTime, voice>
        std::map<std::pair<ReducedFraction, int>,
                 std::multimap<ReducedFraction, MidiChord>::iterator> onTimes;

        for (auto it = chords.begin(); it != chords.end();) {
            const auto& onTime = it->first;
            const int voice = it->second.voice;
            auto fit = onTimes.find({ onTime, voice });
            if (fit == onTimes.end()) {
                onTimes.insert({ { onTime, voice }, it });
            } else {
                auto& oldNotes = fit->second->second.notes;
                auto& newNotes = it->second.notes;
                oldNotes.append(newNotes);
                it = chords.erase(it);
                continue;
            }
            ++it;
        }
    }
}

int chordAveragePitch(const QList<MidiNote>& notes, int beg, int end)
{
    Q_ASSERT_X(!notes.isEmpty(), "MChord::chordAveragePitch", "Empty notes");
    Q_ASSERT_X(end > 0 && beg >= 0 && end > beg,
               "MChord::chordAveragePitch", "Invalid note indexes");

    int sum = 0;
    for (int i = beg; i != end; ++i) {
        sum += notes[i].pitch;
    }
    return qRound(sum * 1.0 / (end - beg));
}

int chordAveragePitch(const QList<MidiNote>& notes)
{
    Q_ASSERT_X(!notes.isEmpty(), "MChord::chordAveragePitch", "Empty notes");

    return chordAveragePitch(notes, 0, notes.size());
}

// it's an optimization function: we can don't check chords
// with (on time + max chord len) < given time moment
// because chord cannot be longer than found max length

ReducedFraction findMaxChordLength(const std::multimap<ReducedFraction, MidiChord>& chords)
{
    ReducedFraction maxChordLength;

    for (const auto& chord: chords) {
        const auto offTime = maxNoteOffTime(chord.second.notes);
        if (offTime - chord.first > maxChordLength) {
            maxChordLength = offTime - chord.first;
        }
    }
    return maxChordLength;
}

std::vector<std::multimap<ReducedFraction, MidiChord>::const_iterator>
findChordsForTimeRange(
    int voice,
    const ReducedFraction& onTime,
    const ReducedFraction& offTime,
    const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& maxChordLength)
{
    std::vector<std::multimap<ReducedFraction, MidiChord>::const_iterator> result;

    if (chords.empty()) {
        return result;
    }

    auto it = chords.lower_bound(offTime);
    if (it == chords.begin()) {
        return result;
    }
    --it;

    while (it->first + maxChordLength > onTime) {
        const MidiChord& chord = it->second;
        if (chord.voice == voice) {
            const auto chordInterval = std::make_pair(it->first, maxNoteOffTime(chord.notes));
            const auto durationInterval = std::make_pair(onTime, offTime);

            if (MidiTuplet::haveIntersection(chordInterval, durationInterval)) {
                result.push_back(it);
            }
        }
        if (it == chords.begin()) {
            break;
        }
        --it;
    }

    return result;
}

void setBarIndexes(
    std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant,
    const ReducedFraction& lastTick,
    const engraving::TimeSigMap* sigmap)
{
    if (chords.empty()) {
        return;
    }
    auto it = chords.begin();
    for (int barIndex = 0;; ++barIndex) {         // iterate over all measures by indexes
        const auto endBarTick = ReducedFraction::fromTicks(sigmap->bar2tick(barIndex + 1, 0));
        if (endBarTick <= it->first) {
            continue;
        }
        for (; it != chords.end(); ++it) {
            const auto onTime = Quantize::findQuantizedChordOnTime(*it, basicQuant);
#ifdef QT_DEBUG
            const auto barStart = ReducedFraction::fromTicks(sigmap->bar2tick(barIndex, 0));
            Q_ASSERT_X(!(it->first >= barStart && onTime < barStart),
                       "MChord::setBarIndexes", "quantized on time cannot be in previous bar");
#endif
            if (onTime < endBarTick) {
                it->second.barIndex = barIndex;
                continue;
            }
            break;
        }
        if (it == chords.end() || endBarTick > lastTick) {
            break;
        }
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(areBarIndexesSet(chords),
               "MChord::setBarIndexes", "Not all bar indexes were set");
    Q_ASSERT_X(areBarIndexesSuccessive(chords),
               "MChord::setBarIndexes", "Bar indexes are not successive");
#endif
}
} // namespace MChord
} // namespace mu::iex::midi
