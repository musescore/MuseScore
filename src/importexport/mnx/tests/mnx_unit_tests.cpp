/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/tuplet.h"
#include "engraving/types/typesconv.h"

#include "importexport/mnx/internal/shared/mnxtypesconv.h"
#include "mnxtestutils.h"

#ifdef MNXDOM_SYSTEM
#include <mnxdom/mnxdom.h>
#else
#include "mnxdom.h"
#endif

using namespace mu::engraving;
using namespace mu::iex::mnxio;
using namespace muse;

//---------------------------------------------------------
//   Synthesized MNX tests
//   Tests whose input is built here rather than read from a fixture file. The file-driven
//   import/export comparisons live in mnx_tests.cpp; keep this translation unit for cases
//   that exercise one behavior directly.
//---------------------------------------------------------

class Mnx_UnitTests : public ::testing::Test
{
};

//---------------------------------------------------------
//   dynamic spelling tests
//   toMnxDynamicFromLetters is how a dynamic MuseScore renders without a music font, and
//   therefore cannot classify, still reaches MNX. No score fixture spells a dynamic that
//   way -- they all carry glyphs -- so exercise the grammar directly.
//---------------------------------------------------------

namespace {
using DynPrefix = mnx::DynamicPrefix;
using DynSuffix = mnx::DynamicSuffix;
using DynValue = mnx::DynamicValue;

MnxDynamicMapping plainDynamic(DynValue value)
{
    MnxDynamicMapping mapping;
    mapping.value = value;
    return mapping;
}

MnxDynamicMapping accentDynamic(DynPrefix prefix, DynValue value, DynSuffix suffix,
                                std::optional<DynValue> residual = std::nullopt)
{
    MnxDynamicMapping mapping;
    mapping.isAccent = true;
    mapping.accentPrefix = prefix;
    mapping.accentSuffix = suffix;
    mapping.value = value;
    mapping.residualValue = residual;
    return mapping;
}

void expectDynamicLetters(const char* letters, const MnxDynamicMapping& expected)
{
    SCOPED_TRACE(letters);
    const auto actual = toMnxDynamicFromLetters(letters);
    ASSERT_TRUE(actual.has_value());
    EXPECT_EQ(actual->value, expected.value);
    EXPECT_EQ(actual->residualValue, expected.residualValue);
    EXPECT_EQ(actual->isAccent, expected.isAccent);
    EXPECT_EQ(actual->accentPrefix, expected.accentPrefix);
    EXPECT_EQ(actual->accentSuffix, expected.accentSuffix);
}
} // namespace

TEST_F(Mnx_UnitTests, dynamicValueSpellingsCarryNoAffixLetters)
{
    // toMnxDynamicFromLetters reads the accent affixes one character at a time, which is only
    // unambiguous because no DynamicValue spells itself with s, r, or z. That is a property of
    // mnxdom, not of this code, so assert it here rather than let a schema change silently
    // turn "sf" into a value lookup that swallows the prefix.
    for (const auto& [value, spelling] : mnx::EnumStringMapping<mnx::DynamicValue>::enumToString()) {
        SCOPED_TRACE(spelling);
        EXPECT_FALSE(spelling.empty());
        for (const char ch : spelling) {
            EXPECT_TRUE(ch == 'p' || ch == 'm' || ch == 'f' || ch == 'n')
                << "DynamicValue \"" << spelling << "\" contains '" << ch
                << "', which the letters grammar treats as an accent affix.";
        }
    }
}

