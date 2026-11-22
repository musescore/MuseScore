/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "midirecordingimporter.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include "engraving/editing/paste.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/select.h"
#include "engraving/dom/selectionfilter.h"
#include "engraving/dom/staff.h"
#include "engraving/engravingerrors.h"
#include "engraving/rw/xmlreader.h"

#include "midishared/midifile.h"
#include "midiimport/importmidi_fraction.h"
#include "midiimport/importmidi_operations.h"
#include "midiimport/importmidi_quant.h"

#include "log.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::iex::midi;

namespace mu::iex::midi {
extern Err importRecordedMidi(MasterScore* score, const MidiFile& midiFile, MidiOperations::QuantValue quantValue, bool searchTuplets,
                              bool simplifyDurations);
}

static constexpr int MIDI_CHANNEL = 0;

static int log2OfPowerOfTwo(int value)
{
    if (value <= 0 || (value & (value - 1)) != 0) {
        return -1;
    }

    int result = 0;
    while (value > 1) {
        value >>= 1;
        ++result;
    }

    return result;
}

static MidiEvent makeTimeSigEvent(int numerator, int log2Denominator)
{
    MidiEvent event;
    event.setType(ME_META);
    event.setMetaType(META_TIME_SIGNATURE);
    event.setEData({ static_cast<uchar>(numerator), static_cast<uchar>(log2Denominator), 24, 8 });
    event.setLen(4);

    return event;
}

static MidiEvent makeKeySigEvent(Key key)
{
    MidiEvent event;
    event.setType(ME_META);
    event.setMetaType(META_KEY_SIGNATURE);
    event.setEData({ static_cast<uchar>(static_cast<signed char>(key)), 0 });
    event.setLen(2);

    return event;
}

static MidiOperations::QuantValue quantValueFromTicks(int ticks)
{
    const MidiOperations::QuantValue value = Quantize::fractionToQuantValue(ReducedFraction::fromTicks(ticks));
    if (value == MidiOperations::QuantValue::Q_INVALID) {
        LOGW() << "unsupported quantization: " << ticks << " ticks, using 16th notes";
        return MidiOperations::QuantValue::Q_16;
    }

    return value;
}

RecordedMidiNoteList MidiRecordingImporter::sanitizeNotes(const RecordedMidiNoteList& notes, int maxTicks)
{
    RecordedMidiNoteList result;
    result.reserve(notes.size());

    for (const RecordedMidiNote& note : notes) {
        RecordedMidiNote n = note;
        n.onTick = std::max(n.onTick, 0);
        n.offTick = std::min(n.offTick, maxTicks);

        if (n.onTick >= maxTicks || n.offTick <= n.onTick) {
            continue;
        }

        if (n.pitch < 0 || n.pitch > 127) {
            continue;
        }

        n.velocity = std::clamp(n.velocity, 1, 127);
        result.push_back(n);
    }

    std::sort(result.begin(), result.end(), [](const RecordedMidiNote& n1, const RecordedMidiNote& n2) {
        return n1.onTick < n2.onTick;
    });

    return result;
}

RecordedMidiNoteList MidiRecordingImporter::applyLegato(const RecordedMidiNoteList& notes, int maxGapTicks)
{
    // The notes of a chord are never played exactly together:
    // the next note must start at least this later to be considered the next one
    const int chordTolerance = maxGapTicks / 2;

    RecordedMidiNoteList result = notes;

    for (RecordedMidiNote& note : result) {
        const auto next = std::find_if(notes.cbegin(), notes.cend(), [&note, chordTolerance](const RecordedMidiNote& other) {
            return other.onTick > note.onTick + chordTolerance;
        });

        if (next == notes.cend()) {
            continue; // the last note (or chord): keep its duration
        }

        const int gap = next->onTick - note.offTick;
        if (gap > 0 && gap < maxGapTicks) {
            note.offTick = next->onTick;
        }
    }

    return result;
}

