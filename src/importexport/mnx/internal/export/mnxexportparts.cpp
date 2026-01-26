/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
#include "mnxexporter.h"
#include "internal/shared/mnxtypesconv.h"
#include "log.h"

#include <algorithm>
#include <optional>
#include <string>

#include "engraving/dom/clef.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/interval.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/spannermap.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/volta.h"
#include "engraving/editing/transpose.h"

using namespace mu::engraving;

namespace mu::iex::mnxio {
//---------------------------------------------------------
//   exportDrumsetKit
//---------------------------------------------------------

void MnxExporter::exportDrumsetKit(const Part* part, const Instrument* instrument, mnx::Part& mnxPart)
{
    IF_ASSERT_FAILED(part && instrument) {
        return;
    }
    IF_ASSERT_FAILED(instrument->useDrumset()) {
        return;
    }

    const Drumset* drumset = instrument->drumset();
    IF_ASSERT_FAILED(drumset) {
        return;
    }

    const Staff* staffForKit = part->staff(0);
    const Measure* firstMeasure = m_score ? m_score->firstMeasure() : nullptr;
    const StaffType* staffType = nullptr;
    if (staffForKit && firstMeasure) {
        staffType = staffForKit->staffType(firstMeasure->tick());
    }
    if (!staffType) {
        staffType = StaffType::preset(StaffTypes::PERC_DEFAULT);
    }
    const int middleLine = staffType ? staffType->middleLine() : 4;

    auto mnxKit = mnxPart.ensure_kit();
    auto mnxSounds = m_mnxDocument.global().ensure_sounds();

    for (int pitch = 0; pitch < DRUM_INSTRUMENTS; ++pitch) {
        if (!drumset->isValid(pitch)) {
            continue;
        }

        const std::string kitId = "drum-midi-" + std::to_string(pitch);
        const int line = drumset->line(pitch);
        const int staffPosition = middleLine - line;

        auto kitComponent = mnxKit.append(kitId, staffPosition);
        String name = drumset->name(pitch);
        if (name.isEmpty()) {
            name = String(u"Percussion note");
        }
        kitComponent.set_name(name.toStdString());

        const std::string soundId = "drum-midi-" + std::to_string(pitch);
        kitComponent.set_sound(soundId);
        if (!mnxSounds.contains(soundId)) {
            auto sound = mnxSounds.append(soundId);
            sound.set_name(name.toStdString());
            sound.set_midiNumber(pitch);
        }
    }
}

//---------------------------------------------------------
//   appendClefsForMeasure
//   export clefs for a single measure
//---------------------------------------------------------

static void appendClefsForMeasure(const Part* part, const Measure* measure, mnx::part::Measure& mnxMeasure)
{
    if (part && part->instrument() && part->instrument()->useDrumset()) {
        /// @todo Export percussion and tab staves/clefs when MNX supports them.
        return;
    }

    const bool isFirstMeasure = (measure->prevMeasure() == nullptr);
    const size_t staves = part->nstaves();
    std::optional<mnx::Array<mnx::part::PositionedClef> > mnxClefs;

    for (Segment* segment = measure->first(); segment; segment = segment->next()) {
        const SegmentType segmentType = segment->segmentType();
        if (!(segmentType & SegmentType::ClefType)) {
            continue;
        }
        if (segmentType & SegmentType::CourtesyClefType) {
            continue;
        }
        if ((segmentType == SegmentType::HeaderClef) && !isFirstMeasure) {
            continue;
        }

        const Fraction rTick = segment->rtick();
        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            const track_idx_t track = part->startTrack() + VOICES * staffIdx;
            Clef* clef = toClef(segment->element(track));
            if (!clef || clef->isCourtesy()) {
                continue;
            }
            if (clef->generated() && !(rTick.isZero() && isFirstMeasure)) {
                continue;
            }

            const auto required = toMnxClef(clef->clefType());
            if (!required) {
                LOGW() << "Skipping nsupported clef type in MNX export: " << int(clef->clefType());
                continue;
            }

            if (!mnxClefs) {
                mnxClefs = mnxMeasure.ensure_clefs();
            }

            auto mnxClef = mnxClefs->append(required->clefSign,
                                            required->staffPosition,
                                            required->octaveAdjustment);
            if (staves > 1) {
                mnxClef.set_staff(static_cast<int>(staffIdx + 1));
            }
            if (!rTick.isZero()) {
                mnxClef.ensure_position(mnx::FractionValue(rTick.numerator(), rTick.denominator()));
            }
        }
    }
}

//---------------------------------------------------------
//   findFirstChordRest
//---------------------------------------------------------

