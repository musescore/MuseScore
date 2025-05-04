/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "engraving/dom/box.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordlist.h"
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
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/utils.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

void EnigmaXmlImporter::import()
{
    importParts();
    importBrackets();
    importMeasures();
}

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

Staff* EnigmaXmlImporter::createStaff(Part* part, const std::shared_ptr<const others::Staff> musxStaff, const InstrumentTemplate* it)
{
    Staff* s = Factory::createStaff(part);

    /// @todo staff settings can change at any tick
    Fraction eventTick{0, 1};

    // initialise MuseScore's default values
    if (it) {
        s->init(it, 0, 0);
        // don't load bracket from template, we add it later (if it exists)
        s->setBracketType(0, BracketType::NO_BRACKET);
        s->setBracketSpan(0, 0);
        s->setBarLineSpan(0);
    }
    /// @todo This staffLines setting will move to wherever we parse staff styles
    /// @todo Need to intialize the staff type from presets?

    // # of staff lines (only load if custom)
    if (musxStaff->staffLines.has_value()) {
        if (musxStaff->staffLines.value() != s->lines(eventTick)) {
            s->setLines(eventTick, musxStaff->staffLines.value());
        }
    } else if (musxStaff->customStaff.has_value()) {
        int customStaffSize = static_cast<int>(musxStaff->customStaff.value().size());
        if (customStaffSize != s->lines(eventTick)) {
            s->setLines(eventTick, customStaffSize);
        }
    }

    // barline vertical offsets relative to staff
    auto calcBarlineOffsetHalfSpaces = [](Evpu offset) -> int {
        // Finale and MuseScore use opposite signs for up/down
        double halfSpaces = (double(-offset) * 2.0) / EVPU_PER_SPACE;
        return int(std::lround(halfSpaces));
    };
    s->setBarLineFrom(calcBarlineOffsetHalfSpaces(musxStaff->topBarlineOffset));
    s->setBarLineTo(calcBarlineOffsetHalfSpaces(musxStaff->botBarlineOffset));

    // hide when empty
    /// @todo inherit
    s->setHideWhenEmpty(Staff::HideMode::INSTRUMENT);

    // clefs
    if (auto defaultClefs = clefTypeListFromMusxStaff(musxStaff)) {
        s->setDefaultClefType(defaultClefs.value());
    } else {
        s->staffType(eventTick)->setGenClef(false);
    }
    m_staff2Inst.emplace(s->idx(), InstCmper(musxStaff->getCmper()));
    m_inst2Staff.emplace(InstCmper(musxStaff->getCmper()), s->idx());
    m_score->appendStaff(s);
    return s;
}

