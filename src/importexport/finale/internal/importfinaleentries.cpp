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
#include "internal/importfinalescoremap.h"
#include "internal/importfinalelogger.h"
#include "internal/finaletypesconv.h"
#include "dom/sig.h"

#include <vector>
#include <exception>

#include "musx/musx.h"

#include "types/string.h"

#include "engraving/dom/accidental.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/box.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordlist.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/laissezvib.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/utils.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

void EnigmaXmlImporter::mapLayers()
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

std::unordered_map<int, voice_idx_t> EnigmaXmlImporter::mapFinaleVoices(const std::map<LayerIndex, bool>& finaleVoiceMap,
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
            } else {
                logger()->logWarning(String(u"Layer %1 was already mapped to a voice").arg(int(layerIndex) + 1), m_doc, curStaff, curMeas);
            }
        } else {
            logger()->logWarning(String(u"Layer %1 was not mapped to a voice").arg(int(layerIndex) + 1), m_doc, curStaff, curMeas);
        }
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

    // separate bracket/number offset not supported, just add it to the whole tuplet for now
    /// @todo needs to be negated?
    scoreTuplet->setOffset(PointF((musxTuplet->tupOffX + musxTuplet->brackOffX) / EVPU_PER_SPACE,
                                  (musxTuplet->tupOffY + musxTuplet->brackOffY) / EVPU_PER_SPACE));
    scoreTuplet->setVisible(!musxTuplet->hidden);
    if (musxTuplet->autoBracketStyle != options::TupletOptions::AutoBracketStyle::Always) {
        // Can't be determined until we write all the notes/beams
        /// @todo write this setting on a second pass, along with musxTuplet->posStyle
        logger->logWarning(String(u"Unsupported"));
    }
    if (musxTuplet->avoidStaff) {
        // supported globally as a style: Sid::tupletOutOfStaff
        logger->logWarning(String(u"Unsupported"));
    }
    if (musxTuplet->metricCenter) {
        // center number using duration
        /// @todo will be supported globally as a style
        logger->logWarning(String(u"Unsupported"));
    }
    if (musxTuplet->fullDura) {
        // extend bracket to full duration
        /// @todo will be supported globally as a style
        logger->logWarning(String(u"Unsupported"));
    }
    // unsupported: breakBracket, ignoreHorzNumOffset, allowHorz, useBottomNote, smartTuplet, leftHookLen / rightHookLen (style for both)

    // bracket extensions
    /// @todo account for the fact that Finale always includes head widths in total bracket width, an option not yet in musescore. See PR and the related issues
    scoreTuplet->setUserPoint1(PointF(-musxTuplet->leftHookExt / EVPU_PER_SPACE, 0.0));
    scoreTuplet->setUserPoint2(PointF(musxTuplet->rightHookExt, -musxTuplet->manualSlopeAdj) / EVPU_PER_SPACE);
    if (musxTuplet->alwaysFlat) {
        scoreTuplet->setUserPoint2(PointF(scoreTuplet->userP2().x(), scoreTuplet->userP1().y()));
    }
}

static size_t indexOfParentTuplet(std::vector<ReadableTuplet> tupletMap, size_t index) {
    size_t i = index;
    while (i >= 1) {
        --i;
        if (tupletMap[i].layer + 1 == tupletMap[index].layer) {
            return i;
        }
    }
    return i;
}

static Tuplet* bottomTupletFromTick(std::vector<ReadableTuplet> tupletMap, Fraction pos)
{
    for (size_t i = 0; i < tupletMap.size(); ++i) {
        // first, find the lowest tuplet
        while (i + 1 < tupletMap.size() && tupletMap[i+1].absBegin == tupletMap[i].absBegin && tupletMap[i+1].layer > tupletMap[i].layer) {
            ++i;
        }
        // return it if the pos is contained within it
        if (pos >= tupletMap[i].absBegin && pos < tupletMap[i].absEnd) {
            return tupletMap[i].scoreTuplet;
        }
        // next, iterate backwards through all the parent tuplets (which could be larger) and check if its contained there
        size_t j = i;
        while (j >= 1) {
            j = indexOfParentTuplet(tupletMap, j);
            if (pos > tupletMap[j].absBegin && pos < tupletMap[j].absEnd) {
                return tupletMap[j].scoreTuplet;
            }
        }
        // continue after the lowest tuplet (ones preceding it don't contain pos, no need to check them again)
    }
    return nullptr;
}

