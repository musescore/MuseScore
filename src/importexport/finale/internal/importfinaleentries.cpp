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
#include "engraving/dom/tie.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/utils.h"

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

    const auto mapLayer = [&](const std::shared_ptr<others::LayerAttributes>& layerAttr) {
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
                const bool stemUpVoice = (idx % 2 == 0); // voices 0,2
                if (!layerAttr->freezeLayer || (layerAttr->freezeStemsUp != stemUpVoice)) {
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
                                                                        musx::dom::InstCmper curStaff, musx::dom::MeasCmper curMeas) const
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

static void transferTupletProperties(std::shared_ptr<const details::TupletDef> musxTuplet, Tuplet* scoreTuplet, FinaleLoggerPtr& logger)
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
        // center number using duration
        /// @todo will be supported globally as a style
        logger->logWarning(String(u"Tuplet: Centering number metrically is soon to be supported globally as a style, not for individual elements"));
    }
    if (musxTuplet->fullDura) {
        // extend bracket to full duration
        /// @todo will be supported globally as a style
        logger->logWarning(String(u"Tuplet: Bracket filling duration is soon to be supported globally as a style, not for individual elements"));
    }
    // unsupported: breakBracket, ignoreHorzNumOffset, allowHorz, useBottomNote, leftHookLen / rightHookLen (style for both)

    // bracket extensions
    /// @todo account for the fact that Finale always includes head widths in total bracket width, an option not yet in musescore. See PR and the related issues
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

