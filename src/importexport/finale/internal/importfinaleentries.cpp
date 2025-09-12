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
#include "internal/importfinaleparser.h"
#include "internal/importfinalelogger.h"
#include "internal/finaletypesconv.h"
#include "dom/sig.h"

#include <vector>
#include <exception>

#include "musx/musx.h"

#include "types/string.h"

#include "engraving/dom/accidental.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hook.h"
#include "engraving/dom/laissezvib.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/utils.h"

#include "engraving/rendering/score/restlayout.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

void FinaleParser::mapLayers()
{
    // This function maps layers to voices based on the layer stem directions and the hardwired MuseScore directions.
    // MuseScore hardwire voices 0,2 up and voices 1,3 down.

    m_layer2Voice.clear();
    m_layerForceStems.clear();
    std::unordered_map<voice_idx_t, LayerIndex> reverseMap;

    auto layerAttrs = m_doc->getOthers()->getArray<others::LayerAttributes>(m_currentMusxPartId);

    const auto mapLayer = [&](const MusxInstance<others::LayerAttributes>& layerAttr) {
        std::array<voice_idx_t, 4> tryOrder;
        if (!layerAttr->freezeLayer) {
            tryOrder = { 0, 1, 2, 3 }; // default
        } else if (layerAttr->freezeStemsUp) {
            tryOrder = { 0, 2, 1, 3 }; // prefer upstem voices
        } else {
            tryOrder = { 1, 3, 0, 2 }; // prefer downstem voices
        }
        const LayerIndex layerIndex = layerAttr->getCmper();
        for (voice_idx_t idx : tryOrder) {
            auto [it, emplaced] = reverseMap.emplace(idx, layerIndex);
            if (emplaced) {
                m_layer2Voice.emplace(layerIndex, idx);
                // If direction is unspecified or mismatched, force stems
                if (!layerAttr->freezeLayer || (layerAttr->freezeStemsUp != isUpVoice(idx))) {
                    m_layerForceStems.insert(layerIndex);
                }
                return;
            }
        }
        logger()->logWarning(String(u"Unable to map Finale layer %1 to a MuseScore voice due to incompatible layer attributes").arg(int(layerIndex) + 1));
    };
    for (const auto& layerAttr : layerAttrs) {
        if (layerAttr->freezeLayer) {
            mapLayer(layerAttr);
        }
    }
    for (const auto& layerAttr : layerAttrs) {
        if (!layerAttr->freezeLayer) {
            mapLayer(layerAttr);
        }
    }
}

std::unordered_map<int, voice_idx_t> FinaleParser::mapFinaleVoices(const std::map<LayerIndex, bool>& finaleVoiceMap,
                                                                        musx::dom::StaffCmper curStaff, musx::dom::MeasCmper curMeas) const
{
    using FinaleVoiceID = int;
    std::unordered_map<FinaleVoiceID, voice_idx_t> result;
    std::unordered_map<voice_idx_t, FinaleVoiceID> reverseMap;
    for (const auto& [layerIndex, usesV2] : finaleVoiceMap) {
        const auto& it = m_layer2Voice.find(layerIndex);
        if (it != m_layer2Voice.end()) {
            auto [revIt, emplaced] = reverseMap.emplace(it->second, FinaleTConv::createFinaleVoiceId(layerIndex, false));
            if (emplaced) {
                result.emplace(revIt->second, revIt->first);
                continue;
            }
        }
        logger()->logWarning(String(u"Layer %1 was not mapped to a voice").arg(int(layerIndex) + 1), m_doc, curStaff, curMeas);
    }
    for (const auto& [layerIndex, usesV2] : finaleVoiceMap) {
        if (usesV2) {
            bool foundVoice = false;
            for (voice_idx_t v : {0, 1, 2, 3}) {
                auto [revIt, emplaced] = reverseMap.emplace(v, FinaleTConv::createFinaleVoiceId(layerIndex, true));
                if (emplaced) {
                    result.emplace(revIt->second, revIt->first);
                    foundVoice = true;
                    break;
                }
            }
            if (!foundVoice) {
                logger()->logWarning(String(u"Voice 2 exceeded available MuseScore voices for layer %1.").arg(int(layerIndex) + 1), m_doc, curStaff, curMeas);
                break;
            }
        }
    }
    return result;
}

engraving::Note* FinaleParser::noteFromEntryInfoAndNumber(const EntryInfoPtr& entryInfoPtr, NoteNumber nn)
{
    if (!entryInfoPtr) {
        return nullptr;
    }
    return muse::value(m_entryNoteNumber2Note, std::make_pair(entryInfoPtr->getEntry()->getEntryNumber(), nn), nullptr);
}

engraving::Note* FinaleParser::noteFromNoteInfoPtr(const musx::dom::NoteInfoPtr& noteInfoPtr)
{
    if (!noteInfoPtr) {
        return nullptr;
    }
    return noteFromEntryInfoAndNumber(noteInfoPtr.getEntryInfo(), noteInfoPtr->getNoteId());
}

ChordRest* FinaleParser::chordRestFromEntryInfoPtr(const musx::dom::EntryInfoPtr& entryInfoPtr)
{
    return muse::value(m_entryNumber2CR, entryInfoPtr->getEntry()->getEntryNumber());
}

