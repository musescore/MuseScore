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

#include "engraving/dom/barline.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/utils.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;
using namespace mu::iex::finale;

namespace mu::iex::finale {

Staff* FinaleParser::createStaff(Part* part, const std::shared_ptr<const others::Staff> musxStaff, const InstrumentTemplate* it)
{
    auto clefTypeListFromMusxStaff = [&](const std::shared_ptr<const others::Staff> musxStaff) -> std::optional<ClefTypeList>
    {
        const std::shared_ptr<options::ClefOptions::ClefDef>& concerClefDef = musxOptions().clefOptions->getClefDef(musxStaff->calcFirstClefIndex());
        ClefType concertClef = toMuseScoreClefType(concerClefDef, musxStaff);
        ClefType transposeClef = concertClef;
        if (musxStaff->transposition && musxStaff->transposition->setToClef) {
            const std::shared_ptr<options::ClefOptions::ClefDef>& trasposeClefDef = musxOptions().clefOptions->getClefDef(musxStaff->transposition->setToClef);
            transposeClef = toMuseScoreClefType(trasposeClefDef, musxStaff);
        }
        if (concertClef == ClefType::INVALID || transposeClef == ClefType::INVALID) {
            return std::nullopt;
        }
        return ClefTypeList(concertClef, transposeClef);
    };

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

        measure->setRepeatStart(musxMeasure->forwardRepeatBar);
        measure->setRepeatEnd(musxMeasure->backwardsRepeatBar);
        measure->setBreakMultiMeasureRest(musxMeasure->breakMmRest);
        measure->setIrregular(musxMeasure->noMeasNum);

    }
}

void FinaleParser::importParts()
{
    std::vector<std::shared_ptr<others::InstrumentUsed>> scrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);

    std::unordered_map<InstCmper, QString> inst2Part;
    int partNumber = 0;
    for (const std::shared_ptr<others::InstrumentUsed>& item : scrollView) {
        std::shared_ptr<others::Staff> staff = item->getStaffInstance();
        IF_ASSERT_FAILED(staff) {
            continue; // safety check
        }
        auto compositeStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, staff->getCmper(), 1, 0);
        IF_ASSERT_FAILED(compositeStaff) {
            continue; // safety check
        }

        const auto instIt = m_doc->getInstruments().find(staff->getCmper());
        if (instIt == m_doc->getInstruments().end()) {
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

        const auto& [topStaffId, instInfo] = *instIt;
        for (const InstCmper inst : instInfo.getSequentialStaves()) {
            if (auto instStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, inst, 1, 0)) {
                createStaff(part, instStaff, it);
                inst2Part.emplace(inst, partId);
            }
        }

        // names of part
        auto nameFromEnigmaText = [&](const musx::util::EnigmaParsingContext& parsingContext, const String& sidNamePrefix) {
            EnigmaParsingOptions options;
            options.initialFont = FontTracker(m_score->style(), sidNamePrefix);
            // Finale staff/group names do not scale with individual staff scaling whereas MS instrument names do
            // Compensate here.
            options.scaleFontSizeBy = 1.0;
            // userMag is not set yet, so use musx data
            const std::vector<std::shared_ptr<others::InstrumentUsed>> systemOneStaves = m_doc->getOthers()->getArray<others::InstrumentUsed>(m_currentMusxPartId, 1);
            if (std::optional<size_t> index = others::InstrumentUsed::getIndexForStaff(systemOneStaves, staff->getCmper())) {
                const musx::util::Fraction staffMag = systemOneStaves[index.value()]->calcEffectiveScaling() / musxOptions().combinedDefaultStaffScaling;
                options.scaleFontSizeBy /= staffMag.toDouble();
            }
            return stringFromEnigmaText(parsingContext, options);
        };
        part->setPartName(nameFromEnigmaText(staff->getFullInstrumentNameCtx(m_currentMusxPartId), u"longInstrument"));
        part->setLongName(nameFromEnigmaText(compositeStaff->getFullInstrumentNameCtx(m_currentMusxPartId), u"longInstrument"));
        part->setShortName(nameFromEnigmaText(compositeStaff->getAbbreviatedInstrumentNameCtx(m_currentMusxPartId), u"shortInstrument"));

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
        auto musxStartStaff = others::InstrumentUsed::getStaffInstanceAtIndex(scrollView, Cmper(groupInfo.info.startSlot.value()));
        auto musxEndStaff = others::InstrumentUsed::getStaffInstanceAtIndex(scrollView, Cmper(groupInfo.info.endSlot.value()));
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

ClefType FinaleParser::toMuseScoreClefType(const std::shared_ptr<const musx::dom::options::ClefOptions::ClefDef>& clefDef,
                                           const std::shared_ptr<const musx::dom::others::Staff>& musxStaff)
{
    // Musx staff positions start with the reference line as 0. (The reference line on a standard 5-line staff is the top line.)
    // Positive values are above the reference line. Negative values are below the reference line.

    using MusxClefType = music_theory::ClefType;
    auto [clefType, octaveShift] = clefDef->calcInfo(musxStaff);

    // key:     musx middle-C staff position (35 minus MuseScore-pitchOffset of the clef type)
    // value:   MuseScore clef type

    static const std::unordered_map<int, ClefType> gClefTypes = {
        { -10,  ClefType::G },
        {  +4,  ClefType::G15_MB },
        {  -3,  ClefType::G8_VB },
        { -17,  ClefType::G8_VA },
        { -24,  ClefType::G15_MA },
        { -12,  ClefType::G_1 },
    };

    static const std::unordered_map<int, ClefType> cClefTypes = {
        {   0,  ClefType::C5 },
        {  -2,  ClefType::C4 },
        {  -4,  ClefType::C3 },
        {  -6,  ClefType::C2 },
        {  -8,  ClefType::C1 },
        { -10,  ClefType::C_19C },
        {  +5,  ClefType::C4_8VB },
    };

    static const std::unordered_map<int, ClefType> fClefTypes = {
        {  +2,  ClefType::F },
        { +16,  ClefType::F15_MB },
        {  +9,  ClefType::F8_VB },
        {  -5,  ClefType::F_8VA },
        { -12,  ClefType::F_15MA },
        {  +4,  ClefType::F_C },
        {   0,  ClefType::F_B },
    };

    /// @todo Use clefDef->clefChar to differentiate clef types that only differ by glyph.
    /// To handle non-SMuFL glyphs we'll need glyph mappings, which are planned.

    switch (clefType) {
        case MusxClefType::G: return muse::value(gClefTypes, clefDef->middleCPos, ClefType::INVALID);
        case MusxClefType::C: return muse::value(cClefTypes, clefDef->middleCPos, ClefType::INVALID);
        case MusxClefType::F: return muse::value(fClefTypes, clefDef->middleCPos, ClefType::INVALID);
        case MusxClefType::Percussion1: return ClefType::PERC;
        case MusxClefType::Percussion2: return ClefType::PERC2;
        case MusxClefType::Tab: return musxStaff->calcNumberOfStafflines() <= 4 ? ClefType::TAB4 : ClefType::TAB;
        case MusxClefType::TabSerif: return musxStaff->calcNumberOfStafflines() <= 4 ? ClefType::TAB4_SERIF : ClefType::TAB_SERIF;
        default: break;
    }
    return ClefType::INVALID;
}

}