bool EnigmaXmlImporter::processEntryInfo(EntryInfoPtr entryInfo, track_idx_t curTrackIdx, Measure* measure,
                                         std::vector<ReadableTuplet>& tupletMap, std::unordered_map<size_t, ChordRest*>& entryMap)
{
    if (entryInfo->getEntry()->graceNote) {
        logger()->logWarning(String(u"Grace notes not yet supported"), m_doc, entryInfo.getStaff(), entryInfo.getMeasure());
        return true;
    }

    Fraction entryStartTick = FinaleTConv::musxFractionToFraction(entryInfo->elapsedDuration).reduced();
    Segment* segment = measure->getSegment(SegmentType::ChordRest, entryStartTick);

    // Retrieve entry from entryInfo
    std::shared_ptr<const Entry> currentEntry = entryInfo->getEntry();
    if (!currentEntry) {
        logger()->logWarning(String(u"Failed to get entry"));
        return false;
    }

    // durationType
    std::pair<musx::dom::NoteType, int> noteInfo = currentEntry->calcNoteInfo();
    TDuration d = FinaleTConv::noteTypeToDurationType(noteInfo.first);
    if (d == DurationType::V_INVALID) {
        logger()->logWarning(String(u"Given ChordRest duration not supported in MuseScore"));
        return false;
    }
    d.setDots(static_cast<int>(noteInfo.second));

    ChordRest* cr = nullptr;
    int crossStaffMove = 0;
    staff_idx_t staffIdx = track2staff(curTrackIdx);

    // because we need the real staff to calculate when to show accidentals,
    // we have to calculate cross-staffing before pitches
    for (size_t i = 0; i < currentEntry->notes.size(); ++i) {
        NoteInfoPtr noteInfoPtr = NoteInfoPtr(entryInfo, i);
        if (noteInfoPtr->crossStaff) {
            staff_idx_t crossStaffIdx = muse::value(m_inst2Staff, noteInfoPtr.calcStaff(), muse::nidx);
            IF_ASSERT_FAILED(crossStaffIdx != muse::nidx) {
                logger()->logWarning(String(u"Collect cross staffing: Musx inst value not found for staff cmper %1").arg(String::fromStdString(std::to_string(noteInfoPtr.calcStaff()))));
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

    if (currentEntry->isNote) {
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
                bool hasEnd = noteInfoPtr.calcTieTo()->tieEnd;
                Tie* tie = hasEnd ? Factory::createTie(m_score->dummy()) : Factory::createLaissezVib(m_score->dummy()->note());
                tie->setStartNote(note);
                tie->setTick(note->tick());
                tie->setTrack(note->track());
                tie->setParent(note); //needed?
                note->setTieFor(tie);
            }
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
            /// @todo clear map after each part (ties can be between staves (cross-staff) but not parts)
        }
        if (currentEntry->freezeStem || currentEntry->voice2 || entryInfo->v2Launch
            || m_layerForceStems.find(entryInfo.getLayerIndex()) != m_layerForceStems.end()) {
            /// @todo beams: this works for non-beamable notes, but beams appear to have their own up/down status
            DirectionV d = currentEntry->upStem ? DirectionV::UP : DirectionV::DOWN;
            chord->setStemDirection(d);
        }
        cr = toChordRest(chord);
    } else {
        Rest* rest = Factory::createRest(segment, d);

        for (size_t i = 0; i < currentEntry->notes.size(); ++i) {
            // NoteInfoPtr noteInfoPtr = NoteInfoPtr(entryInfo, i);
            /// @todo calculate y-offset here (remember is also staff-dependent)
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
        cr->setTuplet(parentTuplet);
    }
    entryMap.emplace(entryInfo.getIndexInFrame(), cr);
    return true;
}

static std::vector<ReadableTuplet> createTupletMap(std::vector<EntryFrame::TupletInfo> tupletInfo, int voice)
{
    const size_t n = tupletInfo.size();
    std::vector<ReadableTuplet> result;
    result.reserve(n);

    const bool forVoice2 = bool(voice);
    for (const auto& tuplet : tupletInfo) {
        if (forVoice2 != tuplet.voice2) {
            continue;
        }
        ReadableTuplet rTuplet;
        rTuplet.absBegin    = FinaleTConv::musxFractionToFraction(tuplet.startDura).reduced();
        rTuplet.absDuration = FinaleTConv::musxFractionToFraction(tuplet.endDura - tuplet.startDura).reduced();
        rTuplet.absEnd      = FinaleTConv::musxFractionToFraction(tuplet.endDura).reduced();
        rTuplet.musxTuplet = tuplet.tuplet;
        rTuplet.layer = 0;
        result.emplace_back(rTuplet);
    }

   for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if(i == j) {
                continue;
            }

            if (result[i].absBegin >= result[j].absBegin && result[i].absEnd <= result[j].absEnd
                && (result[i].absBegin > result[j].absBegin || result[i].absEnd < result[j].absEnd)) {
                result[i].layer = std::max(result[i].layer, result[j].layer + 1);
            }
        }
    }

    std::sort(result.begin(), result.end(), [](const ReadableTuplet& a, const ReadableTuplet& b) {
        if (a.absBegin.reduced() != b.absBegin.reduced()) {
            return a.absBegin < b.absBegin;
        }
        return a.layer < b.layer;
    });

    return result;
}