static const ChordRest* findFirstChordRest(const Slur* s)
{
    /// @todo this function is copied from same function in musicxml and should probably be a method on Slur

    const EngravingItem* e1 = s->startElement();
    if (!e1 || !(e1->isChordRest())) {
        LOGD("no valid start element for slur %p", s);
        return nullptr;
    }

    const EngravingItem* e2 = s->endElement();
    if (!e2 || !(e2->isChordRest())) {
        LOGD("no valid end element for slur %p", s);
        return nullptr;
    }

    if (e1->tick() < e2->tick()) {
        return toChordRest(e1);
    } else if (e1->tick() > e2->tick()) {
        return toChordRest(e2);
    }

    if (e1->isRest() || e2->isRest()) {
        return nullptr;
    }

    const Chord* c1 = toChord(e1);
    const Chord* c2 = toChord(e2);

    // c1->tick() == c2->tick()
    if (!c1->isGrace() && !c2->isGrace()) {
        // slur between two regular notes at the same tick
        // probably shouldn't happen but handle just in case
        LOGD("invalid slur between chords %p and %p at tick %d", c1, c2, c1->tick().ticks());
        return 0;
    } else if (c1->isGraceBefore() && !c2->isGraceBefore()) {
        return c1;            // easy case: c1 first
    } else if (c1->isGraceAfter() && !c2->isGraceAfter()) {
        return c2;            // easy case: c2 first
    } else if (c2->isGraceBefore() && !c1->isGraceBefore()) {
        return c2;            // easy case: c2 first
    } else if (c2->isGraceAfter() && !c1->isGraceAfter()) {
        return c1;            // easy case: c1 first
    } else {
        // both are grace before or both are grace after -> compare grace indexes
        // (note: higher means closer to the non-grace chord it is attached to)
        if ((c1->isGraceBefore() && c1->graceIndex() < c2->graceIndex())
            || (c1->isGraceAfter() && c1->graceIndex() > c2->graceIndex())) {
            return c1;
        } else {
            return c2;
        }
    }
}

//---------------------------------------------------------
//   createEnding
//   export volta as an MNX ending
//---------------------------------------------------------

static void createEnding(const Spanner* sp, MnxExporter* exporter)
{
    IF_ASSERT_FAILED(sp && exporter) {
        return;
    }

    if (!sp->isVolta()) {
        LOGW() << "Skipping spanner that has ElementType::VOLTA but is not a volta.";
        return;
    }

    const Volta* volta = toVolta(sp);
    Measure* startMeasure = toMeasure(volta->startElement());
    IF_ASSERT_FAILED(startMeasure) {
        LOGW() << "Skipping volta with missing start measure.";
        return;
    }

    const size_t mnxMeasureIndex = exporter->mnxMeasureIndexFromMeasure(startMeasure);
    auto mnxMeasure = exporter->mnxDocument().global().measures().at(mnxMeasureIndex);

    const Fraction endTick = volta->tick2() - Fraction::eps();
    Measure* endMeasure = startMeasure->score()->tick2measure(endTick);
    if (!endMeasure) {
        LOGW() << "Skipping volta with missing end measure.";
        return;
    }
    if (endMeasure->tick() < startMeasure->tick()) {
        LOGW() << "Skipping volta with end measure before start measure.";
        return;
    }

    int duration = 0;
    bool foundEnd = false;
    for (const Measure* cursor = startMeasure; cursor; cursor = cursor->nextMeasure()) {
        ++duration;
        if (cursor == endMeasure) {
            foundEnd = true;
            break;
        }
    }
    if (!foundEnd) {
        LOGW() << "Skipping volta with unsupported end measure.";
        return;
    }

    auto mnxEnding = mnxMeasure.ensure_ending(duration);
    mnxEnding.set_open(volta->voltaType() == Volta::Type::OPEN);
    const std::vector<int> endings = volta->endings();
    if (!endings.empty()) {
        auto mnxNumbers = mnxEnding.ensure_numbers();
        for (int ending : endings) {
            mnxNumbers.push_back(ending);
        }
    }
}

//---------------------------------------------------------
//   createSlur
//   export a single slur spanner
//---------------------------------------------------------