TEST_F(Mnx_UnitTests, dynamicLettersAccepted)
{
    expectDynamicLetters("p", plainDynamic(DynValue::p));
    expectDynamicLetters("mf", plainDynamic(DynValue::mf));
    expectDynamicLetters("n", plainDynamic(DynValue::n));
    // A value is taken as far as it goes, so "fff" is one dynamic rather than f followed by a
    // residual ff. A residualValue here would make this an accent, which it is not.
    expectDynamicLetters("fff", plainDynamic(DynValue::fff));
    expectDynamicLetters("ffffff", plainDynamic(DynValue::ffffff));

    expectDynamicLetters("sf", accentDynamic(DynPrefix::s, DynValue::f, DynSuffix::None));
    expectDynamicLetters("sfz", accentDynamic(DynPrefix::s, DynValue::f, DynSuffix::z));
    expectDynamicLetters("rfz", accentDynamic(DynPrefix::r, DynValue::f, DynSuffix::z));
    expectDynamicLetters("fz", accentDynamic(DynPrefix::None, DynValue::f, DynSuffix::z));
    expectDynamicLetters("sfpp", accentDynamic(DynPrefix::s, DynValue::f, DynSuffix::None, DynValue::pp));

    // fp and pf carry no accent letters at all, but residualValue is accent-only in MNX.
    expectDynamicLetters("fp", accentDynamic(DynPrefix::None, DynValue::f, DynSuffix::None, DynValue::p));
    expectDynamicLetters("pf", accentDynamic(DynPrefix::None, DynValue::p, DynSuffix::None, DynValue::f));

    // Spellings MuseScore has no DynamicType for. These are the reason the fallback exists:
    // MNX builds a dynamic from parts, so it can say what MuseScore can only draw.
    expectDynamicLetters("sfzp", accentDynamic(DynPrefix::s, DynValue::f, DynSuffix::z, DynValue::p));
    expectDynamicLetters("ffz", accentDynamic(DynPrefix::None, DynValue::ff, DynSuffix::z));
}

TEST_F(Mnx_UnitTests, dynamicLettersRejected)
{
    // Anything that is not wholly a dynamic spelling has to be refused rather than guessed at,
    // since the caller hands us whatever text the user typed. Note that a bare affix is
    // rejected too: MNX has nothing to hang an accent on without a value.
    for (const char* letters : { "", "s", "r", "z", "sz",
                                 "sempre f", "f subito", "poco", "f-p",
                                 "SFZ", "Sfz",                          // spelling is case sensitive
                                 "fx", "fzz", "ffzz" }) {
        SCOPED_TRACE(letters);
        EXPECT_FALSE(toMnxDynamicFromLetters(letters).has_value());
    }
}

TEST_F(Mnx_UnitTests, dynamicLettersMatchDynamicTypeTable)
{
    for (int index = 0; index < int(DynamicType::LAST); index++) {
        const DynamicType type = DynamicType(index);
        const MnxDynamicMapping expected = toMnxDynamicType(type);
        if (!expected.value) {
            continue; // MNX cannot express this one
        }
        SCOPED_TRACE(TConv::toXml(type).ascii());

        // Every dynamic MNX can express must parse back from the letters MuseScore spells it
        // with, or the grammar and toMnxDynamicType have drifted apart.
        expectDynamicLetters(TConv::toXml(type).ascii(), expected);

        // toMuseScoreDynamicType returns the first table entry that matches, which is only
        // correct because no two types share a mapping. Check that they still do not.
        EXPECT_EQ(toMuseScoreDynamicType(expected), std::make_optional(type));
    }
}

//---------------------------------------------------------
//   tempo tests
//---------------------------------------------------------

namespace {
std::string fractionalTempoMnx()
{
    return R"({
  "mnx": { "version": )" + std::to_string(mnx::MNX_VERSION)
           +
           R"( },
  "global": {
    "measures": [
      {
        "id": "m1",
        "key": { "fifths": 0 },
        "tempos": [
          { "bpm": 92.5, "value": { "base": "quarter" } }
        ],
        "time": { "count": 4, "unit": 4 }
      }
    ]
  },
  "parts": [
    {
      "id": "P1",
      "measures": [
        {
          "clefs": [ { "clef": { "sign": "G", "staffPosition": -2 } } ],
          "sequences": [
            {
              "content": [
                {
                  "type": "event",
                  "duration": { "base": "whole" },
                  "notes": [ { "pitch": { "octave": 4, "step": "C" } } ]
                }
              ]
            }
          ]
        }
      ]
    }
  ]
})";
}

const TempoText* findFirstTempoText(const Score* score)
{
    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
            for (const EngravingItem* item : segment->annotations()) {
                if (item && item->isTempoText()) {
                    return toTempoText(item);
                }
            }
        }
    }
    return nullptr;
}
} // namespace