bool FinaleParser::processEntryInfo(EntryInfoPtr entryInfo, track_idx_t curTrackIdx, Measure* measure,
                                         std::vector<ReadableTuplet>& tupletMap, std::unordered_map<Rest*, NoteInfoPtr>& fixedRests)
{
    if (entryInfo->getEntry()->graceNote) {
        logger()->logWarning(String(u"Grace notes not yet supported"), m_doc, entryInfo.getStaff(), entryInfo.getMeasure());
        return true;
    }
    if (entryInfo.calcIsCue()) {
        logger()->logWarning(String(u"Cue notes not yet supported"), m_doc, entryInfo.getStaff(), entryInfo.getMeasure());
        return true;
    }

    Fraction entryStartTick = FinaleTConv::musxFractionToFraction(entryInfo.calcGlobalElapsedDuration()).reduced();
    Segment* segment = measure->getSegmentR(SegmentType::ChordRest, entryStartTick);

    // Retrieve entry from entryInfo
    std::shared_ptr<const Entry> currentEntry = entryInfo->getEntry();
    if (!currentEntry) {
        logger()->logWarning(String(u"Failed to get entry"));
        return false;
    }

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
    for (size_t i = 0; i < currentEntry->notes.size(); ++i) {
        NoteInfoPtr noteInfoPtr = NoteInfoPtr(entryInfo, i);
        if (noteInfoPtr->crossStaff) {
            InstCmper nextMusxStaff = noteInfoPtr.calcStaff();
            staff_idx_t crossStaffIdx = muse::value(m_inst2Staff, nextMusxStaff, muse::nidx);
            IF_ASSERT_FAILED(crossStaffIdx != muse::nidx) {
                logger()->logWarning(String(u"Collect cross staffing: Musx inst value not found for staff cmper %1").arg(String::fromStdString(std::to_string(nextMusxStaff))));
                continue;
            }
            int newStaffMove = int(crossStaffIdx) - int(staffIdx);
            // MuseScore doesn't support individual cross-staff notes, either move the whole chord or nothing
            // When moving, prioritise outermost staves
            if (std::abs(newStaffMove) > std::abs(crossStaffMove)) {
                crossStaffMove = newStaffMove;
            }
        }
    }
    // check if the calculate cross-staff position is valid
    staff_idx_t idx = static_cast<staff_idx_t>(int(staffIdx) + crossStaffMove);
    const Staff* baseStaff = m_score->staff(staffIdx);
    const StaffType* baseStaffType = baseStaff->staffType(segment->tick());
    const Staff* targetStaff = m_score->staff(idx);
    const StaffType* targetStaffType = targetStaff ? targetStaff->staffType(segment->tick()) : nullptr;
    staff_idx_t minStaff = track2staff(baseStaff->part()->startTrack());
    staff_idx_t maxStaff = track2staff(baseStaff->part()->endTrack());
    if (!(targetStaff && targetStaff->visible() && idx >= minStaff && idx < maxStaff
          && targetStaffType->group() == baseStaffType->group() && targetStaff->isLinked() == baseStaff->isLinked())) {
        crossStaffMove = 0;
        targetStaff = baseStaff;
        idx = staffIdx;
    }

    if (!entryInfo.calcDisplaysAsRest()) {
        Chord* chord = Factory::createChord(segment);

        for (size_t i = 0; i < currentEntry->notes.size(); ++i) {
            NoteInfoPtr noteInfoPtr = NoteInfoPtr(entryInfo, i);

            // calculate pitch & accidentals
            NoteVal nval = FinaleTConv::notePropertiesToNoteVal(noteInfoPtr.calcNoteProperties(), baseStaff->concertKey(segment->tick()));
            AccidentalVal accVal = tpc2alter(nval.tpc1);
            ///@todo transposition
            nval.tpc2 = nval.tpc1;

            engraving::Note* note = Factory::createNote(chord);
            note->setParent(chord);
            note->setTrack(curTrackIdx);
            note->setNval(nval);

            // Add accidental if needed
            bool forceAccidental = noteInfoPtr->freezeAcci;
            if (!forceAccidental) {
                int line = noteValToLine(nval, targetStaff, segment->tick());
                bool error = false;
                engraving::Note* startN = note->firstTiedNote() ? note->firstTiedNote() : note;
                if (Segment* startSegment = startN->chord()->segment()) {
                    AccidentalVal defaultAccVal = startSegment->measure()->findAccidental(startSegment, idx, line, error);
                    if (error) {
                        defaultAccVal = Accidental::subtype2value(AccidentalType::NONE); // needed?
                    }
                    forceAccidental = defaultAccVal != accVal;
                }
            }
            if (forceAccidental) {
                AccidentalType at = Accidental::value2subtype(accVal);
                Accidental* a = Factory::createAccidental(note);
                a->setAccidentalType(at);
                a->setRole(noteInfoPtr->freezeAcci ? AccidentalRole::USER : AccidentalRole::AUTO);
                a->setParent(note);
                note->add(a);
            }
            chord->add(note);

            // set up ties
            if (noteInfoPtr->tieStart) {
                // Finale can sometimes be missing a tieEnd setting on a tied-to note,
                // so the existence of a tiedTo candidate is sufficient when the `tieStart` bit is set.
                // The test file v2v2Ties2.musx contains an example of a missing tieEnd bit with the tied E6 across the barline.
                NoteInfoPtr tiedTo = noteInfoPtr.calcTieTo();
                Tie* tie = tiedTo ? Factory::createTie(m_score->dummy()) : Factory::createLaissezVib(m_score->dummy()->note());
                tie->setStartNote(note);
                tie->setTick(note->tick());
                tie->setTrack(note->track());
                tie->setParent(note); //needed?
                note->setTieFor(tie);
            }
            /// @todo Since we may create a tie-start from a note without its corresponding `tieEnd` set (see above), I'm not sure the best way to handle this.
            /// FWIW: This code seems, however, to be producing correct ties as-is, but it may not be correct in every case.
            if (noteInfoPtr->tieEnd) {
                engraving::Note* prevTied = muse::value(m_noteInfoPtr2Note, noteInfoPtr.calcTieFrom(), nullptr);
                Tie* tie = prevTied ? prevTied->tieFor() : nullptr;
                if (tie) {
                    tie->setEndNote(note);
                    tie->setTick2(note->tick());
                    note->setTieBack(tie);
                } else {
                    logger()->logInfo(String(u"Tie does not have starting note. Possibly a partial tie, currently unsupported."));
                }           
            }
            m_noteInfoPtr2Note.emplace(noteInfoPtr, note);
        }
        if (currentEntry->freezeStem || currentEntry->voice2 || entryInfo->v2Launch
            || m_layerForceStems.find(entryInfo.getLayerIndex()) != m_layerForceStems.end()) {
            /// @todo beams: this works for non-beamable notes, but beams appear to have their own up/down status
            DirectionV dir = currentEntry->upStem ? DirectionV::UP : DirectionV::DOWN;
            chord->setStemDirection(dir);
        }
        cr = toChordRest(chord);
    } else {
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
    }

    cr->setDurationType(d);
    cr->setStaffMove(crossStaffMove);
    cr->setParent(segment);
    cr->setTrack(curTrackIdx);
    if (cr->durationTypeTicks() < Fraction(1, 4)) {
        cr->setBeamMode(BeamMode::NONE); // this is changed in the next pass to match the beaming.
    }
    cr->setTicks(cr->actualDurationType().fraction());
    segment->add(cr);
    Tuplet* parentTuplet = bottomTupletFromTick(tupletMap, entryStartTick);
    if (parentTuplet) {
        parentTuplet->add(cr);
    }
    logger()->logInfo(String(u"Adding entry of duration %2 at tick %1").arg(entryStartTick.toString(), cr->durationTypeTicks().toString()));
    return true;
}