static void createSlur(const Spanner* sp, MnxExporter* exporter)
{
    IF_ASSERT_FAILED(sp && exporter) {
        return;
    }
    if (!sp->isSlur()) {
        LOGW() << "Skipping spanner that has ElementType::SLUR but is not a slur.";
        return;
    }
    if (!sp->startElement() || !sp->endElement()) {
        LOGW() << "Skipping slur with missing endpoints.";
        return;
    }
    if (!sp->startElement()->isChordRest() || !sp->endElement()->isChordRest()) {
        return;
    }

    const Slur* s = toSlur(sp);
    if (const ChordRest* startCR = findFirstChordRest(s)) {
        const ChordRest* endCR = startCR == sp->startElement()
                                 ? toChordRest(sp->endElement())
                                 : toChordRest(sp->startElement());
        auto mnxEvent = exporter->mnxEventFromCR(startCR);
        if (mnxEvent && endCR) {
            auto mnxSlur = mnxEvent->ensure_slurs().append(endCR->eid().toStdString());
            mnxSlur.set_lineType(toMnxSlurLineType(s->styleType()));
            if (s->slurDirection() != DirectionV::AUTO) {
                mnxSlur.set_side(s->up() ? mnx::SlurTieSide::Up : mnx::SlurTieSide::Down);
            }
            /// @todo export side and sideEnd in opposite directions, if/when MuseScore supports it.
            /// @todo endNote and startNote are not supported by MuseScore (yet?)
        }
    }
}

//---------------------------------------------------------
//   createOttava
//   export a single ottava spanner
//---------------------------------------------------------

static void createOttava(const Spanner* sp, MnxExporter* exporter)
{
    IF_ASSERT_FAILED(sp && exporter) {
        return;
    }

    if (!sp->isOttava()) {
        LOGW() << "Skipping spanner that has ElementType::OTTAVA but is not an ottava.";
        return;
    }

    if (!sp->startElement() || !sp->endElement()) {
        LOGW() << "Skipping ottava with missing endpoints.";
        return;
    }

    const Ottava* ottava = toOttava(sp);
    if (!ottava) {
        return;
    }

    const std::optional<mnx::OttavaAmount> amount = toMnxOttavaAmount(ottava->ottavaType());
    if (!amount) {
        LOGW() << "Skipping unsupported ottava type in MNX export: " << int(ottava->ottavaType());
        return;
    }

    const EngravingItem* startElement = sp->startElement();
    const EngravingItem* endElement = sp->endElement();

    const Measure* startMeasure = startElement->findMeasure();
    const Measure* endMeasure = endElement->findMeasure();

    if (!startMeasure) {
        LOGW() << "Skipping ottava with missing start measure.";
        return;
    }

    if (!endMeasure) {
        // Adjust end measure by tick if end element is generated outside normal measure lookup.
        const Fraction adjustedEndTick = sp->tick2() - Fraction::eps();
        endMeasure = startMeasure->score()->tick2measure(adjustedEndTick);
        if (!endMeasure) {
            LOGW() << "Skipping ottava with missing end measure.";
            return;
        }
    }

    const Fraction startOffset = sp->tick() - startMeasure->tick();
    const Fraction adjustedEndTick = sp->endElement()->isChordRest()
                                     ? toChordRest(sp->endElement())->tick()
                                     : sp->tick();
    const Fraction endOffset = adjustedEndTick - endMeasure->tick();

    // Resolve part and staff so we can attach the ottava to the correct part measure.
    const Staff* staff = startElement->staff();
    if (!staff || !staff->part()) {
        LOGW() << "Skipping ottava with missing staff or part context.";
        return;
    }

    const Part* part = staff->part();
    std::pair<size_t, int> partStaff;
    try {
        partStaff = exporter->mnxPartStaffFromStaffIdx(staff->idx());
    } catch (const std::exception& ex) {
        LOGW() << "Skipping ottava because the owning part/staff could not be resolved: " << ex.what();
        return;
    }

    const size_t mnxMeasureIndex = exporter->mnxMeasureIndexFromMeasure(startMeasure);
    auto mnxPart = exporter->mnxDocument().parts().at(partStaff.first);
    auto mnxMeasure = mnxPart.measures().at(mnxMeasureIndex);

    const size_t endMeasureIndex = exporter->mnxMeasureIndexFromMeasure(endMeasure);
    const auto mnxEndMeasure = exporter->mnxDocument().global().measures().at(endMeasureIndex);

    auto mnxOttava = mnxMeasure.ensure_ottavas().append(*amount,
                                                        mnx::FractionValue(startOffset.numerator(),
                                                                           startOffset.denominator()).reduced(),
                                                        mnxEndMeasure.calcMeasureIndex(),
                                                        mnx::FractionValue(endOffset.numerator(),
                                                                           endOffset.denominator()).reduced());

    if (const Chord* endC = toChord(sp->endElement()); endC && !endC->graceNotesBefore().empty()) {
        // Ensure grace notes before the end of the ottava are included.
        mnxOttava.end().position().set_graceIndex(0);
    }

    if (part && part->nstaves() > 1) {
        mnxOttava.set_staff(partStaff.second);
    }

    /// @todo map track-specific ottava to ottava.voice() if MuseScore decides to implements it.
}