TEST_F(Mnx_UnitTests, fractionalTempoRoundTrips)
{
    // MNX widened bpm from an integer to a number, so a tempo such as 92.5 has to survive both
    // directions unrounded. MuseScore has always carried tempo as a double, so the whole loss
    // was on the MNX side.
    std::unique_ptr<MasterScore> score(importMnxFromJson(fractionalTempoMnx(), u"<fractionalTempo>/tempo.mnx"));
    ASSERT_TRUE(score);
    fixupAndLayoutScore(score.get());

    const TempoText* tempoText = findFirstTempoText(score.get());
    ASSERT_TRUE(tempoText);
    // The bpm is spelled into the tempo text, which is where MuseScore reads it back from.
    EXPECT_DOUBLE_EQ(tempoText->tempoBpm(), 92.5);
    // A quarter note at 92.5 bpm is 92.5 quarter notes per minute.
    EXPECT_NEAR(tempoText->tempo().val, 92.5 / 60.0, 1e-9);

    const std::string json = exportMnxJson(score.get());
    ASSERT_FALSE(json.empty());

    auto doc = mnx::Document::create(json.data(), json.size());
    ASSERT_TRUE(mnx::validation::schemaValidate(doc));
    ASSERT_FALSE(doc.global().measures().empty());
    const std::optional<mnx::Array<mnx::global::Tempo> > tempos = doc.global().measures()[0].tempos();
    ASSERT_TRUE(tempos.has_value());
    ASSERT_EQ(tempos->size(), size_t(1));
    EXPECT_DOUBLE_EQ((*tempos)[0].bpm(), 92.5);
}

//---------------------------------------------------------
//   synthesized document helpers
//---------------------------------------------------------

namespace {
//! A 4/4 document with one single-staff part whose measures hold the given sequences arrays.
//! partExtras is spliced into each part measure after its sequences, e.g. `, "staffConfigs": [...]`.
std::string singleStaffMnx(const std::vector<std::string>& sequencesPerMeasure,
                           const std::vector<std::string>& partExtras = {})
{
    std::string globalMeasures;
    std::string partMeasures;
    for (size_t i = 0; i < sequencesPerMeasure.size(); ++i) {
        if (i > 0) {
            globalMeasures += ",";
            partMeasures += ",";
        }
        globalMeasures += i == 0
                          ? R"({ "id": "m1", "key": { "fifths": 0 }, "time": { "count": 4, "unit": 4 } })"
                          : R"({ "id": "m)" + std::to_string(i + 1) + R"(" })";
        partMeasures += "{";
        if (i == 0) {
            partMeasures += R"("clefs": [ { "clef": { "sign": "G", "staffPosition": -2 } } ], )";
        }
        partMeasures += R"("sequences": )" + sequencesPerMeasure[i];
        if (i < partExtras.size()) {
            partMeasures += partExtras[i];
        }
        partMeasures += "}";
    }
    return R"({
  "mnx": { "version": )" + std::to_string(mnx::MNX_VERSION) + R"( },
  "global": { "measures": [ )" + globalMeasures + R"( ] },
  "parts": [ { "id": "P1", "measures": [ )" + partMeasures + R"( ] } ]
})";
}

//! A sequence of one whole note, with an optional direction hint.
std::string wholeNoteSequence(const char* step, int octave, const char* hint = nullptr)
{
    std::string result = R"({ "content": [ { "duration": { "base": "whole" }, "notes": [ { "pitch": { "octave": )"
                         + std::to_string(octave) + R"(, "step": ")" + step + R"(" } } ] } ])";
    if (hint) {
        result += R"(, "directionHint": ")" + std::string(hint) + R"(")";
    }
    return result + " }";
}

//! A sequence of four quarter notes on one pitch, with an optional direction hint.
std::string quarterNoteSequence(const char* step, int octave, const char* hint = nullptr)
{
    const std::string event = R"({ "duration": { "base": "quarter" }, "notes": [ { "pitch": { "octave": )"
                              + std::to_string(octave) + R"(, "step": ")" + step + R"(" } } ] })";
    std::string result = R"({ "content": [ )" + event + "," + event + "," + event + "," + event + " ]";
    if (hint) {
        result += R"(, "directionHint": ")" + std::string(hint) + R"(")";
    }
    return result + " }";
}

std::string sequencesArray(const std::vector<std::string>& sequences)
{
    std::string result = "[ ";
    for (size_t i = 0; i < sequences.size(); ++i) {
        result += (i > 0 ? ", " : "") + sequences[i];
    }
    return result + " ]";
}

const Measure* nthMeasure(const Score* score, size_t index)
{
    const Measure* measure = score->firstMeasure();
    for (size_t i = 0; measure && i < index; ++i) {
        measure = measure->nextMeasure();
    }
    return measure;
}

