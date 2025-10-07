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
#include "internal/text/finaletextconv.h"

#include <vector>
#include <exception>

#include "musx/musx.h"

#include "types/string.h"

#include "engraving/dom/barline.h"
#include "engraving/dom/box.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/instrchange.h"
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

static NoteHeadGroup consolidateDrumNoteHeads(DrumInstrument di)
{
    for (int direction = 1; direction >= 0; --direction) {
        for (NoteHeadGroup group = NoteHeadGroup::HEAD_NORMAL; group < NoteHeadGroup::HEAD_GROUPS; group = NoteHeadGroup(int(group) + 1)) {
            if (engraving::Note::noteHead(direction, group, NoteHeadType::HEAD_QUARTER) == di.noteheads[static_cast<int>(NoteHeadType::HEAD_QUARTER)]) {
                for (NoteHeadType type = NoteHeadType::HEAD_WHOLE; type < NoteHeadType::HEAD_TYPES; type = NoteHeadType(int(type) + 1)) {
                    if (engraving::Note::noteHead(direction, group, type) != di.noteheads[static_cast<int>(type)]) {
                        return NoteHeadGroup::HEAD_CUSTOM;
                    }
                }
                return group;
            }
        }
    }
    return NoteHeadGroup::HEAD_CUSTOM;
}

static Drumset* createDrumset(const MusxInstanceList<others::PercussionNoteInfo>& percNoteInfoList, const MusxInstance<others::Staff>& musxStaff)
{
    Drumset* drumset = new Drumset();
    Drumset* defaultDrumset = mu::engraving::smDrumset;
    int numberOfColumns = 8;

    for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
        drumset->drum(i) = DrumInstrument(String(), NoteHeadGroup::HEAD_INVALID, 0, DirectionV::AUTO);
    }

    for (size_t i = 0; i < percNoteInfoList.size(); ++i) {
        const auto& percNoteInfo = percNoteInfoList.at(i);
        int midiPitch = percNoteInfo->getNoteType().generalMidi;
        const bool hasDefault = defaultDrumset->isValid(midiPitch);
        if (!pitchIsValid(midiPitch)) {
            /// @todo Finale percussion doesn't seem well mapped to midi, so lots of notes are currently ignored.
            continue;
        }
        drumset->drum(midiPitch) = DrumInstrument(
            String(percNoteInfo->getNoteType().rawName),
            NoteHeadGroup::HEAD_CUSTOM,
            -(percNoteInfo->calcStaffReferencePosition() - musxStaff->calcToplinePosition()), // staff line
            hasDefault ? defaultDrumset->stemDirection(midiPitch) : DirectionV::AUTO,
            int(i) / numberOfColumns, // row
            int(i) % numberOfColumns, // column
            hasDefault ? defaultDrumset->voice(midiPitch) : 0,
            hasDefault ? defaultDrumset->shortcut(midiPitch) : String()
        );

        const MusxInstance<FontInfo> percFont = options::FontOptions::getFontInfo(percNoteInfo->getDocument(), options::FontOptions::FontType::Percussion);
        drumset->drum(midiPitch).noteheads[static_cast<int>(NoteHeadType::HEAD_QUARTER)] = FinaleTextConv::symIdFromFinaleChar(percNoteInfo->closedNotehead, percFont);
        drumset->drum(midiPitch).noteheads[static_cast<int>(NoteHeadType::HEAD_HALF)] = FinaleTextConv::symIdFromFinaleChar(percNoteInfo->halfNotehead, percFont);
        drumset->drum(midiPitch).noteheads[static_cast<int>(NoteHeadType::HEAD_WHOLE)] = FinaleTextConv::symIdFromFinaleChar(percNoteInfo->wholeNotehead, percFont);
        drumset->drum(midiPitch).noteheads[static_cast<int>(NoteHeadType::HEAD_BREVIS)] = FinaleTextConv::symIdFromFinaleChar(percNoteInfo->dwholeNotehead, percFont);

        drumset->drum(midiPitch).notehead = consolidateDrumNoteHeads(drumset->drum(midiPitch));

        // MuseScore requires a note name
        if (drumset->drum(midiPitch).name.empty()) {
            drumset->drum(midiPitch).name = String(u"Percussion note");
        }
    }

    return drumset;
}

static StringData createStringData(const MusxInstance<others::FretInstrument> fretInstrument)
{
    std::vector<instrString> strings;
    strings.reserve(fretInstrument->numStrings);
    for (auto it = fretInstrument->strings.rbegin(); it != fretInstrument->strings.rend(); ++it) {
        strings.emplace_back(instrString((*it)->pitch, false, (*it)->nutOffset));
    }
    return StringData(fretInstrument->numFrets, strings);
}

static void loadInstrument(const MusxInstance<others::Staff> musxStaff, Instrument* instrument)
{
    // Initialise drumset
    if (musxStaff->percussionMapId.has_value()) {
        const MusxInstanceList<others::PercussionNoteInfo> percNoteInfoList = musxStaff->getDocument()->getOthers()->getArray<others::PercussionNoteInfo>(musxStaff->getSourcePartId(), musxStaff->percussionMapId.value());
        instrument->setUseDrumset(true);
        instrument->setDrumset(createDrumset(percNoteInfoList, musxStaff));
    } else {
        instrument->setUseDrumset(false);
    }

    // Transposition
    // note that transposition in MuseScore is instrument-based,
    // so it can change throughout the score but not differ between staves of the same part
    /// @todo convert diatonic transposition
    if (musxStaff->transposition && musxStaff->transposition->chromatic) {
        const auto& i = *musxStaff->transposition->chromatic;
        instrument->setTranspose(Interval(i.diatonic, step2pitch(i.diatonic) + i.alteration));
    }

    // Fret and string data
    if (const MusxInstance<others::FretInstrument> fretInstrument = musxStaff->getDocument()->getOthers()->get<others::FretInstrument>(musxStaff->getSourcePartId(), musxStaff->fretInstId)) {
        instrument->setStringData(createStringData(fretInstrument));
    }
}

