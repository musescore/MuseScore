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
#include <array>
#include <vector>

#include "mnximporter.h"
#include "internal/shared/mnxtypesconv.h"

#include "engraving/dom/barline.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/score.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/volta.h"

#include "engraving/types/symnames.h"

using namespace mu::engraving;

namespace mu::iex::mnxio {
//---------------------------------------------------------
//   createDrumset
//   Build a MuseScore Drumset from an MNX part kit definition.
//---------------------------------------------------------

static Drumset* createDrumset(const mnx::Part& mnxPart, const mnx::Document& doc,
                              std::map<std::pair<size_t, std::string>, int>& kitComponentToMidi)
{
    if (!mnxPart.kit()) {
        return nullptr;
    }

    Drumset* drumset = new Drumset();
    Drumset* defaultDrumset = mu::engraving::smDrumset;
    for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
        drumset->drum(i) = DrumInstrument(String(), NoteHeadGroup::HEAD_INVALID, 0, DirectionV::AUTO);
    }

    const auto sounds = doc.global().sounds();
    const StaffType* percStaffType = StaffType::preset(StaffTypes::PERC_DEFAULT);
    /// @todo If MNX gains explicit staff type info, use it here instead of PERC_DEFAULT.
    const int middleLine = percStaffType ? percStaffType->middleLine() : 4;
    const size_t partIdx = mnxPart.calcArrayIndex();
    const auto kit = mnxPart.kit().value();
    struct KitEntry {
        std::string id;
        mnx::part::KitComponent component;
        int midiPitch = -1;
    };
    std::vector<KitEntry> kitEntries;
    kitEntries.reserve(kit.size());
    for (const auto& [kitId, kitComponent] : kit) {
        kitEntries.push_back({ kitId, kitComponent, -1 });
    }

    std::array<bool, DRUM_INSTRUMENTS> usedPitches {};
    const auto findFallbackPitch = [&usedPitches]() -> int {
        for (int pitch = 0; pitch < DRUM_INSTRUMENTS; ++pitch) {
            if (!usedPitches[pitch]) {
                return pitch;
            }
        }
        return -1;
    };
    for (auto& entry : kitEntries) {
        if (entry.component.sound() && sounds && sounds->contains(entry.component.sound().value())) {
            const auto sound = sounds->at(entry.component.sound().value());
            if (const auto midiNumber = sound.midiNumber()) {
                entry.midiPitch = static_cast<int>(midiNumber.value());
            }
        }

        if (pitchIsValid(entry.midiPitch)) {
            usedPitches[entry.midiPitch] = true;
        }
    }

    for (auto& entry : kitEntries) {
        if (pitchIsValid(entry.midiPitch)) {
            continue;
        }
        int fallbackPitch = defaultDrumset
                            ? defaultDrumset->defaultPitchForLine(middleLine - entry.component.staffPosition())
                            : -1;
        if (!pitchIsValid(fallbackPitch) || usedPitches[fallbackPitch]) {
            fallbackPitch = findFallbackPitch();
        }
        if (!pitchIsValid(fallbackPitch)) {
            LOGW() << "Kit component \"" << entry.id << "\" lacks a valid MIDI pitch and no fallback is available.";
            continue;
        }
        entry.midiPitch = fallbackPitch;
        usedPitches[fallbackPitch] = true;
        LOGW() << "Kit component \"" << entry.id << "\" lacks a valid MIDI pitch. Using fallback pitch "
               << fallbackPitch << ".";
    }