//---------------------------------------------------------
//   exportSpanners
//---------------------------------------------------------

void MnxExporter::exportSpanners()
{
    const SpannerMap::IntervalList& spanners = m_score->spannerMap().findOverlapping(
        m_score->firstMeasure()->tick().ticks(), m_score->lastMeasure()->endTick().ticks());

    for (const auto& entry : spanners) {
        Spanner* sp = entry.value;
        if (sp->generated()) {
            continue;
        }
        switch (sp->type()) {
        case ElementType::VOLTA:
            createEnding(sp, this);
            break;
        case ElementType::SLUR:
            createSlur(sp, this);
            break;
        case ElementType::OTTAVA:
            createOttava(sp, this);
            break;
        default:
            break;
        }
    }
}

//---------------------------------------------------------
//   createParts
//---------------------------------------------------------

bool MnxExporter::createParts()
{
    if (!m_score) {
        return false;
    }

    auto mnxParts = m_mnxDocument.parts();
    m_staffToPartStaff.clear();
    m_exportedStaves.clear();
    bool hasPart = false;

    for (Part* part : m_score->parts()) {
        bool hasTabStaff = false;
        const Measure* firstMeasure = m_score->firstMeasure();
        const Fraction tick = firstMeasure ? firstMeasure->tick() : Fraction(0, 1);
        for (staff_idx_t s = 0; s < part->nstaves(); ++s) {
            if (const Staff* staff = part->staff(s)) {
                if (staff->isTabStaff(tick)) {
                    hasTabStaff = true;
                    break;
                }
            }
        }
        if (hasTabStaff) {
            LOGI() << "Skipping tab part \"" << part->partName() << "\" in MNX export.";
            continue;
        }

        auto mnxPart = mnxParts.append();
        mnxPart.set_id(getOrAssignEID(part).toStdString());
        hasPart = true;

        const String longName = part->longName();
        if (!longName.isEmpty()) {
            mnxPart.set_name(longName.toStdString());
        }

        const String shortName = part->shortName();
        if (!shortName.isEmpty()) {
            mnxPart.set_shortName(shortName.toStdString());
        }

        mnxPart.set_or_clear_staves(static_cast<int>(part->nstaves()));

        const Instrument* instrument = part->instrument();
        if (instrument) {
            const Interval transpose = instrument->transpose();
            if (!transpose.isZero()) {
                const Interval mnxTranspose(-transpose.diatonic, -transpose.chromatic);
                auto mnxTransposition = mnxPart.ensure_transposition(
                    mnx::Interval::make(mnxTranspose.diatonic, mnxTranspose.chromatic));
                const int keyChange = static_cast<int>(Transpose::transposeKey(Key::C, mnxTranspose, part->preferSharpFlat()))
                                      - static_cast<int>(Key::C);
                const int flipAt = keyChange >= 0 ? static_cast<int>(Key::MAX) : static_cast<int>(Key::MIN);
                mnxTransposition.set_keyFifthsFlipAt(flipAt);
            }
            if (instrument->useDrumset()) {
                exportDrumsetKit(part, instrument, mnxPart);
            }
        }

        for (staff_idx_t s = 0; s < part->nstaves(); ++s) {
            if (const Staff* staff = part->staff(s)) {
                m_staffToPartStaff.emplace(staff->idx(),
                                           std::make_pair(mnxPart.calcArrayIndex(), static_cast<int>(s + 1)));
                m_exportedStaves.push_back(m_score->staff(staff->idx()));
            }
        }

        auto mnxMeasures = mnxPart.measures();
        for (const Measure* measure = m_score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            auto mnxMeasure = mnxMeasures.append();

            appendClefsForMeasure(part, measure, mnxMeasure);
            /// @todo Dynamics are deferred pending MNX spec clarifications.
            createSequences(part, measure, mnxMeasure);
        }
    }

    exportSpanners();

    if (!m_lyricLineIds.empty()) {
        auto mnxLyricsGlobal = mnxDocument().global().ensure_lyrics();
        /// @todo line metadata if MNX spec expands to include anything we can export.
        auto mnxLineOrder = mnxLyricsGlobal.ensure_lineOrder();
        for (const auto& lineId : m_lyricLineIds) {
            mnxLineOrder.push_back(lineId);
        }
    }

    std::sort(m_exportedStaves.begin(), m_exportedStaves.end(),
              [](const Staff* lhs, const Staff* rhs) {
        if (!lhs || !rhs) {
            return lhs != nullptr;
        }
        return lhs->idx() < rhs->idx();
    });

    return hasPart;
}
} // namespace mu::iex::mnxio