Staff* FinaleParser::createStaff(Part* part, const MusxInstance<others::Staff> musxStaff, const InstrumentTemplate* it)
{
    auto clefTypeListFromMusxStaff = [&](const MusxInstance<others::Staff> musxStaff) -> std::optional<ClefTypeList>
    {
        const MusxInstance<options::ClefOptions::ClefDef>& concerClefDef = musxOptions().clefOptions->getClefDef(musxStaff->calcFirstClefIndex());
        ClefType concertClef = toMuseScoreClefType(concerClefDef, musxStaff);
        ClefType transposeClef = concertClef;
        if (musxStaff->transposition && musxStaff->transposition->setToClef) {
            const MusxInstance<options::ClefOptions::ClefDef>& trasposeClefDef = musxOptions().clefOptions->getClefDef(musxStaff->transposition->setToClef);
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

    // Drumset and transposition
    loadInstrument(musxStaff, part->instrument());

    // barline vertical offsets relative to staff
    auto calcBarlineOffsetHalfSpaces = [](Evpu offset) -> int {
        // Finale and MuseScore use opposite signs for up/down
        return int(std::lround(doubleFromEvpu(-offset) * 2.0));
    };
    s->setBarLineFrom(calcBarlineOffsetHalfSpaces(musxStaff->topBarlineOffset));
    s->setBarLineTo(calcBarlineOffsetHalfSpaces(musxStaff->botBarlineOffset));

    // hide when empty
    if (musxStaff->noOptimize) {
        s->setHideWhenEmpty(AutoOnOff::OFF);
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
    m_inst2Staff.emplace(StaffCmper(musxStaff->getCmper()), s->idx());
    m_staff2Inst.emplace(s->idx(), StaffCmper(musxStaff->getCmper()));
    return s;
}

void FinaleParser::importMeasures()
{
    // add default time signature
    Fraction currTimeSig = Fraction(4, 4);
    m_score->sigmap()->clear();
    m_score->sigmap()->add(0, currTimeSig);

    MusxInstanceList<others::Measure> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    MusxInstanceList<others::StaffSystem> staffSystems = m_doc->getOthers()->getArray<others::StaffSystem>(m_currentMusxPartId);
    for (const MusxInstance<others::Measure>& musxMeasure : musxMeasures) {
        MeasureBase* lastMeasure = m_score->measures()->last();
        Fraction tick(lastMeasure ? lastMeasure->endTick() : Fraction(0, 1));

        Measure* measure = Factory::createMeasure(m_score->dummy()->system());
        measure->setTick(tick);
        m_meas2Tick.emplace(musxMeasure->getCmper(), tick);
        m_tick2Meas.emplace(tick, musxMeasure->getCmper());
        // create global time signatures. local timesigs are set up later
        MusxInstance<TimeSignature> musxTimeSig = musxMeasure->createTimeSignature();
        Fraction scoreTimeSig = simpleMusxTimeSigToFraction(musxTimeSig->calcSimplified(), logger());
        if (scoreTimeSig != currTimeSig) {
            m_score->sigmap()->add(tick.ticks(), scoreTimeSig);
            currTimeSig = scoreTimeSig;
        }
        measure->setTimesig(scoreTimeSig);
        measure->setTicks(scoreTimeSig);
        m_score->measures()->append(measure);

        // Needed for later calculations, saves a global layout call
        if (!m_score->noStaves()) {
            measure->createStaves(m_score->nstaves() - 1);
        }

        // Barlines
        /// @todo choose when to set generated based on default staff barline settings
        /// (and presence of keysigs for double barlines)
        bool startsStaffSystem = false;
        bool endsStaffSystem = false;
        for (MusxInstance<others::StaffSystem> staffSystem : staffSystems) {
            startsStaffSystem = staffSystem->startMeas == musxMeasure->getCmper();
            endsStaffSystem = staffSystem->getLastMeasure() == musxMeasure->getCmper();
            if (startsStaffSystem || endsStaffSystem) {
                break;
            }
        }

        auto changeBarline = [&](bool start, others::Measure::BarlineType type) {
            /// @todo separate into two methods, always checking for start is excessively complicated
            if (start) {
                /// @todo filter by visibility as well?
                if (startsStaffSystem) {
                    if (m_score->nstaves() > 1) {
                        m_score->style().set(Sid::startBarlineMultiple, true); // default
                    } else {
                        m_score->style().set(Sid::startBarlineSingle, true); // not default
                    }
                } else {
                    return;
                }
            }

            engraving::BarLineType blt = engraving::BarLineType::NORMAL;
            bool blVisible = musxOptions().barlineOptions->drawBarlines;
            if (start && startsStaffSystem && blVisible) {
                blVisible = (m_score->nstaves() > 1) ? musxOptions().barlineOptions->drawLeftBarlineMultipleStaves : musxOptions().barlineOptions->drawLeftBarlineSingleStaff;
            }
            bool blSpan = start ? startsStaffSystem : (endsStaffSystem && musxOptions().barlineOptions->drawCloseSystemBarline);
            if (!start && endsStaffSystem && !measure->nextMeasureMM()) {
                blSpan = musxOptions().barlineOptions->drawCloseFinalBarline; // && drawFinalBarlineOnLastMeas ?
            }
            switch (type) {
            case others::Measure::BarlineType::None:
                blVisible = false;
                break;
            case others::Measure::BarlineType::OptionsDefault:
                if (start && musxOptions().barlineOptions->leftBarlineUsePrevStyle) {
                    Measure* prevM = measure->prevMeasureMM();
                    if (prevM && prevM->endBarLine() && !prevM->repeatEnd()) {
                        blt = prevM->endBarLine()->barLineType();
                    }
                    break;
                }
                [[fallthrough]];
            case others::Measure::BarlineType::Custom:
                /// @todo support?
                [[fallthrough]];
            default:
                blt = toMuseScoreBarLineType(type);
                break;
            }
            Segment* bls = start ? measure->getSegmentR(SegmentType::BeginBarLine, Fraction(0, 1)) : measure->getSegmentR(SegmentType::EndBarLine, measure->ticks());
            for (track_idx_t track = 0; track < m_score->ntracks(); track += VOICES) {
                BarLine* bl = Factory::createBarLine(bls);
                bl->setTrack(track);
                if (!blVisible) {
                    bl->setVisible(false);
                    // Finale adds distance between 'segments' even if barlines are invisible,
                    // but (like MuseScore) not their width. So we add the distance as leading space.
                    /// @todo these segments probably don't exist yet?
                    if (startsStaffSystem && bls->next()) {
                        if (bls->next()->isKeySigType()) {
                            bls->next()->setExtraLeadingSpace(m_score->style().styleS(Sid::keysigLeftMargin));
                        } else if (bls->next()->isTimeSigType()) {
                            bls->next()->setExtraLeadingSpace(m_score->style().styleS(Sid::timesigLeftMargin));
                        } else /* if (bls->next()->isClefType()) */ {
                            // default to clef distance
                            bls->next()->setExtraLeadingSpace(m_score->style().styleS(Sid::clefLeftMargin));
                        }
                    }
                }
                if (blSpan) {
                    bl->setSpanStaff(true);
                } else if (startsStaffSystem) {
                    // MuseScore spans barlines at start of system by default, so here
                    // we need to disable (reset to staff preset) span if not set by Finale
                    bl->setSpanStaff(bl->staff()->barLineSpan());
                }
                bl->setBarLineType(blt);
                if (type == others::Measure::BarlineType::Tick) {
                    bl->setSpanFrom(mu::engraving::BARLINE_SPAN_TICK1_FROM);
                    bl->setSpanTo(mu::engraving::BARLINE_SPAN_TICK1_TO);
                }
                bls->add(bl);
            }
        };
        changeBarline(true, musxMeasure->leftBarlineType);
        changeBarline(false, musxMeasure->barlineType);

        // Set repeats and other measure properties
        measure->setRepeatStart(musxMeasure->forwardRepeatBar);
        measure->setRepeatEnd(musxMeasure->backwardsRepeatBar);
        measure->setBreakMultiMeasureRest(musxMeasure->breakMmRest);
        measure->setIrregular(musxMeasure->noMeasNum);
    }
}

void FinaleParser::importParts()
{
    MusxInstanceList<others::StaffUsed> scrollView = m_doc->getOthers()->getArray<others::StaffUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);

    std::unordered_map<StaffCmper, QString> inst2Part;
    int partNumber = 0;
    for (const MusxInstance<others::StaffUsed>& item : scrollView) {
        MusxInstance<others::Staff> staff = item->getStaffInstance();
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
        const InstrumentTemplate* it = searchTemplate(instrTemplateIdfromUuid(compositeStaff->instUuid));
        if (it) {
            part->initFromInstrTemplate(it);
        }

        QString partId = String("P%1").arg(++partNumber);
        part->setId(partId);

        const auto& [topStaffId, instInfo] = *instIt;
        if (instInfo.staffGroupId != 0) {
            if (MusxInstance<details::StaffGroup> group = m_doc->getDetails()->get<details::StaffGroup>(m_currentMusxPartId, BASE_SYSTEM_ID, instInfo.staffGroupId)) {
                switch (group->hideStaves) {
                case details::StaffGroup::HideStaves::None:
                    part->setHideWhenEmpty(AutoOnOff::OFF);
                    break;
                case details::StaffGroup::HideStaves::AsGroup:
                    part->setHideWhenEmpty(AutoOnOff::ON);
                    // [[fallthrough]]; incorrect, see lower line?
                    part->setHideStavesWhenIndividuallyEmpty(false);
                    break;
                case details::StaffGroup::HideStaves::Normally:
                    part->setHideStavesWhenIndividuallyEmpty(true);
                    break;
                default:
                    break;
                }
            }
        }
        for (const StaffCmper inst : instInfo.getSequentialStaves()) {
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
            const MusxInstanceList<others::StaffUsed> systemOneStaves = m_doc->getOthers()->getArray<others::StaffUsed>(m_currentMusxPartId, 1);
            if (std::optional<size_t> index = systemOneStaves.getIndexForStaff(staff->getCmper())) {
                const musx::util::Fraction staffMag = systemOneStaves[index.value()]->calcEffectiveScaling() / musxOptions().combinedDefaultStaffScaling;
                options.scaleFontSizeBy /= staffMag.toDouble();
            }
            return stringFromEnigmaText(parsingContext, options);
        };
        const String longName = nameFromEnigmaText(staff->getFullInstrumentNameCtx(m_currentMusxPartId), u"longInstrument");
        part->setPartName(longName);
        part->setLongName(longName);
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
    const MusxInstanceList<others::StaffUsed> scrollView = m_doc->getOthers()->getArray<others::StaffUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);

    auto staffGroups = details::StaffGroupInfo::getGroupsAtMeasure(1, m_currentMusxPartId, scrollView);
    auto groupsByLayer = computeStaffGroupLayers(staffGroups);
    for (const auto& groupInfo : groupsByLayer) {
        IF_ASSERT_FAILED(groupInfo.info.startSlot && groupInfo.info.endSlot) {
            logger()->logWarning(String(u"Group info encountered without start or end slot information"));
            continue;
        }
        auto musxStartStaff = scrollView.getStaffInstanceAtIndex(Cmper(groupInfo.info.startSlot.value()));
        auto musxEndStaff = scrollView.getStaffInstanceAtIndex(Cmper(groupInfo.info.endSlot.value()));
        IF_ASSERT_FAILED(musxStartStaff && musxEndStaff) {
            logger()->logWarning(String(u"Group info encountered missing start or end staff information"));
            continue;
        }
        staff_idx_t startStaffIdx = muse::value(m_inst2Staff, StaffCmper(musxStartStaff->getCmper()), muse::nidx);
        IF_ASSERT_FAILED(startStaffIdx != muse::nidx) {
            logger()->logWarning(String(u"Create brackets: Musx inst value not found for staff cmper %1").arg(String::fromStdString(std::to_string(musxStartStaff->getCmper()))));
            continue;
        }
        BracketItem* bi = Factory::createBracketItem(m_score->dummy());
        bi->setBracketType(toMuseScoreBracketType(groupInfo.info.group->bracket->style));
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

ClefType FinaleParser::toMuseScoreClefType(const MusxInstance<musx::dom::options::ClefOptions::ClefDef>& clefDef,
                                           const MusxInstance<musx::dom::others::Staff>& musxStaff)
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

Clef* FinaleParser::createClef(Score* score, const MusxInstance<musx::dom::others::Staff>& musxStaff,
                               staff_idx_t staffIdx, ClefIndex musxClef, Measure* measure, Edu musxEduPos,
                               bool afterBarline, bool visible)
{
    const MusxInstance<options::ClefOptions::ClefDef>& clefDef = musxOptions().clefOptions->getClefDef(musxClef);
    ClefType entryClefType = toMuseScoreClefType(clefDef, musxStaff);
    if (entryClefType == ClefType::INVALID) {
        return nullptr;
    }
    Clef* clef = Factory::createClef(score->dummy()->segment());
    clef->setTrack(staffIdx * VOICES);
    clef->setConcertClef(entryClefType);
    clef->setTransposingClef(entryClefType);
    // clef->setShowCourtesy();
    // clef->setForInstrumentChange();
    clef->setVisible(visible && !clefDef->isBlank());
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
    Fraction clefTick = eduToFraction(musxEduPos) / timeStretch;
    Segment* clefSeg = measure->getSegmentR(clef->isHeader() ? SegmentType::HeaderClef : SegmentType::Clef, clefTick);
    clefSeg->add(clef);
    return clef;
}

void FinaleParser::importClefs(const MusxInstance<others::StaffUsed>& musxScrollViewItem,
                                    const MusxInstance<others::Measure>& musxMeasure, Measure* measure, staff_idx_t curStaffIdx,
                                    ClefIndex& musxCurrClef, const MusxInstance<others::Measure>& prevMusxMeasure)
{
    // The Finale UI requires transposition to be a full-measure staff-style assignment, so checking only the beginning of the bar should be sufficient.
    // However, it is possible to defeat this requirement using plugins. That said, doing so produces erratic results, so I'm not sure we should support it.
    // For now, only check the start of the measure.
    auto musxStaffAtMeasureStart = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper(), 0);
    if (musxStaffAtMeasureStart && musxStaffAtMeasureStart->transposition && musxStaffAtMeasureStart->transposition->setToClef) {
        if (musxStaffAtMeasureStart->transposedClef != musxCurrClef) {
            if (Clef* clef = createClef(m_score, musxStaffAtMeasureStart, curStaffIdx, musxStaffAtMeasureStart->transposedClef, measure, /*xEduPos*/ 0, false, true)) {
                clef->setShowCourtesy(!prevMusxMeasure || !prevMusxMeasure->hideCaution);
                musxCurrClef = musxStaffAtMeasureStart->transposedClef;
            }
        }
        return;
    }
    if (auto gfHold = m_doc->getDetails()->get<details::GFrameHold>(m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper())) {
        if (gfHold->clefId.has_value()) {
            if (gfHold->clefId.value() != musxCurrClef || gfHold->showClefMode == ShowClefMode::Always) {
                const bool visible = gfHold->showClefMode != ShowClefMode::Never;
                if (createClef(m_score, musxStaffAtMeasureStart, curStaffIdx, gfHold->clefId.value(), measure, /*xEduPos*/ 0, gfHold->clefAfterBarline, visible)) {
                    musxCurrClef = gfHold->clefId.value();
                }
            }
        } else {
            MusxInstanceList<others::ClefList> midMeasureClefs = m_doc->getOthers()->getArray<others::ClefList>(m_currentMusxPartId, gfHold->clefListId);
            for (const MusxInstance<others::ClefList>& midMeasureClef : midMeasureClefs) {
                if (midMeasureClef->xEduPos > 0 || midMeasureClef->clefIndex != musxCurrClef || midMeasureClef->clefMode == ShowClefMode::Always) {
                    const bool visible = midMeasureClef->clefMode != ShowClefMode::Never;
                    const bool afterBarline = midMeasureClef->xEduPos == 0 && midMeasureClef->afterBarline;
                    auto currStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper(), midMeasureClef->xEduPos);
                    if (Clef* clef = createClef(m_score, currStaff, curStaffIdx, midMeasureClef->clefIndex, measure, midMeasureClef->xEduPos, afterBarline, visible)) {
                        // only set y offset because MuseScore automatically calculates the horizontal spacing offset
                        clef->setOffset(0.0, -doubleFromEvpu(midMeasureClef->yEvpuPos) * clef->spatium());
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
    const bool isNotEqual = [&]() {
        if constexpr(std::is_floating_point_v<T>) {
            return !muse::RealIsEqual(a, b);
        } else {
            return a != b;
        }
    }();
    result = result || isNotEqual;
    return isNotEqual;
}

bool FinaleParser::applyStaffSyles(StaffType* staffType, const MusxInstance<musx::dom::others::StaffComposite>& currStaff)
{
    bool result = false;
    if (changed(staffType->genClef(), !currStaff->hideClefs, result)) {
        staffType->setGenClef(!currStaff->hideClefs);
    }
    if (changed(staffType->genKeysig(), !currStaff->hideKeySigs, result)) {
        staffType->setGenKeysig(!currStaff->hideKeySigs);
    }
    if (changed(staffType->genTimesig(), !currStaff->hideTimeSigs, result)) {
        staffType->setGenTimesig(!currStaff->hideTimeSigs);
    }
    StaffGroup staffGroup = staffGroupFromNotationStyle(currStaff->notationStyle);
    if (changed(staffType->group(), staffGroup, result)) {
        staffType->setGroup(staffGroup);
    }
    int numLines = currStaff->calcNumberOfStafflines();
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

    // userMag is not based on staff styles but on others::StaffUsed
    if (MusxInstance<others::StaffSystem> system = m_doc->calculateSystemFromMeasure(m_currentMusxPartId, currStaff->getMeasureId())) {
        const MusxInstanceList<others::StaffUsed> systemStaves = m_doc->getOthers()->getArray<others::StaffUsed>(m_currentMusxPartId, system->getCmper());
        if (std::optional<size_t> index = systemStaves.getIndexForStaff(currStaff->getCmper())) {
            const size_t x = index.value();
            const double newUserMag = (systemStaves[x]->calcEffectiveScaling() / musxOptions().combinedDefaultStaffScaling).toDouble();
            if (changed(staffType->userMag(), newUserMag, result)) {
                staffType->setUserMag(newUserMag);
            }
        }
    }

    /// @todo use userMag instead of smallClef? (But it requires a separate system-by-system search.)
    /// @todo tablature options
    /// @todo others?

    return result;
}

void FinaleParser::importStaffItems()
{
    MusxInstanceList<others::Measure> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    if (!m_score->firstMeasure()) {
        return;
    }
    MusxInstanceList<others::StaffUsed> musxScrollView = m_doc->getOthers()->getArray<others::StaffUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);
    MusxInstanceList<others::StaffSystem> musxSystems = m_doc->getOthers()->getArray<others::StaffSystem>(m_currentMusxPartId);
    for (const MusxInstance<others::StaffUsed>& musxScrollViewItem : musxScrollView) {
        // per staff style calculations
        const MusxInstance<others::Staff>& rawStaff = m_doc->getOthers()->get<others::Staff>(m_currentMusxPartId, musxScrollViewItem->staffId);
        IF_ASSERT_FAILED(rawStaff) {
            logger()->logWarning(String(u"Unable to retrieve musx raw staff"), m_doc, musxScrollViewItem->staffId, 1);
            return;
        }
        std::set<MeasCmper> styleChanges; // MuseScore style changes must occur on measure boundaries
        styleChanges.emplace(1); // there is always a style change on the first measure.
        if (rawStaff->hasStyles) {
            MusxInstanceList<others::StaffStyleAssign> musxStyleChanges = m_doc->getOthers()->getArray<others::StaffStyleAssign>(m_currentMusxPartId, rawStaff->getCmper());
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
        for (const auto& musxSystem : musxSystems) {
            if (musxSystem->startMeas > 1) { // we already added measure 1 at init time
                const MusxInstanceList<others::StaffUsed> systemStaves = m_doc->getOthers()->getArray<others::StaffUsed>(m_currentMusxPartId, musxSystem->getCmper());
                if (systemStaves.getIndexForStaff(rawStaff->getCmper())) {
                    styleChanges.emplace(musxSystem->startMeas);
                }
            }
        }
        std::string prevUuid;
        for (MeasCmper measNum : styleChanges) {
            Fraction currTick = muse::value(m_meas2Tick, measNum, Fraction(-1, 1));
            Measure* measure = !currTick.negative() ? m_score->tick2measure(currTick) : nullptr;
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
            MusxInstance<others::StaffComposite> currStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId, measNum, 0);
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

            // Instrument changes
            if (isValidUuid(currStaff->instUuid)) {
                if (currStaff->instUuid != prevUuid && tick > Fraction(0, 1)) {
                    Segment* segment = measure->getSegmentR(SegmentType::ChordRest, Fraction(0, 1));
                    InstrumentChange* c = Factory::createInstrumentChange(segment, Instrument::fromTemplate(searchTemplate(instrTemplateIdfromUuid(currStaff->instUuid))));
                    loadInstrument(currStaff, c->instrument());
                    c->setTrack(staff2track(staffIdx));
                    const String newInstrChangeText = muse::mtrc("engraving", "To %1").arg(c->instrument()->trackName());
                    c->setXmlText(TextBase::plainToXmlText(newInstrChangeText));
                    c->setVisible(false);
                    segment->add(c);
                }
                prevUuid = currStaff->instUuid;
            }
        }
        // per measure calculations
        MusxInstance<TimeSignature> currMusxTimeSig;
        MusxInstance<KeySignature> currMusxKeySig;
        std::optional<KeySigEvent> currKeySigEvent;
        MusxInstance<others::Measure> prevMusxMeasure;
        ClefIndex musxCurrClef = others::Staff::calcFirstClefIndex(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId);
        /// @todo handle pickup measures and other measures where display and actual timesigs differ
        for (const MusxInstance<others::Measure>& musxMeasure : musxMeasures) {
            Fraction currTick = muse::value(m_meas2Tick, musxMeasure->getCmper(), Fraction(-1, 1));
            Measure* measure = !currTick.negative() ? m_score->tick2measure(currTick) : nullptr;
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
            const MusxInstance<TimeSignature> globalTimeSig = musxMeasure->createTimeSignature();
            const MusxInstance<TimeSignature> musxTimeSig = musxMeasure->createTimeSignature(musxScrollViewItem->staffId);
            if (!currMusxTimeSig || !currMusxTimeSig->isSame(*musxTimeSig) || musxMeasure->showTime == others::Measure::ShowTimeSigMode::Always) {
                Fraction timeSig = simpleMusxTimeSigToFraction(musxTimeSig->calcSimplified(), logger());
                Segment* seg = measure->getSegmentR(SegmentType::TimeSig, Fraction(0, 1));
                TimeSig* ts = Factory::createTimeSig(seg);
                ts->setSig(timeSig);
                ts->setTrack(staffIdx * VOICES);
                ts->setVisible(musxMeasure->showTime != others::Measure::ShowTimeSigMode::Never);
                ts->setShowCourtesySig(!prevMusxMeasure || !prevMusxMeasure->hideCaution);
                Fraction stretch = Fraction(musxTimeSig->calcTotalDuration().calcEduDuration(), globalTimeSig->calcTotalDuration().calcEduDuration()).reduced();
                ts->setStretch(stretch);
                /// @todo other time signature options? Beaming? Composite list?
                seg->add(ts);
                staff->addTimeSig(ts);
            }
            currMusxTimeSig = musxTimeSig;

            // clefs
            importClefs(musxScrollViewItem, musxMeasure, measure, staffIdx, musxCurrClef, prevMusxMeasure);

            // keysig
            const MusxInstance<KeySignature> musxKeySig = musxMeasure->createKeySignature(musxScrollViewItem->staffId);
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
                        Key concertKey = keyless ? Key::C : keyFromAlteration(musxKeySig->getAlteration(KeySignature::KeyContext::Concert));
                        Key key = keyless ? Key::C : keyFromAlteration(musxKeySig->getAlteration(KeySignature::KeyContext::Written));
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
                            km = keyModeFromDiatonicMode(mode.value());
                        }
                        ksEvent.setMode(km);
                    } else if (musxKeySig->isNonLinear()) {
                        MusxInstance<others::AcciOrderSharps> musxAccis = m_doc->getOthers()->get<others::AcciOrderSharps>(m_currentMusxPartId, musxKeySig->getKeyMode());
                        MusxInstance<others::AcciAmountSharps> musxAmounts = m_doc->getOthers()->get<others::AcciAmountSharps>(m_currentMusxPartId, musxKeySig->getKeyMode());
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
                            SymId accidental = acciSymbolFromAcciAmount(amount);
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
                    /// @todo this creates a visible keysig even on percussion staves, but those might still need the event.
                    Segment* seg = measure->getSegmentR(SegmentType::KeySig, Fraction(0, 1));
                    KeySig* ks = Factory::createKeySig(seg);
                    ks->setKeySigEvent(keySigEvent.value());
                    ks->setTrack(staffIdx * VOICES);
                    ks->setVisible(musxMeasure->showKey != others::Measure::ShowKeySigMode::Never);
                    ks->setShowCourtesy(!prevMusxMeasure || !prevMusxMeasure->hideCaution);
                    seg->add(ks);
                    staff->setKey(currTick, ks->keySigEvent());
                }
                currKeySigEvent = keySigEvent;
            }
            currMusxKeySig = musxKeySig;
            prevMusxMeasure = musxMeasure;
        }
    }
}

void FinaleParser::importPageLayout()
{
    /// @todo Scan each system's staves and make certain that every staff on each system is included even if it is empty or excluded even if it is not.
    /// @todo Match staff separation in Finale better.
    /// @todo Take into account per-staff scaling. This affects the vertical spacing of staves. I need to add a helper func for this in musx. It requires some
    /// reverse engineering of Finale behavior.

    // Handle blank pages
    MusxInstanceList<others::Page> pages = m_doc->getOthers()->getArray<others::Page>(m_currentMusxPartId);
    size_t blankPagesToAdd = 0;
    for (const auto& page : pages) {
        if (page->isBlank()) {
            ++blankPagesToAdd;
        } else if (blankPagesToAdd) {
            const MusxInstance<others::StaffSystem>& firstPageSystem = m_doc->getOthers()->get<others::StaffSystem>(m_currentMusxPartId, page->firstSystemId);
            IF_ASSERT_FAILED(firstPageSystem) {
                continue;
            }
            Fraction pageStartTick = muse::value(m_meas2Tick, firstPageSystem->startMeas, Fraction(-1, 1));
            if (pageStartTick.negative()) {
                continue;
            }
            Measure* afterBlank = m_score->tick2measure(pageStartTick);
            MeasureBase* prev = afterBlank ? afterBlank->prev() : nullptr;
            for (; blankPagesToAdd > 0; --blankPagesToAdd) {
                VBox* pageFrame = Factory::createVBox(m_score->dummy()->system());
                pageFrame->setTick(pageStartTick);
                pageFrame->setNext(afterBlank);
                pageFrame->setPrev(prev);
                m_score->measures()->insert(pageFrame, pageFrame);
                LayoutBreak* lb = Factory::createLayoutBreak(pageFrame);
                lb->setLayoutBreakType(LayoutBreakType::PAGE);
                pageFrame->add(lb);
                prev = pageFrame;
            }
        }
    }
    for (; blankPagesToAdd > 0; --blankPagesToAdd) {
        VBox* pageFrame = Factory::createVBox(m_score->dummy()->system());
        pageFrame->setTick(m_score->last() ? m_score->last()->endTick() : Fraction(0, 1));
        m_score->measures()->append(pageFrame);
        LayoutBreak* lb = Factory::createLayoutBreak(pageFrame);
        lb->setLayoutBreakType(LayoutBreakType::PAGE);
        pageFrame->add(lb);
    }

    // No measures or staves means no valid staff systems
    if (!m_score->firstMeasure() || m_score->noStaves()) {
        return;
    }
    MusxInstanceList<others::StaffSystem> staffSystems = m_doc->getOthers()->getArray<others::StaffSystem>(m_currentMusxPartId);
    logger()->logDebugTrace(String(u"Document contains %1 staff systems and %2 pages.").arg(staffSystems.size(), pages.size()));
    size_t currentPageIndex = 0;
    for (size_t i = 0; i < staffSystems.size(); ++i) {
        const MusxInstance<others::StaffSystem>& leftStaffSystem = staffSystems[i];
        MusxInstance<others::StaffSystem>& rightStaffSystem = staffSystems[i];

        //retrieve leftmost measure of system
        Fraction startTick = muse::value(m_meas2Tick, leftStaffSystem->startMeas, Fraction(-1, 1));
        Measure* startMeasure = !startTick.negative() ? m_score->tick2measure(startTick) : nullptr;

        // determine if system is first on the page
        // determine the current page the staffsystem is on
        bool isFirstSystemOnPage = false;
        for (size_t j = currentPageIndex; j < pages.size(); ++j) {
            const MusxInstance<others::Page>& page = pages[j];
            if (page->isBlank()) {
                continue;
            }
            const MusxInstance<others::StaffSystem>& firstPageSystem = m_doc->getOthers()->get<others::StaffSystem>(m_currentMusxPartId, page->firstSystemId);
            IF_ASSERT_FAILED(firstPageSystem) {
                break;
            }
            Fraction pageStartTick = muse::value(m_meas2Tick, firstPageSystem->startMeas, Fraction(-2, 1));
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
                    Measure* distMeasure = !distTick.negative() ? m_score->tick2measure(distTick) : nullptr;
                    IF_ASSERT_FAILED(distMeasure) {
                        break;
                    }
                    logger()->logInfo(String(u"Adding space between systems at tick %1").arg(distTick.toString()));
                    HBox* distBox = Factory::createHBox(m_score->dummy()->system());
                    distBox->setBoxWidth(Spatium(doubleFromEvpu(dist)));
                    distBox->setSizeIsSpatiumDependent(false);
                    distBox->setTick(distMeasure->tick());
                    distBox->setNext(distMeasure);
                    distBox->setPrev(distMeasure->prev());
                    m_score->measures()->insert(distBox, distBox);
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
        Measure* endMeasure = !endTick.negative() ? m_score->tick2measure(endTick) : nullptr;
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
                m_score->style().set(Sid::firstSystemIndentationValue, doubleFromEvpu(leftStaffSystem->left));
            } else {
                HBox* leftBox = Factory::createHBox(m_score->dummy()->system());
                leftBox->setBoxWidth(Spatium(doubleFromEvpu(leftStaffSystem->left)));
                leftBox->setSizeIsSpatiumDependent(false);
                leftBox->setTick(startMeasure->tick());
                leftBox->setNext(startMeasure);
                leftBox->setPrev(startMeasure->prev());
                m_score->measures()->insert(leftBox, leftBox);
                // leftBox->manageExclusionFromParts(/*exclude =*/ true); // excluded by default
                sysStart = leftBox;
            }
        } else {
            logger()->logInfo(String(u"No need to add left margin for system %1").arg(i));
        }
        MeasureBase* sysEnd = endMeasure;
        if (!muse::RealIsEqual(double(-rightStaffSystem->right), 0.0)) {
            HBox* rightBox = Factory::createHBox(m_score->dummy()->system());
            rightBox->setBoxWidth(Spatium(doubleFromEvpu(-rightStaffSystem->right)));
            rightBox->setSizeIsSpatiumDependent(false);
            Fraction rightTick = endMeasure->nextMeasure() ? endMeasure->nextMeasure()->tick() : m_score->last()->endTick();
            rightBox->setTick(rightTick);
            rightBox->setNext(endMeasure->next());
            rightBox->setPrev(endMeasure);
            m_score->measures()->insert(rightBox, rightBox);
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
        for (const MusxInstance<others::Page>& page : pages) {
            if (page->isBlank()) {
                continue;
            }
            const MusxInstance<others::StaffSystem>& firstPageSystem = m_doc->getOthers()->get<others::StaffSystem>(m_currentMusxPartId, page->firstSystemId);
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

        // If following measure should show full instrument names, add section break to sysEnd
        const MusxInstance<others::Measure>& nextMeasure = m_doc->getOthers()->get<others::Measure>(m_currentMusxPartId, rightStaffSystem->endMeas);
        if (nextMeasure && nextMeasure->showFullNames) {
            LayoutBreak* lb = Factory::createLayoutBreak(sysEnd);
            lb->setLayoutBreakType(LayoutBreakType::SECTION);
            lb->setStartWithMeasureOne(false);
            lb->setStartWithLongNames(true);
            lb->setPause(0.0);
            lb->setFirstSystemIndentation(false);
            const MusxInstance<others::Measure>& lastMeasure = m_doc->getOthers()->get<others::Measure>(m_currentMusxPartId, rightStaffSystem->getLastMeasure());
            lb->setShowCourtesy(!lastMeasure->hideCaution);
            sysEnd->add(lb);
        }

        // In Finale, up is positive and down is negative. That means we have to reverse the signs of the vertical axis for MuseScore.
        // HOWEVER, the top and right margins have signs reversed from the U.I. Are we confused yet?
        // create system top and bottom margins
        if (isFirstSystemOnPage) {
            Spacer* upSpacer = Factory::createSpacer(startMeasure);
            upSpacer->setSpacerType(SpacerType::UP);
            upSpacer->setTrack(0);
            upSpacer->setGap(absoluteSpatiumFromEvpu(-leftStaffSystem->top - leftStaffSystem->distanceToPrev, upSpacer)); // (signs reversed)
            /// @todo account for title frames / perhaps header frames
            startMeasure->add(upSpacer);
        }
        if (!isLastSystemOnPage) {
            Spacer* downSpacer = Factory::createSpacer(startMeasure);
            downSpacer->setSpacerType(SpacerType::FIXED);
            downSpacer->setTrack((m_score->nstaves() - 1) * VOICES); // invisible staves are correctly accounted for on layout
            downSpacer->setGap(absoluteSpatiumFromEvpu((-rightStaffSystem->bottom - 96) - staffSystems[i+1]->distanceToPrev - staffSystems[i+1]->top, downSpacer)); // (signs reversed)
            startMeasure->add(downSpacer);
        }

        // Add distance between the staves
        /// @todo do we need to account for staff visibility here, using startMeasure->system()->staff(staffIdx)->show() ?
        /// Possibly a layout call would needed before here, since we may need check for the visibility of SysStaves
        auto instrumentsUsedInSystem = m_doc->getOthers()->getArray<others::StaffUsed>(m_currentMusxPartId, leftStaffSystem->getCmper());
        for (size_t j = 1; j < instrumentsUsedInSystem.size(); ++j) {
            MusxInstance<others::StaffUsed>& prevMusxStaff = instrumentsUsedInSystem[j - 1];
            MusxInstance<others::StaffUsed>& nextMusxStaff = instrumentsUsedInSystem[j];
            staff_idx_t prevStaffIdx = muse::value(m_inst2Staff, prevMusxStaff->staffId, muse::nidx);
            staff_idx_t nextStaffIdx = muse::value(m_inst2Staff, nextMusxStaff->staffId, muse::nidx);
            if (nextStaffIdx == muse::nidx || prevStaffIdx == muse::nidx) {
                continue;
            }
            Spacer* staffSpacer = Factory::createSpacer(startMeasure);
            staffSpacer->setSpacerType(SpacerType::FIXED);
            staffSpacer->setTrack(staff2track(prevStaffIdx));
            Spatium dist = absoluteSpatiumFromEvpu(-nextMusxStaff->distFromTop + prevMusxStaff->distFromTop, staffSpacer)
            // This line (probably) requires importStaffItems() be called before importPageLayout().
                           - Spatium::fromMM(m_score->staff(prevStaffIdx)->staffHeight(startMeasure->tick()), staffSpacer->spatium());
            staffSpacer->setGap(dist);
            startMeasure->add(staffSpacer);
        }
    }
}

}