    for (const auto& entry : kitEntries) {
        if (!pitchIsValid(entry.midiPitch)) {
            continue;
        }
        kitComponentToMidi[{ partIdx, entry.id }] = entry.midiPitch;

        const bool hasDefault = defaultDrumset && defaultDrumset->isValid(entry.midiPitch);
        NoteHeadGroup notehead = hasDefault ? defaultDrumset->noteHead(entry.midiPitch) : NoteHeadGroup::HEAD_NORMAL;
        DirectionV stemDirection = hasDefault ? defaultDrumset->stemDirection(entry.midiPitch) : DirectionV::AUTO;
        int voice = hasDefault ? defaultDrumset->voice(entry.midiPitch) : 0;
        String shortcut = hasDefault ? defaultDrumset->shortcut(entry.midiPitch) : String();

        String name;
        if (entry.component.name()) {
            name = String::fromStdString(entry.component.name().value());
        }
        if (name.isEmpty() && entry.component.sound() && sounds && sounds->contains(entry.component.sound().value())) {
            const auto sound = sounds->at(entry.component.sound().value());
            if (const auto soundName = sound.name()) {
                name = String::fromStdString(soundName.value());
            }
        }
        if (name.isEmpty()) {
            // MuseScore requires a note name
            name = String(u"Percussion note");
        }

        int line = middleLine - entry.component.staffPosition();
        drumset->drum(entry.midiPitch) = DrumInstrument(name, notehead, line, stemDirection, -1, -1, voice, shortcut);
        if (notehead == NoteHeadGroup::HEAD_CUSTOM && hasDefault) {
            for (int type = 0; type < static_cast<int>(NoteHeadType::HEAD_TYPES); ++type) {
                drumset->drum(entry.midiPitch).noteheads[type]
                    =defaultDrumset->noteHeads(entry.midiPitch, NoteHeadType(type));
            }
        }
    }

    return drumset;
}

//---------------------------------------------------------
//   loadInstrument
//   Populate a MuseScore Instrument from an MNX part definition.
//---------------------------------------------------------

static void loadInstrument(const mnx::Document& doc, Part* part, const mnx::Part& mnxPart, Instrument* instrument,
                           std::map<std::pair<size_t, std::string>, int>& kitComponentToMidi)
{
    // Initialize drumset
    if (mnxPart.kit().has_value()) {
        instrument->setUseDrumset(true);
        Drumset* drumset = createDrumset(mnxPart, doc, kitComponentToMidi);
        if (drumset) {
            instrument->setDrumset(drumset);
            instrument->channel(0)->setBank(128);
        }
    } else {
        instrument->setUseDrumset(false);
    }

    // Names
    instrument->setTrackName(part->partName());
    instrument->setLongName(part->longName());
    instrument->setShortName(part->shortName());

    // Transposition
    // MNX transposition has opposite signs.
    if (const std::optional<mnx::part::PartTransposition> mnxTransp = mnxPart.transposition()) {
        instrument->setTranspose(Interval(-mnxTransp->interval().staffDistance(), -mnxTransp->interval().halfSteps()));
    }
}

//---------------------------------------------------------
//   mnxPartStaffToStaffIdx
//   Map MNX part/staff numbers to MuseScore staff indices.
//---------------------------------------------------------

staff_idx_t MnxImporter::mnxPartStaffToStaffIdx(const mnx::Part& mnxPart, int staffNum)
{
    staff_idx_t idx = muse::value(m_mnxPartStaffToStaff,
                                  std::make_pair(mnxPart.calcArrayIndex(), staffNum),
                                  muse::nidx);
    IF_ASSERT_FAILED(idx != muse::nidx) {
        throw std::logic_error("Unmapped staff encountered");
    }
    return idx;
}

//---------------------------------------------------------
//   mnxLayoutStaffToStaffIdx
//   Resolve MNX layout staff sources to MuseScore staff indices.
//---------------------------------------------------------

staff_idx_t MnxImporter::mnxLayoutStaffToStaffIdx(const mnx::layout::Staff& mnxStaff)
{
    const auto sources = mnxStaff.sources();
    for (const auto& source : sources) {
        if (const auto part = mnxDocument().getEntityMap().tryGet<mnx::Part>(source.part())) {
            return mnxPartStaffToStaffIdx(part.value(), source.staff());
        } else {
            LOGE() << "Staff source points to invalid part\"" << source.part() << "\" " << source.pointer().to_string();
            LOGE() << source.dump(2);
        }
    }
    return muse::nidx;
}

//---------------------------------------------------------
//   mnxMeasureToMeasure
//   Convert an MNX measure index to a MuseScore Measure pointer.
//---------------------------------------------------------

