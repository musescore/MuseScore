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

#include <vector>
#include <exception>

#include "musx/musx.h"

#include "types/string.h"

#include "engraving/dom/box.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/spacer.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftypechange.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/utils.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;
using namespace mu::iex::finale;

namespace mu::iex::finale {

static std::optional<ClefTypeList> clefTypeListFromMusxStaff(const std::shared_ptr<const others::Staff> musxStaff)
{
    ClefType concertClef = FinaleTConv::toMuseScoreClefType(musxStaff->calcFirstClefIndex());
    ClefType transposeClef = concertClef;
    if (musxStaff->transposition && musxStaff->transposition->setToClef) {
        transposeClef = FinaleTConv::toMuseScoreClefType(musxStaff->transposedClef);
    }
    if (concertClef == ClefType::INVALID || transposeClef == ClefType::INVALID) {
        return std::nullopt;
    }
    return ClefTypeList(concertClef, transposeClef);
}

static std::string trimNewLineFromString(const std::string& src)
{
    size_t pos = src.find('\n');
    if (pos != std::string::npos) {
        return src.substr(0, pos);  // Truncate at the newline, excluding it
    }
    return src;
}

Staff* FinaleParser::createStaff(Part* part, const std::shared_ptr<const others::Staff> musxStaff, const InstrumentTemplate* it)
{
    Staff* s = Factory::createStaff(part);

    StaffType* staffType = s->staffType(Fraction(0, 1));

    // initialise MuseScore's default values
    if (it) {
        s->init(it, 0, 0);
        // don't load bracket from template, we add it later (if it exists)
        s->setBracketType(0, BracketType::NO_BRACKET);
        s->setBracketSpan(0, 0);
        s->setBarLineSpan(0);
    }
    /// @todo Need to intialize the staff type from presets?

    // barline vertical offsets relative to staff
    auto calcBarlineOffsetHalfSpaces = [](Evpu offset) -> int {
        // Finale and MuseScore use opposite signs for up/down
        return int(std::lround(FinaleTConv::doubleFromEvpu(-offset) * 2.0));
    };
    s->setBarLineFrom(calcBarlineOffsetHalfSpaces(musxStaff->topBarlineOffset));
    s->setBarLineTo(calcBarlineOffsetHalfSpaces(musxStaff->botBarlineOffset));

    // hide when empty
    /// @todo inherit
    s->setHideWhenEmpty(Staff::HideMode::INSTRUMENT);

    // calculate whether to use small staff size from first system
    if (const auto& firstSystem = musxStaff->getDocument()->getOthers()->get<others::StaffSystem>(m_currentMusxPartId, 1)) {
        if (firstSystem->hasStaffScaling) {
            if (auto staffSize = musxStaff->getDocument()->getDetails()->get<details::StaffSize>(m_currentMusxPartId, 1, musxStaff->getCmper())) {
                double userMag = FinaleTConv::doubleFromPercent(staffSize->staffPercent);
                if (!muse::RealIsEqual(staffType->userMag(), userMag)) {
                    staffType->setUserMag(userMag);
                }
            }
        }
    }

    // clefs
    if (std::optional<ClefTypeList> defaultClefs = clefTypeListFromMusxStaff(musxStaff)) {
        s->setDefaultClefType(defaultClefs.value());
    } else {
        // Finale has a "blank" clef type that is used for percussion staves. For now we emulate this
        // at the beginning of the piece only.
        /// @todo revisit how to handle blank clef types or changes to other clefs for things such as cues
        staffType->setGenClef(false);
    }
    m_score->appendStaff(s);
    m_inst2Staff.emplace(InstCmper(musxStaff->getCmper()), s->idx());
    m_staff2Inst.emplace(s->idx(), InstCmper(musxStaff->getCmper()));
    return s;
}

void FinaleParser::importMeasures()
{
    // add default time signature
    Fraction currTimeSig = Fraction(4, 4);
    m_score->sigmap()->clear();
    m_score->sigmap()->add(0, currTimeSig);

    // create global time signatures. local timesigs are set up later
    std::vector<std::shared_ptr<others::Measure>> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    for (const std::shared_ptr<others::Measure>& musxMeasure : musxMeasures) {
        MeasureBase* lastMeasure = m_score->measures()->last();
        Fraction tick(lastMeasure ? lastMeasure->endTick() : Fraction(0, 1));

        Measure* measure = Factory::createMeasure(m_score->dummy()->system());
        measure->setTick(tick);
        m_meas2Tick.emplace(musxMeasure->getCmper(), tick);
        m_tick2Meas.emplace(tick, musxMeasure->getCmper());
        std::shared_ptr<TimeSignature> musxTimeSig = musxMeasure->createTimeSignature();
        Fraction scoreTimeSig = FinaleTConv::simpleMusxTimeSigToFraction(musxTimeSig->calcSimplified(), logger());
        if (scoreTimeSig != currTimeSig) {
            m_score->sigmap()->add(tick.ticks(), scoreTimeSig);
            currTimeSig = scoreTimeSig;
        }
        measure->setTimesig(scoreTimeSig);
        measure->setTicks(scoreTimeSig);
        m_score->measures()->append(measure);
        if (!m_score->noStaves()) {
            measure->createStaves(m_score->nstaves() - 1);
        }
    }
}

void FinaleParser::importParts()
{
    std::vector<std::shared_ptr<others::InstrumentUsed>> scrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);