//! The chords on a track in a measure, in order.
std::vector<const Chord*> chordsOnTrack(const Measure* measure, track_idx_t track)
{
    std::vector<const Chord*> result;
    for (const Segment* segment = measure->first(SegmentType::ChordRest); segment;
         segment = segment->next(SegmentType::ChordRest)) {
        const EngravingItem* item = segment->element(track);
        if (item && item->isChord()) {
            result.push_back(toChord(item));
        }
    }
    return result;
}

int firstPitchOnTrack(const Measure* measure, track_idx_t track)
{
    const auto chords = chordsOnTrack(measure, track);
    return chords.empty() ? -1 : chords.front()->notes().front()->pitch();
}

//! Measures of a document exercising direction-hint voice allocation. Pitches identify the
//! sequences: C4 = 60, D4 = 62, E4 = 64, C5 = 72, E5 = 76, A5 = 81, C6 = 84.
std::string directionHintMnx()
{
    return singleStaffMnx({
            // m1: a lower voice listed before its upper voice
            sequencesArray({ wholeNoteSequence("C", 4, "lower"), wholeNoteSequence("C", 5, "upper") }),
            // m2: three upper voices; the third overflows into voice 2
            sequencesArray({ wholeNoteSequence("C", 6, "upper"), wholeNoteSequence("A", 5, "upper"),
                             quarterNoteSequence("E", 5, "upper") }),
            // m3: the hinted voices fill the staff before the unhinted one is placed
            sequencesArray({ wholeNoteSequence("D", 4), wholeNoteSequence("C", 6, "upper"), wholeNoteSequence("A", 5, "upper"),
                             quarterNoteSequence("E", 5, "upper"), wholeNoteSequence("C", 4, "lower") }),
            // m4: unhinted voices take what the hinted one leaves
            sequencesArray({ wholeNoteSequence("D", 4), wholeNoteSequence("C", 4, "lower"), wholeNoteSequence("E", 4) }),
            // m5: a lone lower voice
            sequencesArray({ wholeNoteSequence("C", 4, "lower") }),
        });
}
} // namespace

//---------------------------------------------------------
//   direction hint tests
//---------------------------------------------------------