bool FinaleParser::processBeams(EntryInfoPtr entryInfoPtr, track_idx_t curTrackIdx, Measure* measure)
{
    if (!entryInfoPtr.calcIsBeamStart()) {
        return true;
    }
    /// @todo detect special cases for beams over barlines created by the Beam Over Barline plugin
    ChordRest* cr = measure->findChordRest(measure->tick() + FinaleTConv::musxFractionToFraction(entryInfoPtr.calcGlobalElapsedDuration()), curTrackIdx);
    if (!cr) { // once grace notes are supported, use IF_ASSERT_FAILED(cr)
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
        std::shared_ptr<const Entry> currentEntry = nextInBeam->getEntry();
        lastCr = measure->findChordRest(measure->tick() + FinaleTConv::musxFractionToFraction(nextInBeam.calcGlobalElapsedDuration()), curTrackIdx);
        IF_ASSERT_FAILED(lastCr) {
            logger()->logWarning(String(u"Entry %1 was not mapped").arg(nextInBeam->getEntry()->getEntryNumber()), m_doc, nextInBeam.getStaff(), nextInBeam.getMeasure());
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
        InstCmper targetMusxStaffId = muse::value(m_staff2Inst, rest->staffIdx(), 0);
        IF_ASSERT_FAILED (targetMusxStaffId) {
            logger()->logWarning(String(u"Entry %1 (a rest) was not mapped to a known musx staff.").arg(entryInfoPtr->getEntry()->getEntryNumber()), m_doc, entryInfoPtr.getStaff(), entryInfoPtr.getMeasure());
            return false;
        }
        Staff* targetStaff = rest->staff();
        if (noteInfoPtr->getNoteId() == musx::dom::Note::RESTID) {
            /// @todo correctly calculate default rest position in multi-voice situation. rest->setAutoPlace is problematic because it also affects spacing
            /// (and does not cover all vertical placement situations either).
            std::shared_ptr<const others::StaffComposite> currMusxStaff = noteInfoPtr.getEntryInfo().createCurrentStaff(targetMusxStaffId);
            IF_ASSERT_FAILED(currMusxStaff) {
                logger()->logWarning(String(u"Target staff %1 not found.").arg(targetMusxStaffId), m_doc, entryInfoPtr.getStaff(), entryInfoPtr.getMeasure());
            }
            auto [pitchClass, octave, alteration, staffPosition] = noteInfoPtr.calcNoteProperties();
            StaffType* staffType = targetStaff->staffType(rest->tick());
            // following code copied from TLayout::layoutRest:
            Rest::LayoutData layoutData;
            const int naturalLine = rest->computeNaturalLine(staffType->lines()); // Measured in 1sp steps
            const int voiceOffset = rest->computeVoiceOffset(staffType->lines(), &layoutData); // Measured in 1sp steps
            const int wholeRestOffset = rest->computeWholeOrBreveRestOffset(voiceOffset, staffType->lines());
            const int finalLine = naturalLine + voiceOffset + wholeRestOffset;
            // convert finalLine to staff position offset for Finale rest. This value is measured in 0.5sp steps.
            const int staffPositionOffset = 2 * finalLine - currMusxStaff->calcToplinePosition();
            const double lineSpacing = staffType->lineDistance().val();
            rest->ryoffset() = double(-staffPosition - staffPositionOffset) * targetStaff->spatium(rest->tick()) * lineSpacing / 2.0;
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
        Chord* c1 = measure->findChord(measure->tick() + tuplet.startTick, curTrackIdx); // timestretch?
        Chord* c2 = measure->findChord(measure->tick() + ((tuplet.startTick + tuplet.endTick) / 2), curTrackIdx);
        IF_ASSERT_FAILED(c1 && c2 && c1->ticks() == c2->ticks()) {
            continue;
        }

        // the current (too short) duration indicates the
        // tremolo type, we calculate it from number of beams
        int tremoloBeamsNum = int(TremoloType::C8) - 1 + c1->durationType().hooks();
        // rather than not import the tremolo, force it to be a valid type
        tremoloBeamsNum = std::clamp(tremoloBeamsNum, int(TremoloType::C8), int(TremoloType::C64));

        // now we have to set the correct duration for the chords to 
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
        IF_ASSERT_FAILED(f.reduced() == (tupletMap[i].endTick - tupletMap[i].startTick).reduced()) {
            logger->logWarning(String(u"Tuplet duration is corrupted"));
            /// @todo account for tuplets with invalid durations, i.e. durations not attainable in MuseScore
        }
        transferTupletProperties(tupletMap[i].musxTuplet, tupletMap[i].scoreTuplet, logger);
        // reparent tuplet if needed
        size_t parentIndex = indexOfParentTuplet(tupletMap, i);
        if (tupletMap[parentIndex].layer >= 0) {
            tupletMap[parentIndex].scoreTuplet->add(tupletMap[i].scoreTuplet);
        }
    }
}

void FinaleParser::importEntries()
{
    // Add entries (notes, rests, tuplets)
    if (m_score->measures()->empty()) {
        logger()->logWarning(String(u"Add entries: No measures in score"));
        return;
    }
    std::vector<std::shared_ptr<others::Measure>> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    std::vector<std::shared_ptr<others::InstrumentUsed>> musxScrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);
    staff_idx_t lastStaffIdxInPart = 0;
    for (const std::shared_ptr<others::InstrumentUsed>& musxScrollViewItem : musxScrollView) {
        staff_idx_t curStaffIdx = muse::value(m_inst2Staff, InstCmper(musxScrollViewItem->staffId), muse::nidx);
        IF_ASSERT_FAILED (curStaffIdx != muse::nidx) {
            logger()->logWarning(String(u"Add entries: Musx inst value not found."), m_doc, musxScrollViewItem->staffId, 1);
            continue;
        }

        Staff* curStaff = m_score->staff(curStaffIdx);
        // reset tie tracking when appropriate
        if (track2staff(curStaff->part()->endTrack()) > lastStaffIdxInPart) {
            lastStaffIdxInPart = track2staff(curStaff->part()->endTrack());
            m_noteInfoPtr2Note.clear();
        }

        for (const std::shared_ptr<others::Measure>& musxMeasure : musxMeasures) {
            Fraction currTick = muse::value(m_meas2Tick, musxMeasure->getCmper(), Fraction(-1, 1));
            Measure* measure = currTick >= Fraction(0, 1)  ? m_score->tick2measure(currTick) : nullptr;
            if (!measure) {
                logger()->logWarning(String(u"Unable to retrieve measure by tick"), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                break;
            }
            details::GFrameHoldContext gfHold(musxMeasure->getDocument(), m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper());
            // Note from RGP: You cannot short-circuit out of this code with a `if (!gfHold) continue` statement.
            // The code after the if statement must still be executed.
            if (gfHold) {
                // gfHold.calcVoices() guarantees that every layer/voice returned contains entries
                std::map<LayerIndex, bool> finaleLayers = gfHold.calcVoices();
                std::unordered_map<int, track_idx_t> finaleVoiceMap = mapFinaleVoices(finaleLayers, musxScrollViewItem->staffId, musxMeasure->getCmper());
                std::unordered_map<Rest*, musx::dom::NoteInfoPtr> fixedRests;
                for (const auto& finaleLayer : finaleLayers) {
                    const LayerIndex layer = finaleLayer.first;
                    /// @todo reparse with forWrittenPitch true, to obtain correct transposed keysigs/clefs/enharmonics
                    std::shared_ptr<const EntryFrame> entryFrame = gfHold.createEntryFrame(layer, /*forWrittenPitch*/ false);
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

                        track_idx_t curTrackIdx = curStaffIdx * VOICES + voiceOff;

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
                            processEntryInfo(entryInfoPtr, curTrackIdx, measure, tupletMap, fixedRests);
                        }

                        // add tremolos
                        processTremolos(tremoloMap, curTrackIdx, measure);

                        // create beams and position non-floating rests
                        for (EntryInfoPtr entryInfoPtr = entryFrame->getFirstInVoice(voice + 1); entryInfoPtr; entryInfoPtr = entryInfoPtr.getNextInVoice(voice + 1)) {
                            processBeams(entryInfoPtr, curTrackIdx, measure);
                        }
                    }
                }
                // position fixed rests after all layers have been imported
                positionFixedRests(fixedRests);
            }
            // Avoid corruptions: fill in any gaps in existing voices...
            logger()->logInfo(String(u"Fixing corruptions for measure at staff %1, tick %2").arg(String::number(curStaffIdx), currTick.toString()));
            measure->checkMeasure(curStaffIdx);
            // ...and make sure voice 1 exists.
            if (!measure->hasVoice(curStaffIdx * VOICES)) {
                Segment* segment = measure->getSegmentR(SegmentType::ChordRest, Fraction(0, 1));
                Rest* rest = Factory::createRest(segment, TDuration(DurationType::V_MEASURE));
                rest->setScore(m_score);
                rest->setTicks(measure->timesig() * curStaff->timeStretch(measure->tick()));
                rest->setTrack(curStaffIdx * VOICES);
                segment->add(rest);
            }
        }
    }
}

}