RecordedMidiNoteList MidiRecordingImporter::applyMinimumQuantization(const RecordedMidiNoteList& notes, int gridTicks,
                                                                    bool keepStartTimes)
{
    if (gridTicks <= 0) {
        return notes;
    }

    const auto snap = [gridTicks](int tick) {
        return static_cast<int>(std::lround(static_cast<double>(tick) / gridTicks)) * gridTicks;
    };

    const int chordTolerance = gridTicks / 2;

    RecordedMidiNoteList result;
    result.reserve(notes.size());

    for (const RecordedMidiNote& note : notes) {
        RecordedMidiNote n = note;

        if (keepStartTimes) {
            if (n.offTick - n.onTick < gridTicks) {
                const auto next = std::find_if(notes.cbegin(), notes.cend(), [&note, chordTolerance](const RecordedMidiNote& other) {
                    return other.onTick > note.onTick + chordTolerance;
                });

                int newOffTick = n.onTick + gridTicks;
                if (next != notes.cend()) {
                    newOffTick = std::min(newOffTick, next->onTick);
                }
                n.offTick = std::max(n.offTick, newOffTick);
            }
        } else {
            n.onTick = snap(n.onTick);
            n.offTick = std::max(snap(n.offTick), n.onTick + gridTicks);
        }

        // The same key twice at the same position (e.g. a repeated note moved onto the grid): keep the longest
        auto same = std::find_if(result.begin(), result.end(), [&n](const RecordedMidiNote& other) {
            return other.pitch == n.pitch && other.onTick == n.onTick;
        });

        if (same != result.end()) {
            same->offTick = std::max(same->offTick, n.offTick);
            same->velocity = std::max(same->velocity, n.velocity);
            continue;
        }

        result.push_back(n);
    }

    std::stable_sort(result.begin(), result.end(), [](const RecordedMidiNote& n1, const RecordedMidiNote& n2) {
        return n1.onTick < n2.onTick;
    });

    return result;
}

MidiFile MidiRecordingImporter::makeMidiFile(const Score* score, staff_idx_t staffIdx, const Fraction& startTick,
                                             const RecordedMidiNoteList& notes)
{
    MidiFile midiFile;
    midiFile.setFormat(1);
    midiFile.setDivision(Constants::DIVISION);

    MidiTrack track;
    track.setOutChannel(MIDI_CHANNEL);

    int lastOffTick = 0;
    for (const RecordedMidiNote& note : notes) {
        lastOffTick = std::max(lastOffTick, note.offTick);
    }

    // Reproduce the bar structure of the score from the start measure,
    // using the actual length of each measure (e.g. for irregular measures)
    int prevNumerator = 0;
    int prevDenominator = 0;

    for (const Measure* m = score->tick2measure(startTick); m; m = m->nextMeasure()) {
        const int relTick = (m->tick() - startTick).ticks();
        if (relTick >= lastOffTick) {
            break;
        }

        const Fraction nominal = m->timesig();
        int denominator = nominal.denominator();
        Fraction actualInDenominatorUnits = (m->ticks() * Fraction(denominator, 1)).reduced();

        int numerator = nominal.numerator();
        if (actualInDenominatorUnits.denominator() == 1 && actualInDenominatorUnits.numerator() > 0) {
            numerator = actualInDenominatorUnits.numerator();
        }

        const int log2Denominator = log2OfPowerOfTwo(denominator);
        if (log2Denominator < 0 || numerator > 255) {
            LOGW() << "unsupported time signature: " << numerator << "/" << denominator;
            continue;
        }

        if (numerator == prevNumerator && denominator == prevDenominator) {
            continue;
        }

        track.insert(relTick, makeTimeSigEvent(numerator, log2Denominator));
        prevNumerator = numerator;
        prevDenominator = denominator;
    }

    if (const Staff* staff = score->staff(staffIdx)) {
        // MIDI pitches are sounding pitches, so the concert key is used
        track.insert(0, makeKeySigEvent(staff->concertKey(startTick)));
    }

    for (const RecordedMidiNote& note : notes) {
        MidiEvent event(ME_NOTE, MIDI_CHANNEL, static_cast<uchar>(note.pitch), static_cast<uchar>(note.velocity));
        event.setLen(note.offTick - note.onTick);
        track.insert(note.onTick, event);
    }

    midiFile.tracks().push_back(track);

    return midiFile;
}

//! The end of the measure containing the given position (the range is always made of whole measures)
static Fraction measureEndAtOrAfter(const Score* score, const Fraction& startTick, const Fraction& tick)
{
    if (tick <= startTick) {
        return startTick;
    }

    const Measure* measure = score->tick2measure(tick - Fraction::fromTicks(1));
    return measure ? measure->endTick() : tick;
}