static ChordRest* importEntry(const std::shared_ptr<const musx::dom::EntryInfo> entryInfo, Segment** segment)
{
    using MusxNote = musx::dom::Note;

    // Retrieve entry from entryInfo
    Entry currentEntry = entryInfo->getEntry();
    if (!currentEntry) {
        return nullptr;
    }

    // durationType
    std::pair<musx::dom::NoteType, int> noteInfo = currentEntry->calcNoteInfo();
    TDuration d = FinaleTConv::noteTypeToDurationType(noteInfo.first);
    if (d == DurationType::V_INVALID) {
        LOGE() << "ChordRest duration not supported in MuseScore";
        return nullptr;
    }

    ChordRest* cr;
    bool chordIsCrossStaff = true;

    auto harmLev2pitch = [&](int harmLev, MusxNote::NoteName tonic, Key key) -> int {
        int absStep = 35 /*middle C*/ + int(tonic) + harmLev;
        return absStep2pitchByKey(absStep, key);
    };

    if (currentEntry->isNote) {
        Chord* chord = Factory::createChord(s);

        for (MusxNote n : currentEntry->notes) {
            // calculate pitch, accidentals and transposition
            NoteVal nval;
            nval.pitch = harmLev2pitch(n->harmLev, MusxNote::NoteName::C, Key::C) + n->harmAlt; /// @todo use real keysig and tonic

            // Finale supports up to 7 alterations (relative to keysig), MuseScore only 3 (up to 3# or 3b).
            // If the alteration is too large, we keep the pitch but rewrite the note on a new line
            /// @todo Do we need to set tpc1 if we're in a transposition?
            /// @todo Is the pitch value stored in the entry tranposed? Code assumes pitch stored is absolute
            nval.tpc1 = step2tpcByKeyAndAccAlteration(n->harmLev + int(MusxNote::NoteName::C), Key::C, n->harmAlt); /// @todo use real keysig, need to add NoteNamevalue?
            if (!tpcIsValid(nval.tpc1)) {
                nval.tpc1 = pitch2tpc(nval.pitch, Key::C, Prefer::NEAREST); // concert pitch
            }
            AccidentalVal accVal = tpc2alter(nval.tpc1);

            if (false /*currently in a transposition*/) {
                // nval.pitch += m_score->staff(size_t(0))->part()->instrument(s->tick())->transpose().chromatic;
                nval.pitch = harmLev2pitch(n->harmLev, MusxNote::NoteName::C, Key::C) + n->harmAlt; /// @todo use real transposed tonic and keysig
                /// @todo use real transposed keysig, need to add NoteNamevalue? is this method correct?
                nval.tpc2 = step2tpcByKeyAndAccAlteration(n->harmLev + int(MusxNote::NoteName::C), /*transposed key*/ Key::C, n->harmAlt);
                if (!tpcIsValid(nval.tpc2)) {
                    nval.tpc2 = pitch2tpc(nval.pitch, Key::C, Prefer::NEAREST); // pitch is now transposed
                }
                accVal = tpc2alter(nval.tpc2);
            } else {
                nval.tpc2 = nval.tpc1; // needed?
            }

            engraving::Note* note = Factory::createNote(chord);
            note->setParent(chord);
            note->setTrack(chord->track());
            note->setNval(nval);

            // Add accidental if needed
            if (n->freezeAcci || n->harmAlt == 0) {
                AccidentalType at = Accidental::value2subtype(accVal);
                Accidental* a = Factory::createAccidental(note);
                a->setAccidentalType(at);
                a->setRole(AccidentalRole::USER);
                a->setParent(note);
                note->add(a);
            }

            // MuseScore doesn't support individual cross-staff notes, either move the whole chord or nothing
            chordIsCrossStaff = chordIsCrossStaff && n->crossStaff;
        }
        cr = toChordRest(chord);
    } else {
        Rest* rest = Factory::createRest(s, d);
        cr = toChordRest(rest);

        for (MusxNote n : currentEntry->notes) {
            //todo: calculate y-offset here (remember is also staff-dependent)
            chordIsCrossStaff = chordIsCrossStaff && n->crossStaff;
        }
    }

    cr->setDurationType(d);
    cr->setDots(static_cast<int>(noteInfo.second));
    return cr;
}

static std::optional<Fraction> musxFractionToFraction(musx::util::Fraction count)
{
    if (count.remainder()) {
        LOGE() << "Fraction has fractional portion that could not be reduced.";
        return std::nullopt;
    }
    return Fraction(count.numerator(), count.denominator());
}