TEST_F(Mnx_UnitTests, directionHintAllocation)
{
    std::unique_ptr<MasterScore> score(importMnxFromJson(directionHintMnx(), u"<directionHints>/hints.mnx"));
    ASSERT_TRUE(score);
    fixupAndLayoutScore(score.get());

    // m1: upper takes voice 1 and lower voice 2, whatever order they are listed in.
    const Measure* m1 = nthMeasure(score.get(), 0);
    ASSERT_TRUE(m1);
    EXPECT_EQ(firstPitchOnTrack(m1, 0), 72);
    EXPECT_EQ(firstPitchOnTrack(m1, 1), 60);

    // m2: upper voices fill voices 1 and 3, then the third takes voice 2 with its stems held up.
    const Measure* m2 = nthMeasure(score.get(), 1);
    ASSERT_TRUE(m2);
    EXPECT_EQ(firstPitchOnTrack(m2, 0), 84);
    EXPECT_EQ(firstPitchOnTrack(m2, 2), 81);
    const auto overflow = chordsOnTrack(m2, 1);
    ASSERT_EQ(overflow.size(), size_t(4));
    for (const Chord* chord : overflow) {
        EXPECT_EQ(chord->notes().front()->pitch(), 76);
        EXPECT_EQ(chord->stemDirection(), DirectionV::UP);
    }
    EXPECT_EQ(chordsOnTrack(m2, 0).front()->stemDirection(), DirectionV::AUTO);

    // m3: the hinted voices are placed first and fill all four voices, so the unhinted D4 is dropped.
    const Measure* m3 = nthMeasure(score.get(), 2);
    ASSERT_TRUE(m3);
    EXPECT_EQ(firstPitchOnTrack(m3, 0), 84);
    EXPECT_EQ(firstPitchOnTrack(m3, 2), 81);
    EXPECT_EQ(firstPitchOnTrack(m3, 1), 76);
    EXPECT_EQ(firstPitchOnTrack(m3, 3), 60);

    // m4: unhinted voices take the lowest voices the lower voice leaves free.
    const Measure* m4 = nthMeasure(score.get(), 3);
    ASSERT_TRUE(m4);
    EXPECT_EQ(firstPitchOnTrack(m4, 0), 62);
    EXPECT_EQ(firstPitchOnTrack(m4, 1), 60);
    EXPECT_EQ(firstPitchOnTrack(m4, 2), 64);

    // m5: a lone lower voice goes to voice 2, over a hidden full-measure rest in voice 1.
    const Measure* m5 = nthMeasure(score.get(), 4);
    ASSERT_TRUE(m5);
    EXPECT_EQ(firstPitchOnTrack(m5, 1), 60);
    const Segment* first = m5->first(SegmentType::ChordRest);
    ASSERT_TRUE(first);
    const EngravingItem* voice1 = first->element(0);
    ASSERT_TRUE(voice1 && voice1->isRest());
    EXPECT_TRUE(toRest(voice1)->durationType().isMeasure());
    EXPECT_FALSE(voice1->visible());
    for (const Segment* segment = first->next(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
        EXPECT_FALSE(segment->element(0)) << "voice 1 should hold only the hidden full-measure rest";
    }
}

TEST_F(Mnx_UnitTests, directionHintExport)
{
    std::unique_ptr<MasterScore> score(importMnxFromJson(directionHintMnx(), u"<directionHints>/hints.mnx"));
    ASSERT_TRUE(score);
    fixupAndLayoutScore(score.get());

    const std::string json = exportMnxJson(score.get());
    ASSERT_FALSE(json.empty());
    auto doc = mnx::Document::create(json.data(), json.size());
    ASSERT_TRUE(mnx::validation::schemaValidate(doc));
    const auto partMeasures = doc.parts()[0].measures();
    ASSERT_EQ(partMeasures.size(), size_t(5));

    const auto hintsAndStems = [](const mnx::part::Measure& measure) {
        std::vector<std::pair<mnx::DirectionHint, bool> > result;
        for (const auto& sequence : measure.sequences()) {
            bool hasStem = false;
            for (const auto& item : sequence.content()) {
                if (item.type() == mnx::sequence::Event::ContentTypeValue && item.get<mnx::sequence::Event>().stemDirection()) {
                    hasStem = true;
                }
            }
            result.emplace_back(sequence.directionHint(), hasStem);
        }
        return result;
    };
    using Hint = mnx::DirectionHint;
    using HintsAndStems = std::vector<std::pair<Hint, bool> >;

    // Voices are written in voice order, each hinted by where MuseScore points its stems.
    EXPECT_EQ(hintsAndStems(partMeasures[0]), (HintsAndStems { { Hint::Upper, false }, { Hint::Lower, false } }));
    // The overflowing upper voice sits in voice 2, so it is written as lower with its forced stems.
    EXPECT_EQ(hintsAndStems(partMeasures[1]),
              (HintsAndStems { { Hint::Upper, false }, { Hint::Lower, true }, { Hint::Upper, false } }));
    // The lone lower voice is written alone: its hidden full-measure rest in voice 1 is omitted.
    EXPECT_EQ(hintsAndStems(partMeasures[4]), (HintsAndStems { { Hint::Lower, false } }));
}

TEST_F(Mnx_UnitTests, directionHintOmittedForSingleVoice)
{
    std::unique_ptr<MasterScore> score(importMnxFromJson(singleStaffMnx({ sequencesArray({ wholeNoteSequence("C", 5) }) }),
                                                         u"<directionHints>/single.mnx"));
    ASSERT_TRUE(score);
    fixupAndLayoutScore(score.get());

    const std::string json = exportMnxJson(score.get());
    ASSERT_FALSE(json.empty());
    auto doc = mnx::Document::create(json.data(), json.size());
    const auto sequences = doc.parts()[0].measures()[0].sequences();
    ASSERT_EQ(sequences.size(), size_t(1));
    EXPECT_EQ(sequences[0].dump().find("directionHint"), std::string::npos);
}

//---------------------------------------------------------
//   caesura tests
//---------------------------------------------------------

TEST_F(Mnx_UnitTests, caesuraMapping)
{
    struct Case {
        mnx::CaesuraShape shape;
        unsigned marks;
        SymId sym;
    };
    const Case cases[] = {
        { mnx::CaesuraShape::Normal, 2, SymId::caesura },
        { mnx::CaesuraShape::Normal, 1, SymId::caesuraSingleStroke },
        { mnx::CaesuraShape::Curved, 2, SymId::caesuraCurved },
        { mnx::CaesuraShape::Short, 2, SymId::caesuraShort },
        { mnx::CaesuraShape::Thick, 2, SymId::caesuraThick },
    };
    for (const Case& c : cases) {
        SCOPED_TRACE(TConv::toXml(c.sym).ascii());
        EXPECT_EQ(toMuseScoreCaesuraSym(c.shape, c.marks), c.sym);
        const MnxCaesura back = toMnxCaesura(c.sym);
        EXPECT_EQ(back.shape, c.shape);
        EXPECT_EQ(back.marks, c.marks);
    }
    // MuseScore has no single-stroke form of the other shapes, so the shape wins.
    EXPECT_EQ(toMuseScoreCaesuraSym(mnx::CaesuraShape::Thick, 1), SymId::caesuraThick);
}

TEST_F(Mnx_UnitTests, caesuraReplacesBreathMarkOnSameEvent)
{
    const std::string sequences
        =
            R"([ { "content": [ { "duration": { "base": "whole" },
        "markings": { "breath": { "symbol": "comma" }, "caesura": { "shape": "curved" } },
        "notes": [ { "pitch": { "octave": 5, "step": "C" } } ] } ] } ])";
    std::unique_ptr<MasterScore> score(importMnxFromJson(singleStaffMnx({ sequences }), u"<caesura>/both.mnx"));
    ASSERT_TRUE(score);
    fixupAndLayoutScore(score.get());

    const Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);
    std::vector<const Breath*> breaths;
    for (const Segment* segment = measure->first(SegmentType::Breath); segment; segment = segment->next(SegmentType::Breath)) {
        if (const EngravingItem* item = segment->element(0)) {
            breaths.push_back(toBreath(item));
        }
    }
    ASSERT_EQ(breaths.size(), size_t(1));
    EXPECT_TRUE(breaths.front()->isCaesura());
    EXPECT_EQ(breaths.front()->symId(), SymId::caesuraCurved);
}