//! Deletes the notes, rests and ties of the staff in the range (they become rests);
//! dynamics, lyrics, chord symbols etc. are preserved
static void clearStaffRange(Score* score, staff_idx_t staffIdx, Measure* startMeasure, const Fraction& endTick)
{
    Segment* start = startMeasure->first(SegmentType::ChordRest);
    Segment* end = score->tick2segment(endTick, true, SegmentType::ChordRest);
    if (!start) {
        return;
    }

    SelectionFilter notesOnlyFilter;
    notesOnlyFilter.setFiltered(ElementsSelectionFilterTypes::ALL, false);
    notesOnlyFilter.setFiltered(ElementsSelectionFilterTypes::TIE, true);

    const track_idx_t startTrack = staffIdx * VOICES;
    score->deleteRange(start, end, startTrack, startTrack + VOICES, notesOnlyFilter, /*selectionContainsMultiNoteChords*/ false);
}

std::pair<RecordedMidiNoteList, RecordedMidiNoteList> MidiRecordingImporter::splitNotes(const RecordedMidiNoteList& notes, int splitPitch)
{
    std::pair<RecordedMidiNoteList, RecordedMidiNoteList> result;

    for (const RecordedMidiNote& note : notes) {
        if (note.pitch >= splitPitch) {
            result.first.push_back(note);
        } else {
            result.second.push_back(note);
        }
    }

    return result;
}

RetVal<int> MidiRecordingImporter::writeRecordedNotesOnTwoStaves(Score* score, staff_idx_t upperStaffIdx, staff_idx_t lowerStaffIdx,
                                                                 const Fraction& startTick, const RecordedMidiNoteList& notes,
                                                                 int splitPitch, const MidiRecordingOptions& options)
{
    RetVal<int> result;
    result.val = 0;

    IF_ASSERT_FAILED(score) {
        result.ret = make_ret(Ret::Code::InternalError);
        return result;
    }

    const Staff* upperStaff = score->staff(upperStaffIdx);
    const Staff* lowerStaff = score->staff(lowerStaffIdx);
    if (!upperStaff || !lowerStaff || upperStaff->part() != lowerStaff->part()) {
        result.ret = make_ret(Ret::Code::NotSupported, std::string("The two staves must belong to the same instrument"));
        return result;
    }

    const Measure* lastMeasure = score->lastMeasure();
    if (!lastMeasure) {
        result.ret = make_ret(Ret::Code::InternalError);
        return result;
    }

    const int availableTicks = (lastMeasure->endTick() - startTick).ticks();
    const RecordedMidiNoteList cleanNotes = sanitizeNotes(notes, availableTicks);
    if (cleanNotes.empty()) {
        result.ret = make_ret(Ret::Code::Cancel);
        return result;
    }

    // Both staves are replaced over the same measures: the whole recording, at least
    int recordedTicks = 0;
    for (const RecordedMidiNote& note : cleanNotes) {
        recordedTicks = std::max(recordedTicks, note.offTick);
    }

    MidiRecordingOptions staffOptions = options;
    staffOptions.minReplacedTicks = std::max(options.minReplacedTicks, recordedTicks);

    const auto [upperNotes, lowerNotes] = splitNotes(cleanNotes, splitPitch);

    RetVal<int> upper = writeRecordedNotes(score, upperStaffIdx, startTick, upperNotes, staffOptions);
    if (!upper.ret) {
        return upper;
    }

    staffOptions.minReplacedTicks = std::max(staffOptions.minReplacedTicks, upper.val);

    const RetVal<int> lower = writeRecordedNotes(score, lowerStaffIdx, startTick, lowerNotes, staffOptions);
    if (!lower.ret) {
        return lower;
    }

    // The quantization made the lower staff longer: replace the same measures in the upper one
    if (lower.val > upper.val) {
        staffOptions.minReplacedTicks = lower.val;
        upper = writeRecordedNotes(score, upperStaffIdx, startTick, upperNotes, staffOptions);
        if (!upper.ret) {
            return upper;
        }
    }

    result.ret = make_ok();
    result.val = std::max(upper.val, lower.val);

    return result;
}