static void transferTupletProperties(MusxInstance<details::TupletDef> musxTuplet, Tuplet* scoreTuplet, FinaleLoggerPtr& logger)
{
    scoreTuplet->setNumberType(FinaleTConv::toMuseScoreTupletNumberType(musxTuplet->numStyle));
    // actual number object is generated on score layout

    scoreTuplet->setAutoplace(musxTuplet->smartTuplet);
    // separate bracket/number offset not supported, just add it to the whole tuplet for now
    /// @todo needs to be negated?
    scoreTuplet->setOffset(FinaleTConv::evpuToPointF(musxTuplet->tupOffX + musxTuplet->brackOffX,
                                                     musxTuplet->tupOffY + musxTuplet->brackOffY));
    scoreTuplet->setVisible(!musxTuplet->hidden);
    if (musxTuplet->autoBracketStyle != options::TupletOptions::AutoBracketStyle::Always) {
        // Can't be determined until we write all the notes/beams
        /// @todo write this setting on a second pass, along with musxTuplet->posStyle
        logger->logWarning(String(u"Unsupported"));
    }
    if (musxTuplet->avoidStaff) {
        // supported globally as a style: Sid::tupletOutOfStaff
        logger->logWarning(String(u"Tuplet: Avoiding staves is supported globally as a style, not for individual elements"));
    }
    if (musxTuplet->metricCenter) {
        // supported globally as a style: Sid::tupletNumberRythmicCenter
        logger->logWarning(String(u"Tuplet: Centering number metrically is supported globally as a style, not for individual elements"));
    }
    if (musxTuplet->fullDura) {
        // supported globally as a style: Sid::tupletExtendToEndOfDuration
        logger->logWarning(String(u"Tuplet: Bracket filling duration is supported globally as a style, not for individual elements"));
    }
    // unsupported: breakBracket, ignoreHorzNumOffset, allowHorz, useBottomNote, leftHookLen / rightHookLen (style for both)

    // bracket extensions
    /// @todo account for the fact that Finale always includes head widths in total bracket width, an option not yet in MuseScore. See #16973
    scoreTuplet->setUserPoint1(FinaleTConv::evpuToPointF(-musxTuplet->leftHookExt, 0));
    scoreTuplet->setUserPoint2(FinaleTConv::evpuToPointF(musxTuplet->rightHookExt, -musxTuplet->manualSlopeAdj));
    if (musxTuplet->alwaysFlat) {
        scoreTuplet->setUserPoint2(PointF(scoreTuplet->userP2().x(), scoreTuplet->userP1().y()));
    }
}

static size_t indexOfParentTuplet(std::vector<ReadableTuplet> tupletMap, size_t index)
{
    for (size_t i = index; i > 0; --i) {
        if (tupletMap[i].layer + 1 == tupletMap[index].layer) {
            if (tupletMap[i].startTick <= tupletMap[index].startTick && tupletMap[i].endTick >= tupletMap[index].endTick) {
                return i;
            }
        }
    }
    return 0;
}

static Tuplet* bottomTupletFromTick(std::vector<ReadableTuplet> tupletMap, Fraction pos)
{
    // return first tuplet the pos is contained within,
    // starting from the end (bottom layers) and working our way up
    for (size_t i = tupletMap.size() - 1; i > 0; --i) {
        if (pos >= tupletMap[i].startTick && pos < tupletMap[i].endTick) {
            return tupletMap[i].scoreTuplet;
        }
    }
    return nullptr;
}

static Fraction findParentTickForGraceNote(EntryInfoPtr entryInfo, bool& insertAfter, FinaleLoggerPtr& logger)
{
    for (EntryInfoPtr entryInfoPtr = entryInfo; entryInfoPtr; entryInfoPtr = entryInfoPtr.getNextSameV()) {
        if (!entryInfoPtr->getEntry()->graceNote) {
            if (!entryInfoPtr.calcDisplaysAsRest()) {
                return FinaleTConv::musxFractionToFraction(entryInfoPtr.calcGlobalElapsedDuration()).reduced();
            }
        }
    }
    for (EntryInfoPtr entryInfoPtr = entryInfo; entryInfoPtr; entryInfoPtr = entryInfoPtr.getPreviousSameV()) {
        if (!entryInfoPtr->getEntry()->graceNote) {
            if (!entryInfoPtr.calcDisplaysAsRest()) {
                insertAfter = true;
                return FinaleTConv::musxFractionToFraction(entryInfoPtr.calcGlobalElapsedDuration()).reduced();
            }
        }
    }
    // MuseScore requires grace notes be attached to a chord. The above code
    // tries to find a chord adjacent to the grace notes, but will not yield
    // a result in certain edge cases. One day MuseScore will allow the same
    // flexibility for grace notes as Finale, but until then we must do this
    logger->logWarning(String(u"Failed to attach grace notes to a chord."));
    return Fraction(-1, 1);
}