//---------------------------------------------------------
//   staff config tests
//---------------------------------------------------------

TEST_F(Mnx_UnitTests, staffConfigMidMeasureMovesToNextBarline)
{
    const std::string whole = sequencesArray({ wholeNoteSequence("C", 5) });
    // m1 changes to one line halfway through; m3 changes to three lines halfway through, with
    // no measure after it to take the change.
    std::unique_ptr<MasterScore> score(importMnxFromJson(
                                           singleStaffMnx({ whole, whole, whole },
    {
        R"(, "staffConfigs": [ { "config": { "lines": 1 }, "position": { "fraction": [1, 2] } } ])",
        "",
        R"(, "staffConfigs": [ { "config": { "lines": 3 }, "position": { "fraction": [1, 2] } } ])" }),
                                           u"<staffConfig>/midMeasure.mnx"));
    ASSERT_TRUE(score);
    fixupAndLayoutScore(score.get());

    const Staff* staff = score->staff(0);
    ASSERT_TRUE(staff);
    EXPECT_EQ(staff->lines(nthMeasure(score.get(), 0)->tick()), 5);
    EXPECT_EQ(staff->lines(nthMeasure(score.get(), 1)->tick()), 1);
    EXPECT_EQ(staff->lines(nthMeasure(score.get(), 2)->tick()), 1);
}

TEST_F(Mnx_UnitTests, staffConfigExportWritesChangesAndOmitsDefaults)
{
    const std::string whole = sequencesArray({ wholeNoteSequence("C", 5) });
    std::unique_ptr<MasterScore> score(importMnxFromJson(
                                           singleStaffMnx({ whole, whole, whole },
                                                          { R"(, "staffConfigs": [ { "config": { "lines": 1 } } ])",
                                                            "",
                                                            R"(, "staffConfigs": [ { "config": {} } ])" }),
                                           u"<staffConfig>/defaults.mnx"));
    ASSERT_TRUE(score);
    fixupAndLayoutScore(score.get());
    EXPECT_EQ(score->staff(0)->lines(nthMeasure(score.get(), 1)->tick()), 1);
    EXPECT_EQ(score->staff(0)->lines(nthMeasure(score.get(), 2)->tick()), 5);

    const std::string json = exportMnxJson(score.get());
    ASSERT_FALSE(json.empty());
    auto doc = mnx::Document::create(json.data(), json.size());
    ASSERT_TRUE(mnx::validation::schemaValidate(doc));
    const auto partMeasures = doc.parts()[0].measures();
    ASSERT_EQ(partMeasures.size(), size_t(3));

    // A staff config describes the whole staff, so only changes are written, and a return to
    // the defaults is an empty config.
    ASSERT_TRUE(partMeasures[0].staffConfigs());
    ASSERT_EQ(partMeasures[0].staffConfigs()->size(), size_t(1));
    EXPECT_EQ((*partMeasures[0].staffConfigs())[0].config().lines(), 1u);
    EXPECT_FALSE(partMeasures[1].staffConfigs());
    ASSERT_TRUE(partMeasures[2].staffConfigs());
    ASSERT_EQ(partMeasures[2].staffConfigs()->size(), size_t(1));
    EXPECT_EQ((*partMeasures[2].staffConfigs())[0].config().dump(), "{}");
}