bool processEntryInfo(const std::shared_ptr<const musx::dom::EntryInfo> entryInfo, track_idx_t curTrackIdx,
                      Segment** segment, std::vector<ReadableTuplet>* tupletMap, size_t* lastAddedTupletIndex);
{
    Fraction currentEntryInfoStart = musxFractionToFraction(entryInfo->elapsedDuration);
    if (!currentEntryInfoStart || !segment) {
        LOGW() << "Position in measure unknown";
        return false;
    }

    if (entryInfo->graceIndex != 0) {
        LOGW() << "Grace notes not yet supported";
        return true;
    }

    if (entryInfo->v2Launch) {
        LOGW() << "voice 2 unspported";
    }

    if (segment->rTick() != currentEntryInfoStart.value()) {
        if (segment->rTick() < currentEntryInfoStart.value()) {
            // invisible fill with rests up to start
        } else {
            LOGW() << "Incorrect position in measure";
            return false;
        }
    }

    // create Tuplets as needed, starting with the outermost
    for (size_t i = 0; i < tupletMap.size(); ++i) {
        if (!tupletMap[i].valid) {
            continue;
        }
        if (tupletMap[i].absBegin == currentEntryInfoStart.value()) {
            tupletMap[i].scoreTuplet = Factory::createTuplet(segment->measure());
            tupletMap[i].scoreTuplet->setTrack(curTrackIdx);
            tupletMap[i].scoreTuplet->setTick(segment->tick());
            tupletMap[i].scoreTuplet->setParent(segment->measure());
            /// @todo set tuplet duration, ratio, number, styled properties
            if (i != 0 && tupletMap[i-1].layer < tupletMap[i].layer) {
                tupletMap[i-1].scoreTuplet->add(tupletMap[i].scoreTuplet);
            }
            lastTupletIndex = i;
        } else if (tupletMap[i].absBegin > currentEntryInfoStart.value()) {
            break;
        }
    }
    // locate current tuplet to parent the ChordRests to (if it exists)
    Tuplet* parentTuplet = nullptr;
    if (tupletMap[lastTupletIndex].scoreTuplet) {
        do {
            if (tupletMap[lastTupletIndex].absBegin + tupletMap[i].absDuration >= currentEntryInfoStart.value()) {
                break;
            }
        } while (lastAddedTupletIndex > 0 && --lastAddedTupletIndex);
        parentTuplet = tupletMap[lastTupletIndex].scoreTuplet;
    }

    // load entry
    ChordRest* cr = importEntry(entryInfo, segment);
    if (cr) {
        cr->setTicks(/*entryInfo->actualDuration does it need global duration here, or the written duration in the tuplet*/);
        cr->setTrack(curTrackIdx);
        s->add(cr);
        if (parentTuplet) {
            parentTuplet->add(cr);
        }
    } else {
        LOGW() << "Failed to read entry contents";
        // Fill space with invisible rests
    }

    // add clef change
    ClefType entryClefType = FinaleTConv::toMuseScoreClefType(entryInfo->clefIndex);
    if (entryClefType != ClefType::INVALID) {
        /// @todo visibility options
        Clef* clef = Factory::createClef(m_score->dummy()->segment());
        clef->setTrack(curTrackIdx);
        clef->setConcertClef(entryClefType);
        // c->setTransposingClef(entryClefType);
        // c->setShowCourtesy();
        // c->setForInstrumentChange();
        clef->setGenerated(false);
        clef->setIsHeader(false); /// @todo is this correct?
        // clef->setClefToBarlinePosition(ClefToBarlinePosition::BEFORE);

        segment = measure->getSegment(false ? SegmentType::HeaderClef : SegmentType::Clef, segment->tick());
        segment->add(clef);
    }

    segment = m_score->tick2measure(tickEnd)->getSegment(SegmentType::ChordRest, tickEnd)
}

static Fraction simpleMusxTimeSigToFraction(const std::pair<musx::util::Fraction, musx::dom::NoteType>& simpleMusxTimeSig, const FinaleLogger& logger)
{
    auto [count, noteType] = simpleMusxTimeSig;
    if (count.remainder()) {
        if ((Edu(noteType) % count.denominator()) == 0) {
            noteType = musx::dom::NoteType(Edu(noteType) / count.denominator());
            count *= count.denominator();
        } else {
            logger.logWarning(String::fromUtf8("Time signature has fractional portion that could not be reduced."));
            return Fraction(4, 4);
        }
    }
    return Fraction(count.quotient(),  musx::util::Fraction::fromEdu(Edu(noteType)).denominator());
}

static std::vector<ReadableTuplet> createTupletMap(std::vector<EntryFrame::TupletInfo> tupletInfo)
{
    const size_t n = tupletInfo.size();
    std::vector<ReadableTuplet> result;
    result.reserve(n);

    for (EntryFrame::TupletInfo tuplet : tupletInfo) {
        std::optional<Fraction> absBegin = musxFractionToFraction(tuplet->startDura);
        std::optional<Fraction> absDuration = musxFractionToFraction(tuplet->endDura - tuplet->startDura);
        result.emplace_back({
            absBegin ? absBegin.value() : Fraction(-1, 1),
            absDuration ? absDuration.value() : Fraction(-1, 1),
            tuplet->tuplet,
            nullptr,
            0,
            absBegin && absDuration
        });
    }

   for (size_t i = 0; i < n; ++i) {
        if (!result[i].valid) {
            continue;
        }

        for (size_t j = 0; j < n; ++j) {
            if(i == j || !result[j].valid) {
                continue;
            }

            if (result[i].absBegin >= result[j].absBegin
                && result[i].absBegin + result[i].absDuration <= result[j].absBegin + result[j].absDuration
                && (result[i].absBegin > result[j].absBegin
                || result[i].absBegin + result[i].absDuration < result[j].absBegin + result[j].absDuration)
            ) {
                result[i].layer = std::max(result[i].layer, result[j].layer + 1);
            }
        }
    }

    std::sort(result.begin(), result.end(), [](const ReadableTuplet& a, const ReadableTuplet& b) {
        if (a.absBegin != b.absBegin) {
            return a.absBegin < b.absBegin;
        }
        return a.layer < b.layer;
    });

    return result;
}