bool FinaleParser::processEntryInfo(EntryInfoPtr entryInfo, track_idx_t curTrackIdx, Measure* measure, bool graceNotes,
                                         std::vector<engraving::Note*>& notesWithUnmanagedTies,
                                         std::vector<ReadableTuplet>& tupletMap, std::unordered_map<Rest*, NoteInfoPtr>& fixedRests)
{
    // Retrieve entry from entryInfo
    MusxInstance<Entry> currentEntry = entryInfo->getEntry();
    if (!currentEntry) {
        logger()->logWarning(String(u"Failed to get entry"));
        return false;
    }
    EntryNumber currentEntryNumber = currentEntry->getEntryNumber();

    const bool isGrace = entryInfo->getEntry()->graceNote;
    if (isGrace != graceNotes) {
        return true;
    }
    bool graceAfterType = false;

    Fraction entryStartTick = Fraction(-1, 1);
    // todo: save the fraction to avoid calling this function for every grace note
    // And the grace note code is sparse in safety checks by comparison with the rest of the code.
    if (isGrace) {
        if (entryInfo.calcDisplaysAsRest()) {
            logger()->logWarning(String(u"Grace rests are not supported"));
            return false;
        }
        entryStartTick = findParentTickForGraceNote(entryInfo, graceAfterType, logger());
    } else {
        entryStartTick = FinaleTConv::musxFractionToFraction(entryInfo.calcGlobalElapsedDuration()).reduced();
    }
    if (entryStartTick.negative()) {
        // Return true for non-anchorable grace notes, else false
        return isGrace;
    }
    Segment* segment = measure->getSegmentR(SegmentType::ChordRest, entryStartTick);

    // durationType
    TDuration d = FinaleTConv::noteInfoToDuration(currentEntry->calcNoteInfo());
    if (!d.isValid()) {
        logger()->logWarning(String(u"Given ChordRest duration not supported in MuseScore"));
        return false;
    }

    ChordRest* cr = nullptr;
    int crossStaffMove = 0;
    staff_idx_t staffIdx = track2staff(curTrackIdx);

    // because we need the real staff to calculate when to show accidentals,
    // we have to calculate cross-staffing before pitches
    /// @todo what if cross-staff chord from other staves interfere with this one
    for (size_t i = 0; i < currentEntry->notes.size(); ++i) {
        NoteInfoPtr noteInfoPtr = NoteInfoPtr(entryInfo, i);
        if (noteInfoPtr->crossStaff) {
            StaffCmper nextMusxStaff = noteInfoPtr.calcStaff();
            staff_idx_t crossStaffIdx = muse::value(m_inst2Staff, nextMusxStaff, muse::nidx);
            IF_ASSERT_FAILED(crossStaffIdx != muse::nidx) {
                logger()->logWarning(String(u"Collect cross staffing: Musx inst value not found for staff cmper %1").arg(String::fromStdString(std::to_string(nextMusxStaff))));
                continue;
            }
            int newStaffMove = int(crossStaffIdx) - int(staffIdx);
            // MuseScore doesn't support individual cross-staff notes, either move the whole chord or nothing.
            // When moving, prioritise outermost staves
            if (std::abs(newStaffMove) > std::abs(crossStaffMove)) {
                crossStaffMove = newStaffMove;
            }
        }
    }
    // check if the calculate cross-staff position is valid
    staff_idx_t idx = static_cast<staff_idx_t>(int(staffIdx) + crossStaffMove);
    const Staff* baseStaff = m_score->staff(staffIdx);
    const Staff* targetStaff = m_score->staff(idx);
    if (!(targetStaff && targetStaff->visible() && targetStaff->isLinked() == baseStaff->isLinked()
          && staff2track(idx) >= baseStaff->part()->startTrack() && staff2track(idx) < baseStaff->part()->endTrack()
          && targetStaff->staffType(segment->tick())->group() == baseStaff->staffType(segment->tick())->group())) {
        crossStaffMove = 0;
        targetStaff = baseStaff;
        idx = staffIdx;
    }

    if (!entryInfo.calcDisplaysAsRest()) {
        engraving::Chord* chord = Factory::createChord(segment);

        for (size_t i = 0; i < currentEntry->notes.size(); ++i) {
            NoteInfoPtr noteInfoPtr = NoteInfoPtr(entryInfo, i);

            // calculate pitch & accidentals
            NoteVal nval = FinaleTConv::notePropertiesToNoteVal(noteInfoPtr.calcNotePropertiesConcert(), baseStaff->concertKey(segment->tick()));
            AccidentalVal accVal = tpc2alter(nval.tpc1);
            ///@todo transposition
            nval.tpc2 = nval.tpc1;

            engraving::Note* note = Factory::createNote(chord);
            note->setParent(chord);
            note->setTrack(curTrackIdx);
            note->setNval(nval);
            note->setVisible(!currentEntry->isHidden);

            // Add accidental if needed
            /// @todo Do we really need to explicitly add the accidental object if it's not frozen?
            /// RGP: if it has been manually moved, it looks like it.Otherwise perhaps not.
            if (targetStaff->isPitchedStaff(segment->tick())) {
                bool forceAccidental = noteInfoPtr->freezeAcci;
                if (!forceAccidental) {
                    int line = noteValToLine(nval, targetStaff, segment->tick());
                    bool error = false;
                    engraving::Note* startN = note->firstTiedNote();
                    if (Segment* startSegment = startN->chord()->segment()) {
                        AccidentalVal defaultAccVal = startSegment->measure()->findAccidental(startSegment, idx, line, error);
                        if (error) {
                            defaultAccVal = Accidental::subtype2value(AccidentalType::NONE); // needed?
                        }
                        forceAccidental = defaultAccVal != accVal;
                    }
                }
                /// @todo An accidental can have AccidentalAlterations even if it is not forced.
                if (forceAccidental) {
                    AccidentalType at = Accidental::value2subtype(accVal);
                    Accidental* a = Factory::createAccidental(note);
                    a->setAccidentalType(at);
                    a->setRole(noteInfoPtr->freezeAcci ? AccidentalRole::USER : AccidentalRole::AUTO);
                    a->setVisible(note->visible());
                    a->setParent(note);
                    if (currentEntry->noteDetail) {
                        // Accidental size and offset
                        /// @todo Finale doesn't offset notes for ledger lines, MuseScore offsets
                        /// rightmost accidentals matching the type of an accidental on a note with ledger lines.
                        /// @todo decide when to disable autoplace
                        if (const MusxInstance<details::AccidentalAlterations>& accidentalInfo = m_doc->getDetails()->getForNote<details::AccidentalAlterations>(noteInfoPtr)) {
                            if (muse::RealIsEqualOrLess(FinaleTConv::doubleFromPercent(accidentalInfo->percent), m_score->style().styleD(Sid::smallNoteMag))) {
                                a->setSmall(true);
                            }
                            /// @todo this calculation needs to take into account the default accidental separation amounts in accidentalOptions. The options
                            /// should allow us to calculate the default position of the accidental relative to the note. (But it may not be easy.)
                            /// The result will probably also need to be multiplied by SPATIUM20, if other items are any guide.
                            a->setOffset(FinaleTConv::evpuToPointF(accidentalInfo->hOffset, accidentalInfo->allowVertPos ? -accidentalInfo->vOffset : 0));
                        }
                    }
                    note->add(a);
                }
            }

            if (currentEntry->noteDetail) {
                if (const MusxInstance<details::NoteAlterations> noteInfo = m_doc->getDetails()->getForNote<details::NoteAlterations>(noteInfoPtr)) {
                    if (muse::RealIsEqualOrLess(FinaleTConv::doubleFromPercent(noteInfo->percent), m_score->style().styleD(Sid::smallNoteMag))) {
                        note->setSmall(true);
                    }
                    note->setOffset(FinaleTConv::evpuToPointF(noteInfo->nxdisp, noteInfo->allowVertPos ? -noteInfo->nydisp : 0));
                    /// @todo interpret notehead type from altNhead (and perhaps useOwnFont/customFont as well).
                }
            }

            chord->add(note);

            // set up ties
            if (noteInfoPtr->tieStart) {
                // We can't tell for sure at this point whether a tie will have an end note,
                // so we decide between real tie and l.v. tie later on.
                notesWithUnmanagedTies.emplace_back(note);
            }
            /// @todo This code won't work if the start note is in a currently unmapped voice. But because of
            /// the fact we explicitly create accidentals, we need the ties to be correct during processEntryInfo. (Do we?)
            // Don't use noteInfoPtr->tieEnd as it's unreliable
            engraving::Note* prevTied = noteFromNoteInfoPtr(noteInfoPtr.calcTieFrom());
            if (prevTied) {
                Tie* tie = Factory::createTie(m_score->dummy());
                tie->setStartNote(prevTied);
                tie->setTick(prevTied->tick());
                tie->setTrack(prevTied->track());
                // Finale offers independent visibility controls for tie segments, MuseScore currently does not.
                // We set the visibility based on
                tie->setVisible(prevTied->visible());
                tie->setParent(prevTied); //needed?
                prevTied->setTieFor(tie);
                tie->setEndNote(note);
                tie->setTick2(note->tick());
                tie->setTrack2(note->track());
                note->setTieBack(tie);
                muse::remove(notesWithUnmanagedTies, prevTied);
            } else {
                logger()->logInfo(String(u"Tie does not have starting note. Possibly a partial tie, currently unsupported."));
            }
            m_entryNoteNumber2Note.emplace(std::make_pair(currentEntryNumber, noteInfoPtr->getNoteId()), note);
        }
        if (currentEntry->freezeStem || currentEntry->voice2 || currentEntry->v2Launch
            || muse::contains(m_layerForceStems, entryInfo.getLayerIndex())) {
            // Additionally, beams have their own vertical direction, which is set in processBeams.
            DirectionV dir = currentEntry->upStem ? DirectionV::UP : DirectionV::DOWN;
            chord->setStemDirection(dir);
        }
        // Only create explicitly if properties need changing
        if (currentEntry->isHidden) {
            Stem* stem = Factory::createStem(chord);
            stem->setVisible(false);
            chord->add(stem);
        }
        if (d.hooks() > 0 && entryInfo.calcUnbeamed()) {
            // Notes with flags are not accounted for in beaming code,
            // and rests are unbeamed by default
            chord->setBeamMode(BeamMode::NONE);
            if (currentEntry->isHidden) {
                Hook* hook = new Hook(chord);
                hook->setVisible(false);
                chord->setHook(hook);
                chord->add(hook);
            }
        }
        /// @todo We can't use CustomStem directly, we need to use CustomUpStem or CustomDownStem, which depends on (beam) layout
        /// @todo StemAlterations and StemAlterationsUnderBeam
        /// @todo is this where stemSlash is determined?
        /// @todo does this chord have a stem already?
        /* if (chord->stem() && currentEntry->stemDetail) {
            // Stem visibility and offset
            MusxInstanceList<details::CustomStem> customStems = m_doc->getDetails()->getArray<details::CustomStem>(m_currentMusxPartId); // RGP: pass in the entry number as 2nd param here
            for (const MusxInstance<details::CustomStem>& customStem : customStems) {
                chord->stem()->setVisible(customStem->calcIsHiddenStem());
                if (customStem->hOffset != 0 || customStem->vOffset != 0) {
                    chord->stem()->setOffset(FinaleTConv::evpuToPointF(customStem->hOffset, -customStem->vOffset));
                    chord->stem()->setAutoPlace(false); // make more nuanced?
                }
            }
        } */
        // chord->setIsChordPlayable(!currentEntry->noPlayback); //this is an undo method
        cr = toChordRest(chord);
    } else {
        const MusxInstance<others::Staff> musxStaff = entryInfo.createCurrentStaff();
        if (entryInfo.calcIsFullMeasureRest()) {
            d = TDuration(DurationType::V_MEASURE);
        }
        Rest* rest = Factory::createRest(segment, d);
        // Fixed-positioning for rests is calculated in a 2nd pass after all voices in all layers have been created.
        // This allows MuseScore code to calculate correctly the voice offset for the rest.
        if (!currentEntry->floatRest && !currentEntry->notes.empty()) {
            fixedRests.emplace(rest, NoteInfoPtr(entryInfo, 0));
        }
        cr = toChordRest(rest);
        cr->setVisible(!musxStaff->hideRests && !currentEntry->isHidden);
    }

    int entrySize = entryInfo.calcEntrySize();
    if (entrySize <= musx::dom::MAX_CUE_PERCENTAGE) {
        double crMag = FinaleTConv::doubleFromPercent(entrySize);
        if (muse::RealIsEqualOrLess(crMag, m_score->style().styleD(Sid::smallNoteMag))) { // is just less enough here?
            if (m_smallNoteMagFound) {
                logger()->logWarning(String(u"Inconsistent cue note sizes found. Using the smallest encountered."), m_doc, entryInfo.getStaff(), entryInfo.getMeasure());
            }
            m_score->style().set(Sid::smallNoteMag, crMag);
        }
        m_smallNoteMagFound = true;
        cr->setSmall(true);
    }

    cr->setDurationType(d);
    cr->setStaffMove(crossStaffMove);
    cr->setTrack(curTrackIdx);
    if (cr->durationType().type() == DurationType::V_MEASURE) {
        cr->setTicks(measure->timesig() * baseStaff->timeStretch(measure->tick())); // baseStaff because that's the staff the cr 'belongs to'
    } else {
        cr->setTicks(cr->actualDurationType().fraction());
    }
    if (isGrace) {
        engraving::Chord* gc = toChord(cr);
        /// @todo Account for stem slash plugin instead of just document options
        gc->setNoteType((!graceAfterType /* && gc->beams() > 0 */ && entryInfo.calcUnbeamed() && (currentEntry->slashGrace || musxOptions().graceOptions->slashFlaggedGraceNotes))
                         ? engraving::NoteType::ACCIACCATURA : FinaleTConv::durationTypeToNoteType(d.type(), graceAfterType));
        engraving::Chord* graceParentChord = toChord(segment->element(curTrackIdx));
        gc->setGraceIndex(static_cast<int>(graceAfterType ? graceParentChord->graceNotesAfter().size() : graceParentChord->graceNotesBefore().size()));
        graceParentChord->add(gc);
    } else {
        segment->add(cr);
        if (Tuplet* parentTuplet = bottomTupletFromTick(tupletMap, entryStartTick)) {
            parentTuplet->add(cr);
        }
        logger()->logInfo(String(u"Adding entry of duration %2 at tick %1").arg(entryStartTick.toString(), cr->durationTypeTicks().toString()));
    }

    /// Currently we only generate dots if they have modified properties.
    /// Is this correct? Probably. -RGP: I think so too.
    // Dot offset
    /// @todo MuseScore's dot placement is smarter than Finale's, ideally we should account for the difference in effective positioning.
    if (currentEntry->dotTieAlt) {
        MusxInstanceList<details::DotAlterations> dotAlterations = m_doc->getDetails()->getArray<details::DotAlterations>(m_currentMusxPartId, currentEntryNumber);
        for (const MusxInstance<details::DotAlterations>& da : dotAlterations) {
            engraving::Note* n = cr->isChord() ? noteFromEntryInfoAndNumber(entryInfo, da->getNoteId()) : nullptr;
            Rest* r = cr->isRest() ? toRest(cr) : nullptr;
            EvpuFloat museInterdot = EvpuFloat(da->interdotSpacing) - evpuAugmentationDotWidth(); /// @todo not sure about this, but it gets the closest result to Finale in my testing
            if (n) {
                for (int i = 0; i < cr->dots(); ++i) {
                    NoteDot* dot = Factory::createNoteDot(n);
                    dot->setParent(n);
                    dot->setVisible(n->visible());
                    dot->setTrack(cr->track());
                    dot->setOffset(FinaleTConv::evpuToPointF((da->hOffset + i * museInterdot) * SPATIUM20, -da->vOffset * SPATIUM20));
                    n->add(dot);
                }
            } else if (r) {
                for (int i = 0; i < cr->dots(); ++i) {
                    NoteDot* dot = Factory::createNoteDot(r);
                    dot->setParent(r);
                    dot->setVisible(r->visible());
                    dot->setTrack(cr->track());
                    dot->setOffset(FinaleTConv::evpuToPointF((da->hOffset + i * museInterdot) * SPATIUM20, -da->vOffset * SPATIUM20));
                    r->add(dot);
                }
            } else {
                break;
            }
        }
    }
    m_entryNumber2CR.emplace(currentEntryNumber, cr);
    return true;
}

