/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "scoreelementsscanner.h"

#include "engraving/types/typesconv.h"
#include "engraving/dom/score.h"
#include "engraving/dom/part.h"
#include "engraving/dom/note.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/dynamic.h"

using namespace mu::converter;
using namespace mu::engraving;

struct ScannerData {
    // out
    ElementMap elements;
    std::set<Chord*> chords;
    std::set<Spanner*> spanners;
};

static bool itemAccepted(const EngravingItem* item)
{
    // Ignore temporary / invalid elements and elements that cannot be interacted with
    if (!item || !item->part() || !item->selectable() || !item->isInteractionAvailable()) {
        return false;
    }

    static const ElementTypeSet NOTE_PARTS {
        ElementType::ACCIDENTAL,
        ElementType::STEM,
        ElementType::HOOK,
        ElementType::BEAM,
        ElementType::NOTEDOT,
    };

    if (muse::contains(NOTE_PARTS, item->type())) {
        return false;
    }

    if (item->isBarLine() && item->tick().ticks() == 0) {
        return false;
    }

    return true;
}

static bool isChordArticulation(const EngravingItem* item)
{
    const EngravingItem* parent = item->parentItem();
    if (!parent || !parent->isChord()) {
        return false;
    }

    static const ElementTypeSet CHORD_ARTICULATION_TYPES {
        ElementType::ARPEGGIO,
        ElementType::TREMOLO_SINGLECHORD,
        ElementType::ORNAMENT,
    };

    return muse::contains(CHORD_ARTICULATION_TYPES, item->type());
}

static muse::String noteName(const Note* note)
{
    return tpc2name(note->tpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO) + String::number(note->octave());
}

static muse::String chordToNotes(const Chord* chord)
{
    muse::StringList notes;
    notes.reserve(chord->notes().size());

    for (const Note* note : chord->notes()) {
        notes.push_back(noteName(note));
    }

    return notes.join(u" ");
}

static void addElementInfoIfNeed(ScannerData* scannerData, EngravingItem* item)
{
    if (!itemAccepted(item)) {
        return;
    }

    ElementType type = item->type();
    ElementInfo info;
    bool locationIsSet = false;

    if (item->isNote()) {
        Note* note = toNote(item);
        Chord* chord = note->chord();
        if (chord->notes().size() > 1) {
            if (muse::contains(scannerData->chords, chord)) {
                return;
            }

            type = chord->type();
            scannerData->chords.insert(chord);
            info.notes = chordToNotes(chord);
        } else {
            info.name = noteName(note);
        }
        info.duration = chord->durationUserName();
    } else if (isChordArticulation(item)) {
        Chord* chord = toChord(item->parentItem());
        scannerData->chords.insert(chord);
        info.name = item->translatedSubtypeUserName();
        info.notes = chordToNotes(chord);
        info.duration = chord->durationUserName();
    } else if (item->isRest()) {
        info.duration = toRest(item)->durationUserName();
    } else if (item->isSpannerSegment()) {
        Spanner* spanner = toSpannerSegment(item)->spanner();
        if (muse::contains(scannerData->spanners, spanner)) {
            return;
        }

        item = spanner;
        type = spanner->type();
        info.name = spanner->translatedSubtypeUserName();
        scannerData->spanners.insert(spanner);

        const Segment* startSegment = spanner->startSegment();
        if (startSegment) {
            const EngravingItem::BarBeat barbeat = startSegment->barbeat();
            info.start.measureIdx = barbeat.bar - 1;
            info.start.beat = barbeat.beat - 1.;
            info.start.staffIdx = track2staff(spanner->track());
            info.start.voiceIdx = track2voice(spanner->track());
        }

        const Segment* endSegment = spanner->endSegment();
        if (endSegment) {
            const EngravingItem::BarBeat barbeat = endSegment->barbeat();
            info.end.measureIdx = barbeat.bar - 1;
            info.end.beat = barbeat.beat - 1.;
            info.end.staffIdx = track2staff(spanner->track2());
            info.end.voiceIdx = track2voice(spanner->track2());
        }

        locationIsSet = startSegment || endSegment;
    } else if (item->isHarmony()) {
        info.name = toHarmony(item)->harmonyName();
    } else if (item->isTempoText()) {
        info.text = toTempoText(item)->tempoInfo();
    } else if (item->isPlayTechAnnotation()) {
        info.name = item->translatedSubtypeUserName();
    } else if (item->isDynamic()) {
        info.name = TConv::toXml(toDynamic(item)->dynamicType()).ascii();
    } else if (item->isTextBase()) {
        info.text = toTextBase(item)->plainText();
    } else {
        info.name = item->translatedSubtypeUserName();
    }

    if (info.name.empty() && info.notes.empty() && info.text.empty()) {
        info.name = item->typeUserName().translated();
    }

    info.type = type;

    const Part* part = item->part();
    const InstrumentTrackId trackId {
        part->id(),
        part->instrumentId(item->tick())
    };

    if (!locationIsSet) {
        const EngravingItem::BarBeat barbeat = item->barbeat();
        info.start.staffIdx = item->staffIdx();
        info.start.voiceIdx = item->voice();
        info.start.measureIdx = barbeat.bar - 1;
        info.start.beat = barbeat.beat - 1.;
        info.end = info.start;
    }

    scannerData->elements[trackId].emplace_back(std::move(info));
}

ElementMap ScoreElementScanner::scanElements(Score* score)
{
    TRACEFUNC;

    ScannerData data;
    score->scanElements([&](mu::engraving::EngravingItem* item) { addElementInfoIfNeed(&data, item); });

    // Sort elements: staff -> measure -> beat -> voice
    for (auto& pair : data.elements) {
        std::stable_sort(pair.second.begin(), pair.second.end(), [](const ElementInfo& a, const ElementInfo& b) {
            if (a.start.staffIdx != b.start.staffIdx) {
                return a.start.staffIdx < b.start.staffIdx;
            }
            if (a.start.measureIdx != b.start.measureIdx) {
                return a.start.measureIdx < b.start.measureIdx;
            }
            if (a.start.beat != b.start.beat) {
                return a.start.beat < b.start.beat;
            }
            return a.start.voiceIdx < b.start.voiceIdx;
        });
    }

    return data.elements;
}