Measure* MnxImporter::mnxMeasureToMeasure(const size_t mnxMeasIdx)
{
    Fraction measTick = muse::value(m_mnxMeasToTick, mnxMeasIdx, { -1, 1 });
    IF_ASSERT_FAILED(measTick >= Fraction(0, 1)) {
        throw std::logic_error("MNX measure index " + std::to_string(mnxMeasIdx)
                               + " is not mapped.");
    }
    Measure* measure = m_score->tick2measure(measTick);
    IF_ASSERT_FAILED(measure) {
        throw std::logic_error("MNX measure index " + std::to_string(mnxMeasIdx)
                               + " has invalid tick " + measTick.toString().toStdString());
    }
    return measure;
}

//---------------------------------------------------------
//   mnxEventIdToCR
//   Look up a MuseScore ChordRest by MNX event id.
//---------------------------------------------------------

engraving::ChordRest* MnxImporter::mnxEventIdToCR(const std::string& eventId)
{
    const auto& docMapping = mnxDocument().getEntityMap();
    const auto event = docMapping.tryGet<mnx::sequence::Event>(eventId);
    if (!event.has_value()) {
        return nullptr;
    }
    return muse::value(m_mnxEventToCR, event->pointer().to_string());
}

//---------------------------------------------------------
//   mnxNoteIdToNote
//   Look up a MuseScore Note by MNX note id.
//---------------------------------------------------------

engraving::Note* MnxImporter::mnxNoteIdToNote(const std::string& noteId)
{
    const auto& docMapping = mnxDocument().getEntityMap();
    const auto note = docMapping.tryGet<mnx::sequence::NoteBase>(noteId);
    if (!note.has_value()) {
        return nullptr;
    }
    return muse::value(m_mnxNoteToNote, note->pointer().to_string());
}

//---------------------------------------------------------
//   setAndStyleProperty
//   Set a property and apply style flags respecting inheritance.
//---------------------------------------------------------

void MnxImporter::setAndStyleProperty(EngravingObject* e, Pid id, PropertyValue v)
{
    if (v.isValid()) {
        e->setProperty(id, v);
    }
    if (e->propertyFlags(id) == PropertyFlags::NOSTYLE) {
        return;
    }
    const bool canLeaveStyled = (e->getProperty(id) == e->propertyDefault(id));
    e->setPropertyFlags(id, canLeaveStyled ? PropertyFlags::STYLED : PropertyFlags::UNSTYLED);
}

//---------------------------------------------------------
//   mnxMeasurePosToTick
//   Convert an MNX measure-relative position to absolute tick.
//---------------------------------------------------------

Fraction MnxImporter::mnxMeasurePosToTick(const mnx::MeasureRhythmicPosition& measPos)
{
    const auto globalMeas = mnxDocument().getEntityMap().get<mnx::global::Measure>(measPos.measure());
    const size_t measIdx = globalMeas.calcArrayIndex();
    const Fraction measTick = muse::value(m_mnxMeasToTick, measIdx, Fraction(-1, 1));
    IF_ASSERT_FAILED(measTick.positive()) {
        throw std::logic_error("MNX global measure at " + std::to_string(measIdx) + " was not mapped.");
    }
    return measTick + toMuseScoreRTick(measPos.position());
}

//---------------------------------------------------------
//   importSettings
//   Import MNX score-level settings.
//---------------------------------------------------------

void MnxImporter::importSettings()
{
    /// @todo add settings as MNX adds them

    // MNX specifies that the barline of the last bar is a finale barline by default.
    // This appears always to be the case for MuseScore as well, so nothing needs to be done for this.
}

//---------------------------------------------------------
//   createStaff
//   Create a MuseScore staff from an MNX part/staff entry.
//---------------------------------------------------------

void MnxImporter::createStaff(Part* part, const mnx::Part& mnxPart, int staffNum)
{
    Staff* staff = Factory::createStaff(part);
    if (part->instrument()->useDrumset() && !staff->isDrumStaff(Fraction(0, 1))) {
        /// @todo If MNX gains explicit staff type/line-count info, use it here.
        staff->setStaffType(Fraction(0, 1), *StaffType::preset(StaffTypes::PERC_DEFAULT));
        staff->setDefaultClefType(ClefTypeList(ClefType::PERC2, ClefType::PERC2));
        staff->clefList().setClef(0, ClefTypeList(ClefType::PERC2, ClefType::PERC2));
    }
    m_score->appendStaff(staff);
    m_mnxPartStaffToStaff.emplace(std::make_pair(mnxPart.calcArrayIndex(), staffNum), staff->idx());
    m_StaffToMnxPart.emplace(staff->idx(), mnxPart.calcArrayIndex());
}