static void createTupletsFromMap(Measure* measure, track_idx_t curTrackIdx, std::vector<ReadableTuplet>& tupletMap, FinaleLoggerPtr& logger)
{
    // create Tuplets as needed, starting with the outermost
    for (size_t i = 0; i < tupletMap.size(); ++i) {
        if (tupletMap[i].layer < 0) {
            continue;
        }
        tupletMap[i].scoreTuplet = Factory::createTuplet(measure);
        tupletMap[i].scoreTuplet->setTrack(curTrackIdx);
        tupletMap[i].scoreTuplet->setTick(measure->tick() + tupletMap[i].absBegin);
        tupletMap[i].scoreTuplet->setParent(measure);
        // musxTuplet::calcRatio is the reciprocal of what MuseScore needs
        /// @todo skip case where finale numerator is 0: often used for changing beams
        Fraction tupletRatio = FinaleTConv::musxFractionToFraction(tupletMap[i].musxTuplet->calcRatio().reciprocal());
        tupletMap[i].scoreTuplet->setRatio(tupletRatio);
        std::pair<musx::dom::NoteType, unsigned> musxBaseLen = calcNoteInfoFromEdu(tupletMap[i].musxTuplet->referenceDuration);
        TDuration baseLen = FinaleTConv::noteTypeToDurationType(musxBaseLen.first);
        baseLen.setDots(static_cast<int>(musxBaseLen.second));
        tupletMap[i].scoreTuplet->setBaseLen(baseLen);
        Fraction f = tupletMap[i].scoreTuplet->baseLen().fraction() * tupletMap[i].scoreTuplet->ratio().denominator();
        tupletMap[i].scoreTuplet->setTicks(f.reduced());
        logger->logInfo(String(u"Detected Tuplet: Starting at %1, duration: %2, ratio: %3").arg(
                        tupletMap[i].absBegin.toString(), f.reduced().toString(), tupletRatio.toString()));
        size_t parentIndex = indexOfParentTuplet(tupletMap, i);
        if (tupletMap[parentIndex].layer >= 0) {
            // finale value doesn't include parent tuplet ratio, but is global. Our setup should be correct though, so hack the assert
            f /= tupletMap[parentIndex].scoreTuplet->ratio();
        }
        IF_ASSERT_FAILED(f.reduced() == tupletMap[i].absDuration.reduced()) {
            logger->logWarning(String(u"Tuplet duration is corrupted"));
            /// @todo account for tuplets with invalid durations, i.e. durations not attainable in MuseScore
        }
        transferTupletProperties(tupletMap[i].musxTuplet, tupletMap[i].scoreTuplet, logger);
        // reparent tuplet if needed
        if (tupletMap[parentIndex].layer >= 0) {
            tupletMap[parentIndex].scoreTuplet->add(tupletMap[i].scoreTuplet);
        }
    }
}