bool FinaleParser::processBeams(EntryInfoPtr entryInfoPtr, track_idx_t curTrackIdx)
{
    if (!entryInfoPtr.calcIsBeamStart()) {
        return true;
    }
    /// @todo detect special cases for beams over barlines created by the Beam Over Barline plugin
    ChordRest* cr = chordRestFromEntryInfoPtr(entryInfoPtr);
    IF_ASSERT_FAILED(cr) {
        logger()->logWarning(String(u"Entry %1 was not mapped").arg(entryInfoPtr->getEntry()->getEntryNumber()), m_doc, entryInfoPtr.getStaff(), entryInfoPtr.getMeasure());
        return false;
    }
    Beam* beam = Factory::createBeam(m_score->dummy()->system());
    beam->setTrack(curTrackIdx);
    if (entryInfoPtr->getEntry()->isNote && cr->isChord()) {
        beam->setDirection(toChord(cr)->stemDirection());
    }
    beam->add(cr);
    cr->setBeamMode(BeamMode::BEGIN);
    ChordRest* lastCr = nullptr;
    for (EntryInfoPtr nextInBeam = entryInfoPtr.getNextInBeamGroup(); nextInBeam; nextInBeam = nextInBeam.getNextInBeamGroup()) {
        MusxInstance<Entry> currentEntry = nextInBeam->getEntry();
        if (entryInfoPtr->getEntry()->graceNote && !currentEntry->isNote) {
            // Grace rests are unmapped and not supported
            continue;
        }
        lastCr = chordRestFromEntryInfoPtr(nextInBeam);
        IF_ASSERT_FAILED(lastCr) {
            logger()->logWarning(String(u"Entry %1 was not mapped").arg(currentEntry->getEntryNumber()), m_doc, nextInBeam.getStaff(), nextInBeam.getMeasure());
            continue;
        }
        /// @todo fully test secondary beam breaks: can a smaller beam than 32nd be broken?
        /// XM: No, not currently in MuseScore.
        unsigned secBeamStart = 0;
        if (currentEntry->secBeam) {
            if (auto secBeamBreak = m_doc->getDetails()->get<details::SecondaryBeamBreak>(nextInBeam.getFrame()->getRequestedPartId(), currentEntry->getEntryNumber())) {
                secBeamStart = secBeamBreak->calcLowestBreak();
            }
        }
        if (secBeamStart <= 1) {
            lastCr->setBeamMode(BeamMode::MID);
        } else if (secBeamStart == 2) {
            lastCr->setBeamMode(BeamMode::BEGIN16);
        } else {
            lastCr->setBeamMode(BeamMode::BEGIN32);
        }
        beam->add(lastCr);
    }
    if (lastCr) {
        lastCr->setBeamMode(BeamMode::END);
    }
    return true;
}