//---------------------------------------------------------
//   importParts
//   Create MuseScore parts, instruments, and staves from MNX parts.
//---------------------------------------------------------

void MnxImporter::importParts()
{
    size_t partNum = 0;
    for (const mnx::Part& mnxPart : mnxDocument().parts()) {
        partNum++;
        Part* part = new Part(m_score);
        /// @todo a better way to find the instrument, perhaps by part name or else some future mnx enhancement
        const InstrumentTemplate* it = [&]() {
            if (mnxPart.kit()) {
                return searchTemplate(u"drumset");
            }
            return searchTemplate(u"piano");
        }();
        if (it) {
            part->initFromInstrTemplate(it);
        }
        part->setPartName(String::fromStdString(mnxPart.name_or("Part " + mnxPart.id_or(std::to_string((partNum))))));
        part->setLongName(String::fromStdString(mnxPart.name_or("")));
        part->setShortName(String::fromStdString(mnxPart.shortName_or("")));
        loadInstrument(mnxDocument(), part, mnxPart, part->instrument(), m_mnxKitComponentToMidi);
        for (int staffNum = 1; staffNum <= mnxPart.staves(); staffNum++) {
            createStaff(part, mnxPart, staffNum);
        }
        m_score->appendPart(part);
    }
}

//---------------------------------------------------------
//   importBrackets
//   Import bracket and barline span information from MNX layouts.
//---------------------------------------------------------

void MnxImporter::importBrackets()
{
    auto fullScoreLayout = mnxDocument().findFullScoreLayout();
    if (!fullScoreLayout) {
        LOGI() << "Unable to find full score layout. Using default layout from parts.";
    }
    const auto layoutSpans = [&]() {
        if (fullScoreLayout) {
            if (const auto& layoutSpans = mnx::util::buildLayoutSpans(fullScoreLayout.value())) {
                return layoutSpans.value();
            }
            fullScoreLayout = std::nullopt;
        }
        return mnx::util::buildDefaultLayoutSpans(mnxDocument().parts());
    }();

    std::optional<std::vector<mnx::layout::Staff> > layoutStaves;
    if (fullScoreLayout) {
        layoutStaves = mnx::util::flattenLayoutStaves(fullScoreLayout.value());
        IF_ASSERT_FAILED(layoutStaves) {
            LOGE() << "Layout staves for full score layout were invalid.";
            return;
        }
    }

    for (const auto& span : layoutSpans) {
        BracketType brt = toMuseScoreBracketType(span.symbol.value_or(mnx::LayoutSymbol::NoSymbol));
        if (brt == BracketType::NO_BRACKET && span.startIndex >= span.endIndex) {
            continue;
        }
        const staff_idx_t staffIdx = layoutStaves
                                     ? mnxLayoutStaffToStaffIdx(layoutStaves->at(span.startIndex))
                                     : static_cast<staff_idx_t>(span.startIndex);
        if (staffIdx == muse::nidx) {
            LOGE() << "Staff not found for span starting at " << span.startIndex
                   << " and ending at " << span.endIndex << ".";
            continue;
        }
        BracketItem* bi = Factory::createBracketItem(m_score->dummy());
        bi->setBracketType(brt);
        const int groupSpan = static_cast<int>(span.endIndex - span.startIndex + 1);
        bi->setBracketSpan(groupSpan);
        bi->setColumn(size_t(span.depth));
        /// @todo as MNX adds barline options to groups, this will become more complicated.
        m_score->staff(staffIdx)->addBracket(bi);
        if (groupSpan > 1) {
            size_t currIndex = m_barlineSpans.size();
            m_barlineSpans.push_back(std::make_pair(staffIdx, staffIdx + static_cast<staff_idx_t>(groupSpan - 1)));
            // Barline defaults (these will be overridden later, but good to have nice defaults)
            for (staff_idx_t idx = staffIdx; idx < staffIdx + static_cast<staff_idx_t>(groupSpan - 1); idx++) {
                m_score->staff(idx)->setBarLineSpan(true);
                m_score->staff(idx)->setBarLineTo(0);
                m_staffToSpan.emplace(idx, currIndex);
            }
        }
    }
}

