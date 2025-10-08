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

#include "engraving/dom/score.h"
#include "engraving/dom/part.h"
#include "engraving/dom/note.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/tempotext.h"

using namespace mu::converter;
using namespace mu::engraving;

struct ScannerData {
    using ElementKey = std::pair<ElementType, int /*subtype*/>;

    // in
    ScoreElementScanner::Options options;

    // out
    ScoreElementScanner::InstrumentElementMap elements;
    std::set<Chord*> chords;
    std::set<Spanner*> spanners;
    std::map<InstrumentTrackId, std::map<ElementKey, std::set<muse::String> > > uniqueNames;
};

static bool itemAccepted(const EngravingItem* item, const ElementTypeSet& acceptedTypes)
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

    if (!item->visible() && !item->score()->isShowInvisible()) {
        return false;
    }

    if (acceptedTypes.empty()) {
        return true;
    }

    ElementType type = item->type();

    if (item->isNote()) {
        const Chord* chord = toNote(item)->chord();
        if (chord->notes().size() > 1) {
            type = chord->type();
        }
    } else if (item->isSpannerSegment()) {
        type = toSpannerSegment(item)->spanner()->type();
    }

    return muse::contains(acceptedTypes, type);
}

static bool isChordArticulation(const EngravingItem* item)
{
    const EngravingItem* parent = item->parentItem();
    if (!parent || !parent->isChord()) {
        return false;
    }

    static const std::unordered_set<ElementType> CHORD_ARTICULATION_TYPES {
        ElementType::ARPEGGIO,
        ElementType::TREMOLO_SINGLECHORD,
        ElementType::ORNAMENT,
    };

    return muse::contains(CHORD_ARTICULATION_TYPES, item->type());
}

static muse::String chordToNotes(const Chord* chord)
{
    muse::StringList notes;

    for (const Note* note : chord->notes()) {
        notes.push_back(note->tpcUserName());
    }

    return notes.join(u" ");
}

static void addElementInfoIfNeed(ScannerData* scannerData, EngravingItem* item)
{
    if (!itemAccepted(item, scannerData->options.acceptedTypes)) {
        return;
    }

    ElementType type = item->type();
    ScoreElementScanner::ElementInfo info;
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
            info.name = note->tpcUserName();
        }
    } else if (isChordArticulation(item)) {
        Chord* chord = toChord(item->parentItem());
        scannerData->chords.insert(chord);
        info.name = item->translatedSubtypeUserName();
        info.notes = chordToNotes(chord);
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
            info.start.trackIdx = spanner->track();
        }

        const Segment* endSegment = spanner->endSegment();
        if (endSegment) {
            const EngravingItem::BarBeat barbeat = endSegment->barbeat();
            info.end.measureIdx = barbeat.bar - 1;
            info.end.beat = barbeat.beat - 1.;
            info.end.trackIdx = spanner->track2();
        }

        locationIsSet = startSegment || endSegment;
    } else if (item->isHarmony()) {
        info.name = toHarmony(item)->harmonyName();
    } else if (item->isTempoText()) {
        info.text = toTempoText(item)->tempoInfo();
    } else if (item->isPlayTechAnnotation() || item->isDynamic()) {
        info.name = item->translatedSubtypeUserName();
    } else if (item->isTextBase()) {
        info.text = toTextBase(item)->plainText();
    } else {
        info.name = item->translatedSubtypeUserName();
    }

    if (info.name.empty() && info.notes.empty() && info.text.empty()) {
        info.name = item->typeUserName().translated();
    }

    const Part* part = item->part();
    const InstrumentTrackId trackId {
        part->id(),
        part->instrumentId(item->tick())
    };

    if (scannerData->options.avoidDuplicates) {
        const muse::String& name = !info.name.empty() ? info.name : info.notes;
        const ScannerData::ElementKey key = std::make_pair(type, item->subtype());
        std::set<muse::String>& uniqueNames = scannerData->uniqueNames[trackId][key];

        if (muse::contains(uniqueNames, name)) {
            return;
        }

        uniqueNames.insert(name);
    }

    if (!locationIsSet) {
        const EngravingItem::BarBeat barbeat = item->barbeat();
        info.start.trackIdx = item->track();
        info.start.measureIdx = barbeat.bar - 1;
        info.start.beat = barbeat.beat - 1.;
        info.end = info.start;
    }

    scannerData->elements[trackId][type].push_back(info);
}

ScoreElementScanner::InstrumentElementMap ScoreElementScanner::scanElements(Score* score, const Options& options)
{
    TRACEFUNC;

    ScannerData data;
    data.options = options;

    score->scanElements([&](mu::engraving::EngravingItem* item) { addElementInfoIfNeed(&data, item); });

    return data.elements;
}
