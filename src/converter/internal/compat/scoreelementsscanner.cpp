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

struct ScannerData {
    using ElementKey = std::pair<mu::engraving::ElementType, int /*subtype*/>;

    // in
    ScoreElementScanner::Options options;

    // out
    ScoreElementScanner::InstrumentElementMap elements;
    std::set<mu::engraving::Chord*> chords;
    std::set<mu::engraving::Spanner*> spanners;
    std::map<mu::engraving::InstrumentTrackId, std::map<ElementKey, std::set<muse::String> > > uniqueNames;
};

static bool itemAccepted(const mu::engraving::EngravingItem* item, const mu::engraving::ElementTypeSet& acceptedTypes)
{
    // Ignore temporary / invalid elements and elements that cannot be interacted with
    if (!item || !item->part() || item->generated() || !item->selectable() || !item->isInteractionAvailable()) {
        return false;
    }

    if (acceptedTypes.empty()) {
        return true;
    }

    mu::engraving::ElementType type = item->type();

    if (item->isNote()) {
        const mu::engraving::Chord* chord = toNote(item)->chord();
        if (chord->notes().size() > 1) {
            type = chord->type();
        }
    } else if (item->isSpannerSegment()) {
        type = mu::engraving::toSpannerSegment(item)->spanner()->type();
    }

    return muse::contains(acceptedTypes, type);
}

static bool isChordArticulation(const mu::engraving::EngravingItem* item)
{
    const mu::engraving::EngravingItem* parent = item->parentItem();
    if (!parent || !parent->isChord()) {
        return false;
    }

    static const std::unordered_set<mu::engraving::ElementType> CHORD_ARTICULATION_TYPES {
        mu::engraving::ElementType::ARPEGGIO,
        mu::engraving::ElementType::TREMOLO_SINGLECHORD,
        mu::engraving::ElementType::ORNAMENT,
    };

    return muse::contains(CHORD_ARTICULATION_TYPES, item->type());
}

static muse::String chordToNotes(const mu::engraving::Chord* chord)
{
    muse::StringList notes;

    for (const mu::engraving::Note* note : chord->notes()) {
        notes.push_back(note->tpcUserName());
    }

    return notes.join(u" ");
}

static void addElementInfoIfNeed(void* data, mu::engraving::EngravingItem* item)
{
    ScannerData* scannerData = static_cast<ScannerData*>(data);
    if (!itemAccepted(item, scannerData->options.acceptedTypes)) {
        return;
    }

    mu::engraving::ElementType type = item->type();
    ScoreElementScanner::ElementInfo info;
    bool locationIsSet = false;

    if (item->isNote()) {
        mu::engraving::Note* note = mu::engraving::toNote(item);
        mu::engraving::Chord* chord = note->chord();
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
    } else if (item->isSpannerSegment()) {
        mu::engraving::Spanner* spanner = mu::engraving::toSpannerSegment(item)->spanner();
        if (muse::contains(scannerData->spanners, spanner)) {
            return;
        }

        item = spanner;
        type = spanner->type();
        info.name = spanner->translatedSubtypeUserName();
        scannerData->spanners.insert(spanner);

        const mu::engraving::Segment* startSegment = spanner->startSegment();
        if (startSegment) {
            const mu::engraving::EngravingItem::BarBeat barbeat = startSegment->barbeat();
            info.start.measureIdx = barbeat.bar - 1;
            info.start.beat = barbeat.beat - 1.;
            info.start.trackIdx = spanner->track();
        }

        const mu::engraving::Segment* endSegment = spanner->endSegment();
        if (endSegment) {
            const mu::engraving::EngravingItem::BarBeat barbeat = endSegment->barbeat();
            info.end.measureIdx = barbeat.bar - 1;
            info.end.beat = barbeat.beat - 1.;
            info.end.trackIdx = spanner->track2();
        }

        locationIsSet = startSegment || endSegment;
    } else if (item->isHarmony()) {
        info.name = mu::engraving::toHarmony(item)->harmonyName();
    } else if (isChordArticulation(item)) {
        mu::engraving::Chord* chord = mu::engraving::toChord(item->parentItem());
        scannerData->chords.insert(chord);
        info.name = item->translatedSubtypeUserName();
        info.notes = chordToNotes(chord);
    } else if (item->isTempoText()) {
        info.text = mu::engraving::toTempoText(item)->tempoInfo();
    } else if (item->isPlayTechAnnotation() || item->isDynamic()) {
        info.name = item->translatedSubtypeUserName();
    } else if (item->isTextBase()) {
        info.text = mu::engraving::toTextBase(item)->plainText();
    } else {
        info.name = item->translatedSubtypeUserName();
    }

    if (info.name.empty() && info.notes.empty() && info.text.empty()) {
        info.name = item->typeUserName().translated();
    }

    const mu::engraving::Part* part = item->part();
    const mu::engraving::InstrumentTrackId trackId {
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
        const mu::engraving::EngravingItem::BarBeat barbeat = item->barbeat();
        info.start.trackIdx = item->track();
        info.start.measureIdx = barbeat.bar - 1;
        info.start.beat = barbeat.beat - 1.;
        info.end = info.start;
    }

    scannerData->elements[trackId][type].push_back(info);
}

ScoreElementScanner::InstrumentElementMap ScoreElementScanner::scanElements(mu::engraving::Score* score, const Options& options)
{
    TRACEFUNC;

    ScannerData data;
    data.options = options;

    score->scanElements(&data, addElementInfoIfNeed, false);

    return data.elements;
}