//---------------------------------------------------------
//   createKeySig
//   Create MuseScore key signatures for all staves at a measure.
//---------------------------------------------------------

void MnxImporter::createKeySig(engraving::Measure* measure, int keyFifths)
{
    const Key concertKey = toMuseScoreKey(keyFifths);
    if (concertKey == Key::INVALID) {
        LOGE() << "invalid mnx key fifths " << keyFifths << " for measure " << measure->measureIndex();
        return;
    }
    for (staff_idx_t idx = 0; idx < m_score->nstaves(); idx++) {
        Staff* staff = m_score->staff(idx);
        KeySigEvent keySigEvent;
        keySigEvent.setConcertKey(concertKey);
        keySigEvent.setKey(concertKey);
        if (!score()->style().styleB(Sid::concertPitch)) {
            const size_t mnxPartIndex = muse::value(m_StaffToMnxPart, idx, muse::nidx);
            IF_ASSERT_FAILED(mnxPartIndex != muse::nidx) {
                throw std::logic_error("Staff " + std::to_string(idx) + " is not mapped.");
            }
            const mnx::Part mnxPart = mnxDocument().parts()[mnxPartIndex];
            if (const std::optional<mnx::part::PartTransposition>& partTransposition = mnxPart.transposition()) {
                int transpFifths = partTransposition->calcTransposedKey(mnx::KeySignature::make(keyFifths)).fifths;
                const Key transpKey = toMuseScoreKey(transpFifths);
                if (transpKey != Key::INVALID) {
                    keySigEvent.setKey(transpKey);
                } else {
                    // measure has not been added to score yet, so use nmeasures to calculate the measure.
                    LOGW() << "invalid mnx transposed key fifths " << transpFifths
                           << " for measure at index " << m_score->nmeasures();
                    // set the document to concert pitch and let MuseScore deal with it.
                    m_score->style().set(Sid::concertPitch, true);
                }
            }
        }
        Segment* seg = measure->getSegmentR(SegmentType::KeySig, Fraction(0, 1));
        KeySig* ks = Factory::createKeySig(seg);
        ks->setKeySigEvent(keySigEvent);
        ks->setTrack(staff2track(idx));
        seg->add(ks);
        staff->setKey(measure->tick(), ks->keySigEvent());
    }
}

//---------------------------------------------------------
//   createTimeSig
//   Create MuseScore time signatures from MNX data.
//---------------------------------------------------------

void MnxImporter::createTimeSig(engraving::Measure* measure, const mnx::TimeSignature& timeSig)
{
    /// @todo Eventually, as mnx develops, we may get more sophisticated here than just a Fraction.
    const Fraction sigFraction = toMuseScoreFraction(timeSig);
    for (track_idx_t trackIdx = 0; trackIdx < m_score->ntracks(); trackIdx += VOICES) {
        Segment* seg = measure->getSegmentR(SegmentType::TimeSig, Fraction(0, 1));
        TimeSig* ts = Factory::createTimeSig(seg);
        ts->setSig(sigFraction);
        ts->setTrack(trackIdx);
        seg->add(ts);
    }
}

//---------------------------------------------------------
//   setBarline
//   Create MuseScore barlines matching the MNX barline type.
//---------------------------------------------------------