    std::unordered_map<InstCmper, QString> inst2Part;
    int partNumber = 0;
    for (const std::shared_ptr<others::InstrumentUsed>& item : scrollView) {
        std::shared_ptr<others::Staff> staff = item->getStaff();
        IF_ASSERT_FAILED(staff) {
            continue; // safety check
        }
        auto compositeStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, staff->getCmper(), 1, 0);
        IF_ASSERT_FAILED(compositeStaff) {
            continue; // safety check
        }

        auto multiStaffInst = staff->getMultiStaffInstVisualGroup();
        if (multiStaffInst && inst2Part.find(staff->getCmper()) != inst2Part.end()) {
            continue;
        }

        Part* part = new Part(m_score);

        // load default part settings
        /// @todo overwrite most of these settings later
        const InstrumentTemplate* it = searchTemplate(FinaleTConv::instrTemplateIdfromUuid(compositeStaff->instUuid));
        if (it) {
            part->initFromInstrTemplate(it);
        }

        QString partId = String("P%1").arg(++partNumber);
        part->setId(partId);

        // names of part
        std::string fullBaseName = staff->getFullInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        part->setPartName(QString::fromStdString(trimNewLineFromString(fullBaseName)));

        std::string fullEffectiveName = compositeStaff->getFullInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        part->setLongName(QString::fromStdString(trimNewLineFromString(fullEffectiveName)));

        std::string abrvName = compositeStaff->getAbbreviatedInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        part->setShortName(QString::fromStdString(trimNewLineFromString(abrvName)));

        if (multiStaffInst) {
            for (auto inst : multiStaffInst->visualStaffNums) {
                if (auto instStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, inst, 1, 0)) {
                    createStaff(part, instStaff, it);
                    inst2Part.emplace(inst, partId);
                }
            }
        } else {
            createStaff(part, compositeStaff, it);
            inst2Part.emplace(staff->getCmper(), partId);
        }
        m_score->appendPart(part);
    }
}

void FinaleParser::importBrackets()
{
    struct StaffGroupLayer
    {
        details::StaffGroupInfo info;
        int layer{};
    };

    auto computeStaffGroupLayers = [](std::vector<details::StaffGroupInfo> groups) -> std::vector<StaffGroupLayer> {
        const size_t n = groups.size();
        std::vector<StaffGroupLayer> result;
        result.reserve(n);

        for (auto& g : groups)
            result.push_back({ std::move(g), 0 });

        for (size_t i = 0; i < n; ++i) {
            const auto& gi = result[i].info;
            if (!gi.startSlot || !gi.endSlot) {
                continue;
            }

            for (size_t j = 0; j < n; ++j) {
                if (i == j) {
                    continue;
                }
                const auto& gj = result[j].info;
                if (!gj.startSlot || !gj.endSlot) {
                    continue;
                }

                if (*gi.startSlot >= *gj.startSlot && *gi.endSlot <= *gj.endSlot &&
                    (*gi.startSlot > *gj.startSlot || *gi.endSlot < *gj.endSlot)) {
                    result[i].layer = std::max(result[i].layer, result[j].layer + 1);
                }
            }
        }

        std::sort(result.begin(), result.end(), [](const StaffGroupLayer& a, const StaffGroupLayer& b) {
            if (a.layer != b.layer) {
                return a.layer < b.layer;
            }
            if (!a.info.startSlot || !b.info.startSlot) {
                return static_cast<bool>(b.info.startSlot);
            }
            return *a.info.startSlot < *b.info.startSlot;
        });

        return result;
    };

    auto scorePartInfo = m_doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, m_currentMusxPartId);
    if (!scorePartInfo) {
        throw std::logic_error("Unable to read PartDefinition for score");
        return;
    }
    auto scrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);

    auto staffGroups = details::StaffGroupInfo::getGroupsAtMeasure(1, m_currentMusxPartId, scrollView);
    auto groupsByLayer = computeStaffGroupLayers(staffGroups);
    for (const auto& groupInfo : groupsByLayer) {
        IF_ASSERT_FAILED(groupInfo.info.startSlot && groupInfo.info.endSlot) {
            logger()->logWarning(String(u"Group info encountered without start or end slot information"));
            continue;
        }
        auto musxStartStaff = others::InstrumentUsed::getStaffAtIndex(scrollView, Cmper(groupInfo.info.startSlot.value()));
        auto musxEndStaff = others::InstrumentUsed::getStaffAtIndex(scrollView, Cmper(groupInfo.info.endSlot.value()));
        IF_ASSERT_FAILED(musxStartStaff && musxEndStaff) {
            logger()->logWarning(String(u"Group info encountered missing start or end staff information"));
            continue;
        }
        staff_idx_t startStaffIdx = muse::value(m_inst2Staff, InstCmper(musxStartStaff->getCmper()), muse::nidx);
        IF_ASSERT_FAILED(startStaffIdx != muse::nidx) {
            logger()->logWarning(String(u"Create brackets: Musx inst value not found for staff cmper %1").arg(String::fromStdString(std::to_string(musxStartStaff->getCmper()))));
            continue;
        }
        BracketItem* bi = Factory::createBracketItem(m_score->dummy());
        bi->setBracketType(FinaleTConv::toMuseScoreBracketType(groupInfo.info.group->bracket->style));
        int groupSpan = int(groupInfo.info.endSlot.value() - groupInfo.info.startSlot.value() + 1);
        bi->setBracketSpan(groupSpan);
        bi->setColumn(size_t(groupInfo.layer));
        m_score->staff(startStaffIdx)->addBracket(bi);
        if (groupInfo.info.group->drawBarlines == details::StaffGroup::DrawBarlineStyle::ThroughStaves) {
            for (staff_idx_t idx = startStaffIdx; idx < startStaffIdx + groupSpan - 1; idx++) {
                // RGP: Unlike bi->setBracketSpan, staff->setBarLineSpan is 1 staff at-a-time. Although the parameter is int, it is interpreted as bool.
                m_score->staff(idx)->setBarLineSpan(1);
                m_score->staff(idx)->setBarLineTo(0);
            }
        }
    }
}

