/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include <unordered_set>

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/interval.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/score.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/spannermap.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/volta.h"
#include "engraving/editing/transpose.h"
#include "engraving/types/symnames.h"

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
    const TrackRange trackRange = part->trackRange();
    std::optional<mnx::Array<mnx::part::PositionedClef> > mnxClefs;

    constexpr SegmentType cleftTypes = SegmentType::Clef | SegmentType::HeaderClef;
    for (Segment* segment = measure->first(cleftTypes); segment; segment = segment->next(cleftTypes)) {
        const SegmentType segmentType = segment->segmentType();
        if ((segmentType == SegmentType::HeaderClef) && !isFirstMeasure) {
            continue;
        }

        const Fraction rTick = segment->rtick();
        for (track_idx_t track = trackRange.startTrack; track < trackRange.endTrack; track += VOICES) {
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
                const size_t staffIdx = (track - trackRange.startTrack) / VOICES;
                mnxClef.set_staff(static_cast<int>(staffIdx + 1));
            }
            if (rTick.isNotZero()) {
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

static mnx::MultiStaffOrientation mnxMultiStaffOrientFromDirection(const EngravingItem* item)
{
    switch (item->getProperty(Pid::DIRECTION).value<DirectionV>()) {
    case DirectionV::UP:
        return mnx::MultiStaffOrientation::Above;
    case DirectionV::DOWN:
        return mnx::MultiStaffOrientation::Below;
    case DirectionV::AUTO:
        if (item->part()->staves().size() > 1) {
            if (item->getProperty(Pid::CENTER_BETWEEN_STAVES) == AutoOnOff::ON) {
                return mnx::MultiStaffOrientation::Between;
            }
        }
    }

    return mnx::MultiStaffOrientation::Auto;
}

static void exportMnxVoiceAssignment(mnx::part::DynamicGroupBase& mnxDynamic, int mnxStaffNum,
                                     VoiceAssignment voiceAssignment, track_idx_t curTrackIdx)
{
    switch (voiceAssignment) {
    case VoiceAssignment::CURRENT_VOICE_ONLY:
        mnxDynamic.set_staff(mnxStaffNum);
        mnxDynamic.set_voice(makeMnxVoiceIdFromTrack(mnxDynamic.staff_or(1), curTrackIdx));
        break;
    case VoiceAssignment::ALL_VOICE_IN_STAFF:
        mnxDynamic.set_staff(mnxStaffNum);
        break;
    case VoiceAssignment::ALL_VOICE_IN_INSTRUMENT:
        break;
    }
}

//---------------------------------------------------------
//   createVolta
//   export volta as an MNX ending
//---------------------------------------------------------

void MnxExporter::createVolta(const Volta* volta)
{
    IF_ASSERT_FAILED(volta) {
        return;
    }

    Measure* startMeasure = toMeasure(volta->startElement());
    IF_ASSERT_FAILED(startMeasure) {
        LOGW() << "Skipping volta with missing start measure.";
        return;
    }

    const size_t mnxMeasureIndex = mnxMeasureIndexFromMeasure(startMeasure);
    auto mnxMeasure = mnxDocument().global().measures().at(mnxMeasureIndex);

    const Fraction endTick = volta->tick2() - Fraction::eps();
    Measure* endMeasure = startMeasure->score()->tick2measure(endTick);
    IF_ASSERT_FAILED(endMeasure) {
        LOGW() << "Skipping volta with missing end measure.";
        return;
    }
    IF_ASSERT_FAILED(endMeasure->tick() >= startMeasure->tick()) {
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
    IF_ASSERT_FAILED(foundEnd) {
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

void MnxExporter::createSlur(const Slur* s)
{
    IF_ASSERT_FAILED(s) {
        return;
    }
    IF_ASSERT_FAILED(s->startElement() && s->endElement()) {
        LOGW() << "Skipping slur with missing endpoints.";
        return;
    }
    if (!s->startElement()->isChordRest() || !s->endElement()->isChordRest()) {
        LOGW() << "Skipping slur not attached to chord-rests.";
        return;
    }

    if (const ChordRest* startCR = findFirstChordRest(s)) {
        const ChordRest* endCR = startCR == s->startElement()
                                 ? toChordRest(s->endElement())
                                 : toChordRest(s->startElement());
        auto mnxEvent = mnxEventFromCR(startCR);
        auto endEvent = endCR ? mnxEventFromCR(endCR) : std::nullopt;
        if (mnxEvent && endEvent) {
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
//   createHairpin
//   export a single hairpin spanner
//---------------------------------------------------------

void MnxExporter::createHairpin(const Hairpin* hairpin)
{
    IF_ASSERT_FAILED(hairpin) {
        return;
    }

    if (hairpin->isLineType()) {
        LOGI() << "Line-type hairpins not yet supported by MNX export.";
        return;
    }

    const Staff* startStaff = m_score->staff(track2staff(hairpin->track()));
    const Staff* endStaff = m_score->staff(track2staff(hairpin->effectiveTrack2()));
    IF_ASSERT_FAILED(startStaff && startStaff->part()) {
        LOGW() << "Skipping hairpin with missing start staff or part context.";
        return;
    }

    if (!endStaff || startStaff->idx() != endStaff->idx()) {
        LOGW() << "Hairpin crosses staves and will be exported using the start staff/voice only.";
    }

    const Measure* startMeasure = m_score->tick2measure(hairpin->tick());
    IF_ASSERT_FAILED(startMeasure) {
        LOGW() << "Skipping hairpin with missing start measure for tick " << hairpin->tick().ticks() << ".";
        return;
    }

    const Fraction adjustedEndTick = hairpin->tick2() - Fraction::eps();
    Measure* endMeasure = m_score->tick2measure(adjustedEndTick);
    IF_ASSERT_FAILED(endMeasure) {
        LOGW() << "Skipping hairpin with missing end measure.";
        return;
    }
    IF_ASSERT_FAILED(endMeasure->tick() >= startMeasure->tick()) {
        LOGW() << "Skipping hairpin with end measure before start measure.";
        return;
    }

    const size_t mnxMeasureIndex = mnxMeasureIndexFromMeasure(startMeasure);
    const auto [mnxPartIdx, mnxStaffNum] = mnxPartStaffFromStaffIdx(startStaff->idx());
    auto mnxPart = mnxDocument().parts().at(mnxPartIdx);
    auto mnxMeasure = mnxPart.measures().at(mnxMeasureIndex);

    const Fraction startOffset = hairpin->tick() - startMeasure->tick();
    const Fraction endOffset = hairpin->tick2() - endMeasure->tick();
    const std::string endMeasureId = getOrAssignEID(const_cast<Measure*>(endMeasure)).toStdString();

    auto mnxHairpin = mnxMeasure.ensure_dynamics().appendGradual(
        hairpin->hairpinType() == HairpinType::CRESC_HAIRPIN
        ? mnx::DynamicWedgeType::Increasing
        : mnx::DynamicWedgeType::Decreasing,
        toMnxFractionValue(startOffset).reduced(),
        mnx::MeasureRhythmicPosition::make(
            endMeasureId,
            toMnxFractionValue(endOffset).reduced()));

    mnxHairpin.set_or_clear_orient(mnxMultiStaffOrientFromDirection(hairpin));
    exportMnxVoiceAssignment(mnxHairpin, mnxStaffNum, hairpin->voiceAssignment(), hairpin->track());
}

//---------------------------------------------------------
//   createOttava
//   export a single ottava spanner
//---------------------------------------------------------

void MnxExporter::createOttava(const Ottava* ottava)
{
    IF_ASSERT_FAILED(ottava) {
        return;
    }

    if (!ottava->startElement() || !ottava->endElement()) {
        LOGW() << "Skipping ottava with missing endpoints.";
        return;
    }

    const std::optional<mnx::OttavaAmount> amount = toMnxOttavaAmount(ottava->ottavaType());
    if (!amount) {
        LOGW() << "Skipping unsupported ottava type in MNX export: " << int(ottava->ottavaType());
        return;
    }

    const EngravingItem* startElement = ottava->startElement();
    const EngravingItem* endElement = ottava->endElement();

    const Measure* startMeasure = startElement->findMeasure();
    const Measure* endMeasure = endElement->findMeasure();

    if (!startMeasure) {
        LOGW() << "Skipping ottava with missing start measure.";
        return;
    }

    if (!endMeasure) {
        // Adjust end measure by tick if end element is generated outside normal measure lookup.
        const Fraction adjustedEndTick = ottava->tick2() - Fraction::eps();
        endMeasure = startMeasure->score()->tick2measure(adjustedEndTick);
        if (!endMeasure) {
            LOGW() << "Skipping ottava with missing end measure.";
            return;
        }
    }

    const Fraction startOffset = ottava->tick() - startMeasure->tick();
    const Fraction adjustedEndTick = ottava->endElement()->isChordRest()
                                     ? toChordRest(ottava->endElement())->tick()
                                     : ottava->tick2();
    const Fraction endOffset = adjustedEndTick - endMeasure->tick();

    // Resolve part and staff so we can attach the ottava to the correct part measure.
    const Staff* staff = startElement->staff();
    IF_ASSERT_FAILED(staff && staff->part()) {
        LOGW() << "Skipping ottava with missing staff or part context.";
        return;
    }

    const Part* part = staff->part();
    const auto [mnxPartIdx, mnxStaffNum] = mnxPartStaffFromStaffIdx(staff->idx());
    const size_t mnxMeasureIndex = mnxMeasureIndexFromMeasure(startMeasure);
    auto mnxPart = mnxDocument().parts().at(mnxPartIdx);
    auto mnxMeasure = mnxPart.measures().at(mnxMeasureIndex);
    const std::string endMeasureId = getOrAssignEID(const_cast<Measure*>(endMeasure)).toStdString();

    auto mnxOttava = mnxMeasure.ensure_ottavas().append(amount.value(),
                                                        toMnxFractionValue(startOffset).reduced(),
                                                        mnx::MeasureRhythmicPosition::make(
                                                            endMeasureId,
                                                            toMnxFractionValue(endOffset).reduced()));

    if (const Chord* endC = toChord(ottava->endElement()); endC && !endC->graceNotesBefore().empty()) {
        // Ensure grace notes before the end of the ottava are included.
        mnxOttava.end().position().set_graceIndex(0);
    }

    if (part && part->nstaves() > 1) {
        mnxOttava.set_staff(mnxStaffNum);
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
        case ElementType::HAIRPIN:
            createHairpin(toHairpin(sp));
            break;
        case ElementType::OTTAVA:
            createOttava(toOttava(sp));
            break;
        case ElementType::SLUR:
            createSlur(toSlur(sp));
            break;
        case ElementType::VOLTA:
            createVolta(toVolta(sp));
            break;
        default:
            break;
        }
    }
}

//---------------------------------------------------------
//   createArpeggios
//   emit MNX measure-level arpeggios and non-arpeggios
//---------------------------------------------------------

void MnxExporter::createArpeggios(const Part* part, const Measure* measure, mnx::part::Measure& mnxMeasure)
{
    IF_ASSERT_FAILED(part && measure) {
        return;
    }

    std::unordered_set<const Arpeggio*> exportedArpeggios;

    auto isExportedChord = [this](const Chord* chord) {
        return chord && m_crToMnxEvent.find(chord) != m_crToMnxEvent.end();
    };

    auto isExportableEndpoint = [&](const Note* note) {
        if (!note || !isExportedChord(note->chord())) {
            return false;
        }
        const Staff* staff = note->staff();
        if (staff && staff->isDrumStaff(note->tick())) {
            return pitchIsValid(note->pitch());
        }
        return toMnxPitch(note).has_value();
    };

    auto findBottomChord = [](const Arpeggio* arpeggio) -> Chord* {
        Chord* chord = arpeggio ? arpeggio->chord() : nullptr;
        Segment* segment = chord ? chord->segment() : nullptr;
        if (!chord || !segment) {
            return nullptr;
        }

        Chord* bottomChord = chord;
        for (track_idx_t track = arpeggio->track(); track <= arpeggio->endTrack(); ++track) {
            EngravingItem* item = segment->element(track);
            if (!item || !item->isChord()) {
                continue;
            }
            Chord* spanChord = toChord(item);
            if (spanChord == chord || spanChord->spanArpeggio() == arpeggio) {
                bottomChord = spanChord;
            }
        }
        return bottomChord;
    };

    auto setMnxGraceIndex = [](mnx::part::ArpeggioBase& item, const MnxChordTargetPosition& position) {
        if (position.graceIndex) {
            item.position().set_graceIndex(position.graceIndex.value());
        }
    };

    auto processArpeggio = [&](Chord* chord) {
        IF_ASSERT_FAILED(chord) {
            return;
        }

        Arpeggio* arpeggio = chord->arpeggio();
        if (!arpeggio || !exportedArpeggios.insert(arpeggio).second) {
            return;
        }
        if (!isExportedChord(chord)) {
            LOGW() << "Skipping arpeggio whose chord was not exported.";
            return;
        }

        Chord* bottomChord = chord;
        if (chord->isGrace()) {
            if (arpeggio->span() != 1) {
                LOGW() << "Skipping multi-track grace-note arpeggio.";
                return;
            }
        } else {
            bottomChord = findBottomChord(arpeggio);
            if (!bottomChord || bottomChord->track() < chord->track()) {
                LOGW() << "Skipping arpeggio with invalid span.";
                return;
            }
        }

        if (!isExportedChord(bottomChord)) {
            LOGW() << "Skipping arpeggio whose endpoint chord was not exported.";
            return;
        }

        Note* topNote = chord->upNote();
        Note* bottomNote = bottomChord->downNote();
        if (!isExportableEndpoint(topNote) || !isExportableEndpoint(bottomNote)) {
            LOGW() << "Skipping arpeggio with non-exportable endpoint note.";
            return;
        }

        const std::optional<MnxChordTargetPosition> position = mnxChordTargetPosition(chord, measure);
        if (!position) {
            return;
        }
        const mnx::IdPair::Required span = mnx::IdPair::make(getOrAssignEID(topNote).toStdString(),
                                                             getOrAssignEID(bottomNote).toStdString());

        switch (arpeggio->arpeggioType()) {
        case ArpeggioType::NORMAL: {
            auto mnxArpeggio = mnxMeasure.ensure_arpeggios().append(position->fraction, span);
            setMnxGraceIndex(mnxArpeggio, *position);
            break;
        }
        case ArpeggioType::UP:
        case ArpeggioType::UP_STRAIGHT: {
            auto mnxArpeggio = mnxMeasure.ensure_arpeggios().append(position->fraction, span);
            mnxArpeggio.set_arrow(true);
            mnxArpeggio.set_direction(mnx::MarkingUpDownAuto::Up);
            setMnxGraceIndex(mnxArpeggio, *position);
            break;
        }
        case ArpeggioType::DOWN:
        case ArpeggioType::DOWN_STRAIGHT: {
            auto mnxArpeggio = mnxMeasure.ensure_arpeggios().append(position->fraction, span);
            mnxArpeggio.set_arrow(true);
            mnxArpeggio.set_direction(mnx::MarkingUpDownAuto::Down);
            setMnxGraceIndex(mnxArpeggio, *position);
            break;
        }
        case ArpeggioType::BRACKET: {
            auto mnxNonArpeggio = mnxMeasure.ensure_nonArpeggios().append(position->fraction, span);
            setMnxGraceIndex(mnxNonArpeggio, *position);
            break;
        }
        default:
            LOGW() << "Skipping unsupported arpeggio type: " << static_cast<int>(arpeggio->arpeggioType());
            break;
        }
    };

    for (Segment* segment = measure->first(SegmentType::ChordRest);
         segment;
         segment = segment->next(SegmentType::ChordRest)) {
        for (track_idx_t track = part->startTrack(); track < part->endTrack(); ++track) {
            EngravingItem* item = segment->element(track);
            if (!item || !item->isChord()) {
                continue;
            }
            Chord* chord = toChord(item);
            processArpeggio(chord);
            for (Chord* graceChord : chord->graceNotes()) {
                processArpeggio(graceChord);
            }
        }
    }
}

static void splitDynamicText(const Dynamic* dynamic, String& prefix, std::vector<std::string>& glyphs, String& suffix)
{
    const auto engravingFonts = [&]() -> std::shared_ptr<mu::engraving::IEngravingFontsProvider> {
        return muse::modularity::globalIoc()->resolve<mu::engraving::IEngravingFontsProvider>("engraving");
    }();

    bool gotGlyph{};
    bool gotSuffix{};
    for (const TextFragment& fragment : dynamic->fragmentList()) {
        const CharFormat& format = fragment.format;
        if (format.fontFamily() == u"ScoreText") {
            if (gotSuffix) {
                return; // can't encode a dynamic with multiple glyph sequences separated by text.
            }
            for (size_t i = 0; i < fragment.text.size(); ++i) {
                const Char ch = fragment.text.at(i);
                const SymId symId = engravingFonts->fallbackFont()->fromCode(ch.unicode());
                if (symId != SymId::noSym) {
                    gotGlyph = true;
                    glyphs.push_back(SymNames::nameForSymId(symId).ascii());
                }
            }
        } else {
            if (gotGlyph) {
                suffix += fragment.text;
                gotSuffix = true;
            } else {
                prefix += fragment.text;
            }
        }
    }
    prefix = prefix.trimmed();
    suffix = suffix.trimmed();
}

static std::vector<std::string> parseSymGlyphNames(const muse::String& source)
{
    std::vector<std::string> result;

    static const muse::String openTag(u"<sym>");
    static const muse::String closeTag(u"</sym>");

    size_t pos = 0;
    while (true) {
        size_t open = source.indexOf(openTag, pos);
        if (open == std::string::npos) {
            break;
        }

        open += openTag.size();

        size_t close = source.indexOf(closeTag, open);
        if (close == std::string::npos) {
            break;
        }

        result.push_back(source.mid(open, close - open).toStdString());
        pos = close + closeTag.size();
    }

    return result;
}

//---------------------------------------------------------
//   createDynamic
//   export a dynamic text annotation
//---------------------------------------------------------

static void createDynamic(const Dynamic* dynamic, const Fraction& rTick, int mnxStaffNum,
                          mnx::part::Measure& mnxMeasure)
{
    IF_ASSERT_FAILED(dynamic) {
        return;
    }

    String prefix{};
    String suffix{};
    std::vector<std::string> glyphs{};
    splitDynamicText(dynamic, prefix, glyphs, suffix);

    bool sawIncrease{};
    bool sawDecrease{};
    auto checkRelative = [&](const String& value) {
        if (value == u"piu" || value == u"più") {
            sawIncrease = true;
        }
        if (value == u"menos" || value == u"meno") {
            sawDecrease = true;
        }
    };
    checkRelative(prefix);
    checkRelative(suffix);

    bool isAccent{};
    bool copyGlyphs{};
    const auto [mnxDynamicValue, mnxAttackValue] = toMnxDynamicType(dynamic->dynamicType(), copyGlyphs, isAccent);
    if (!mnxDynamicValue && ((!sawIncrease && !sawDecrease) || (prefix.empty() && glyphs.empty() && suffix.empty()))) {
        return;
    }

    auto mnxDynamic = [&]() -> mnx::part::DynamicGroupBase {
        using DynRelVal = mnx::DynamicRelativeValue;
        if (sawIncrease || sawDecrease) {
            auto relValue = sawIncrease ? DynRelVal::Louder : DynRelVal::Softer;
            auto dyn = mnxMeasure.ensure_dynamics().appendRelative(relValue, toMnxFractionValue(rTick));
            if (mnxDynamicValue) {
                dyn.set_value(mnxDynamicValue.value());
            }
            return dyn;
        } else if (isAccent) {
            return mnxMeasure.ensure_dynamics().appendAccent(mnxDynamicValue.value(), toMnxFractionValue(rTick));
        } else {
            return mnxMeasure.ensure_dynamics().appendImmediate(mnxDynamicValue.value(), toMnxFractionValue(rTick));
        }
    }();

    if (copyGlyphs && !glyphs.empty()) {
        glyphs = parseSymGlyphNames(Dynamic::dynamicText(dynamic->dynamicType()));
    }

    if (mnxAttackValue) {
        mnxDynamic.set_attackValue(mnxAttackValue.value());
    }
    if (!prefix.empty()) {
        mnxDynamic.set_prefix(prefix.toStdString());
    }
    if (!suffix.empty()) {
        mnxDynamic.set_suffix(suffix.toStdString());
    }
    if (!glyphs.empty()) {
        mnxDynamic.ensure_glyphs().assign(glyphs);
    }

    mnxDynamic.set_or_clear_orient(mnxMultiStaffOrientFromDirection(dynamic));
    exportMnxVoiceAssignment(mnxDynamic, mnxStaffNum, dynamic->voiceAssignment(), dynamic->track());
}

//---------------------------------------------------------
//   exportTextAnnotations
//   export dynamics, staff text, etc. for a single measure
//---------------------------------------------------------

static void exportTextAnnotations(const Part* part, const Measure* measure, mnx::part::Measure& mnxMeasure)
{
    const size_t staves = part->nstaves();
    constexpr SegmentType timeSegments = SegmentType::Duration;
    for (Segment* segment = measure->first(timeSegments); segment; segment = segment->next(timeSegments)) {
        const Fraction rTick = segment->rtick();
        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            const track_idx_t track = part->startTrack() + VOICES * staffIdx;
            for (EngravingItem* annotation : segment->annotations()) {
                if (annotation->track() < track || annotation->track() >= track + VOICES || !annotation->isTextBase()) {
                    continue;
                }
                switch (annotation->type()) {
                case ElementType::DYNAMIC:
                    createDynamic(toDynamic(annotation), rTick, int(staffIdx) + 1, mnxMeasure);
                    break;
                /// @todo other kinds of staff text when defined by MNX
                default:
                    break;
                }
            }
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
                const PreferSharpFlat prefer = part->preferSharpFlat();
                const int flipAt = toMnxKeyFifthsFlipValue(prefer, mnxTranspose);
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
            createSequences(part, measure, mnxMeasure);
            exportTextAnnotations(part, measure, mnxMeasure);
            /// the items below must come after createSequences.
            createArpeggios(part, measure, mnxMeasure);
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