void MnxImporter::setBarline(engraving::Measure* measure, const mnx::global::Barline& barline)
{
    const mnx::BarlineType mnxBlt = barline.type();
    BarLineType blt = toMuseScoreBarLineType(mnxBlt);
    Segment* bls = measure->getSegmentR(SegmentType::EndBarLine, measure->ticks());

    for (staff_idx_t idx = 0; idx < m_score->staves().size(); idx++) {
        BarLine* bl = Factory::createBarLine(bls);
        bl->setParent(bls);
        bl->setTrack(staff2track(idx));
        bl->setVisible(mnxBlt != mnx::BarlineType::NoBarline);
        bl->setGenerated(false);
        bl->setBarLineType(blt);
        if (mnxBlt == mnx::BarlineType::Tick) {
            int lines = bl->staff()->lines(bls->tick() - Fraction::eps()) - 1;
            bl->setSpanFrom(BARLINE_SPAN_TICK1_FROM + (lines == 0 ? BARLINE_SPAN_1LINESTAFF_FROM : 0));
            bl->setSpanTo((lines == 0 ? BARLINE_SPAN_1LINESTAFF_FROM : (2 * -lines)) + 1);
        } else if (mnxBlt == mnx::BarlineType::Short) {
            bl->setSpanFrom(BARLINE_SPAN_SHORT1_FROM);
            bl->setSpanTo(BARLINE_SPAN_SHORT1_TO);
        } else {
            bl->setSpanStaff(m_staffToSpan.find(idx) != m_staffToSpan.end());
            bl->setSpanFrom(0);
            bl->setSpanTo(0);
        }
        bls->add(bl);
    }
}

//---------------------------------------------------------
//   createVolta
//   Create MuseScore volta (ending) spanning measures.
//---------------------------------------------------------

void MnxImporter::createVolta(engraving::Measure* measure, const mnx::global::Ending& ending)
{
    constexpr track_idx_t voltaTrackIdx = 0; /// @todo more options as indicated by mnx spec.

    Measure* endMeasure = measure;
    for (int countdown = ending.duration() - 1; countdown > 0; countdown--) {
        Measure* next = endMeasure->nextMeasure();
        if (!next) {
            LOGW() << "Ending at " << ending.pointer().to_string() << " specifies non-existent end measure.";
            LOGW() << ending.dump(2);
        }
        endMeasure = next;
    }

    Volta* volta = Factory::createVolta(m_score->dummy());
    volta->setTrack(voltaTrackIdx);
    volta->setTick(measure->tick());
    volta->setTick2(endMeasure->endTick());
    volta->setVisible(true);
    if (const auto& numbers = ending.numbers()) {
        volta->setEndings(numbers->toStdVector());
        // use default MuseScore ending text format, based on observed defaults in 4.6.x
        String text;
        for (int number : *numbers) {
            if (!text.empty()) {
                text += u", ";
            }
            text += String("%1").arg(number);
        }
        text += u".";
        volta->setText(text);
    }
    volta->setVoltaType(ending.open() ? Volta::Type::OPEN : Volta::Type::CLOSED);
    m_score->addElement(volta);
}

//---------------------------------------------------------
//   createJumpOrMarker
//   MuseScore does not allow jumps or markers to be assigned to a measure location.
//   Passed in for completeness to the MNX spec but currently unused by engraving.
//---------------------------------------------------------

void MnxImporter::createJumpOrMarker(engraving::Measure* measure, const mnx::FractionValue&,
                                     std::variant<JumpType, MarkerType> type,
                                     const std::optional<std::string> glyphName)
{
    constexpr track_idx_t curTrackIdx = 0; /// @todo more options as offered by new versions of mnx spec.

    const ElementType elementType = std::holds_alternative<JumpType>(type)
                                    ? ElementType::JUMP
                                    : ElementType::MARKER;

    TextBase* item = toTextBase(Factory::createItem(elementType, measure));
    item->setParent(measure);
    item->setTrack(curTrackIdx);

    std::visit([&](const auto& v) {
        using T = std::decay_t<decltype(v)>;

        if constexpr (std::is_same_v<T, JumpType>) {
            IF_ASSERT_FAILED(item->isJump()) {
                throw std::logic_error("Variant is JumpType but created item is not a Jump.");
            }
            IF_ASSERT_FAILED(v != JumpType::USER) {
                throw std::logic_error("JumpType USER not supported.");
            }
            toJump(item)->setJumpType(v);
        } else if constexpr (std::is_same_v<T, MarkerType>) {
            IF_ASSERT_FAILED(item->isMarker()) {
                throw std::logic_error("Variant is MarkerType but created item is not a Marker.");
            }
            toMarker(item)->setMarkerType(v);
        } else {
            static_assert(!sizeof(T), "Unhandled std::variant alternative in createJumpOrMarker");
        }
    }, type);

    if (item->textStyleType() == TextStyleType::REPEAT_RIGHT) {
        // MuseScore should do this but doesn't, at least for Fine.
        item->setPosition(m_score->style().value(Sid::repeatRightPosition).value<AlignH>());
    }

    if (glyphName) {
        if (SymNames::symIdByName(glyphName.value()) != SymId::noSym) {
            item->setXmlText(String(u"<sym>%1</sym>").arg(String::fromStdString(glyphName.value())));
        }
    }

    measure->add(item);
}