bool FinaleParser::positionFixedRests(const std::unordered_map<Rest*, musx::dom::NoteInfoPtr>& fixedRests)
{
    for (const auto& next : fixedRests) {
        Rest* rest = next.first;
        NoteInfoPtr noteInfoPtr = next.second;
        EntryInfoPtr entryInfoPtr = noteInfoPtr.getEntryInfo();
        StaffCmper targetMusxStaffId = muse::value(m_staff2Inst, rest->staffIdx(), 0);
        IF_ASSERT_FAILED (targetMusxStaffId) {
            logger()->logWarning(String(u"Entry %1 (a rest) was not mapped to a known musx staff.").arg(entryInfoPtr->getEntry()->getEntryNumber()), m_doc, entryInfoPtr.getStaff(), entryInfoPtr.getMeasure());
            return false;
        }
        if (noteInfoPtr->getNoteId() == musx::dom::Note::RESTID) {
            /// @todo correctly calculate default rest position in multi-voice situation. rest->setAutoPlace is problematic because it also affects spacing
            /// (and does not cover all vertical placement situations either).
            MusxInstance<others::StaffComposite> currMusxStaff = noteInfoPtr.getEntryInfo().createCurrentStaff(targetMusxStaffId);
            IF_ASSERT_FAILED(currMusxStaff) {
                logger()->logWarning(String(u"Target staff %1 not found.").arg(targetMusxStaffId), m_doc, entryInfoPtr.getStaff(), entryInfoPtr.getMeasure());
            }
            auto [pitchClass, octave, alteration, staffPosition] = noteInfoPtr.calcNotePropertiesInView();
            const StaffType* staffType = rest->staffType();
            // following code copied from TLayout::layoutRest:
            /// @todo this code is now private to the layout module, so we will need a new method.
            // Rest::LayoutData layoutData;
            // const int naturalLine = rendering::score::RestLayout::computeNaturalLine(staffType->lines()); // Measured in 1sp steps
            // const int voiceOffset = rendering::score::RestLayout::computeVoiceOffset(rest, &layoutData); // Measured in 1sp steps
            // omit call to computeWholeOrBreveRestOffset because it requires layout rectangles to have been created
            int finalLine = 0; //naturalLine + voiceOffset;
            // convert finalLine to staff position offset for Finale rest. This value is measured in 0.5sp steps.
            const int staffPositionOffset = 2 * finalLine - currMusxStaff->calcToplinePosition();
            const double lineSpacing = staffType->lineDistance().val();
            if (rest->isWholeRest()) {
                staffPosition += 2; // account for a whole rest's staff line discrepancy between Finale and MuseScore
            }
            rest->setAlignWithOtherRests(false); // override as much automatic positioning as possible
            rest->ryoffset() = double(-staffPosition - staffPositionOffset) * rest->staff()->spatium(rest->tick()) * lineSpacing / 2.0;
            /// @todo Account for additional default positioning around collision avoidance (when the rest is on the "wrong" side for the voice.)
            /// Unfortunately, we can't set `autoplace` to false because that also suppresses horizontal spacing.
            /// The test file `beamsAndRest.musx` includes an example of this issue in the first 32nd rest in the top staff.
            /// It should be at the same vertical position as the second 32nd rest in the same staff.
        } else {
            logger()->logWarning(String(u"Rest found with unexpected note ID %1").arg(noteInfoPtr->getNoteId()), m_doc, entryInfoPtr.getStaff(), entryInfoPtr.getMeasure());
            return false;
        }
    }
    return true;
}