static Clef* createClef(Score* score, staff_idx_t staffIdx, ClefIndex musxClef, Measure* measure, Edu musxEduPos, bool afterBarline, bool visible)
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
    clef->setVisible(visible);
    clef->setGenerated(false);
    const bool isHeader = !afterBarline && !measure->prevMeasure() && musxEduPos == 0;
    clef->setIsHeader(isHeader);
    if (afterBarline) {
        clef->setClefToBarlinePosition(ClefToBarlinePosition::AFTER);
    } else if (musxEduPos == 0) {
        clef->setClefToBarlinePosition(ClefToBarlinePosition::BEFORE);
    }

    Staff* staff = score->staff(staffIdx);
    Fraction timeStretch = staff->timeStretch(measure->tick());
    // Clef positions in musx are staff-level, so back out any time stretch to get global position.
    Fraction clefTick = FinaleTConv::eduToFraction(musxEduPos) / timeStretch;
    Segment* clefSeg = measure->getSegmentR(clef->isHeader() ? SegmentType::HeaderClef : SegmentType::Clef, clefTick);
    clefSeg->add(clef);
    return clef;
}

void FinaleParser::importClefs(const std::shared_ptr<others::InstrumentUsed>& musxScrollViewItem,
                                    const std::shared_ptr<others::Measure>& musxMeasure, Measure* measure, staff_idx_t curStaffIdx,
                                    ClefIndex& musxCurrClef)
{
    // The Finale UI requires transposition to be a full-measure staff-style assignment, so checking only the beginning of the bar should be sufficient.
    // However, it is possible to defeat this requirement using plugins. That said, doing so produces erratic results, so I'm not sure we should support it.
    // For now, only check the start of the measure.
    auto musxStaffAtMeasureStart = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper(), 0);
    if (musxStaffAtMeasureStart && musxStaffAtMeasureStart->transposition && musxStaffAtMeasureStart->transposition->setToClef) {
        if (musxStaffAtMeasureStart->transposedClef != musxCurrClef) {
            if (createClef(m_score, curStaffIdx, musxStaffAtMeasureStart->transposedClef, measure, /*xEduPos*/ 0, false, true)) {
                musxCurrClef = musxStaffAtMeasureStart->transposedClef;
            }
        }
        return;
    }
    if (auto gfHold = m_doc->getDetails()->get<details::GFrameHold>(m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper())) {
        if (gfHold->clefId.has_value()) {
            if (gfHold->clefId.value() != musxCurrClef || gfHold->showClefMode == ShowClefMode::Always) {
                const bool visible = gfHold->showClefMode != ShowClefMode::Never;
                if (createClef(m_score, curStaffIdx, gfHold->clefId.value(), measure, /*xEduPos*/ 0, gfHold->clefAfterBarline, visible)) {
                    musxCurrClef = gfHold->clefId.value();
                }
            }
        } else {
            std::vector<std::shared_ptr<others::ClefList>> midMeasureClefs = m_doc->getOthers()->getArray<others::ClefList>(m_currentMusxPartId, gfHold->clefListId);
            for (const std::shared_ptr<others::ClefList>& midMeasureClef : midMeasureClefs) {
                if (midMeasureClef->xEduPos > 0 || midMeasureClef->clefIndex != musxCurrClef || midMeasureClef->clefMode == ShowClefMode::Always) {
                    const bool visible = midMeasureClef->clefMode != ShowClefMode::Never;
                    const bool afterBarline = midMeasureClef->xEduPos == 0 && midMeasureClef->afterBarline;
                    /// @todo Test with stretched staff time. (midMeasureClef->xEduPos is in global edu values.)
                    if (Clef* clef = createClef(m_score, curStaffIdx, midMeasureClef->clefIndex, measure, midMeasureClef->xEduPos, afterBarline, visible)) {
                        // only set y offset because MuseScore automatically calculates the horizontal spacing offset
                        clef->setOffset(0.0, -FinaleTConv::doubleFromEvpu(midMeasureClef->yEvpuPos) * clef->spatium());
                        /// @todo perhaps populate other fields from midMeasureClef, such as clef-specific mag, etc.?
                        musxCurrClef = midMeasureClef->clefIndex;
                    }
                }
            }
        }
    }
}