void EnigmaXmlImporter::importMeasures()
{
    // add default time signature
    Fraction currTimeSig = Fraction(4, 4);
    m_score->sigmap()->clear();
    m_score->sigmap()->add(0, currTimeSig);

    auto musxMeasures = m_doc->getOthers()->getArray<others::Measure>(SCORE_PARTID);
    // int counter = 0; // DBG
    for (const auto& musxMeasure : musxMeasures) {
        Fraction tick{ 0, 1 };
        auto lastMeasure = m_score->measures()->last();
        if (lastMeasure) {
            tick = lastMeasure->tick() + lastMeasure->ticks();
        }

        Measure* measure = Factory::createMeasure(m_score->dummy()->system());
        measure->setTick(tick);
        /// @todo eventually we need to import all the TimeSig features we can. Right now it's just the simplified case.
        auto musxTimeSig = musxMeasure->createTimeSignature();
        auto scoreTimeSig = simpleMusxTimeSigToFraction(musxTimeSig->calcSimplified(), logger());
        if (scoreTimeSig != currTimeSig) {
            m_score->sigmap()->add(tick.ticks(), scoreTimeSig);
            currTimeSig = scoreTimeSig;
        }
        measure->setTimesig(scoreTimeSig);
        measure->setTicks(scoreTimeSig);
        m_score->measures()->add(measure);

        /// @todo key signature
    }

    /// @todo maybe move this to separate function
    const TimeSigMap& sigmap = *m_score->sigmap();

    for (auto is = sigmap.cbegin(); is != sigmap.cend(); ++is) {
        const SigEvent& se = is->second;
        const int tick = is->first;
        Measure* m = m_score->tick2measure(Fraction::fromTicks(tick));
        if (!m) {
            continue;
        }
        Fraction newTimeSig = se.timesig();
        for (staff_idx_t staffIdx = 0; staffIdx < m_score->nstaves(); ++staffIdx) {
            Segment* seg = m->getSegment(SegmentType::TimeSig, Fraction::fromTicks(tick));
            TimeSig* ts = Factory::createTimeSig(seg);
            ts->setSig(newTimeSig);
            ts->setTrack(static_cast<int>(staffIdx) * VOICES);
            seg->add(ts);
        }
        if (newTimeSig != se.timesig()) {     // was a pickup measure - skip next timesig
            ++is;
        }
    }

    // Add entries (notes, rests, tuplets)
    auto musxStaves = m_doc->getOthers()->getArray<others::Staff>(SCORE_PARTID);
    Segment* segment = nullptr;
    for (const auto& musxStaff : musxStaves) {
        staff_idx_t curStaffIdx = muse::value(m_inst2Staff, InstCmper(musxStaff->getCmper()), muse::nidx);
        if (curStaffIdx == muse::nidx) {
            continue;
        }
        segment = m_score->firstSegment(SegmentType::ChordRest);
        for (const auto& musxMeasure : musxMeasures) {
            auto GFHold = musxMeasure->getDocument()->getDetails()->get<details::GFrameHold>(
                musxMeasure->getPartId(), musxStaff->getCmper(), musxMeasure->getCmper());
            if (!GFHold) {
                continue;
            }
            if (segment) {
                Segment* measureStartSegment = segment;
            }
            for (LayerIndex layer = 0; layer < MAX_LAYERS; layer++) {
                auto entryFrame = GFHold->createEntryFrame(layer, /*forWrittenPitch*/ false)
                if (!entryFrame) {
                    continue;
                }
                auto entries = entryFrame->getEntries();
                if (entries.empty()) {
                    continue;
                }

                /// @todo load (measure-specific) key signature from entryFrame->keySignature

                track_idx_t curTrackIdx = curStaffIdx * VOICES + static_cast<voice_idx_t>(layer);
                segment = measureStartSegment;
                std::vector<ReadableTuplet> tupletMap = createTupletMap(entryFrame->tupletInfo);
                size_t lastAddedTupletIndex = 0;
                for (const auto& entryInfo : entries) {
                    if (!processEntryInfo(entryInfo, curTrackIdx, &segment, &tupletMap, &lastAddedTupletIndex)) {
                        // fill expected duration with (invisible) rests using entryInfo->elapsedDuration
                        /// @todo sometimes we need to fill up to the start of the entry, sometimes we need to fill from the entry to the end of the entry (because of gaps).
                        // const std::vector<Rest*> rests = m_score->setRests(segment->tick(), curTrackIdx,
                                                                           // entryInfo->actualDuration, false, /*Tuplet, should be true sometimes*/nullptr);
                        // for (Rest* r : rests) {
                            // r->setVisible(false);
                        // }
                        break; // DBG
                    }
                }
            }
        }
    }
}