static void processTremolos(std::vector<ReadableTuplet>& tremoloMap, track_idx_t curTrackIdx, Measure* measure)
{
    /// @todo account for invalid durations
    Fraction timeStretch = measure->score()->staff(track2staff(curTrackIdx))->timeStretch(measure->tick());
    for (ReadableTuplet tuplet : tremoloMap) {
        engraving::Chord* c1 = measure->findChord(measure->tick() + tuplet.startTick, curTrackIdx); // timestretch?
        engraving::Chord* c2 = measure->findChord(measure->tick() + ((tuplet.startTick + tuplet.endTick) / 2), curTrackIdx);
        IF_ASSERT_FAILED(c1 && c2 && c1->ticks() == c2->ticks()) {
            continue;
        }

        // the current (too short) duration indicates the
        // tremolo type, we calculate it from number of beams
        int tremoloBeamsNum = int(TremoloType::C8) - 1 + c1->durationType().hooks();
        // rather than not import the tremolo, force it to be a valid type
        tremoloBeamsNum = std::clamp(tremoloBeamsNum, int(TremoloType::C8), int(TremoloType::C64));

        // now we have to set the correct duration for the chords to,
        // fill the space (and account for any tuplets they may be in)
        Fraction d = (c2->tick() - c1->tick()) * timeStretch;
        for (Tuplet* t = c1->tuplet(); t; t = t->tuplet()) {
            d *= t->ratio();
        }
        c1->setDurationType(d.reduced());
        c1->setTicks(c1->actualDurationType().fraction());
        // since durationType and ticks aren't global but supposed to
        // not account  for tuplets, c2 can match c1 even if it's in another tuplet
        c2->setDurationType(c1->durationType());
        c2->setTicks(c1->ticks());

        TremoloTwoChord* tremolo = Factory::createTremoloTwoChord(c1);
        tremolo->setTremoloType(TremoloType(tremoloBeamsNum));
        tremolo->setTrack(curTrackIdx);
        tremolo->setVisible(c1->notes().front()->visible());
        tremolo->setParent(c1);
        tremolo->setChords(c1, c2);
        c1->setTremoloTwoChord(tremolo);
    }
}

