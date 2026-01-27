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
#include <algorithm>
#include <unordered_map>

#include "engraving/dom/clef.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/noteval.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/utils.h"
#include "framework/global/containers.h"

#include "mnxtypesconv.h"

using namespace mu::engraving;

namespace mu::iex::mnxio {
namespace {
const std::unordered_map<mnx::BarlineType, BarLineType> barLineTypeTable = {
    { mnx::BarlineType::Regular,    BarLineType::NORMAL },
    { mnx::BarlineType::Dashed,     BarLineType::DASHED },
    { mnx::BarlineType::Dotted,     BarLineType::DOTTED },
    { mnx::BarlineType::Double,     BarLineType::DOUBLE },
    { mnx::BarlineType::Final,      BarLineType::END },
    { mnx::BarlineType::Heavy,      BarLineType::HEAVY },
    { mnx::BarlineType::HeavyHeavy, BarLineType::DOUBLE_HEAVY },
    { mnx::BarlineType::HeavyLight, BarLineType::REVERSE_END },
};
} // namespace

BarLineType toMuseScoreBarLineType(mnx::BarlineType blt)
{
    return muse::value(barLineTypeTable, blt, BarLineType::NORMAL);
}

mnx::BarlineType toMnxBarLineType(BarLineType blt)
{
    return muse::key(barLineTypeTable, blt, mnx::BarlineType::Regular);
}

std::optional<mnx::TimeSignatureUnit> toMnxTimeSignatureUnit(int denominator)
{
    static const std::unordered_map<int, std::optional<mnx::TimeSignatureUnit> > units = {
        { 1, mnx::TimeSignatureUnit::Whole },
        { 2, mnx::TimeSignatureUnit::Half },
        { 4, mnx::TimeSignatureUnit::Quarter },
        { 8, mnx::TimeSignatureUnit::Eighth },
        { 16, mnx::TimeSignatureUnit::Value16th },
        { 32, mnx::TimeSignatureUnit::Value32nd },
        { 64, mnx::TimeSignatureUnit::Value64th },
        { 128, mnx::TimeSignatureUnit::Value128th },
    };
    return muse::value(units, denominator, std::nullopt);
}

std::optional<mnx::part::Clef::Required> toMnxClef(ClefType clefType)
{
    using Required = mnx::part::Clef::Required;
    using ClefSign = mnx::ClefSign;
    using OttavaAmountOrZero = mnx::OttavaAmountOrZero;

    ClefSign sign{};
    OttavaAmountOrZero octave = OttavaAmountOrZero::NoTransposition;

    switch (clefType) {
    case ClefType::G:
    case ClefType::G_1:
        sign = ClefSign::GClef;
        break;
    case ClefType::G8_VB:
    case ClefType::G8_VB_O:
    case ClefType::G8_VB_P:
    case ClefType::G8_VB_C:
        sign = ClefSign::GClef;
        octave = OttavaAmountOrZero::OctaveDown;
        break;
    case ClefType::G8_VA:
        sign = ClefSign::GClef;
        octave = OttavaAmountOrZero::OctaveUp;
        break;
    case ClefType::G15_MB:
        sign = ClefSign::GClef;
        octave = OttavaAmountOrZero::TwoOctavesDown;
        break;
    case ClefType::G15_MA:
        sign = ClefSign::GClef;
        octave = OttavaAmountOrZero::TwoOctavesUp;
        break;
    case ClefType::F:
    case ClefType::F_B:
    case ClefType::F_C:
    case ClefType::F_F18C:
    case ClefType::F_19C:
        sign = ClefSign::FClef;
        break;
    case ClefType::F8_VB:
        sign = ClefSign::FClef;
        octave = OttavaAmountOrZero::OctaveDown;
        break;
    case ClefType::F_8VA:
        sign = ClefSign::FClef;
        octave = OttavaAmountOrZero::OctaveUp;
        break;
    case ClefType::F15_MB:
        sign = ClefSign::FClef;
        octave = OttavaAmountOrZero::TwoOctavesDown;
        break;
    case ClefType::F_15MA:
        sign = ClefSign::FClef;
        octave = OttavaAmountOrZero::TwoOctavesUp;
        break;
    case ClefType::C1:
    case ClefType::C2:
    case ClefType::C3:
    case ClefType::C4:
    case ClefType::C5:
    case ClefType::C_19C:
    case ClefType::C1_F18C:
    case ClefType::C3_F18C:
    case ClefType::C4_F18C:
    case ClefType::C1_F20C:
    case ClefType::C3_F20C:
    case ClefType::C4_F20C:
        sign = ClefSign::CClef;
        break;
    case ClefType::C4_8VB:
        sign = ClefSign::CClef;
        octave = OttavaAmountOrZero::OctaveDown;
        break;
    case ClefType::INVALID:
    case ClefType::PERC:
    case ClefType::PERC2:
    case ClefType::TAB:
    case ClefType::TAB4:
    case ClefType::TAB_SERIF:
    case ClefType::TAB4_SERIF:
    case ClefType::MAX:
        return std::nullopt;
    }

    const int staffPosition = (ClefInfo::line(clefType) - 3) * 2;
    return Required { sign, staffPosition, octave };
}

std::optional<mnx::sequence::Pitch::Required> toMnxPitch(const Note* note)
{
    IF_ASSERT_FAILED(note) {
        return std::nullopt;
    }

    const Staff* staff = note->staff();
    const Fraction tick = note->tick();
    const ClefType clefType = staff->clef(tick);

    if (clefType == ClefType::PERC || clefType == ClefType::PERC2 || staff->isTabStaff(tick)) {
        /// @todo handle tab when MNX supports it
        /// @todo percussion notes will convert to kitNotes (probably elsewhere)
        return std::nullopt;
    }

    // MNX: export CONCERT pitch.
    // In MuseScore, note->tpc1() represents the concert spelling.
    const int concertTpc = note->tpc1();
    const int step  = tpc2step(concertTpc);
    const int alter = tpc2alterByKey(concertTpc, Key::C);

    // Use concert MIDI pitch as the base for octave computation.
    // (Applying instrument->transpose() here tends to double-count transposition.)
    const int basePitch = note->pitch();
    int octave = (basePitch - alter) / PITCH_DELTA_OCTAVE - 1;

    // Correct for ottava lines: ppitch - pitch in semitones.
    // Convert to octave units, clamp to [-3, +3] (8va/15ma/22ma).
    const int pitchDelta = note->ppitch() - note->pitch();
    if ((pitchDelta % PITCH_DELTA_OCTAVE) == 0) {
        int octaveShift = pitchDelta / PITCH_DELTA_OCTAVE;
        octaveShift = std::clamp(octaveShift, -3, 3);
        octave += octaveShift;
    } else {
        LOGW() << "Ignored non-octave playback displacement when computing MNX pitch: "
               << "tick=" << tick.ticks()
               << ", pitch=" << note->pitch()
               << ", ppitch=" << note->ppitch()
               << ", delta=" << pitchDelta
               << " (not a multiple of " << PITCH_DELTA_OCTAVE << " semitones).";
    }

    return mnx::sequence::Pitch::make(static_cast<mnx::NoteStep>(step), octave, alter);
}

BeamMode toMuseScoreBeamMode(int lowestBeamStart)
{
    static const std::unordered_map<int, BeamMode> beamModeTable = {
        { 0,    BeamMode::MID },
        { 1,    BeamMode::BEGIN },
        { 2,    BeamMode::BEGIN16 },
        { 3,    BeamMode::BEGIN32 },
        // MNX can go arbitrarily higher but MuseScore currently cannot
    };
    return muse::value(beamModeTable, lowestBeamStart, BeamMode::MID);
}

namespace {
const std::unordered_map<mnx::LayoutSymbol, BracketType> layoutSymbolTable = {
    { mnx::LayoutSymbol::NoSymbol,   BracketType::NO_BRACKET },
    { mnx::LayoutSymbol::Brace,      BracketType::BRACE },
    { mnx::LayoutSymbol::Bracket,    BracketType::NORMAL },
};
} // namespace

BracketType toMuseScoreBracketType(mnx::LayoutSymbol lys)
{
    return muse::value(layoutSymbolTable, lys, BracketType::NO_BRACKET);
}

mnx::LayoutSymbol toMnxLayoutSymbol(BracketType bracketType)
{
    return muse::key(layoutSymbolTable, bracketType, mnx::LayoutSymbol::NoSymbol);
}

namespace {
const std::unordered_map<mnx::BreathMarkSymbol, SymId> breathMarkTable = {
    { mnx::BreathMarkSymbol::Comma,     SymId::breathMarkComma },
    { mnx::BreathMarkSymbol::Tick,      SymId::breathMarkTick },
    { mnx::BreathMarkSymbol::Upbow,     SymId::breathMarkUpbow },
    { mnx::BreathMarkSymbol::Salzedo,   SymId::breathMarkSalzedo },
};
} // namespace

SymId toMuseScoreBreathMarkSym(std::optional<mnx::BreathMarkSymbol> brSym)
{
    using BreathMark = mnx::BreathMarkSymbol;
    return muse::value(breathMarkTable, brSym.value_or(BreathMark::Comma), SymId::breathMarkComma);
}

std::optional<mnx::BreathMarkSymbol> toMnxBreathMarkSym(SymId sym)
{
    return muse::key(breathMarkTable, sym, std::optional<mnx::BreathMarkSymbol> {});
}

DynamicType toMuseScoreDynamicType(const String& glyph)
{
    // Currently there is very little clarity around dynamics in mnx.
    // This will likely change considerably as the details emerge.
    static const std::unordered_map<String, DynamicType> dynamicTypes {
        { u"<sym>dynamicPPPPPP</sym>",              DynamicType::PPPPPP },
        { u"<sym>dynamicPPPPP</sym>",               DynamicType::PPPPP },
        { u"<sym>dynamicPPPP</sym>",                DynamicType::PPPP },
        { u"<sym>dynamicPPP</sym>",                 DynamicType::PPP },
        { u"<sym>dynamicPP</sym>",                  DynamicType::PP },
        { u"<sym>dynamicP</sym>",                   DynamicType::P },
        { u"<sym>dynamicMP</sym>",                  DynamicType::MP },
        { u"<sym>dynamicMF</sym>",                  DynamicType::MF },
        { u"<sym>dynamicPF</sym>",                  DynamicType::PF },
        { u"<sym>dynamicF</sym>",                   DynamicType::F },
        { u"<sym>dynamicFF</sym>",                  DynamicType::FF },
        { u"<sym>dynamicFFF</sym>",                 DynamicType::FFF },
        { u"<sym>dynamicFFFF</sym>",                DynamicType::FFFF },
        { u"<sym>dynamicFFFFF</sym>",               DynamicType::FFFFF },
        { u"<sym>dynamicFFFFFF</sym>",              DynamicType::FFFFFF },
        { u"<sym>dynamicFortePiano</sym>",          DynamicType::FP },
        { u"<sym>dynamicForzando</sym>",            DynamicType::FZ },
        { u"<sym>dynamicSforzando1</sym>",          DynamicType::SF },
        { u"<sym>dynamicSforzandoPiano</sym>",      DynamicType::SFP },
        { u"<sym>dynamicSforzandoPianissimo</sym>", DynamicType::SFPP },
        { u"<sym>dynamicSforzato</sym>",            DynamicType::SFZ },
        { u"<sym>dynamicSforzatoPiano</sym>",       DynamicType::SFZ }, // SFZP does not exist
        { u"<sym>dynamicSforzatoFF</sym>",          DynamicType::SFFZ },
        { u"<sym>dynamicRinforzando1</sym>",        DynamicType::RF },
        { u"<sym>dynamicRinforzando2</sym>",        DynamicType::RFZ },
    };
    return muse::value(dynamicTypes, glyph, DynamicType::OTHER);
}

namespace {
const std::unordered_map<mnx::NoteValueBase, DurationType> duraTypeTable = {
    { mnx::NoteValueBase::Note4096th,   DurationType::V_INVALID },
    { mnx::NoteValueBase::Note2048th,   DurationType::V_INVALID },
    { mnx::NoteValueBase::Note1024th,   DurationType::V_1024TH },
    { mnx::NoteValueBase::Note512th,    DurationType::V_512TH },
    { mnx::NoteValueBase::Note256th,    DurationType::V_256TH },
    { mnx::NoteValueBase::Note128th,    DurationType::V_128TH },
    { mnx::NoteValueBase::Note64th,     DurationType::V_64TH },
    { mnx::NoteValueBase::Note32nd,     DurationType::V_32ND },
    { mnx::NoteValueBase::Note16th,     DurationType::V_16TH },
    { mnx::NoteValueBase::Eighth,       DurationType::V_EIGHTH },
    { mnx::NoteValueBase::Quarter,      DurationType::V_QUARTER },
    { mnx::NoteValueBase::Half,         DurationType::V_HALF },
    { mnx::NoteValueBase::Whole,        DurationType::V_WHOLE },
    { mnx::NoteValueBase::Breve,        DurationType::V_BREVE },
    { mnx::NoteValueBase::Longa,        DurationType::V_LONG },
    { mnx::NoteValueBase::Maxima,       DurationType::V_INVALID },
    { mnx::NoteValueBase::DuplexMaxima, DurationType::V_INVALID },
};
} // namespace

DurationType toMuseScoreDurationType(mnx::NoteValueBase nvb)
{
    return muse::value(duraTypeTable, nvb, DurationType::V_INVALID);
}

TDuration toMuseScoreDuration(mnx::NoteValue nv)
{
    if (nv.dots() > MAX_DOTS) {
        return TDuration(); // invalid
    }
    return TDuration(DurationTypeWithDots(toMuseScoreDurationType(nv.base()), nv.dots()));
}

std::optional<mnx::NoteValue::Required> toMnxNoteValue(const TDuration& duration)
{
    const auto base = muse::key(duraTypeTable, duration.type(), std::optional<mnx::NoteValueBase> {});
    if (!base) {
        return std::nullopt;
    }

    return mnx::NoteValue::make(*base, static_cast<unsigned>(duration.dots()));
}

namespace {
/// @todo Grow this table as MNX grows it.
static const std::unordered_map<mnx::JumpType, JumpType> jumpTable = {
    { mnx::JumpType::DsAlFine,      JumpType::DS_AL_FINE },
    { mnx::JumpType::Segno,         JumpType::DSS },
};
} // namespace

JumpType toMuseScoreJumpType(mnx::JumpType jt)
{
    return muse::value(jumpTable, jt, JumpType::USER);
}

std::optional<mnx::JumpType> toMnxJumpType(JumpType jt)
{
    return muse::key(jumpTable, jt, std::optional<mnx::JumpType> {});
}

namespace {
static const std::unordered_map<mnx::LyricLineType, LyricsSyllabic> lineTypeTable = {
    { mnx::LyricLineType::Whole,        LyricsSyllabic::SINGLE },
    { mnx::LyricLineType::Start,        LyricsSyllabic::BEGIN },
    { mnx::LyricLineType::Middle,       LyricsSyllabic::MIDDLE },
    { mnx::LyricLineType::End,          LyricsSyllabic::END },
};
} // namespace

LyricsSyllabic toMuseScoreLyricsSyllabic(mnx::LyricLineType llt)
{
    return muse::value(lineTypeTable, llt, LyricsSyllabic::SINGLE);
}

mnx::LyricLineType toMnxLyricLineType(LyricsSyllabic ls)
{
    return muse::key(lineTypeTable, ls, mnx::LyricLineType::Whole);
}

namespace {
const std::unordered_map<mnx::OttavaAmount, OttavaType> ottavaTypeTable = {
    { mnx::OttavaAmount::OctaveDown,       OttavaType::OTTAVA_8VB },
    { mnx::OttavaAmount::OctaveUp,         OttavaType::OTTAVA_8VA },
    { mnx::OttavaAmount::TwoOctavesDown,   OttavaType::OTTAVA_15MB },
    { mnx::OttavaAmount::TwoOctavesUp,     OttavaType::OTTAVA_15MA },
    { mnx::OttavaAmount::ThreeOctavesDown, OttavaType::OTTAVA_22MB },
    { mnx::OttavaAmount::ThreeOctavesUp,   OttavaType::OTTAVA_22MA },
};
} // namespace

OttavaType toMuseScoreOttavaType(mnx::OttavaAmount ottavaAmount)
{
    return muse::value(ottavaTypeTable, ottavaAmount, OttavaType::OTTAVA_8VA);
}

std::optional<mnx::OttavaAmount> toMnxOttavaAmount(OttavaType ottavaType)
{
    return muse::key(ottavaTypeTable, ottavaType, std::optional<mnx::OttavaAmount> {});
}

Fraction toMuseScoreRTick(const mnx::RhythmicPosition& position)
{
    return toMuseScoreFraction(position.fraction());
}

namespace {
const std::unordered_map<int, TremoloType> tremoloTypeTable = {
    { 1,     TremoloType::C8 },
    { 2,     TremoloType::C16 },
    { 3,     TremoloType::C32 },
    { 4,     TremoloType::C64 },
};
} // namespace

TremoloType toMuseScoreTremoloType(int numberOfBeams)
{
    return muse::value(tremoloTypeTable, numberOfBeams, TremoloType::INVALID_TREMOLO);
}

std::optional<int> toMnxTremoloMarks(TremoloType tt)
{
    return muse::key(tremoloTypeTable, tt, std::optional<int> {});
}

namespace {
const std::unordered_map<mnx::LineType, SlurStyleType> slurStyleTable = {
    { mnx::LineType::Dashed,        SlurStyleType::Dashed },
    { mnx::LineType::Dotted,        SlurStyleType::Dotted },
    { mnx::LineType::Solid,         SlurStyleType::Solid },
};
} // namespace

SlurStyleType toMuseScoreSlurStyleType(mnx::LineType lineType)
{
    return muse::value(slurStyleTable, lineType, SlurStyleType::Solid);
}

mnx::LineType toMnxSlurLineType(SlurStyleType sst)
{
    return muse::key(slurStyleTable, sst, mnx::LineType::Solid);
}

namespace {
const std::unordered_map<mnx::AutoYesNo, TupletBracketType> tupletBracketTypeTable = {
    { mnx::AutoYesNo::Auto,     TupletBracketType::AUTO_BRACKET },
    { mnx::AutoYesNo::Yes,      TupletBracketType::SHOW_BRACKET },
    { mnx::AutoYesNo::No,       TupletBracketType::SHOW_NO_BRACKET },
};
} // namespace

TupletBracketType toMuseScoreTupletBracketType(mnx::AutoYesNo bracketOption)
{
    return muse::value(tupletBracketTypeTable, bracketOption, TupletBracketType::AUTO_BRACKET);
}

mnx::AutoYesNo toMnxTupletBracketType(TupletBracketType bracketOption)
{
    return muse::key(tupletBracketTypeTable, bracketOption, mnx::AutoYesNo::Auto);
}

namespace {
const std::unordered_map<mnx::TupletDisplaySetting, TupletNumberType> tupletNumberTypeTable = {
    { mnx::TupletDisplaySetting::Inner,     TupletNumberType::SHOW_NUMBER },
    { mnx::TupletDisplaySetting::NoNumber,  TupletNumberType::NO_TEXT },
    { mnx::TupletDisplaySetting::Both,      TupletNumberType::SHOW_RELATION },
};
} // namespace

TupletNumberType toMuseScoreTupletNumberType(mnx::TupletDisplaySetting numberStyle)
{
    return muse::value(tupletNumberTypeTable, numberStyle, TupletNumberType::SHOW_NUMBER);
}

mnx::TupletDisplaySetting toMnxTupletNumberType(TupletNumberType numberStyle)
{
    return muse::key(tupletNumberTypeTable, numberStyle, mnx::TupletDisplaySetting::Inner);
}

NoteVal toMuseScoreNoteVal(const mnx::sequence::Pitch::Required& pitch, Key key, int octaveShift)
{
    int step = static_cast<int>(pitch.step);
    int alteration = pitch.alter;
    NoteVal nval;
    nval.pitch = 60 /*middle C*/ + (octaveShift + pitch.octave - 4) * PITCH_DELTA_OCTAVE + step2pitch(step) + alteration;
    if (alteration < int(AccidentalVal::MIN) || alteration > int(AccidentalVal::MAX) || !pitchIsValid(nval.pitch)) {
        nval.pitch = clampPitch(nval.pitch);
        nval.tpc1 = pitch2tpc(nval.pitch, key, Prefer::NEAREST);
    } else {
        nval.tpc1 = step2tpc(step, AccidentalVal(alteration));
    }
    nval.tpc2 = nval.tpc1;
    return nval;
}

ClefType toMuseScoreClefType(const mnx::part::Clef& mnxClef)
{
    using ClefSign = mnx::ClefSign;
    using OttavaAmountOrZero = mnx::OttavaAmountOrZero;

    const auto snapToLine = [](int staffPositionHalfSpaces) -> int {
        // clefTable only supports clefs centered on lines (even half-spaces).
        if (staffPositionHalfSpaces & 1) {
            staffPositionHalfSpaces += (staffPositionHalfSpaces > 0 ? -1 : +1); // snap toward 0
        }
        return staffPositionHalfSpaces;
    };

    const int sp = snapToLine(mnxClef.staffPosition());
    const auto sign = mnxClef.sign();
    const auto octave = mnxClef.octave();

    switch (sign) {
    case ClefSign::GClef:
        // G clef: standard at sp == -2, also supports G_1 at sp == -4
        if (sp == -4) {
            return ClefType::G_1; // best-effort: ignore octave for this placement
        }

        switch (octave) {
        case OttavaAmountOrZero::NoTransposition: return ClefType::G;
        case OttavaAmountOrZero::TwoOctavesDown:  return ClefType::G15_MB;
        case OttavaAmountOrZero::OctaveDown:      return ClefType::G8_VB;
        case OttavaAmountOrZero::OctaveUp:        return ClefType::G8_VA;
        case OttavaAmountOrZero::TwoOctavesUp:    return ClefType::G15_MA;
        default:                                  return ClefType::INVALID;
        }

    case ClefSign::FClef:
        // F clef: supports sp == 0 (F_B line 3), sp == +2 (F line 4), sp == +4 (F_C line 5)
        if (sp == 0) {
            return ClefType::F_B; // best-effort: ignore octave for this placement
        }
        if (sp == +4) {
            return ClefType::F_C; // best-effort: ignore octave for this placement
        }

        switch (octave) {
        case OttavaAmountOrZero::NoTransposition: return ClefType::F;
        case OttavaAmountOrZero::TwoOctavesDown:  return ClefType::F15_MB;
        case OttavaAmountOrZero::OctaveDown:      return ClefType::F8_VB;
        case OttavaAmountOrZero::OctaveUp:        return ClefType::F_8VA;
        case OttavaAmountOrZero::TwoOctavesUp:    return ClefType::F_15MA;
        default:                                  return ClefType::INVALID;
        }

    case ClefSign::CClef:
        // Special-case explicit octave variant in your clefTable
        if (sp == +2 && octave == OttavaAmountOrZero::OctaveDown) {
            return ClefType::C4_8VB;
        }

        switch (sp) {
        case -4: return ClefType::C1;
        case -2: return ClefType::C2;
        case  0: return ClefType::C3;
        case +2: return ClefType::C4;
        case +4: return ClefType::C5;
        default:
            // If MNX gives something outside these, pick a sane fallback:
            return ClefType::C3;
        }

    default:
        return ClefType::INVALID;
    }
}

Fraction toMuseScoreFraction(const mnx::FractionValue& fraction)
{
    return Fraction(fraction.numerator(), fraction.denominator());
}

mnx::FractionValue toMnxFractionValue(const engraving::Fraction& fraction)
{
    return mnx::FractionValue(fraction.numerator(), fraction.denominator());
}

Key toMuseScoreKey(int fifths)
{
    if (fifths < int(Key::MIN) || fifths > int(Key::MAX)) {
        return Key::INVALID;
    }
    return Key(fifths);
}

NoteType duraTypeToGraceNoteType(DurationType type, bool useLeft)
{
    if (int(type) < int(DurationType::V_EIGHTH)) {
        return useLeft ? NoteType::GRACE8_AFTER : NoteType::GRACE4;
    }
    if (int(type) >= int(DurationType::V_32ND)) {
        return useLeft ? NoteType::GRACE32_AFTER : NoteType::GRACE32;
    }
    if (type == DurationType::V_16TH) {
        return useLeft ? NoteType::GRACE16_AFTER : NoteType::GRACE16;
    }
    return useLeft ? NoteType::GRACE8_AFTER : NoteType::APPOGGIATURA;
}

// Generates a stable MNX voice identifier string from a MuseScore track index.
// This is a convention used by the MNX import/export layer and is not defined
// by the MNX specification itself.
std::string makeMnxVoiceIdFromTrack(int mnxPartStaffNum, track_idx_t curTrackIdx)
{
    return "s" + std::to_string(mnxPartStaffNum) + "v" + std::to_string(curTrackIdx % VOICES + 1);
}
} // namespace mu::iex::mnxio