RetVal<int> MidiRecordingImporter::writeRecordedNotes(Score* score, staff_idx_t staffIdx, const Fraction& startTick,
                                                      const RecordedMidiNoteList& notes, const MidiRecordingOptions& options)
{
    RetVal<int> result;
    result.val = 0;

    IF_ASSERT_FAILED(score) {
        result.ret = make_ret(Ret::Code::InternalError);
        return result;
    }

    Measure* startMeasure = score->tick2measure(startTick);
    const Staff* staff = score->staff(staffIdx);
    const Measure* lastMeasure = score->lastMeasure();

    if (!startMeasure || startMeasure->tick() != startTick || !staff || !lastMeasure) {
        result.ret = make_ret(Ret::Code::InternalError, std::string("Invalid recording start position"));
        return result;
    }

    if (staff->isDrumStaff(startTick)) {
        result.ret = make_ret(Ret::Code::NotSupported, std::string("MIDI recording on percussion staves is not supported yet"));
        return result;
    }

    const int availableTicks = (lastMeasure->endTick() - startTick).ticks();
    RecordedMidiNoteList cleanNotes = sanitizeNotes(notes, availableTicks);
    if (options.legato) {
        cleanNotes = applyLegato(cleanNotes, options.minQuantizationTicks);
    }

    cleanNotes = applyMinimumQuantization(cleanNotes, options.minQuantizationTicks, /*keepStartTimes*/ options.searchTuplets);
    cleanNotes = sanitizeNotes(cleanNotes, availableTicks); // the notes moved onto the grid may exceed the last measure
    if (cleanNotes.empty()) {
        if (options.minReplacedTicks <= 0) {
            result.ret = make_ret(Ret::Code::Cancel);
            return result;
        }

        // Nothing played on this staff (e.g. the lower staff of a split recording): only rests
        const Fraction endTick = std::min(measureEndAtOrAfter(score, startTick, startTick + Fraction::fromTicks(options.minReplacedTicks)),
                                          lastMeasure->endTick());
        clearStaffRange(score, staffIdx, startMeasure, endTick);

        result.ret = make_ok();
        result.val = (endTick - startTick).ticks();
        return result;
    }

    // 1. Convert the performance with the MIDI import engine, in a temporary score
    const MidiFile midiFile = makeMidiFile(score, staffIdx, startTick, cleanNotes);

    std::unique_ptr<MasterScore> tempScore(compat::ScoreAccess::createMasterScoreWithBaseStyle(score->iocContext()));
    tempScore->checkChordList();

    const Err err = importRecordedMidi(tempScore.get(), midiFile, quantValueFromTicks(options.minQuantizationTicks),
                                       options.searchTuplets, /*simplifyDurations*/ options.legato);
    if (err != Err::NoError) {
        result.ret = make_ret(err);
        return result;
    }

    if (tempScore->nstaves() == 0 || !tempScore->firstMeasure()) {
        result.ret = make_ret(Ret::Code::InternalError, std::string("MIDI conversion produced no staves"));
        return result;
    }

    tempScore->doLayout();

    // 2. Copy the converted staff, exactly like a range copy
    Segment* srcStart = tempScore->firstMeasure()->first(SegmentType::ChordRest);
    if (!srcStart) {
        result.ret = make_ret(Ret::Code::InternalError);
        return result;
    }

    Selection& srcSelection = tempScore->selection();
    srcSelection.setRange(srcStart, nullptr, 0, 1);
    srcSelection.updateSelectedElements();

    const Fraction convertedTicks = tempScore->lastMeasure()->endTick();
    // In range state, the mime data is the staff list (the same used by copy & paste)
    const ByteArray staffData = srcSelection.mimeData();

    // 3. Replace the content of the destination staff over the recorded measures
    // Also the previous take of the same recording, if it was longer
    const Fraction replacedTicks = std::max(convertedTicks, Fraction::fromTicks(options.minReplacedTicks));

    const Fraction dstEndTick = std::min(measureEndAtOrAfter(score, startTick, startTick + replacedTicks), lastMeasure->endTick());
    clearStaffRange(score, staffIdx, startMeasure, dstEndTick);

    // The segments may have been replaced by the deletion
    Segment* dstStart = startMeasure->first(SegmentType::ChordRest);
    if (!dstStart) {
        result.ret = make_ret(Ret::Code::InternalError);
        return result;
    }

    XmlReader reader(staffData);
    Transaction& tx = score->transactionManager()->currentOrDummyTransaction();
    if (!Paste::pasteStaff(tx, score, reader, dstStart, staffIdx, Fraction(1, 1))) {
        result.ret = make_ret(Ret::Code::InternalError, std::string("Unable to write the recorded notes"));
        return result;
    }

    result.ret = make_ok();
    result.val = (dstEndTick - startTick).ticks();

    return result;
}