static void createTupletMap(std::vector<EntryFrame::TupletInfo> tupletInfo,
                            std::vector<ReadableTuplet>& tupletMap, std::vector<ReadableTuplet>& tremoloMap, int voice)
{
    const bool forVoice2 = bool(voice);
    for (const auto& tuplet : tupletInfo) {
        if (forVoice2 != tuplet.voice2 || tuplet.tuplet->calcRatio() == 0) {
            continue;
        }
        ReadableTuplet rTuplet;
        rTuplet.startTick  = FinaleTConv::musxFractionToFraction(tuplet.startDura).reduced();
        rTuplet.endTick    = FinaleTConv::musxFractionToFraction(tuplet.endDura).reduced();
        rTuplet.musxTuplet = tuplet.tuplet;
        if (tuplet.calcIsTremolo()) {
            tremoloMap.emplace_back(rTuplet);
        } else {
            tupletMap.emplace_back(rTuplet);
        }
    }

   for (size_t i = 0; i < tupletMap.size(); ++i) {
        for (size_t j = 0; j < tupletMap.size(); ++j) {
            if (i == j) {
                continue;
            }

            if (tupletMap[i].startTick >= tupletMap[j].startTick && tupletMap[i].endTick <= tupletMap[j].endTick
                && (tupletMap[i].startTick > tupletMap[j].startTick || tupletMap[i].endTick < tupletMap[j].endTick
                    || tupletMap[i].layer == tupletMap[j].layer)) {
                tupletMap[i].layer = std::max(tupletMap[i].layer, tupletMap[j].layer + 1);
            }
        }
    }

    std::sort(tupletMap.begin(), tupletMap.end(), [](const ReadableTuplet& a, const ReadableTuplet& b) {
        return (a.layer == b.layer) ? (a.startTick < b.startTick) : (a.layer < b.layer);
    });
}

static void createTupletsFromMap(Measure* measure, track_idx_t curTrackIdx, std::vector<ReadableTuplet>& tupletMap, FinaleLoggerPtr& logger)
{
    // create Tuplets as needed, starting with the outermost
    for (size_t i = 1; i < tupletMap.size(); ++i) {
        TDuration baseLen = FinaleTConv::noteInfoToDuration(calcNoteInfoFromEdu(tupletMap[i].musxTuplet->referenceDuration));
        if (!baseLen.isValid()) {
            logger->logWarning(String(u"Given Tuplet duration not supported in MuseScore"));
            continue;
        }
        tupletMap[i].scoreTuplet = Factory::createTuplet(measure);
        tupletMap[i].scoreTuplet->setTrack(curTrackIdx);
        tupletMap[i].scoreTuplet->setTick(measure->tick() + tupletMap[i].startTick);
        tupletMap[i].scoreTuplet->setParent(measure);
        // musxTuplet::calcRatio is the reciprocal of what MuseScore needs
        /// @todo skip case where finale numerator is 0: often used for changing beams
        Fraction tupletRatio = FinaleTConv::musxFractionToFraction(tupletMap[i].musxTuplet->calcRatio().reciprocal());
        tupletMap[i].scoreTuplet->setRatio(tupletRatio);
        tupletMap[i].scoreTuplet->setBaseLen(baseLen);
        Fraction f = baseLen.fraction() * tupletRatio.denominator();
        tupletMap[i].scoreTuplet->setTicks(f.reduced());
        logger->logInfo(String(u"Detected Tuplet: Starting at %1, duration: %2, ratio: %3").arg(
                        tupletMap[i].startTick.toString(), f.reduced().toString(), tupletRatio.toString()));
        for (size_t ratioIndex = indexOfParentTuplet(tupletMap, i); tupletMap[ratioIndex].layer >= 0; ratioIndex = indexOfParentTuplet(tupletMap, ratioIndex)) {
            // finale value doesn't include parent tuplet ratio, but is global. Our setup should be correct though, so hack the assert
            f /= tupletMap[ratioIndex].scoreTuplet->ratio();
        }
        if (f != tupletMap[i].endTick - tupletMap[i].startTick) { // implement with IF_ASSERT_FAILED after the @todo below is addressed. Otherwise, we can't test with real files.
            logger->logWarning(String(u"Tuplet duration is corrupted"));
            continue;
            /// @todo account for tuplets with invalid durations, i.e. durations not attainable in MuseScore
        }
        transferTupletProperties(tupletMap[i].musxTuplet, tupletMap[i].scoreTuplet, logger);
        // reparent tuplet if needed
        size_t parentIndex = indexOfParentTuplet(tupletMap, i);
        if (tupletMap[parentIndex].layer >= 0) {
            tupletMap[parentIndex].scoreTuplet->add(tupletMap[i].scoreTuplet);
        }
    }
    /// @todo get beaming rules from time signature and only override properties when necessary
}

