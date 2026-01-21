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

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "engraving/dom/accidental.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/tiejumppointlist.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/tuplet.h"

#include "internal/shared/mnxtypesconv.h"
#include "log.h"

using namespace mu::engraving;

namespace mu::iex::mnxio {

//---------------------------------------------------------
//   exportAccidentalDetails
//   export accidental details into accidentalDisplay
//---------------------------------------------------------

static void exportAccidentalDetails(mnx::sequence::Note& mnxNote, const Note* note)
{
    IF_ASSERT_FAILED(note) {
        return;
    }

    if (Accidental* acc = note->accidental()) {
        bool force = acc->role() == AccidentalRole::USER;
        const AccidentalBracket bracket = acc->bracket();
        if (force || bracket != AccidentalBracket::NONE) {
            auto accDisp = mnxNote.ensure_accidentalDisplay(acc->visible());
            accDisp.set_force(force);
            if (bracket != AccidentalBracket::NONE) {
                auto mnxAcciBracket = bracket == AccidentalBracket::PARENTHESIS
                                    ? mnx::AccidentalEnclosureSymbol::Parentheses
                                    : mnx::AccidentalEnclosureSymbol::Brackets;
                auto enclosure = accDisp.ensure_enclosure(mnxAcciBracket);
            }
        }
    }
}

//---------------------------------------------------------
//   createLyrics
//   export lyrics on a chord/rest into mnxEvent
//---------------------------------------------------------

static void createLyrics(mnx::sequence::Event& mnxEvent, const ChordRest* cr,
                         std::set<std::string>& lyricLineIds)
{
    IF_ASSERT_FAILED(cr) {
        return;
    }

    const auto lyrics = cr->lyrics();
    if (lyrics.empty()) {
        return;
    }

    auto mnxLyrics = mnxEvent.ensure_lyrics();
    auto mnxLines = mnxLyrics.ensure_lines();

    for (const auto& lyric : lyrics) {
        const std::string lineId = std::to_string(lyric->verse() + 1);
        lyricLineIds.insert(lineId);
        auto mnxLine = mnxLines.append(lineId, lyric->plainText().toStdString());
        mnxLine.set_type(toMnxLyricLineType(lyric->syllabic()));
        /// @todo export styled text when supported by MNX.
        /// @todo export word extension span when supported by MNX.
    }
}

//---------------------------------------------------------
//   createMarkings
//   export articulations, breath marks, single-note
//   tremolo
//---------------------------------------------------------

static void createMarkings(mnx::sequence::Event& mnxEvent, ChordRest* cr)
{
    IF_ASSERT_FAILED(cr) {
        return;
    }

    auto pointingFromAnchor = [](ArticulationAnchor anchor) -> std::optional<mnx::MarkingUpDown> {
        switch (anchor) {
        case ArticulationAnchor::TOP: return mnx::MarkingUpDown::Up;
        case ArticulationAnchor::BOTTOM: return mnx::MarkingUpDown::Down;
        case ArticulationAnchor::AUTO:
        default:
            return std::nullopt;
        }
    };

    if (cr->isChord()) {
        const Chord* chord = toChord(cr);
        for (Articulation* a : chord->articulations()) {
            auto mnxMarkings = mnxEvent.ensure_markings();
            IF_ASSERT_FAILED(a) {
                continue;
            }
            const SymId sym = a->symId();
            const auto pointing = pointingFromAnchor(a->anchor());

            switch (sym) {
            case SymId::articAccentAbove:
            case SymId::articAccentBelow: {
                auto acc = mnxMarkings.ensure_accent();
                if (pointing) {
                    acc.set_pointing(*pointing);
                }
                break;
            }
            case SymId::articSoftAccentAbove:
            case SymId::articSoftAccentBelow: {
                auto sa = mnxMarkings.ensure_softAccent();
                break;
            }
            case SymId::articStaccatoAbove:
            case SymId::articStaccatoBelow: {
                auto st = mnxMarkings.ensure_staccato();
                break;
            }
            case SymId::articStaccatissimoAbove:
            case SymId::articStaccatissimoBelow: {
                auto st = mnxMarkings.ensure_staccatissimo();
                break;
            }
            case SymId::articStaccatissimoStrokeAbove:
            case SymId::articStaccatissimoStrokeBelow: {
                auto sp = mnxMarkings.ensure_spiccato();
                break;
            }
            case SymId::articStressAbove:
            case SymId::articStressBelow: {
                auto st = mnxMarkings.ensure_stress();
                break;
            }
            case SymId::articUnstressAbove:
            case SymId::articUnstressBelow: {
                auto un = mnxMarkings.ensure_unstress();
                break;
            }
            case SymId::articMarcatoAbove:
            case SymId::articMarcatoBelow: {
                auto sa = mnxMarkings.ensure_strongAccent();
                if (pointing) {
                    sa.set_pointing(*pointing);
                }
                break;
            }
            case SymId::articTenutoAbove:
            case SymId::articTenutoBelow: {
                auto tn = mnxMarkings.ensure_tenuto();
                break;
            }
            default:
                break;
            }
        }

        if (const TremoloSingleChord* trem = chord->tremoloSingleChord()) {
            if (auto marks = toMnxTremoloMarks(trem->tremoloType())) {
                auto mnxMarkings = mnxEvent.ensure_markings();
                mnxMarkings.ensure_tremolo(marks.value());
            }
        }
    }

    if (Segment* seg = cr->segment()) {
        for (EngravingItem* ann : seg->annotations()) {
            if (!ann || ann->type() != ElementType::BREATH || ann->track() != cr->track()) {
                continue;
            }
            const Breath* breath = toBreath(ann);
            if (!breath) {
                continue;
            }
            auto mnxMarkings = mnxEvent.ensure_markings();
            auto mnxBreath = mnxMarkings.ensure_breath();
            if (const auto breathSym = toMnxBreathMarkSym(breath->symId())) {
                mnxBreath.set_symbol(breathSym.value());
            }
        }
    }
}

//---------------------------------------------------------
//   tupletContainsChordRest
//---------------------------------------------------------

static bool tupletContainsChordRest(const Tuplet* tuplet, const ChordRest* chordRest)
{
    IF_ASSERT_FAILED(tuplet && chordRest) {
        return false;
    }

    for (const DurationElement* element : tuplet->elements()) {
        if (!element) {
            continue;
        }
        if (element->isChordRest()) {
            if (toChordRest(element) == chordRest) {
                return true;
            }
            continue;
        }
        if (element->isTuplet()) {
            if (tupletContainsChordRest(toTuplet(element), chordRest)) {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------
//   firstTupletChordRest
//---------------------------------------------------------

static ChordRest* firstTupletChordRest(const Tuplet* tuplet)
{
    IF_ASSERT_FAILED(tuplet) {
        return nullptr;
    }

    for (DurationElement* element : tuplet->elements()) {
        if (!element) {
            continue;
        }
        if (element->isChordRest()) {
            return toChordRest(element);
        }
        if (element->isTuplet()) {
            if (ChordRest* nested = firstTupletChordRest(toTuplet(element))) {
                return nested;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   createTies
//   emits MNX ties for the given note
//---------------------------------------------------------

void MnxExporter::createTies(mnx::sequence::NoteBase& mnxNote, Note* note)
{
    IF_ASSERT_FAILED(note) {
        return;
    }

    auto appendTie = [&](Note* targetNote, const Tie* dirTie, bool isCrossJump) {
        IF_ASSERT_FAILED(targetNote) {
            LOGW() << "Skipping tie with missing target note.";
            return;
        }

        auto mnxTie = mnxNote.ensure_ties().append();
        mnxTie.set_target(getOrAssignEID(targetNote).toStdString());
        if (isCrossJump) {
            mnxTie.set_targetType(mnx::TieTargetType::CrossJump);
        } else {
            const Fraction expectedTick = note->tick() + note->chord()->ticks();
            if (targetNote->tick() == expectedTick) {
                if (targetNote->track() == note->track()) {
                    mnxTie.set_targetType(mnx::TieTargetType::NextNote);
                } else {
                    mnxTie.set_targetType(mnx::TieTargetType::CrossVoice);
                }
            } else {
                mnxTie.set_targetType(mnx::TieTargetType::Arpeggio);
            }
        }

        if (dirTie && dirTie->slurDirection() != DirectionV::AUTO) {
            mnxTie.set_side(dirTie->up() ? mnx::SlurTieSide::Up : mnx::SlurTieSide::Down);
        }
    };

    Tie* tieFor = note->tieFor();
    if (tieFor && !tieFor->isPartialTie()) {
        if (tieFor->isLaissezVib()) {
            auto mnxTie = mnxNote.ensure_ties().append();
            mnxTie.set_lv(true);
            if (tieFor->slurDirection() != DirectionV::AUTO) {
                mnxTie.set_side(tieFor->up() ? mnx::SlurTieSide::Up : mnx::SlurTieSide::Down);
            }
            return;
        } else {
            appendTie(tieFor->endNote(), tieFor, false);
        }
    }

    const TieJumpPointList* jumpPoints = note->tieJumpPoints();
    if (!jumpPoints || jumpPoints->empty()) {
        return;
    }

    for (const TieJumpPoint* jumpPoint : *jumpPoints) {
        if (!jumpPoint || !jumpPoint->active() || jumpPoint->followingNote()) {
            continue;
        }

        appendTie(jumpPoint->note(), jumpPoint->endTie(), true);
    }
}

//---------------------------------------------------------
//   createNotes
//   emits MNX notes for the given chord
//   returns true when appended
//---------------------------------------------------------

bool MnxExporter::createNotes(mnx::sequence::Event& mnxEvent, ChordRest* chordRest)
{
    IF_ASSERT_FAILED(chordRest->isChord()) {
        return false;
    }

    const Chord* chord = toChord(chordRest);
    const std::vector<Note*>& chordNotes = chord->notes();
    IF_ASSERT_FAILED(!chordNotes.empty()) {
        LOGW() << "Skipping chord event with no notes.";
        return false;
    }

    const Staff* staff = chord->staff();
    const bool isDrumset = staff && staff->isDrumStaff(chord->tick());
    if (isDrumset) {
        auto mnxKitNotes = mnxEvent.ensure_kitNotes();
        bool hasNote = false;
        for (Note* note : chordNotes) {
            const int pitch = note->pitch();
            if (!pitchIsValid(pitch)) {
                LOGW() << "Skipping kit note with invalid MIDI pitch: " << pitch;
                continue;
            }
            const std::string kitId = "drum-midi-" + std::to_string(pitch);
            auto mnxKitNote = mnxKitNotes.append(kitId);
            mnxKitNote.set_id(getOrAssignEID(note).toStdString());
            createTies(mnxKitNote, note);
            hasNote = true;
        }
        if (!hasNote) {
            LOGW() << "Skipping chord event with no valid kit notes.";
            return false;
        }
        return true;
    }

    auto mnxNotes = mnxEvent.ensure_notes();
    bool hasNote = false;
    for (Note* note : chordNotes) {
        const auto pitch = toMnxPitch(note);
        if (!pitch) {
            LOGW() << "Skipping note with unsupported pitch.";
            continue;
        }
        auto mnxNote = mnxNotes.append(*pitch);
        mnxNote.set_id(getOrAssignEID(note).toStdString());
        createTies(mnxNote, note);
        exportAccidentalDetails(mnxNote, note);
        hasNote = true;
    }
    if (!hasNote) {
        LOGW() << "Skipping chord event with no convertible notes.";
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   createRest
//   emits a MNX rest for the given chord/rest
//   returns true when appended
//---------------------------------------------------------

bool MnxExporter::createRest(mnx::sequence::Event& mnxEvent, ChordRest* chordRest)
{
    IF_ASSERT_FAILED(chordRest->isRest()) {
        return false;
    }
    auto mnxRest = mnxEvent.ensure_rest();

    // Encode vertical rest position if it has been moved off the default staff line.
    // MuseScore rest offset is in spatium units; convert to staff steps (middle line = 0).
    if (Rest* rest = toRest(chordRest)) {
        const double yOffsSp = -2.0 * rest->offset().y() / rest->spatium(); // +up; 1 staff step = half a spatium
        const int staffPos = static_cast<int>(std::lround(yOffsSp));
        if (staffPos != 0) {
            mnxRest.set_staffPosition(staffPos);
        }
    }

    return true;
}

//---------------------------------------------------------
//   createBeam
//   emits MNX beams for a single primary beam group
//---------------------------------------------------------

void MnxExporter::createBeam(ExportContext& ctx, ChordRest* chordRest)
{
    IF_ASSERT_FAILED(chordRest) {
        return;
    }

    Beam* beam = chordRest->beam();
    if (!beam) {
        return;
    }
    const std::vector<ChordRest*>& elements = beam->elements();
    if (elements.empty() || elements.front() != chordRest) {
        return;
    }
    IF_ASSERT_FAILED(elements.front()->measure() == ctx.measure) {
        return;
    }
    /// @todo Handle beams that cross system breaks, rather than just mirroring layout segments.
    /// This work is deferred on the chance that it may become much simpler if
    /// MuseScore also implements beams across system breaks.

    auto mnxBeams = ctx.mnxMeasure.ensure_beams();

    enum class BeamAction {
        Begin,
        Continue,
        End,
        ForwardHook,
        BackwardHook
    };

    auto appendBeamLevel = [&](auto&& self,
                               mnx::Array<mnx::part::Beam>& mnxBeamArray,
                               const std::vector<ChordRest*>& beamElements,
                               size_t startIdx, size_t endIdx, int level) -> void {
        auto beamActionForLevel = [&](size_t idx) -> std::optional<BeamAction> {
            int prevBeams = -1;
            int currentBeams = -1;
            int nextBeams = -1;
            BeamMode currentBeamMode = BeamMode::AUTO;
            BeamMode nextBeamMode = BeamMode::AUTO;

            for (size_t i = idx; i-- > startIdx;) {
                prevBeams = beamElements[i]->beams();
                break;
            }
            currentBeams = beamElements[idx]->beams();
            currentBeamMode = beamElements[idx]->beamMode();
            for (size_t i = idx + 1; i <= endIdx; ++i) {
                nextBeams = beamElements[i]->beams();
                nextBeamMode = beamElements[i]->beamMode();
                break;
            }

            if ((currentBeams >= level && prevBeams < level)
                || (currentBeamMode == BeamMode::BEGIN16 && level > 1)
                || (currentBeamMode == BeamMode::BEGIN32 && level > 2)) {
                return BeamAction::Begin;
            } else if (currentBeams < level && nextBeams >= level) {
                return BeamAction::ForwardHook;
            } else if (currentBeams < level && prevBeams >= level) {
                return BeamAction::BackwardHook;
            } else if ((currentBeams >= level && nextBeams < level)
                       || (nextBeamMode == BeamMode::BEGIN16 && level > 1)
                       || (nextBeamMode == BeamMode::BEGIN32 && level > 2)) {
                return BeamAction::End;
            } else if (currentBeams >= level && prevBeams >= level && nextBeams >= level) {
                return BeamAction::Continue;
            } else if (prevBeams < level && nextBeams < level) {
                if (nextBeams > 0) {
                    return BeamAction::ForwardHook;
                } else if (prevBeams > 0) {
                    return BeamAction::BackwardHook;
                }
            }
            return std::nullopt;
        };

        struct BeamRange {
            mnx::part::Beam beam;
            size_t startIdx = 0;
            size_t endIdx = 0;
        };

        std::optional<size_t> currentStart;
        std::optional<mnx::part::Beam> currentBeam;
        std::vector<BeamRange> ranges;

        for (size_t idx = startIdx; idx <= endIdx; ++idx) {
            const auto action = beamActionForLevel(idx);
            if (!action) {
                continue;
            }
            const std::string eventId = getOrAssignEID(beamElements[idx]).toStdString();

            if (action == BeamAction::Begin) {
                currentBeam = mnxBeamArray.append();
                currentStart = idx;
                currentBeam->events().push_back(eventId);
            } else if (action == BeamAction::Continue) {
                IF_ASSERT_FAILED(currentBeam) {
                    continue;
                }
                currentBeam->events().push_back(eventId);
            } else if (action == BeamAction::End) {
                IF_ASSERT_FAILED(currentBeam && currentStart) {
                    continue;
                }
                currentBeam->events().push_back(eventId);
                ranges.push_back(BeamRange { currentBeam.value(), currentStart.value(), idx });
                currentBeam.reset();
                currentStart.reset();
            } else if (action == BeamAction::ForwardHook || action == BeamAction::BackwardHook) {
                auto hookBeam = mnxBeamArray.append();
                hookBeam.events().push_back(eventId);
                hookBeam.set_direction(action == BeamAction::ForwardHook
                                           ? mnx::BeamHookDirection::Right
                                           : mnx::BeamHookDirection::Left);
            }
        }

        for (auto& range : ranges) {
            bool hasNested = false;
            for (size_t idx = range.startIdx; idx <= range.endIdx; ++idx) {
                if (beamElements[idx]->beams() >= level + 1) {
                    hasNested = true;
                    break;
                }
            }
            if (!hasNested) {
                continue;
            }
            auto nested = range.beam.ensure_beams();
            self(self, nested, beamElements, range.startIdx, range.endIdx, level + 1);
        }
    };

    appendBeamLevel(appendBeamLevel, mnxBeams, elements, 0, elements.size() - 1, 1);
}

//---------------------------------------------------------
//   appendEvent
//   emits a single MNX event (duration + rest/notes)
//   returns true when appended
//---------------------------------------------------------

bool MnxExporter::appendEvent(mnx::ContentArray content, ExportContext& ctx, ChordRest* chordRest)
{
    const TDuration duration = chordRest->durationType();
    const bool isMeasure = duration.isMeasure();
    const bool isRest = chordRest->isRest();
    IF_ASSERT_FAILED(!isMeasure || isRest) {
        LOGW() << "Skipping ChordRest that has measure duration but is not a rest.";
        return false;
    }

    if (isRest && (!chordRest->visible() || toRest(chordRest)->isGap())) {
        /// @todo Revisit doing this for `!visible()` if MNX adds support for explicit rest visibility.
        const Fraction gapTicks = chordRest->ticks();
        const mnx::FractionValue gapDuration(
            static_cast<mnx::FractionValue::NumType>(gapTicks.numerator()),
            static_cast<mnx::FractionValue::NumType>(gapTicks.denominator()));
        content.append<mnx::sequence::Space>(gapDuration);
        return true;
    }

    const auto noteValue = isMeasure ? std::nullopt : toMnxNoteValue(duration);
    if (!noteValue) {
        LOGW() << "Skipping ChordRest with unsupported MNX duration type: "
               << static_cast<int>(duration.type());
        return false;
    }

    createBeam(ctx, chordRest);

    auto mnxEvent = content.append<mnx::sequence::Event>();
    if (isMeasure) {
        mnxEvent.set_measure(true);
    } else {
        mnxEvent.ensure_duration(noteValue->base, noteValue->dots);
    }
    mnxEvent.set_id(getOrAssignEID(chordRest).toStdString());
    createLyrics(mnxEvent, chordRest, m_lyricLineIds);
    createMarkings(mnxEvent, chordRest);
    if (chordRest->staffMove() != 0) {
        const int crossStaff = ctx.mnxPartStaff + chordRest->staffMove();
        if (crossStaff >= 1 && crossStaff <= static_cast<int>(ctx.part->nstaves())) {
            mnxEvent.set_staff(crossStaff);
        } else {
            LOGW() << "Skipping cross staff to staff not contained within part.";
        }
    }
    /// @note slurs are created in exportSpanners

    const bool success = isRest ? createRest(mnxEvent, chordRest)
                                : createNotes(mnxEvent, chordRest);

    if (success) {
        m_crToMnxEvent.emplace(chordRest, mnxEvent.pointer());
    } else {
        content.erase(content.size() - 1);
    }
    return success;
}

//---------------------------------------------------------
//   appendGrace
//   emit a grace container and recurse into its content
//---------------------------------------------------------

void MnxExporter::appendGrace(mnx::ContentArray content, ExportContext& ctx,
                              GraceNotesGroup& graceNotes)
{
    if (graceNotes.empty()) {
        return;
    }

    // Emit separate Grace containers whenever slash visibility changes so runs with
    // identical stem-slash settings stay together.
    for (size_t start = 0; start < graceNotes.size(); ) {
        const bool slash = graceNotes[start]->showStemSlash();
        size_t end = start + 1;
        while (end < graceNotes.size()
               && graceNotes[end]->showStemSlash() == slash) {
            ++end;
        }

        auto mnxGrace = content.append<mnx::sequence::Grace>();
        mnxGrace.set_slash(slash);
        /// @todo Grace note playback type has no obvious mapping from MuseScore. Revisit as appropriate.

        std::vector<ChordRest*> graceChordRests;
        graceChordRests.reserve(end - start);
        for (size_t i = start; i < end; ++i) {
            graceChordRests.push_back(graceNotes[i]);
        }

        appendContent(mnxGrace.content(), ctx, graceChordRests, ContentContext::Grace);
        start = end;
    }
}

//---------------------------------------------------------
//   findTopTuplet
//   find highest tuplet not already on the stack
//---------------------------------------------------------

const Tuplet* MnxExporter::findTopTuplet(ChordRest* chordRest, const ExportContext& ctx) const
{
    const Tuplet* tuplet = chordRest ? chordRest->tuplet() : nullptr;
    const Tuplet* topMost = nullptr;
    for (const Tuplet* cursor = tuplet; cursor; cursor = cursor->tuplet()) {
        const bool activeTuplet = std::find(ctx.tupletStack.begin(),
                                            ctx.tupletStack.end(), cursor)
                                  != ctx.tupletStack.end();
        if (!activeTuplet) {
            topMost = cursor;
        }
    }
    return topMost;
}

//---------------------------------------------------------
//   appendTuplet
//   start a tuplet container and recurse into its content
//   returns last processed index
//---------------------------------------------------------

size_t MnxExporter::appendTuplet(mnx::ContentArray content, ExportContext& ctx,
                                const std::vector<ChordRest*>& chordRests, size_t idx,
                                ChordRest* chordRest, const Tuplet* tuplet)
{
    IF_ASSERT_FAILED(tuplet) {
        return idx;
    }

    std::vector<ChordRest*> tupletChordRests;
    size_t lastTupletIdx = idx;
    for (size_t scan = idx; scan < chordRests.size(); ++scan) {
        ChordRest* scanCR = chordRests[scan];
        if (tupletContainsChordRest(tuplet, scanCR)) {
            tupletChordRests.push_back(scanCR);
            lastTupletIdx = scan;
        } else if (!tupletChordRests.empty()) {
            break;
        }
    }

    const auto baseNoteValue = toMnxNoteValue(tuplet->baseLen());
    const Fraction ratio = tuplet->ratio();
    const bool ratioValid = ratio.numerator() > 0 && ratio.denominator() > 0;
    if (tupletChordRests.empty() || !baseNoteValue || !ratioValid) {
        if (!tupletChordRests.empty()) {
            LOGW() << "Skipping tuplet with unsupported MNX base note value or ratio.";
        }
        return idx;
    }

    if (chordRest->isChord()) {
        const Chord* chord = toChord(chordRest);
        if (ctx.graceBeforeEmitted.insert(chordRest).second) {
            appendGrace(content, ctx, chord->graceNotesBefore());
        }
    }

    auto inner = mnx::NoteValueQuantity::make(static_cast<unsigned>(ratio.numerator()),
                                              *baseNoteValue);
    auto outer = mnx::NoteValueQuantity::make(static_cast<unsigned>(ratio.denominator()),
                                              *baseNoteValue);
    auto mnxTuplet = content.append<mnx::sequence::Tuplet>(inner, outer);
    mnxTuplet.set_or_clear_showNumber(toMnxTupletNumberType(tuplet->numberType()));
    mnxTuplet.set_or_clear_bracket(toMnxTupletBracketType(tuplet->bracketType()));
    /// @todo add `showValue` if MuseScore supports showing note values on tuplet relation text.

    ctx.tupletStack.push_back(tuplet);
    appendContent(mnxTuplet.content(), ctx, tupletChordRests, ContentContext::Tuplet);
    ctx.tupletStack.pop_back();
    const ChordRest* lastTupletCR = tupletChordRests.back();
    if (lastTupletCR && lastTupletCR->isChord()) {
        if (ctx.graceAfterEmitted.insert(lastTupletCR).second) {
            appendGrace(content, ctx, toChord(lastTupletCR)->graceNotesAfter());
        }
    }
    return lastTupletIdx; // shift index to last idx in tuplet
}

//---------------------------------------------------------
//   appendTremolo
//   start a tremolo container and recurse into its content
//   returns last processed index
//---------------------------------------------------------

size_t MnxExporter::appendTremolo(mnx::ContentArray content, ExportContext& ctx,
                                  const std::vector<ChordRest*>& chordRests, size_t idx,
                                  ChordRest* chordRest)
{
    Chord* chord = chordRest->isChord() ? toChord(chordRest) : nullptr;
    const TremoloTwoChord* tremolo = chord ? chord->tremoloTwoChord() : nullptr;
    IF_ASSERT_FAILED(chord && tremolo) {
        LOGW() << "Skipping ChordRest that is not part of a tremolo.";
        return idx;
    }

    IF_ASSERT_FAILED(tremolo->chord1() == chordRest) {
        LOGW() << "Skipping tremolo with unexpected first chord.";
        return idx;
    }

    Chord* chord2 = tremolo->chord2();
    IF_ASSERT_FAILED(chord2) {
        LOGW() << "Skipping tremolo with missing second chord.";
        return idx;
    }

    if (!chord->graceNotesAfter().empty() || !chord2->graceNotesBefore().empty()) {
        LOGW() << "Skipping tremolo with grace notes inside tremolo content.";
        return idx;
    }

    if (idx + 1 >= chordRests.size() || chordRests[idx + 1] != chord2) {
        LOGW() << "Skipping tremolo with non-adjacent chord events.";
        return idx;
    }

    const TremoloType tremoloType = tremolo->tremoloType();
    const int marks = static_cast<int>(tremoloType) - static_cast<int>(TremoloType::C8) + 1;
    if (marks <= 0) {
        LOGW() << "Skipping tremolo with unsupported tremolo type.";
        return idx;
    }

    TDuration tremoloDuration = tremolo->durationType();
    if (tremoloDuration.isValid()) {
        tremoloDuration = tremoloDuration.shiftRetainDots(1); // +1 divides the duration by 2.
    }
    if (!tremoloDuration.isValid()) {
        LOGW() << "Skipping 2-note tremolo with invalide duration typew.";
        return idx;
    }
    const auto tremoloNoteValue = toMnxNoteValue(tremoloDuration);
    if (!tremoloNoteValue) {
        LOGW() << "Skipping tremolo with unsupported MNX duration type.";
        return idx;
    }

    auto outer = mnx::NoteValueQuantity::make(2, *tremoloNoteValue);
    auto mnxTremolo = content.append<mnx::sequence::MultiNoteTremolo>(marks, outer);
    /// @todo Perhaps export tremolo individual duration if MNX provides clarity about it.

    std::vector<ChordRest*> tremoloChordRests { chordRest, chord2 };
    if (ctx.graceBeforeEmitted.insert(chordRest).second) {
        appendGrace(content, ctx, chord->graceNotesBefore());
    }
    appendContent(mnxTremolo.content(), ctx, tremoloChordRests, ContentContext::Tremolo);
    if (ctx.graceAfterEmitted.insert(chord2).second) {
        appendGrace(content, ctx, chord2->graceNotesAfter());
    }
    return idx + 1; // shift index to last idx in tremolo
}

//---------------------------------------------------------
//   appendContent
//   walk chord/rest events into MNX content
//---------------------------------------------------------

void MnxExporter::appendContent(mnx::ContentArray content, ExportContext& ctx,
                               const std::vector<ChordRest*>& chordRests,
                               ContentContext context)
{
    for (size_t idx = 0; idx < chordRests.size(); ++idx) {
        ChordRest* chordRest = chordRests[idx];
        IF_ASSERT_FAILED(chordRest) {
            LOGW() << "Skipping null ChordRest while exporting MNX content.";
            continue;
        }

        const bool inGrace = context == ContentContext::Grace;
        const bool isGrace = chordRest->isGrace();
        IF_ASSERT_FAILED((isGrace && inGrace) || (!isGrace && !inGrace)) {
            LOGW() << "Skipping grace note content with unexpected grace context.";
            continue;
        }

        const bool inTremolo = context == ContentContext::Tremolo;

        if (!inGrace) {
            if (chordRest->tuplet()) {
                const Tuplet* topMost = findTopTuplet(chordRest, ctx);
                if (topMost && firstTupletChordRest(topMost) == chordRest) {
                    const size_t lastIdx = appendTuplet(content, ctx, chordRests, idx,
                                                        chordRest, topMost);
                    if (lastIdx >= idx) {
                        idx = lastIdx;
                        continue;
                    }
                    IF_ASSERT_FAILED(lastIdx < idx) {
                        LOGW() << "Invalid index returned by appendTuplet.";
                    }
                }
            }
            if (chordRest->isChord() && toChord(chordRest)->tremoloTwoChord()) {
                if (!inTremolo) {
                    const Chord* chord = toChord(chordRest);
                    const TremoloTwoChord* tremolo = chord->tremoloTwoChord();
                    if (tremolo && tremolo->chord1() == chordRest) {
                        const size_t lastIdx = appendTremolo(content, ctx, chordRests, idx, chordRest);
                        if (lastIdx >= idx) {
                            idx = lastIdx;
                            continue;
                        }
                        IF_ASSERT_FAILED(lastIdx < idx) {
                            LOGW() << "Invalid index returned by appendTremolo.";
                        }
                    }
                }
            }
        }

        // Tremolos manage their own grace content. Grace notes cannot have grace notes.
        // Tuplet boundaries are handled via graceBeforeEmitted/graceAfterEmitted in appendTuplet.
        if (!inTremolo && !inGrace && chordRest->isChord()) {
            if (ctx.graceBeforeEmitted.insert(chordRest).second) {
                appendGrace(content, ctx, toChord(chordRest)->graceNotesBefore());
            }
        }

        const bool eventAppended = appendEvent(content, ctx, chordRest);

        if (eventAppended && !inTremolo && !inGrace && chordRest->isChord()) {
            if (ctx.graceAfterEmitted.insert(chordRest).second) {
                appendGrace(content, ctx, toChord(chordRest)->graceNotesAfter());
            }
        }
    }
}

//---------------------------------------------------------
//   createSequences
//---------------------------------------------------------

void MnxExporter::createSequences(const Part* part, const Measure* measure, mnx::part::Measure& mnxMeasure)
{
    const size_t staves = part->nstaves();
    auto mnxSequences = mnxMeasure.sequences();

    for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
        for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
            const track_idx_t curTrackIdx = part->startTrack() + VOICES * staffIdx + voice;
            std::vector<ChordRest*> chordRests;

            for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                if (segment->segmentType() != SegmentType::ChordRest) {
                    continue;
                }
                EngravingItem* item = segment->element(curTrackIdx);
                if (!item || !item->isChordRest()) {
                    continue;
                }
                chordRests.push_back(toChordRest(item));
            }

            if (chordRests.empty()) {
                continue;
            }

            auto mnxSequence = mnxSequences.append();
            if (staves > 1) {
                mnxSequence.set_staff(static_cast<int>(staffIdx + 1));
            }
            mnxSequence.set_voice(makeMnxVoiceIdFromTrack(mnxSequence.staff(), curTrackIdx));

            ExportContext ctx(part, measure, mnxMeasure, static_cast<staff_idx_t>(staffIdx), voice, mnxSequence.staff());
            appendContent(mnxSequence.content(), ctx, chordRests, ContentContext::Sequence);
        }
    }
}

} // namespace mu::iex::mnxio