template<typename T>
static bool changed(const T& a, const T& b, bool& result)
{
	result = result || a != b;
	return a != b;
}

bool FinaleParser::applyStaffSyles(StaffType* staffType, const std::shared_ptr<const musx::dom::others::StaffComposite>& currStaff)
{
    bool result = false;
    // only override the Show Clefs setting if it is actually overridden in the staff style. Otherwise,
    // leave the MuseScore setting alone because it may be turned off to simulate a blank clef.
    if (currStaff->masks->negClef && staffType->genClef() == currStaff->hideClefs) {
        staffType->setGenClef(!currStaff->hideClefs);
        result = true;
    }
    if (changed(staffType->genKeysig(), !currStaff->hideKeySigs, result)) {
        staffType->setGenKeysig(!currStaff->hideKeySigs);
    }
    if (changed(staffType->genTimesig(), !currStaff->hideTimeSigs, result)) {
        staffType->setGenTimesig(!currStaff->hideTimeSigs);
    }
    StaffGroup staffGroup = FinaleTConv::staffGroupFromNotationStyle(currStaff->notationStyle);
    if (changed(staffType->group(), staffGroup, result)) {
        staffType->setGroup(staffGroup);
    }
    int numLines = [&]() -> int {
        if (currStaff->staffLines.has_value()) {
            return currStaff->staffLines.value();
        } else if (currStaff->customStaff.has_value()) {
            return static_cast<int>(currStaff->customStaff.value().size());
        }
        return 5;
    }();
    bool staffInvisible = numLines <= 0;
    if (staffInvisible) {
        numLines = 5;
    }
    if (changed(staffType->lines(), numLines, result)) {
        staffType->setLines(numLines);
    }
    if (changed(staffType->invisible(), staffInvisible, result)) {
        staffType->setInvisible(staffInvisible);
    }
    Spatium lineDistance = Spatium(double(currStaff->lineSpace) / EVPU_PER_SPACE);
    if (changed(staffType->lineDistance(), lineDistance, result)) {
        staffType->setLineDistance(lineDistance);
    }
    int stepOffset = currStaff->calcToplinePosition();
    Spatium yoffset = Spatium(-stepOffset /2.0);
    if (changed(staffType->yoffset(), yoffset, result)) {
        staffType->setYoffset(yoffset);
    }
    if (changed(staffType->stepOffset(), stepOffset, result)) {
        staffType->setStepOffset(stepOffset);
    }
    if (changed(staffType->showBarlines(), !currStaff->hideBarlines, result)) {
        staffType->setShowBarlines(!currStaff->hideBarlines);
    }
    if (changed(staffType->showRests(), !currStaff->hideRests, result)) {
        staffType->setShowRests(!currStaff->hideRests);
    }
    if (changed(staffType->stemless(), currStaff->hideStems, result)) {
        staffType->setStemless(currStaff->hideStems);
    }

    /// @todo use userMag instead of smallClef? (But it requires a separate system-by-system search.)
    /// @todo tablature options
    /// @todo others?

    return result;
}