void EnigmaXmlImporter::importParts()
{
    auto scrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(SCORE_PARTID, BASE_SYSTEM_ID);

    int partNumber = 0;
    for (const auto& item : scrollView) {
        auto staff = item->getStaff();
        IF_ASSERT_FAILED(staff) {
            continue; // safety check
        }
        auto compositeStaff = others::StaffComposite::createCurrent(m_doc, SCORE_PARTID, staff->getCmper(), 1, 0);
        IF_ASSERT_FAILED(compositeStaff) {
            continue; // safety check
        }

        auto multiStaffInst = staff->getMultiStaffInstGroup();
        if (multiStaffInst && m_inst2Part.find(staff->getCmper()) != m_inst2Part.end()) {
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
        auto fullBaseName = staff->getFullInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        part->setPartName(QString::fromStdString(trimNewLineFromString(fullBaseName)));

        auto fullEffectiveName = compositeStaff->getFullInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        part->setLongName(QString::fromStdString(trimNewLineFromString(fullEffectiveName)));

        auto abrvName = compositeStaff->getAbbreviatedInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        part->setShortName(QString::fromStdString(trimNewLineFromString(abrvName)));

        if (multiStaffInst) {
            m_part2Inst.emplace(partId, multiStaffInst->staffNums);
            for (auto inst : multiStaffInst->staffNums) {
                if (auto instStaff = others::StaffComposite::createCurrent(m_doc, SCORE_PARTID, inst, 1, 0)) {
                    createStaff(part, instStaff, it);
                    m_inst2Part.emplace(inst, partId);
                }
            }
        } else {
            createStaff(part, compositeStaff, it);
            m_part2Inst.emplace(partId, std::vector<InstCmper>({ InstCmper(staff->getCmper()) }));
            m_inst2Part.emplace(staff->getCmper(), partId);
        }
        m_score->appendPart(part);
    }
}

void EnigmaXmlImporter::importBrackets()
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

    auto scorePartInfo = m_doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, SCORE_PARTID);
    if (!scorePartInfo) {
        throw std::logic_error("Unable to read PartDefinition for score");
        return;
    }
    auto scrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(SCORE_PARTID, BASE_SYSTEM_ID);

    auto staffGroups = details::StaffGroupInfo::getGroupsAtMeasure(1, scorePartInfo, scrollView);
    auto groupsByLayer = computeStaffGroupLayers(staffGroups);
    for (const auto& groupInfo : groupsByLayer) {
        IF_ASSERT_FAILED(groupInfo.info.startSlot && groupInfo.info.endSlot) {
            logger().logWarning(String::fromUtf8("Group info encountered without start or end slot information"));
            continue;
        }
        auto musxStartStaff = others::InstrumentUsed::getStaffAtIndex(scrollView, groupInfo.info.startSlot.value());
        auto musxEndStaff = others::InstrumentUsed::getStaffAtIndex(scrollView, groupInfo.info.endSlot.value());
        IF_ASSERT_FAILED(musxStartStaff && musxEndStaff) {
            logger().logWarning(String::fromUtf8("Group info encountered missing start or end staff information"));
            continue;
        }
        auto getStaffIdx = [&](InstCmper inst) -> std::optional<size_t> {
            auto it = m_inst2Staff.find(inst);
            IF_ASSERT_FAILED_X(it != m_inst2Staff.end(), "Musx inst value not found in m_inst2Staff") {
                return std::nullopt;
            }
            return it->second;
        };
        auto staffIdx = getStaffIdx(musxStartStaff->getCmper());
        IF_ASSERT_FAILED(staffIdx) {
            continue;
        }
        staff_idx_t startStaffIdx = staffIdx.value();
        BracketItem* bi = Factory::createBracketItem(m_score->dummy());
        bi->setBracketType(FinaleTConv::toMuseScoreBracketType(groupInfo.info.group->bracket->style));
        int groupSpan = groupInfo.info.endSlot.value() - groupInfo.info.startSlot.value() + 1;
        bi->setBracketSpan(groupSpan);
        bi->setColumn(size_t(groupInfo.layer));
        m_score->staff(startStaffIdx)->addBracket(bi);
        if (groupInfo.info.group->drawBarlines == details::StaffGroup::DrawBarlineStyle::ThroughStaves) {
            for (staff_idx_t idx = startStaffIdx; idx < startStaffIdx + groupSpan - 1; idx++) {
                Staff* s = m_score->staff(idx);
                s->setBarLineTo(0);
                s->setBarLineSpan(true);
            }
        }
    }
}

}