//---------------------------------------------------------
//   tuplet tests
//---------------------------------------------------------

namespace {
//! An event on C5 with the given note value.
std::string noteEvent(const char* base)
{
    return R"({ "duration": { "base": ")" + std::string(base) + R"(" }, "notes": [ { "pitch": { "octave": 5, "step": "C" } } ] })";
}

std::string dottedNoteEvent(const char* base)
{
    return R"({ "duration": { "base": ")" + std::string(base)
           + R"(", "dots": 1 }, "notes": [ { "pitch": { "octave": 5, "step": "C" } } ] })";
}

std::string spaceItem(int numerator, int denominator)
{
    return R"({ "type": "space", "duration": [)" + std::to_string(numerator) + ", " + std::to_string(denominator) + "] }";
}

//! A tuplet of `innerMultiple` inner note values in the time of `outerMultiple` outer ones.
std::string tupletItem(const char* outerBase, int outerMultiple, const std::string& innerDuration, int innerMultiple,
                       const std::vector<std::string>& content)
{
    std::string items;
    for (size_t i = 0; i < content.size(); ++i) {
        items += (i > 0 ? ", " : "") + content[i];
    }
    return R"({ "type": "tuplet", "outer": { "duration": { "base": ")" + std::string(outerBase) + R"(" }, "multiple": )"
           + std::to_string(outerMultiple) + R"( }, "inner": { "duration": )" + innerDuration + R"(, "multiple": )"
           + std::to_string(innerMultiple) + R"( }, "content": [ )" + items + " ] }";
}

std::string contentSequence(const std::vector<std::string>& content)
{
    std::string items;
    for (size_t i = 0; i < content.size(); ++i) {
        items += (i > 0 ? ", " : "") + content[i];
    }
    return R"({ "content": [ )" + items + " ] }";
}

//! A tuplet MuseScore cannot represent: two dotted quarters in the time of one quarter, whose
//! outer value is not a whole multiple of its inner value.
std::string unimportableTuplet()
{
    return tupletItem("quarter", 1, R"({ "base": "quarter", "dots": 1 })", 2,
                      { dottedNoteEvent("quarter"), dottedNoteEvent("quarter") });
}

const ChordRest* chordRestAt(const Measure* measure, const Fraction& rTick)
{
    const Segment* segment = measure->findSegmentR(SegmentType::ChordRest, rTick);
    const EngravingItem* item = segment ? segment->element(0) : nullptr;
    return item && item->isChordRest() ? toChordRest(item) : nullptr;
}
} // namespace