void EnigmaXmlImporter::importEntries()
{
    // Add entries (notes, rests, tuplets)
    std::vector<std::shared_ptr<others::Measure>> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    std::vector<std::shared_ptr<others::InstrumentUsed>> musxScrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);
    for (const std::shared_ptr<others::InstrumentUsed>& musxScrollViewItem : musxScrollView) {
        staff_idx_t curStaffIdx = muse::value(m_inst2Staff, InstCmper(musxScrollViewItem->staffId), muse::nidx);
        IF_ASSERT_FAILED (curStaffIdx != muse::nidx) {
            logger()->logWarning(String(u"Add entries: Musx inst value not found."), m_doc, musxScrollViewItem->staffId, 1);
            continue;
        } else {
            logger()->logInfo(String(u"Add entries: Successfully read staff_idx_t %1").arg(int(curStaffIdx)), m_doc, musxScrollViewItem->staffId, 1);
        }
        if (!m_score->firstMeasure()) {
            logger()->logWarning(String(u"Add entries: Score has no first measure."), m_doc, musxScrollViewItem->staffId, 1);
            continue;
        }
        for (const std::shared_ptr<others::Measure>& musxMeasure : musxMeasures) {
            Fraction currTick = muse::value(m_meas2Tick, musxMeasure->getCmper(), Fraction(-1, 1));
            Measure* measure = currTick >= Fraction(0, 1)  ? m_score->tick2measure(currTick) : nullptr;
            if (!measure) {
                logger()->logWarning(String(u"Unable to retrieve measure by tick"), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                break;
            }
            details::GFrameHoldContext gfHold(musxMeasure->getDocument(), m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper());
            if (gfHold) {
                // gfHold.calcVoices() guarantees that every layer/voice returned contains entries
                std::map<LayerIndex, bool> finaleLayers = gfHold.calcVoices();
                std::unordered_map<int, track_idx_t> finaleVoiceMap = mapFinaleVoices(finaleLayers, musxScrollViewItem->staffId, musxMeasure->getCmper());
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

                        // generate tuplet map and create tuplets
                        std::vector<ReadableTuplet> tupletMap = createTupletMap(entryFrame->tupletInfo, voice);
                        // trick: insert invalid 'tuplet' spanning the whole measure. useful for fallback when filling with rests
                        /// @todo does this account for local timesigs
                        ReadableTuplet rTuplet;
                        Fraction mDur = FinaleTConv::simpleMusxTimeSigToFraction(musxMeasure->createTimeSignature()->calcSimplified(), logger());
                        rTuplet.absBegin = Fraction(0, 1);
                        rTuplet.absDuration = mDur;
                        rTuplet.absEnd = mDur;
                        rTuplet.layer = -1,
                        tupletMap.insert(tupletMap.begin(), rTuplet);
                        createTupletsFromMap(measure, curTrackIdx, tupletMap, logger());

                        // add chords and rests
                        std::unordered_map<size_t, ChordRest*> entryMap;
                        for (EntryInfoPtr entryInfoPtr = entryFrame->getFirstInVoice(voice + 1); entryInfoPtr; entryInfoPtr = entryInfoPtr.getNextInVoice(voice + 1)) {
                            processEntryInfo(entryInfoPtr, curTrackIdx, measure, tupletMap, entryMap);
                        }

                        // create beams
                        for (EntryInfoPtr entryInfoPtr = entryFrame->getFirstInVoice(voice + 1); entryInfoPtr; entryInfoPtr = entryInfoPtr.getNextInVoice(voice + 1)) {
                            if (entryInfoPtr.calcIsBeamStart()) {
                                /// @todo detect special cases for beams over barlines created by the Beam Over Barline plugin
                                ChordRest* cr = muse::value(entryMap, entryInfoPtr.getIndexInFrame(), nullptr);
                                if (!cr) { // once grace notes are supported, use IF_ASSERT_FAILED(cr)
                                    logger()->logWarning(String(u"Entry %1 was not mapped").arg(entryInfoPtr->getEntry()->getEntryNumber()), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                                    continue;
                                }
                                Beam* beam = Factory::createBeam(m_score->dummy()->system());
                                beam->setTrack(curTrackIdx);
                                if (entryInfoPtr->getEntry()->isNote && cr->isChord()) {
                                    beam->setDirection(toChord(cr)->stemDirection());
                                } else {
                                    beam->setDirection(DirectionV::AUTO);
                                }
                                beam->add(cr);
                                cr->setBeam(beam);
                                cr->setBeamMode(BeamMode::BEGIN);
                                ChordRest* lastCr = nullptr;
                                for (auto nextInBeam = entryInfoPtr.getNextInBeamGroup(); nextInBeam; nextInBeam = nextInBeam.getNextInBeamGroup()) {
                                    std::shared_ptr<const Entry> currentEntry = nextInBeam->getEntry();
                                    lastCr = muse::value(entryMap, nextInBeam.getIndexInFrame(), nullptr);
                                    IF_ASSERT_FAILED(lastCr) {
                                        logger()->logWarning(String(u"Entry %1 was not mapped").arg(nextInBeam->getEntry()->getEntryNumber()), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                                        continue;
                                    }
                                    /// @todo fully test secondary beam breaks: can a smaller beam than 32nd be broken?
                                    /// XM: No, not currently in MuseScore.
                                    unsigned secBeamStart = 0;
                                    if (currentEntry->secBeam) {
                                        if (auto secBeamBreak = m_doc->getDetails()->get<details::SecondaryBeamBreak>(gfHold.getRequestedPartId(), currentEntry->getEntryNumber())) {
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
                                    lastCr->setBeam(beam);
                                }
                                if (lastCr) {
                                    lastCr->setBeamMode(BeamMode::END);
                                }
                            }
                        }
                    }
                }
            }
            // Avoid corruptions: fill in any gaps in existing voices...
            measure->checkMeasure(curStaffIdx);
            // ...and make sure voice 1 exists.
            if (!measure->hasVoice(curStaffIdx * VOICES)) {
                Staff* staff = m_score->staff(curStaffIdx);
                Segment* segment = measure->getSegment(SegmentType::ChordRest, Fraction(0, 1));
                Rest* rest = Factory::createRest(segment, TDuration(DurationType::V_MEASURE));
                rest->setScore(m_score);
                rest->setTicks(measure->timesig() / staff->timeStretch(measure->tick()));
                rest->setTrack(curStaffIdx * VOICES);
                segment->add(rest);
            }
        }
    }
}

}