void FinaleParser::importEntries()
{
    // Add entries (notes, rests, tuplets)
    if (m_score->measures()->empty()) {
        logger()->logWarning(String(u"Add entries: No measures in score"));
        return;
    }
    MusxInstanceList<others::Measure> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    MusxInstanceList<others::StaffUsed> musxScrollView = m_doc->getOthers()->getArray<others::StaffUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);
    std::vector<engraving::Note*> notesWithUnmanagedTies;
    for (const MusxInstance<others::StaffUsed>& musxScrollViewItem : musxScrollView) {
        staff_idx_t curStaffIdx = muse::value(m_inst2Staff, StaffCmper(musxScrollViewItem->staffId), muse::nidx);
        track_idx_t staffTrackIdx = curStaffIdx * VOICES;
        IF_ASSERT_FAILED (curStaffIdx != muse::nidx) {
            logger()->logWarning(String(u"Add entries: Musx inst value not found."), m_doc, musxScrollViewItem->staffId, 1);
            continue;
        }

        Staff* curStaff = m_score->staff(curStaffIdx);

        for (const MusxInstance<others::Measure>& musxMeasure : musxMeasures) {
            Fraction currTick = muse::value(m_meas2Tick, musxMeasure->getCmper(), Fraction(-1, 1));
            Measure* measure = !currTick.negative()  ? m_score->tick2measure(currTick) : nullptr;
            if (!measure) {
                logger()->logWarning(String(u"Unable to retrieve measure by tick"), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                break;
            }
            details::GFrameHoldContext gfHold(musxMeasure->getDocument(), m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper());
            bool processContext = bool(gfHold);
            if (processContext && gfHold.calcIsCuesOnly()) {
                logger()->logWarning(String(u"Cue notes not yet supported"), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                processContext = false;
            }
            // Note from RGP: You cannot short-circuit out of this code with a `if (!processContext) continue` statement.
            // The code after the if statement must still be executed.
            if (processContext) {
                // gfHold.calcVoices() guarantees that every layer/voice returned contains entries
                std::map<LayerIndex, bool> finaleLayers = gfHold.calcVoices();
                std::unordered_map<int, track_idx_t> finaleVoiceMap = mapFinaleVoices(finaleLayers, musxScrollViewItem->staffId, musxMeasure->getCmper());
                std::unordered_map<Rest*, musx::dom::NoteInfoPtr> fixedRests;
                for (const auto& finaleLayer : finaleLayers) {
                    const LayerIndex layer = finaleLayer.first;
                    /// @todo reparse with forWrittenPitch true, to obtain correct transposed keysigs/clefs/enharmonics
                    MusxInstance<EntryFrame> entryFrame = gfHold.createEntryFrame(layer);
                    if (!entryFrame) {
                        logger()->logWarning(String(u"Layer %1 not found.").arg(int(layer)), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                        continue;
                    }
                    const int maxV1V2 = finaleLayer.second ? 1 : 0;
                    for (int voice = 0; voice <= maxV1V2; voice++) {
                        // calculate current track
                        voice_idx_t voiceOff = muse::value(finaleVoiceMap, FinaleTConv::createFinaleVoiceId(layer, bool(voice)), muse::nidx);
                        IF_ASSERT_FAILED(voiceOff != muse::nidx && voiceOff < VOICES) {
                            logger()->logWarning(String(u"Encountered incorrectly mapped voice ID for layer %1").arg(int(layer) + 1), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                            continue;
                        }

                        track_idx_t curTrackIdx = staffTrackIdx + voiceOff;

                        // generate tuplet map, tremolo map and create tuplets
                        // trick: insert invalid 'tuplet' spanning the whole measure. useful for fallback
                        ReadableTuplet rTuplet;
                        rTuplet.startTick = Fraction(0, 1);
                        rTuplet.endTick = (measure->timesig() * curStaff->timeStretch(measure->tick())).reduced(); // account for local timesigs (needed?)
                        rTuplet.layer = -1;
                        std::vector<ReadableTuplet> tupletMap = { rTuplet };
                        std::vector<ReadableTuplet> tremoloMap;
                        createTupletMap(entryFrame->tupletInfo, tupletMap, tremoloMap, voice);
                        createTupletsFromMap(measure, curTrackIdx, tupletMap, logger());

                        // add chords and rests
                        for (EntryInfoPtr entryInfoPtr = entryFrame->getFirstInVoice(voice + 1); entryInfoPtr; entryInfoPtr = entryInfoPtr.getNextInVoice(voice + 1)) {
                            processEntryInfo(entryInfoPtr, curTrackIdx, measure, /*graceNotes*/ false, notesWithUnmanagedTies, tupletMap, fixedRests);
                        }
                        for (EntryInfoPtr entryInfoPtr = entryFrame->getFirstInVoice(voice + 1); entryInfoPtr; entryInfoPtr = entryInfoPtr.getNextInVoice(voice + 1)) {
                            processEntryInfo(entryInfoPtr, curTrackIdx, measure, /*graceNotes*/ true, notesWithUnmanagedTies, tupletMap, fixedRests);
                        }

                        // add tremolos
                        processTremolos(tremoloMap, curTrackIdx, measure);

                        // create beams
                        for (EntryInfoPtr entryInfoPtr = entryFrame->getFirstInVoice(voice + 1); entryInfoPtr; entryInfoPtr = entryInfoPtr.getNextInVoice(voice + 1)) {
                            processBeams(entryInfoPtr, curTrackIdx);
                        }
                        // m_entryNumber2CR.clear(); /// @todo use 2 maps, one of which clears itself, to make beaming more efficient
                    }
                }
                // position fixed rests after all layers have been imported
                positionFixedRests(fixedRests);
            }
            // Avoid corruptions: fill in any gaps in existing voices...
            // logger()->logInfo(String(u"Fixing corruptions for measure at staff %1, tick %2").arg(String::number(curStaffIdx), currTick.toString()));
            measure->checkMeasure(curStaffIdx);
            // ...and make sure voice 1 exists.
            if (!measure->hasVoice(staffTrackIdx)) {
                MusxInstance<others::StaffComposite> currMusxStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper(), 0);
                Segment* segment = measure->getSegmentR(SegmentType::ChordRest, Fraction(0, 1));
                Rest* rest = Factory::createRest(segment, TDuration(DurationType::V_MEASURE));
                rest->setScore(m_score);
                rest->setTicks(measure->timesig() * curStaff->timeStretch(measure->tick()));
                rest->setTrack(staffTrackIdx);
                rest->setVisible(!currMusxStaff->hideRests && !currMusxStaff->blankMeasure);
                segment->add(rest);
            }
        }

        // Ties can only be attached to notes within a single part (instrument).
        // In the last staff of an instrument, add the ties and clear the vector.
        if (curStaff == curStaff->part()->staves().back()) {
            for (engraving::Note* note : notesWithUnmanagedTies) {
                Tie* tie = Factory::createLaissezVib(m_score->dummy()->note());
                tie->setStartNote(note);
                tie->setTick(note->tick());
                tie->setTrack(note->track());
                tie->setParent(note); //needed?
                note->setTieFor(tie);
            }
            notesWithUnmanagedTies.clear();
        }
    }
}

}