TEST_F(Mnx_UnitTests, tupletThatCannotImportIsSkipped)
{
    // The unimportable tuplet appears at the top level and nested inside a valid tuplet. Each
    // becomes a gap in whatever encloses it, and the valid tuplets around it are unaffected.
    const std::string eighthTriplet = tupletItem("eighth", 2, R"({ "base": "eighth" })", 3,
                                                 { noteEvent("eighth"), noteEvent("eighth"), noteEvent("eighth") });
    const std::string quarterTriplet = tupletItem("quarter", 2, R"({ "base": "quarter" })", 3,
                                                  { noteEvent("quarter"), unimportableTuplet(), noteEvent("quarter") });
    std::unique_ptr<MasterScore> score(importMnxFromJson(
                                           singleStaffMnx({ sequencesArray({ contentSequence({ unimportableTuplet(), eighthTriplet,
                                                                                               quarterTriplet }) }) }),
                                           u"<tuplets>/unimportable.mnx"));
    ASSERT_TRUE(score);
    fixupAndLayoutScore(score.get());
    const Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    // The top-level tuplet is a gap belonging to no tuplet.
    const ChordRest* first = chordRestAt(measure, Fraction(0, 1));
    ASSERT_TRUE(first && first->isRest());
    EXPECT_TRUE(toRest(first)->isGap());
    EXPECT_FALSE(first->tuplet());

    // The eighth-note triplet keeps its three chords.
    const ChordRest* e1 = chordRestAt(measure, Fraction(1, 4));
    const ChordRest* e2 = chordRestAt(measure, Fraction(1, 3));
    const ChordRest* e3 = chordRestAt(measure, Fraction(5, 12));
    ASSERT_TRUE(e1 && e2 && e3);
    const Tuplet* eighths = e1->tuplet();
    ASSERT_TRUE(eighths);
    EXPECT_EQ(e2->tuplet(), eighths);
    EXPECT_EQ(e3->tuplet(), eighths);
    EXPECT_EQ(eighths->elements().size(), size_t(3));

    // The quarter-note triplet holds its two chords with the nested tuplet's gap between them.
    const ChordRest* q1 = chordRestAt(measure, Fraction(1, 2));
    const ChordRest* q2 = chordRestAt(measure, Fraction(2, 3));
    const ChordRest* q3 = chordRestAt(measure, Fraction(5, 6));
    ASSERT_TRUE(q1 && q2 && q3);
    const Tuplet* quarters = q1->tuplet();
    ASSERT_TRUE(quarters);
    EXPECT_NE(quarters, eighths);
    EXPECT_TRUE(q1->isChord());
    ASSERT_TRUE(q2->isRest());
    EXPECT_TRUE(toRest(q2)->isGap());
    EXPECT_EQ(q2->tuplet(), quarters);
    EXPECT_TRUE(q3->isChord());
    EXPECT_EQ(q3->tuplet(), quarters);
    EXPECT_EQ(quarters->elements().size(), size_t(3));
}

TEST_F(Mnx_UnitTests, allSpaceTupletBecomesGap)
{
    // A tuplet holding only spaces (and graces) becomes one gap in whatever encloses it: the
    // measure at the top level, or the enclosing tuplet when nested. Its graces have nothing
    // to attach to and are dropped.
    const std::string grace = R"({ "type": "grace", "content": [ )" + noteEvent("eighth") + " ] }";
    const std::string topLevelSpaces = tupletItem("quarter", 2, R"({ "base": "quarter" })", 3,
                                                  { spaceItem(1, 4), grace, spaceItem(1, 4), spaceItem(1, 4) });
    const std::string nestedSpaces = tupletItem("quarter", 1, R"({ "base": "eighth" })", 3,
                                                { spaceItem(1, 8), spaceItem(1, 8), spaceItem(1, 8) });
    const std::string tripletJson = tupletItem("quarter", 2, R"({ "base": "quarter" })", 3,
                                               { noteEvent("quarter"), nestedSpaces, noteEvent("quarter") });
    std::unique_ptr<MasterScore> score(importMnxFromJson(
                                           singleStaffMnx({ sequencesArray({ contentSequence({ topLevelSpaces, tripletJson }) }) }),
                                           u"<tuplets>/allSpaces.mnx"));
    ASSERT_TRUE(score);
    fixupAndLayoutScore(score.get());
    const Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    // At the top level: a single gap belonging to no tuplet, and nothing else until the triplet.
    const ChordRest* topGap = chordRestAt(measure, Fraction(0, 1));
    ASSERT_TRUE(topGap && topGap->isRest());
    EXPECT_TRUE(toRest(topGap)->isGap());
    EXPECT_FALSE(topGap->tuplet());
    EXPECT_FALSE(chordRestAt(measure, Fraction(1, 6)));
    EXPECT_FALSE(chordRestAt(measure, Fraction(1, 3)));

    // Nested: the gap belongs to the enclosing triplet, between its two chords.
    const ChordRest* first = chordRestAt(measure, Fraction(1, 2));
    const ChordRest* nestedGap = chordRestAt(measure, Fraction(2, 3));
    const ChordRest* last = chordRestAt(measure, Fraction(5, 6));
    ASSERT_TRUE(first && nestedGap && last);
    const Tuplet* triplet = first->tuplet();
    ASSERT_TRUE(triplet);
    ASSERT_TRUE(nestedGap->isRest());
    EXPECT_TRUE(toRest(nestedGap)->isGap());
    EXPECT_EQ(nestedGap->tuplet(), triplet);
    EXPECT_EQ(last->tuplet(), triplet);
    EXPECT_EQ(triplet->elements().size(), size_t(3));

    // The skipped grace note is not attached to either neighbor.
    ASSERT_TRUE(first->isChord());
    EXPECT_TRUE(toChord(first)->graceNotes().empty());
}
