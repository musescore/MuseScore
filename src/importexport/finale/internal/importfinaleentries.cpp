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
    std::unordered_map<track_idx_t, LayerIndex> reverseMap;

    auto layerAttrs = m_doc->getOthers()->getArray<others::LayerAttributes>(m_currentMusxPartId);

    const auto mapLayer = [&](const std::shared_ptr<others::LayerAttributes>& layerAttr) {
        std::array<track_idx_t, 4> tryOrder;
        if (!layerAttr->freezeLayer) {
            tryOrder = { 0, 1, 2, 3 }; // default
        } else if (layerAttr->freezeStemsUp) {
            tryOrder = { 0, 2, 1, 3 }; // prefer upstem voices
        } else {
            tryOrder = { 1, 3, 0, 2 }; // prefer downstem voices
        }
        const LayerIndex layerIndex = layerAttr->getCmper();
        for (track_idx_t idx : tryOrder) {
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

std::unordered_map<int, track_idx_t> EnigmaXmlImporter::mapFinaleVoices(const std::map<LayerIndex, bool>& finaleVoiceMap,
                                                                        musx::dom::InstCmper curStaff, musx::dom::MeasCmper curMeas) const
{
    using FinaleVoiceID = int;
    std::unordered_map<FinaleVoiceID, track_idx_t> result;
    std::unordered_map<track_idx_t, FinaleVoiceID> reverseMap;
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
            for (track_idx_t v : {0, 1, 2, 3}) {
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

ChordRest* EnigmaXmlImporter::importEntry(EntryInfoPtr entryInfo, Segment* segment, track_idx_t curTrackIdx)
{
    // Retrieve entry from entryInfo
    std::shared_ptr<const Entry> currentEntry = entryInfo->getEntry();
    if (!currentEntry) {
        return nullptr;
    }

    // durationType
    std::pair<musx::dom::NoteType, int> noteInfo = currentEntry->calcNoteInfo();
    TDuration d = FinaleTConv::noteTypeToDurationType(noteInfo.first);
    if (d == DurationType::V_INVALID) {
        logger()->logWarning(String(u"Given ChordRest duration not supported in MuseScore"));
        return nullptr;
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
    }
    if (!targetStaff) {
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
    return cr;
}

static void transferTupletProperties(std::shared_ptr<const details::TupletDef> musxTuplet, Tuplet* scoreTuplet, FinaleLoggerPtr& logger)
{
    scoreTuplet->setNumberType(FinaleTConv::toMuseScoreTupletNumberType(musxTuplet->numStyle));
    // separate bracket/number offset not supported, just add it to the whole tuplet for now
    /// @todo needs to be negated?
    scoreTuplet->setOffset(PointF((musxTuplet->tupOffX + musxTuplet->brackOffX) / EVPU_PER_SPACE,
                                  (musxTuplet->tupOffY + musxTuplet->brackOffY) / EVPU_PER_SPACE));
    scoreTuplet->setVisible(!musxTuplet->hidden);
    if (musxTuplet->autoBracketStyle != options::TupletOptions::AutoBracketStyle::Always) {
        // Can't be determined until we write all the notes/beams
        /// @todo write this setting on a second pass
        logger->logWarning(String(u"Unsupported"));
    }
    if (musxTuplet->avoidStaff) {
        // supported globally as a style: Sid::tupletOutOfStaff
        logger->logWarning(String(u"Unsupported"));
    }
    if (musxTuplet->metricCenter) {
        // center number using duration
        /// @todo wait until added to MuseScore (soon)
        logger->logWarning(String(u"Unsupported"));
    }
    if (musxTuplet->fullDura) {
        // extend bracket to full duration
        /// @todo wait until added to MuseScore (soon)
        logger->logWarning(String(u"Unsupported"));
    }
    // at the end
    if (musxTuplet->alwaysFlat) {
        scoreTuplet->setUserPoint2(PointF(scoreTuplet->userP2().x(), scoreTuplet->userP1().y()));
    }

    /*} else if (tag == "Number") {
        number = Factory::createText(t, TextStyleType::TUPLET);
        number->setComposition(true);
        number->setParent(t);
        Tuplet::resetNumberProperty(number);
        TRead::read(number, e, ctx);
        number->setVisible(t->visible());         //?? override saved property
        number->setColor(t->color());
        number->setTrack(t->track());
        // font settings
    }
    t->setNumber(number);*/
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

static std::vector<std::tuple<Fraction, Fraction, Tuplet*>> bottomTupletsFromTupletMap(std::vector<ReadableTuplet> tupletMap)
{
    std::vector<std::tuple<Fraction, Fraction, Tuplet*>> result;
    Fraction curTick{0, 1};
    for (size_t i = 0; i < tupletMap.size(); ++i) {
        // first, return the lowest tuplet
        while (i + 1 < tupletMap.size() && tupletMap[i+1].absBegin == tupletMap[i].absBegin && tupletMap[i+1].layer > tupletMap[i].layer) {
            ++i;
        }
        curTick = tupletMap[i].absBegin;
        result.emplace_back(std::make_tuple(curTick, tupletMap[i].absEnd, tupletMap[i].scoreTuplet));
        curTick = tupletMap[i].absEnd;
        size_t j = i;
        while (i >= 1) {
            i = indexOfParentTuplet(tupletMap, i);
            if (tupletMap[i].absEnd > curTick && tupletMap[i].absBegin < curTick) {
                result.emplace_back(std::make_tuple(curTick, tupletMap[i].absEnd, tupletMap[i].scoreTuplet));
                break;
            }
        }
        i = j;
    }
    return result;
}

void EnigmaXmlImporter::fillWithInvisibleRests(Fraction startTick, track_idx_t curTrackIdx, Fraction lengthToFill, std::vector<ReadableTuplet> tupletMap)
{
    /// @todo replace with measure::checkMeasure
    Segment* s = m_score->tick2measure(startTick)->getSegment(SegmentType::ChordRest, startTick);
    if (!s) {
        return;
    }
    Fraction rTick = s->rtick();
    Fraction rEnd = rTick + lengthToFill;

    std::vector<std::tuple<Fraction, Fraction, Tuplet*>> lowestTuplets = bottomTupletsFromTupletMap(tupletMap);

    for (size_t i = 0; i < lowestTuplets.size(); ++i) {
        if (std::get<1>(lowestTuplets[i]) < rTick || std::get<0>(lowestTuplets[i]) > rEnd) {
            continue;
        }
        Fraction tStart = std::get<0>(lowestTuplets[i]);
        if (rTick > std::get<0>(lowestTuplets[i])) {
            tStart = rTick;
        }
        Fraction tEnd = std::get<1>(lowestTuplets[i]);
        if (rEnd < std::get<1>(lowestTuplets[i])) {
            tStart = rEnd;
        }
        /// @todo is this the correct duration to fill with? It's absolute, perhaps tuplets need relative durations
        const std::vector<Rest*> rests = m_score->setRests(m_score->tick2measure(startTick)->first(SegmentType::ChordRest)->tick() + tStart, curTrackIdx,
                                                           tEnd - tStart, false, std::get<2>(lowestTuplets[i]));
        for (Rest* r : rests) {
            r->setVisible(false);
        }
    }
}

bool EnigmaXmlImporter::processEntryInfo(EntryInfoPtr entryInfo, track_idx_t curTrackIdx, Segment* segment,
                                         std::vector<ReadableTuplet>& tupletMap, size_t& lastAddedTupletIndex,
                                         std::unordered_map<size_t, ChordRest*>& entryMap)
{
    if (!segment) {
        logger()->logWarning(String(u"Position in measure unknown"), m_doc, entryInfo.getStaff(), entryInfo.getMeasure());
        return false;
    }

    if (entryInfo->getEntry()->graceNote) {
        logger()->logWarning(String(u"Grace notes not yet supported"), m_doc, entryInfo.getStaff(), entryInfo.getMeasure());
        return true;
    }

    Fraction currentEntryInfoStart      = FinaleTConv::musxFractionToFraction(entryInfo->elapsedDuration).reduced();
    Fraction currentEntryActualDuration = FinaleTConv::musxFractionToFraction(entryInfo->actualDuration).reduced();
    Fraction tickEnd = segment->tick();

    if (segment->rtick().reduced() < currentEntryInfoStart) {
        // The entry starts further into the measure than expected (perfectly normal and caused by gaps)
        // Simply fill with invisible rests up to the starting point
        Fraction tickDifference = currentEntryInfoStart - segment->rtick();
        fillWithInvisibleRests(segment->tick(), curTrackIdx, tickDifference, tupletMap);
        tickEnd += tickDifference;
        segment = m_score->tick2measure(tickEnd)->getSegment(SegmentType::ChordRest, tickEnd);
    } else if (segment->rtick().reduced() > currentEntryInfoStart) {
        // edge case: current entry is at the beginning of the measure
        /// @todo this method needs a different location. tuplet map is from the previous measure
        Fraction tickDifference = segment->measure()->ticks() - segment->rtick();
        if (currentEntryInfoStart == Fraction(0, 1)) {
            fillWithInvisibleRests(segment->tick(), curTrackIdx, tickDifference, tupletMap);
            tickEnd += tickDifference;
            segment = m_score->tick2measure(tickEnd)->getSegment(SegmentType::ChordRest, tickEnd);
        } else {
            logger()->logWarning(String(u"Incorrect position in measure"));
            return false;
        }
    }

    // create Tuplets as needed, starting with the outermost
    for (size_t i = 0; i < tupletMap.size(); ++i) {
        if (tupletMap[i].layer < 0) {
            continue;
        }
        if (tupletMap[i].absBegin == currentEntryInfoStart) {
            tupletMap[i].scoreTuplet = Factory::createTuplet(segment->measure());
            tupletMap[i].scoreTuplet->setTrack(curTrackIdx);
            tupletMap[i].scoreTuplet->setTick(segment->tick());
            tupletMap[i].scoreTuplet->setParent(segment->measure());
            // musxTuplet::calcRatio is the reciprocal of what MuseScore needs
            Fraction tupletRatio = FinaleTConv::musxFractionToFraction(tupletMap[i].musxTuplet->calcRatio().reciprocal());
            tupletMap[i].scoreTuplet->setRatio(tupletRatio);
            std::pair<musx::dom::NoteType, unsigned> musxBaseLen = calcNoteInfoFromEdu(tupletMap[i].musxTuplet->referenceDuration);
            TDuration baseLen = FinaleTConv::noteTypeToDurationType(musxBaseLen.first);
            baseLen.setDots(static_cast<int>(musxBaseLen.second));
            tupletMap[i].scoreTuplet->setBaseLen(baseLen);
            Fraction f = tupletMap[i].scoreTuplet->baseLen().fraction() * tupletMap[i].scoreTuplet->ratio().denominator();
            tupletMap[i].scoreTuplet->setTicks(f.reduced());
            IF_ASSERT_FAILED(tupletMap[i].scoreTuplet->ticks() == tupletMap[i].absDuration.reduced()) {
                logger()->logWarning(String(u"Tuplet duration is corrupted"));
                /// @todo account for tuplets with invalid durations, i.e. durations not attainable in MuseScore
            }
            transferTupletProperties(tupletMap[i].musxTuplet, tupletMap[i].scoreTuplet, logger());
            // reparent tuplet if needed
            size_t parentIndex = indexOfParentTuplet(tupletMap, i);
            if (tupletMap[parentIndex].layer >= 0) {
                tupletMap[parentIndex].scoreTuplet->add(tupletMap[i].scoreTuplet);
            }
            lastAddedTupletIndex = i;
        } else if (tupletMap[i].absBegin > currentEntryInfoStart) {
            break;
        }
    }
    // locate current tuplet to parent the ChordRests to (if it exists)
    Tuplet* parentTuplet = nullptr;
    if (tupletMap[lastAddedTupletIndex].scoreTuplet) {
        do {
            if (tupletMap[lastAddedTupletIndex].absEnd > currentEntryInfoStart) {
                break;
            }
        } while (lastAddedTupletIndex > 0 && --lastAddedTupletIndex);
        parentTuplet = tupletMap[lastAddedTupletIndex].scoreTuplet;
    }

    // load entry
    ChordRest* cr = importEntry(entryInfo, segment, curTrackIdx);
    if (cr) {
        cr->setParent(segment);
        cr->setTrack(curTrackIdx);
        if (cr->durationTypeTicks() < Fraction(1, 4)) {
            cr->setBeamMode(BeamMode::NONE); // this is changed in the next pass to match the beaming.
        }
        cr->setTicks(cr->actualDurationType().fraction());
        segment->add(cr);
        if (parentTuplet) {
            parentTuplet->add(cr);
            cr->setTuplet(parentTuplet);
        }
        entryMap.emplace(entryInfo.getIndexInFrame(), cr);
    } else {
        logger()->logWarning(String(u"Failed to read entry contents"));
        // Fill space with invisible rests instead
        fillWithInvisibleRests(segment->tick(), curTrackIdx, currentEntryActualDuration, tupletMap);
        return false;
    }

    tickEnd += cr->actualTicks(); /// @todo this seems like the correct value, but it does not produce correct results with tuplets or local time sigs
    Measure* nm = m_score->tick2measure(tickEnd);
    if (nm) {
        segment = nm->getSegment(SegmentType::ChordRest, tickEnd);
    }
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

static Clef* createClef(Score* score, staff_idx_t staffIdx, ClefIndex musxClef, Measure* measure, Edu musxEduPos, bool afterBarline)
{
    ClefType entryClefType = FinaleTConv::toMuseScoreClefType(musxClef);
    if (entryClefType == ClefType::INVALID) {
        return nullptr;
    }
    Clef* clef = Factory::createClef(score->dummy()->segment());
    clef->setTrack(staffIdx * VOICES);
    clef->setConcertClef(entryClefType);
    clef->setTransposingClef(entryClefType);
    // clef->setShowCourtesy();
    // clef->setForInstrumentChange();
    clef->setGenerated(false);
    const bool isHeader = !afterBarline && !measure->prevMeasure() && musxEduPos == 0;
    clef->setIsHeader(isHeader);
    if (afterBarline) {
        clef->setClefToBarlinePosition(ClefToBarlinePosition::AFTER);
    } else if (musxEduPos == 0) {
        clef->setClefToBarlinePosition(ClefToBarlinePosition::BEFORE);
    }

    Fraction clefTick = measure->tick() + FinaleTConv::eduToFraction(musxEduPos);
    Segment* clefSeg = measure->getSegment(
                       clef->isHeader() ? SegmentType::HeaderClef : SegmentType::Clef, clefTick);
    clefSeg->add(clef);
    return clef;
}

void EnigmaXmlImporter::importStaffItems()
{
    std::vector<std::shared_ptr<others::Measure>> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    std::vector<std::shared_ptr<others::InstrumentUsed>> musxScrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);
    for (const std::shared_ptr<others::InstrumentUsed>& musxScrollViewItem : musxScrollView) {
        std::shared_ptr<TimeSignature> currMusxTimeSig;
        /// @todo handle pickup measures and other measures where display and actual timesigs differ
        for (const std::shared_ptr<others::Measure>& musxMeasure : musxMeasures) {
            Fraction currTick = muse::value(m_meas2Tick, musxMeasure->getCmper(), Fraction(-1, 1));
            Measure * measure = currTick >= Fraction(0, 1)  ? m_score->tick2measure(currTick) : nullptr;
            IF_ASSERT_FAILED(measure) {
                logger()->logWarning(String(u"Unable to retrieve measure by tick"), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                return;
            }
            staff_idx_t staffIdx = muse::value(m_inst2Staff, musxScrollViewItem->staffId, muse::nidx);
            Staff* staff = staffIdx != muse::nidx ? m_score->staff(staffIdx) : nullptr;
            IF_ASSERT_FAILED(staff) {
                logger()->logWarning(String(u"Unable to retrieve staff by idx"), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                return;
            }
            auto currStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper(), 0);
            IF_ASSERT_FAILED(currStaff) {
                logger()->logWarning(String(u"Unable to retrieve composite staff information"), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                return;
            }
            std::shared_ptr<TimeSignature> globalTimeSig = musxMeasure->createTimeSignature();
            std::shared_ptr<TimeSignature> musxTimeSig = musxMeasure->createTimeSignature(musxScrollViewItem->staffId);
            if (!currMusxTimeSig || !currMusxTimeSig->isSame(*musxTimeSig) || musxMeasure->showTime == others::Measure::ShowTimeSigMode::Always) {
                Fraction timeSig = FinaleTConv::simpleMusxTimeSigToFraction(musxTimeSig->calcSimplified(), logger());
                Segment* seg = measure->getSegment(SegmentType::TimeSig, currTick);
                TimeSig* ts = Factory::createTimeSig(seg);
                ts->setSig(timeSig);
                ts->setTrack(staffIdx * VOICES);
                ts->setVisible(musxMeasure->showTime != others::Measure::ShowTimeSigMode::Never);
                Fraction stretch = Fraction(musxTimeSig->calcTotalDuration().calcEduDuration(), globalTimeSig->calcTotalDuration().calcEduDuration()).reduced();
                ts->setStretch(stretch);
                /// @todo other time signature options? Beaming? Composite list?
                seg->add(ts);
                staff->addTimeSig(ts);
            }
            currMusxTimeSig = musxTimeSig;

            /// @todo key signatures (including independent key sigs)
        }
    }
}

void EnigmaXmlImporter::importClefs(details::GFrameHoldContext gfHold,
                                    const std::shared_ptr<others::InstrumentUsed>& musxScrollViewItem,
                                    const std::shared_ptr<others::Measure>& musxMeasure, Measure* measure, staff_idx_t curStaffIdx,
                                    ClefIndex musxCurrClef)
{
    // The Finale UI requires transposition to be a full-measure staff-style assignment, so checking only the beginning of the bar should be sufficient.
    // However, it is possible to defeat this requirement using plugins. That said, doing so produces erratic results, so I'm not sure we should support it.
    // For now, only check the start of the measure.
    bool transposedClefOverride = false;
    auto musxStaffAtMeasureStart = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper(), 0);
    if (musxStaffAtMeasureStart && musxStaffAtMeasureStart->transposition && musxStaffAtMeasureStart->transposition->setToClef) {
        if (musxStaffAtMeasureStart->transposedClef != musxCurrClef) {
            if (createClef(m_score, curStaffIdx, musxStaffAtMeasureStart->transposedClef, measure, /*xEduPos*/ 0, false)) {
                musxCurrClef = musxStaffAtMeasureStart->transposedClef;
            }
        }
        // transposedClefOverride = true;
        return;
    }
    // if (!transposedClefOverride) {
        if (gfHold->clefId.has_value()) {
            if (gfHold->clefId.value() != musxCurrClef) {
                if (createClef(m_score, curStaffIdx, gfHold->clefId.value(), measure, /*xEduPos*/ 0, gfHold->clefAfterBarline)) {
                    musxCurrClef = gfHold->clefId.value();
                }
            }
        } else {
            std::vector<std::shared_ptr<others::ClefList>> midMeasureClefs = m_doc->getOthers()->getArray<others::ClefList>(gfHold.getRequestedPartId(), gfHold->clefListId);
            for (const std::shared_ptr<others::ClefList>& midMeasureClef : midMeasureClefs) {
                if (midMeasureClef->xEduPos > 0 || midMeasureClef->clefIndex != musxCurrClef) {
                    const bool afterBarline = midMeasureClef->xEduPos == 0 && midMeasureClef->afterBarline;
                    if (createClef(m_score, curStaffIdx, midMeasureClef->clefIndex, measure, midMeasureClef->xEduPos, afterBarline)) {
                        /// @todo perhaps populate other fields from midMeasureClef, such as x/y offsets, clef-specific mag, etc.?
                        musxCurrClef = midMeasureClef->clefIndex;
                    }
                }
            }
        }
    // }
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
        ClefIndex musxCurrClef = others::Staff::calcFirstClefIndex(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId);
        for (const std::shared_ptr<others::Measure>& musxMeasure : musxMeasures) {
            Fraction currTick = muse::value(m_meas2Tick, musxMeasure->getCmper(), Fraction(-1, 1));
            Measure* measure = currTick >= Fraction(0, 1)  ? m_score->tick2measure(currTick) : nullptr;
            if (!measure) {
                logger()->logWarning(String(u"Unable to retrieve measure by tick"), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                break;
            }
            Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
            if (!segment) {
                logger()->logWarning(String(u"Unable to initialise start segment"), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                break;
            }

            bool processedEntries = false;
            details::GFrameHoldContext gfHold(musxMeasure->getDocument(), m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper());
            if (gfHold) {
                importClefs(gfHold, musxScrollViewItem, musxMeasure, measure, curStaffIdx, musxCurrClef);

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
                        // gfHold.calcVoices() guarantees that every layer/voice returned contains entries
                        processedEntries = true;

                        /// @todo load (measure-specific) key signature from entryFrame->keySignature RGP: this todo is probably unnecessary.
                        /// Key sigs should be handled at the measure/staff level. They can only change on barlines in Finale. The one in entryFrame
                        /// is provided for convenience and takes into account transposition (when written pitch is requested).

                        track_idx_t trackOffset = muse::value(finaleVoiceMap, FinaleTConv::createFinaleVoiceId(layer, bool(voice)), muse::nidx);
                        IF_ASSERT_FAILED(int(trackOffset) >= 0 && trackOffset < VOICES) {
                            logger()->logWarning(String(u"Encountered incorrectly mapped voice ID for layer %1").arg(int(layer) + 1), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                            continue;
                        }
                        track_idx_t curTrackIdx = curStaffIdx * VOICES + trackOffset;
                        std::vector<ReadableTuplet> tupletMap = createTupletMap(entryFrame->tupletInfo, voice);
                        // trick: insert invalid 'tuplet' spanning the whole measure. useful for fallback when filling with rests
                        ReadableTuplet rTuplet;
                        Fraction mDur = FinaleTConv::simpleMusxTimeSigToFraction(musxMeasure->createTimeSignature()->calcSimplified(), logger());
                        rTuplet.absBegin = Fraction(0, 1);
                        rTuplet.absDuration = mDur;
                        rTuplet.absEnd = mDur;
                        rTuplet.layer = -1,
                            tupletMap.insert(tupletMap.begin(), rTuplet);
                        size_t lastAddedTupletIndex = 0;
                        std::unordered_map<size_t, ChordRest*> entryMap;
                        for (EntryInfoPtr entryInfoPtr = entryFrame->getFirstInVoice(voice + 1); entryInfoPtr; entryInfoPtr = entryInfoPtr.getNextInVoice(voice + 1)) {
                            processEntryInfo(entryInfoPtr, curTrackIdx, segment, tupletMap, lastAddedTupletIndex, entryMap);
                        }
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
            if (!processedEntries) {
                Staff* staff = m_score->staff(curStaffIdx);
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