//---------------------------------------------------------
//   createTempoMark
//   Create MuseScore tempo text from an MNX tempo element.
//---------------------------------------------------------

void MnxImporter::createTempoMark(engraving::Measure* measure, const mnx::global::Tempo& tempo)
{
    /// @todo more options as offered by new versions of mnx spec.
    constexpr track_idx_t curTrackIdx = 0;

    Fraction rTick(0, 1);
    if (const auto& location = tempo.location()) {
        rTick = toMuseScoreFraction(location->fraction());
    }
    Segment* s = measure->getChordRestOrTimeTickSegment(measure->tick() + rTick);

    TempoText* item = Factory::createTempoText(s);
    item->setParent(s);
    item->setTrack(curTrackIdx);

    mnx::FractionValue noteValueInQuarters = tempo.value() / mnx::FractionValue(1, 4);
    const double bps = (noteValueInQuarters.toDouble() * tempo.bpm()) / 60.0;
    item->setTempo(bps);

    String tempoText = TempoText::duration2tempoTextString(toMuseScoreDuration(tempo.value()));
    tempoText += String(u" = %1").arg(tempo.bpm());
    item->setXmlText(tempoText);

    s->add(item);
}

//---------------------------------------------------------
//   importGlobalMeasures
//   Build MuseScore measures and global items from MNX global section.
//---------------------------------------------------------

void MnxImporter::importGlobalMeasures()
{
    Fraction currTimeSig(4, 4);
    m_score->sigmap()->clear();
    m_score->sigmap()->add(0, currTimeSig);

    // pass 1 creates the measures as it goes
    int lastDisplayNum = 0;
    for (const mnx::global::Measure& mnxMeasure : mnxDocument().global().measures()) {
        const bool isFirst = mnxMeasure.calcArrayIndex() == 0;
        Measure* measure = Factory::createMeasure(m_score->dummy()->system());
        Fraction tick(m_score->last() ? m_score->last()->endTick() : Fraction(0, 1));
        measure->setTick(tick);
        if (const std::optional<mnx::TimeSignature>& mnxTimeSig = mnxMeasure.time()) {
            Fraction thisTimeSig = toMuseScoreFraction(mnxTimeSig.value());
            if (!thisTimeSig.identical(currTimeSig)) {
                m_score->sigmap()->add(tick.ticks(), thisTimeSig);
                currTimeSig = thisTimeSig;
            }
            createTimeSig(measure, mnxTimeSig.value());
        }
        if (const std::optional<mnx::KeySignature>& keySig = mnxMeasure.key(); keySig || isFirst) {
            const int keyFifths = keySig ? keySig->fifths() : 0;
            createKeySig(measure, keyFifths);
        }
        if (const std::optional<mnx::global::Barline>& barline = mnxMeasure.barline()) {
            setBarline(measure, barline.value());
        }
        if (mnxMeasure.repeatStart()) {
            measure->setRepeatStart(true);
        }
        if (const std::optional<mnx::global::RepeatEnd>& rpt = mnxMeasure.repeatEnd()) {
            measure->setRepeatEnd(true);
            if (const std::optional<int> nTimes = rpt->times()) {
                measure->setRepeatCount(nTimes.value());
            }
        }
        if (const std::optional<mnx::global::Fine>& fine = mnxMeasure.fine()) {
            createJumpOrMarker(measure, fine->location().fraction(), MarkerType::FINE);
        }
        if (const std::optional<mnx::global::Jump>& jump = mnxMeasure.jump()) {
            createJumpOrMarker(measure, jump->location().fraction(), toMuseScoreJumpType(jump->type()));
        }
        if (const std::optional<mnx::global::Segno>& segno = mnxMeasure.segno()) {
            createJumpOrMarker(measure, segno->location().fraction(), MarkerType::SEGNO, segno->glyph());
        }
        if (const std::optional<mnx::Array<mnx::global::Tempo> >& tempos = mnxMeasure.tempos()) {
            for (const auto& tempo : tempos.value()) {
                createTempoMark(measure, tempo);
            }
        }

        /// @todo MNX currently offers no way to exclude a measure from having
        /// a measure number.
        int currDisplayNum = mnxMeasure.calcVisibleNumber();
        if (currDisplayNum != lastDisplayNum + 1) {
            measure->setNoOffset(currDisplayNum - lastDisplayNum - 1);
        }
        lastDisplayNum = currDisplayNum;

        measure->setTimesig(currTimeSig);
        measure->setTicks(currTimeSig);
        m_score->measures()->append(measure);
        m_mnxMeasToTick.emplace(mnxMeasure.calcArrayIndex(), tick);
    }

    // pass 2 for items that require all measures to exist already
    for (const mnx::global::Measure& mnxMeasure : mnxDocument().global().measures()) {
        Measure* measure = mnxMeasureToMeasure(mnxMeasure.calcArrayIndex());
        if (const std::optional<mnx::global::Ending>& ending = mnxMeasure.ending()) {
            createVolta(measure, ending.value());
        }
    }
}