void FinaleParser::importStaffItems()
{
    std::vector<std::shared_ptr<others::Measure>> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    std::vector<std::shared_ptr<others::InstrumentUsed>> musxScrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);
    for (const std::shared_ptr<others::InstrumentUsed>& musxScrollViewItem : musxScrollView) {
        // per staff style calculations
        const std::shared_ptr<others::Staff>& rawStaff = m_doc->getOthers()->get<others::Staff>(m_currentMusxPartId, musxScrollViewItem->staffId);
        IF_ASSERT_FAILED(rawStaff) {
            logger()->logWarning(String(u"Unable to retrieve musx raw staff"), m_doc, musxScrollViewItem->staffId, 1);
            return;
        }
        std::set<MeasCmper> styleChanges; // MuseScore style changes must occur on measure boundaries
        styleChanges.emplace(1); // there is always a style change on the first measure.
        if (rawStaff->hasStyles) {
            std::vector<std::shared_ptr<others::StaffStyleAssign>> musxStyleChanges = m_doc->getOthers()->getArray<others::StaffStyleAssign>(m_currentMusxPartId, rawStaff->getCmper());
            for (const auto& musxStyleChange : musxStyleChanges) {
                styleChanges.emplace(musxStyleChange->startMeas);
                if (std::optional<std::pair<MeasCmper, Edu>> nextLoc = musxStyleChange->nextLocation()) {  // staff style locations are global edus
                    auto [nextMeas, nextEdu] = nextLoc.value();
                    if (nextMeas == musxStyleChange->startMeas && nextMeas < MeasCmper(musxMeasures.size())) {
                        styleChanges.emplace(nextMeas + 1);
                    } else {
                        styleChanges.emplace(nextMeas);
                    }
                }
            }
        }
        for (MeasCmper measNum : styleChanges) {
            Fraction currTick = muse::value(m_meas2Tick, measNum, Fraction(-1, 1));
            Measure* measure = currTick >= Fraction(0, 1)  ? m_score->tick2measure(currTick) : nullptr;
            IF_ASSERT_FAILED(measure) {
                logger()->logWarning(String(u"Unable to retrieve measure by tick"), m_doc, musxScrollViewItem->staffId, measNum);
                return;
            }
            staff_idx_t staffIdx = muse::value(m_inst2Staff, musxScrollViewItem->staffId, muse::nidx);
            Staff* staff = staffIdx != muse::nidx ? m_score->staff(staffIdx) : nullptr;
            IF_ASSERT_FAILED(staff) {
                logger()->logWarning(String(u"Unable to retrieve staff by idx"), m_doc, musxScrollViewItem->staffId, measNum);
                return;
            }
            std::shared_ptr<const others::StaffComposite> currStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId, measNum, 0);
            IF_ASSERT_FAILED(currStaff) {
                logger()->logWarning(String(u"Unable to retrieve musx current staff"), m_doc, musxScrollViewItem->staffId, measNum);
                return;
            }
            Fraction tick = measure->tick();
            StaffType* staffType = staff->staffType(tick);
            IF_ASSERT_FAILED(staffType) {
                logger()->logWarning(String(u"Unable to create MuseScore staff type"), m_doc, musxScrollViewItem->staffId, measNum);
                return;
            }
            bool createdStaffType = false;
            if (tick > Fraction(0, 1)) {
                staffType = new StaffType(*staffType);
                createdStaffType = true;
            }
            if (applyStaffSyles(staffType, currStaff) && tick > Fraction(0, 1)) {
                StaffTypeChange* staffChange = Factory::createStaffTypeChange(measure);
                staffChange->setParent(measure);
                staffChange->setTrack(staffIdx * VOICES);
                staffChange->setStaffType(staffType, true);
                measure->add(staffChange);
            } else if (createdStaffType) {
                delete staffType;
            }
        }
        // per measure calculations
        std::shared_ptr<TimeSignature> currMusxTimeSig;
        std::shared_ptr<KeySignature> currMusxKeySig;
        std::optional<KeySigEvent> currKeySigEvent;
        ClefIndex musxCurrClef = others::Staff::calcFirstClefIndex(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId);
        /// @todo handle pickup measures and other measures where display and actual timesigs differ
        for (const std::shared_ptr<others::Measure>& musxMeasure : musxMeasures) {
            Fraction currTick = muse::value(m_meas2Tick, musxMeasure->getCmper(), Fraction(-1, 1));
            Measure* measure = currTick >= Fraction(0, 1)  ? m_score->tick2measure(currTick) : nullptr;
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

            // timesig
            /// @todo figure out how to deal with display vs actual time signature.
            const std::shared_ptr<TimeSignature> globalTimeSig = musxMeasure->createTimeSignature();
            const std::shared_ptr<TimeSignature> musxTimeSig = musxMeasure->createTimeSignature(musxScrollViewItem->staffId);
            if (!currMusxTimeSig || !currMusxTimeSig->isSame(*musxTimeSig) || musxMeasure->showTime == others::Measure::ShowTimeSigMode::Always) {
                Fraction timeSig = FinaleTConv::simpleMusxTimeSigToFraction(musxTimeSig->calcSimplified(), logger());
                Segment* seg = measure->getSegmentR(SegmentType::TimeSig, Fraction(0, 1));
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

            // clefs
            importClefs(musxScrollViewItem, musxMeasure, measure, staffIdx, musxCurrClef);

            // keysig
            const std::shared_ptr<KeySignature> musxKeySig = musxMeasure->createKeySignature(musxScrollViewItem->staffId);
            if (!currMusxKeySig || !currMusxKeySig->isSame(*musxKeySig) || musxMeasure->showKey == others::Measure::ShowKeySigMode::Always) {
                /// @todo microtonal keysigs
                std::optional<KeySigEvent> keySigEvent;
                if (musxKeySig->calcEDODivisions() == music_theory::STANDARD_12EDO_STEPS) {
                    const bool usesChromaticTransposition = currStaff->transposition && currStaff->transposition->chromatic
                                                            && (currStaff->transposition->chromatic->alteration || currStaff->transposition->chromatic->diatonic);
                    if (usesChromaticTransposition && musxKeySig->getAlteration(KeySignature::KeyContext::Concert) != 0) {
                        logger()->logWarning(String(u"Finale's chromatic transposition with a key signature is not supported. Using Keyless instead.").arg(musxKeySig->getKeyMode()),
                                             m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                    }
                    const bool keyless = usesChromaticTransposition || musxKeySig->keyless || musxKeySig->hideKeySigShowAccis || currStaff->hideKeySigsShowAccis;
                    keySigEvent = KeySigEvent();
                    // Note that isLinear and isNonLinear can in theory both return false, although if it happens it is evidence of musx file corruption.
                    KeySigEvent& ksEvent = keySigEvent.value();
                    if (musxKeySig->isLinear() || keyless) {
                        Key concertKey = keyless ? Key::C : FinaleTConv::keyFromAlteration(musxKeySig->getAlteration(KeySignature::KeyContext::Concert));
                        Key key = keyless ? Key::C : FinaleTConv::keyFromAlteration(musxKeySig->getAlteration(KeySignature::KeyContext::Written));
                        ksEvent.setConcertKey(concertKey);
                        ksEvent.setKey(key);
                        KeyMode km = KeyMode::UNKNOWN;
                        if (keyless) {
                            km = KeyMode::NONE;
                        } else if (musxKeySig->isMajor()) {
                            km = KeyMode::MAJOR;
                        } else if (musxKeySig->isMinor()) {
                            km = KeyMode::MINOR;
                        } else if (std::optional<music_theory::DiatonicMode> mode = musxKeySig->calcDiatonicMode()) {
                            km = FinaleTConv::keyModeFromDiatonicMode(mode.value());
                        }
                        ksEvent.setMode(km);
                    } else if (musxKeySig->isNonLinear()) {
                        std::shared_ptr<others::AcciOrderSharps> musxAccis = m_doc->getOthers()->get<others::AcciOrderSharps>(m_currentMusxPartId, musxKeySig->getKeyMode());
                        std::shared_ptr<others::AcciAmountSharps> musxAmounts = m_doc->getOthers()->get<others::AcciAmountSharps>(m_currentMusxPartId, musxKeySig->getKeyMode());
                        IF_ASSERT_FAILED(musxAccis->values.size() >= musxAmounts->values.size()) {
                            logger()->logWarning(String(u"Nonlinear key %1 has insufficient AcciOrderSharps.").arg(musxKeySig->getKeyMode()), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                            return;
                        }
                        ksEvent.setConcertKey(Key::C);
                        ksEvent.setKey(Key::C);
                        ksEvent.setCustom(true);
                        for (size_t x = 0; x < musxAccis->values.size(); x++) {
                            int amount = musxAmounts->values[x];
                            if (amount == 0) {
                                break;
                            }
                            SymId accidental = FinaleTConv::acciSymbolFromAcciAmount(amount);
                            if (accidental != SymId::noSym) {
                                CustDef cd;
                                cd.sym = accidental;
                                cd.degree = musxAccis->values[x];
                                /// @todo calculate any octave offset. Finale specifies the exact octave for every clef
                                /// while MuseScore specifies an offset from the default octave for the currently active clef.
                                /// Since this is a poor mapping, we are taking the MuseScore default for now.
                                ksEvent.customKeyDefs().push_back(cd);
                            } else {
                                logger()->logWarning(String(u"Skipping unknown accidental amount %1 in nonlinear key %2.").arg(amount, musxKeySig->getKeyMode()), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                            }
                        }
                        if (ksEvent.customKeyDefs().empty()) {
                            ksEvent.setCustom(false);
                            ksEvent.setMode(KeyMode::NONE);
                            logger()->logWarning(String(u"Converting non-linear Finale key %1 that has no accidentals to Keyless.").arg(musxKeySig->getKeyMode()), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                        }
                    } else {
                        logger()->logWarning(String(u"Skipping Finale key %1 that is neither linear nor non-linear.").arg(musxKeySig->getKeyMode()), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                    }
                } else {
                    logger()->logWarning(String(u"Microtonal key signatures not supported."), m_doc, musxScrollViewItem->staffId, musxMeasure->getCmper());
                }
                if (keySigEvent && keySigEvent != currKeySigEvent) {
                    Segment* seg = measure->getSegmentR(SegmentType::KeySig, Fraction(0, 1));
                    KeySig* ks = Factory::createKeySig(seg);
                    ks->setKeySigEvent(keySigEvent.value());
                    ks->setTrack(staffIdx * VOICES);
                    ks->setVisible(musxMeasure->showKey != others::Measure::ShowKeySigMode::Never);
                    seg->add(ks);
                    staff->setKey(currTick, ks->keySigEvent());
                }
                currKeySigEvent = keySigEvent;
            }
            currMusxKeySig = musxKeySig;
        }
    }
}

void FinaleParser::importPageLayout()
{
    // No measures or staves means no valid staff systems
    if (m_score->measures()->empty() || m_score->noStaves()) {
        return;
    }
    std::vector<std::shared_ptr<others::Page>> pages = m_doc->getOthers()->getArray<others::Page>(m_currentMusxPartId);
    std::vector<std::shared_ptr<others::StaffSystem>> staffSystems = m_doc->getOthers()->getArray<others::StaffSystem>(m_currentMusxPartId);
    logger()->logDebugTrace(String(u"Document contains %1 staff systems and %2 pages.").arg(staffSystems.size(), pages.size()));
    size_t currentPageIndex = 0;
    for (size_t i = 0; i < staffSystems.size(); ++i) {
        const std::shared_ptr<others::StaffSystem>& leftStaffSystem = staffSystems[i];
        std::shared_ptr<others::StaffSystem>& rightStaffSystem = staffSystems[i];

        //retrieve leftmost measure of system
        Fraction startTick = muse::value(m_meas2Tick, leftStaffSystem->startMeas, Fraction(-1, 1));
        Measure* startMeasure = startTick >= Fraction(0, 1)  ? m_score->tick2measure(startTick) : nullptr;

        // determine if system is first on the page
        // determine the current page the staffsystem is on
        bool isFirstSystemOnPage = false;
        for (size_t j = currentPageIndex; j < pages.size(); ++j) {
            const std::shared_ptr<others::Page>& page = pages[j];
            if (page->isBlank()) {
                /// @todo handle blank page??
                continue;
            }
            const std::shared_ptr<others::StaffSystem>& firstPageSystem = m_doc->getOthers()->get<others::StaffSystem>(m_currentMusxPartId, page->firstSystem);
            IF_ASSERT_FAILED(firstPageSystem) {
                break;
            }
            Fraction pageStartTick = muse::value(m_meas2Tick, firstPageSystem->startMeas, Fraction(-1, 1));
            if (pageStartTick < startTick) {
                continue;
            }
            if (pageStartTick == startTick) {
                isFirstSystemOnPage = true;
                currentPageIndex = j;
            }
            break;
        }

        // Hack: Detect StaffSystems on presumably the same height, and implement them as one system separated by HBoxes
        // Commonly used in Finale for Coda Systems
        for (size_t j = i + 1; j < staffSystems.size(); ++j) {
            // compare system one in advance to previous system
            if (muse::RealIsEqual(double(staffSystems[j]->top), double(staffSystems[j-1]->top))
                && muse::RealIsEqualOrMore(0.0, double(staffSystems[j]->distanceToPrev + (-staffSystems[j]->top) - staffSystems[j-1]->bottom))) {
                double dist = staffSystems[j]->left
                              - (pages[currentPageIndex]->width- pages[currentPageIndex]->margLeft - (-pages[currentPageIndex]->margRight) - (-staffSystems[j-1]->right));
                // check if horizontal distance between systems is larger than 0 and smaller than content width of the page
                if (muse::RealIsEqualOrMore(dist, 0.0)
                    && muse::RealIsEqualOrMore(m_score->style().styleD(Sid::pagePrintableWidth) * EVPU_PER_INCH, dist)) {
                    Fraction distTick = muse::value(m_meas2Tick, staffSystems[j]->startMeas, Fraction(-1, 1));
                    Measure* distMeasure = distTick >= Fraction(0, 1)  ? m_score->tick2measure(distTick) : nullptr;
                    IF_ASSERT_FAILED(distMeasure) {
                        break;
                    }
                    logger()->logInfo(String(u"Adding space between systems at tick %1").arg(distTick.toString()));
                    HBox* distBox = Factory::createHBox(m_score->dummy()->system());
                    distBox->setTick(distMeasure->tick());
                    distBox->setNext(distMeasure);
                    distBox->setPrev(distMeasure->prev());
                    distBox->setBoxWidth(Spatium(FinaleTConv::doubleFromEvpu(dist)));
                    distBox->setSizeIsSpatiumDependent(false);
                    distMeasure->setPrev(distBox);
                    // distBox->manageExclusionFromParts(/*exclude =*/ true); // excluded by default
                    rightStaffSystem = staffSystems[j];
                    i = j; // skip over coda systems, don't parse twice
                    continue;
                }
            }
            break;
        }

        // now we have moved Coda Systems, compute end of System
        Fraction endTick = muse::value(m_meas2Tick, rightStaffSystem->getLastMeasure(), Fraction(-1, 1));
        Measure* endMeasure = endTick >= Fraction(0, 1)  ? m_score->tick2measure(endTick) : nullptr;
        IF_ASSERT_FAILED(startMeasure && endMeasure) {
            logger()->logWarning(String(u"Unable to retrieve measure(s) by tick for staffsystem"));
            continue;
        }

        // create system left and right margins
        MeasureBase* sysStart = startMeasure;
        if (!muse::RealIsEqual(double(leftStaffSystem->left), 0.0)) {
            // for the very first system, create a non-frame indent instead
            if (isFirstSystemOnPage && currentPageIndex == 0) {
                m_score->style().set(Sid::enableIndentationOnFirstSystem, true);
                m_score->style().set(Sid::firstSystemIndentationValue, FinaleTConv::doubleFromEvpu(leftStaffSystem->left));
            } else {
                HBox* leftBox = Factory::createHBox(m_score->dummy()->system());
                leftBox->setTick(startMeasure->tick());
                leftBox->setNext(startMeasure);
                leftBox->setPrev(startMeasure->prev());
                leftBox->setBoxWidth(Spatium(FinaleTConv::doubleFromEvpu(leftStaffSystem->left)));
                leftBox->setSizeIsSpatiumDependent(false);
                startMeasure->setPrev(leftBox);
                // leftBox->manageExclusionFromParts(/*exclude =*/ true); // excluded by default
                sysStart = leftBox;
            }
        } else {
            logger()->logInfo(String(u"No need to add left margin for system %1").arg(i));
        }
        MeasureBase* sysEnd = endMeasure;
        if (!muse::RealIsEqual(double(-rightStaffSystem->right), 0.0)) {
            HBox* rightBox = Factory::createHBox(m_score->dummy()->system());
            Fraction rightTick = endMeasure->nextMeasure() ? endMeasure->nextMeasure()->tick() : m_score->last()->endTick();
            rightBox->setTick(rightTick);
            rightBox->setNext(endMeasure->next());
            rightBox->setPrev(endMeasure);
            rightBox->setBoxWidth(Spatium(FinaleTConv::doubleFromEvpu(-rightStaffSystem->right)));
            rightBox->setSizeIsSpatiumDependent(false);
            endMeasure->setNext(rightBox);
            // rightBox->manageExclusionFromParts(/*exclude =*/ true); // excluded by default
            sysEnd = rightBox;
        } else {
            logger()->logInfo(String(u"No need to add right margin for system %1").arg(i));
        }
        // lock measures in place
        // we lock all systems to guarantee we end up with the correct measure distribution
        m_score->addSystemLock(new SystemLock(sysStart, sysEnd));

        // add page break if needed
        bool isLastSystemOnPage = false;
        for (const std::shared_ptr<others::Page>& page : pages) {
            if (page->isBlank()) {
                /// @todo handle blank page???
                continue;
            }
            const std::shared_ptr<others::StaffSystem>& firstPageSystem = m_doc->getOthers()->get<others::StaffSystem>(m_currentMusxPartId, page->firstSystem);
            Fraction pageStartTick = muse::value(m_meas2Tick, firstPageSystem->startMeas, Fraction(-1, 1));
            // the last staff system in the score can't be compared to the startTick of the preceding page -
            // account for that here too, but don't add a page break
            if (pageStartTick == endTick + endMeasure->ticks() || i + 1 == staffSystems.size()) {
                isLastSystemOnPage = true;
                if (i + 1 < staffSystems.size()) {
                    LayoutBreak* lb = Factory::createLayoutBreak(sysEnd);
                    lb->setLayoutBreakType(LayoutBreakType::PAGE);
                    sysEnd->add(lb);
                }
                break;
            }
        }

        // create system top and bottom margins
        if (isFirstSystemOnPage) {
            Spacer* upSpacer = Factory::createSpacer(startMeasure);
            upSpacer->setSpacerType(SpacerType::UP);
            upSpacer->setTrack(0);
            upSpacer->setGap(Spatium(FinaleTConv::doubleFromEvpu(-leftStaffSystem->top + leftStaffSystem->distanceToPrev)));
            /// @todo account for title frames / perhaps header frames
            startMeasure->add(upSpacer);
        }
        if (!isLastSystemOnPage) {
            Spacer* downSpacer = Factory::createSpacer(startMeasure);
            downSpacer->setSpacerType(SpacerType::FIXED);
            downSpacer->setTrack((m_score->nstaves() - 1) * VOICES); // invisible staves are correctly accounted for on layout
            downSpacer->setGap(Spatium(FinaleTConv::doubleFromEvpu(rightStaffSystem->bottom + rightStaffSystem->distanceToPrev + (-staffSystems[i+1]->top))));
            startMeasure->add(downSpacer);
        }
    }
}

}