//---------------------------------------------------------
//   createClefs
//   Import clefs for a part's staves within a measure.
//---------------------------------------------------------

void MnxImporter::createClefs(const mnx::Part& mnxPart, const mnx::Array<mnx::part::PositionedClef>& mnxClefs,
                              engraving::Measure* measure)
{
    /// @todo honor the MNX clef glyph if MuseScore ever allows it.
    for (const mnx::part::PositionedClef& mnxClef : mnxClefs) {
        staff_idx_t staffIdx = mnxPartStaffToStaffIdx(mnxPart, mnxClef.staff());
        Fraction rTick{};
        if (const std::optional<mnx::RhythmicPosition>& position = mnxClef.position()) {
            rTick = toMuseScoreFraction(position->fraction()).reduced();
        }
        ClefType clefType = toMuseScoreClefType(mnxClef.clef());
        if (clefType != ClefType::INVALID) {
            const bool isHeader = !measure->prevMeasure() && rTick.isZero();
            Segment* clefSeg = measure->getSegmentR(isHeader ? SegmentType::HeaderClef : SegmentType::Clef, rTick);
            Clef* clef = Factory::createClef(clefSeg);
            clef->setTrack(staff2track(staffIdx));
            clef->setConcertClef(clefType);
            clef->setTransposingClef(clefType);
            clef->setGenerated(false);
            clef->setIsHeader(isHeader);
            clefSeg->add(clef);
        } else {
            LOGE() << "Unsupported clef encountered at " << mnxClef.pointer().to_string();
        }
    }
}

//---------------------------------------------------------
//   importMnx
//   Main entry to import an MNX document into a MuseScore score.
//---------------------------------------------------------

void MnxImporter::importMnx()
{
    if (!m_mnxDocument.hasEntityMap()) {
        auto policies = mnx::EntityMapPolicies();
        policies.ottavasRespectGraceTargets = false;    // MuseScore can't target grace notes with ottavas
        policies.ottavasRespectVoiceTargets = false;    // MuseScore can't target voices with ottavas
        m_mnxDocument.buildEntityMap(policies);
    }
    if (const auto& support = m_mnxDocument.mnx().support()) {
        m_useBeams = support->useBeams();
        m_useAccidentalDisplay = support->useAccidentalDisplay();
    }
    importSettings();
    importParts();
    importBrackets();
    importGlobalMeasures();
    importPartMeasures();
}
} // namespace mu::iex::mnxio
