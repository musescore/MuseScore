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
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
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
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "engraving/tests/utils/scorerw.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"

#include "importexport/mei/internal/meiconverter.h"
#include "importexport/mei/internal/meireader.h"
#include "importexport/mei/thirdparty/libmei/att.h"

namespace mu::iex::mei {
namespace {
using engraving::AccidentalType;

struct WrittenCase {
    libmei::data_ACCIDENTAL_WRITTEN mei;
    AccidentalType museScore;
    double cents;
};

constexpr std::array<WrittenCase, 31> WRITTEN_CASES { {
    { libmei::ACCIDENTAL_WRITTEN_NONE, AccidentalType::NONE, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_s, AccidentalType::SHARP, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_f, AccidentalType::FLAT, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_ss, AccidentalType::SHARP_SHARP, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_x, AccidentalType::SHARP2, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_ff, AccidentalType::FLAT2, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_ts, AccidentalType::SHARP3, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_tf, AccidentalType::FLAT3, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_n, AccidentalType::NATURAL, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_nf, AccidentalType::NATURAL_FLAT, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_ns, AccidentalType::NATURAL_SHARP, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_su, AccidentalType::SHARP_ARROW_UP, 150.0 },
    { libmei::ACCIDENTAL_WRITTEN_sd, AccidentalType::SHARP_ARROW_DOWN, 50.0 },
    { libmei::ACCIDENTAL_WRITTEN_fu, AccidentalType::FLAT_ARROW_UP, -50.0 },
    { libmei::ACCIDENTAL_WRITTEN_fd, AccidentalType::FLAT_ARROW_DOWN, -150.0 },
    { libmei::ACCIDENTAL_WRITTEN_nu, AccidentalType::NATURAL_ARROW_UP, 50.0 },
    { libmei::ACCIDENTAL_WRITTEN_nd, AccidentalType::NATURAL_ARROW_DOWN, -50.0 },
    { libmei::ACCIDENTAL_WRITTEN_xu, AccidentalType::SHARP2_ARROW_UP, 250.0 },
    { libmei::ACCIDENTAL_WRITTEN_xd, AccidentalType::SHARP2_ARROW_DOWN, 150.0 },
    { libmei::ACCIDENTAL_WRITTEN_ffu, AccidentalType::FLAT2_ARROW_UP, -150.0 },
    { libmei::ACCIDENTAL_WRITTEN_ffd, AccidentalType::FLAT2_ARROW_DOWN, -250.0 },
    { libmei::ACCIDENTAL_WRITTEN_1qf, AccidentalType::MIRRORED_FLAT, -50.0 },
    { libmei::ACCIDENTAL_WRITTEN_3qf, AccidentalType::MIRRORED_FLAT2, -150.0 },
    { libmei::ACCIDENTAL_WRITTEN_1qs, AccidentalType::SHARP_SLASH, 50.0 },
    { libmei::ACCIDENTAL_WRITTEN_3qs, AccidentalType::SHARP_SLASH4, 150.0 },
    { libmei::ACCIDENTAL_WRITTEN_bms, AccidentalType::SHARP_SLASH2, 89.0 },
    { libmei::ACCIDENTAL_WRITTEN_kms, AccidentalType::SHARP_SLASH3, 56.0 },
    { libmei::ACCIDENTAL_WRITTEN_bf, AccidentalType::FLAT_SLASH, -44.0 },
    { libmei::ACCIDENTAL_WRITTEN_bmf, AccidentalType::FLAT_SLASH2, -89.0 },
    { libmei::ACCIDENTAL_WRITTEN_koron, AccidentalType::KORON, -67.0 },
    { libmei::ACCIDENTAL_WRITTEN_sori, AccidentalType::SORI, 33.0 },
} };

TEST(MeiMicrotonalTests, writtenAccidentalsPreserveIdentityAndCents)
{
    for (const WrittenCase& test : WRITTEN_CASES) {
        bool warning = true;
        const AccidentalType actual = Convert::accidFromMEI(test.mei, warning);
        EXPECT_FALSE(warning) << static_cast<int>(test.mei);
        EXPECT_EQ(actual, test.museScore) << static_cast<int>(test.mei);
        EXPECT_DOUBLE_EQ(engraving::Accidental::subtype2centOffset(actual), test.cents)
            << static_cast<int>(test.mei);
        EXPECT_EQ(Convert::accidToMEI(actual), test.mei) << static_cast<int>(test.mei);
    }
}

struct GesturalDerivationCase {
    libmei::data_ACCIDENTAL_WRITTEN written;
    libmei::data_ACCIDENTAL_GESTURAL gestural;
};

constexpr std::array<GesturalDerivationCase, 20> GESTURAL_DERIVATION_CASES { {
    { libmei::ACCIDENTAL_WRITTEN_su, libmei::ACCIDENTAL_GESTURAL_su },
    { libmei::ACCIDENTAL_WRITTEN_sd, libmei::ACCIDENTAL_GESTURAL_sd },
    { libmei::ACCIDENTAL_WRITTEN_fu, libmei::ACCIDENTAL_GESTURAL_fu },
    { libmei::ACCIDENTAL_WRITTEN_fd, libmei::ACCIDENTAL_GESTURAL_fd },
    { libmei::ACCIDENTAL_WRITTEN_nu, libmei::ACCIDENTAL_GESTURAL_sd },
    { libmei::ACCIDENTAL_WRITTEN_nd, libmei::ACCIDENTAL_GESTURAL_fu },
    { libmei::ACCIDENTAL_WRITTEN_xu, libmei::ACCIDENTAL_GESTURAL_xu },
    { libmei::ACCIDENTAL_WRITTEN_xd, libmei::ACCIDENTAL_GESTURAL_su },
    { libmei::ACCIDENTAL_WRITTEN_ffu, libmei::ACCIDENTAL_GESTURAL_fd },
    { libmei::ACCIDENTAL_WRITTEN_ffd, libmei::ACCIDENTAL_GESTURAL_ffd },
    { libmei::ACCIDENTAL_WRITTEN_1qf, libmei::ACCIDENTAL_GESTURAL_fu },
    { libmei::ACCIDENTAL_WRITTEN_3qf, libmei::ACCIDENTAL_GESTURAL_fd },
    { libmei::ACCIDENTAL_WRITTEN_1qs, libmei::ACCIDENTAL_GESTURAL_sd },
    { libmei::ACCIDENTAL_WRITTEN_3qs, libmei::ACCIDENTAL_GESTURAL_su },
    { libmei::ACCIDENTAL_WRITTEN_bms, libmei::ACCIDENTAL_GESTURAL_bms },
    { libmei::ACCIDENTAL_WRITTEN_kms, libmei::ACCIDENTAL_GESTURAL_kms },
    { libmei::ACCIDENTAL_WRITTEN_bf, libmei::ACCIDENTAL_GESTURAL_bf },
    { libmei::ACCIDENTAL_WRITTEN_bmf, libmei::ACCIDENTAL_GESTURAL_bmf },
    { libmei::ACCIDENTAL_WRITTEN_koron, libmei::ACCIDENTAL_GESTURAL_koron },
    { libmei::ACCIDENTAL_WRITTEN_sori, libmei::ACCIDENTAL_GESTURAL_sori },
} };

TEST(MeiMicrotonalTests, writtenToGesturalKeepsSoundingDisplacement)
{
    for (const GesturalDerivationCase& test : GESTURAL_DERIVATION_CASES) {
        EXPECT_EQ(libmei::Att::AccidentalWrittenToGestural(test.written), test.gestural)
            << static_cast<int>(test.written);
    }
}

TEST(MeiMicrotonalTests, unrepresentableAndUnknownWrittenTokensFailExplicitly)
{
    constexpr std::array<libmei::data_ACCIDENTAL_WRITTEN, 7> unsupported { {
        libmei::ACCIDENTAL_WRITTEN_xs,
        libmei::ACCIDENTAL_WRITTEN_sx,
        libmei::ACCIDENTAL_WRITTEN_bs,
        libmei::ACCIDENTAL_WRITTEN_ks,
        libmei::ACCIDENTAL_WRITTEN_kf,
        libmei::ACCIDENTAL_WRITTEN_kmf,
        libmei::ACCIDENTAL_WRITTEN_MAX,
    } };

    for (libmei::data_ACCIDENTAL_WRITTEN value : unsupported) {
        bool warning = false;
        EXPECT_EQ(Convert::accidFromMEI(value, warning), AccidentalType::NONE)
            << static_cast<int>(value);
        EXPECT_TRUE(warning) << static_cast<int>(value);
    }
}

struct PitchCase {
    libmei::data_ACCIDENTAL_WRITTEN written;
    AccidentalType museScore;
    int basePitch;
    double cents;
};

constexpr std::array<PitchCase, 5> PITCH_CASES { {
    { libmei::ACCIDENTAL_WRITTEN_s, AccidentalType::SHARP, 61, 0.0 },
    { libmei::ACCIDENTAL_WRITTEN_1qs, AccidentalType::SHARP_SLASH, 60, 50.0 },
    { libmei::ACCIDENTAL_WRITTEN_3qs, AccidentalType::SHARP_SLASH4, 60, 150.0 },
    { libmei::ACCIDENTAL_WRITTEN_sd, AccidentalType::SHARP_ARROW_DOWN, 60, 50.0 },
    { libmei::ACCIDENTAL_WRITTEN_su, AccidentalType::SHARP_ARROW_UP, 60, 150.0 },
} };

TEST(MeiMicrotonalTests, pitchTransportSeparatesIntegerPitchAndCentOffset)
{
    for (const PitchCase& test : PITCH_CASES) {
        libmei::Note meiNote;
        meiNote.SetPname(libmei::PITCHNAME_c);
        meiNote.SetOct(4);
        libmei::Accid meiAccid;
        meiAccid.SetAccid(test.written);

        bool warning = true;
        const Convert::PitchStruct pitch = Convert::pitchFromMEI(meiNote, meiAccid, engraving::Interval(), warning);
        EXPECT_FALSE(warning) << static_cast<int>(test.written);
        EXPECT_EQ(pitch.accidType, test.museScore) << static_cast<int>(test.written);
        EXPECT_EQ(pitch.pitch, test.basePitch) << static_cast<int>(test.written);
        EXPECT_DOUBLE_EQ(pitch.centOffset, test.cents) << static_cast<int>(test.written);
        EXPECT_DOUBLE_EQ(pitch.pitch * 100.0 + pitch.centOffset,
                         test.basePitch * 100.0 + test.cents)
            << static_cast<int>(test.written);
    }
}

TEST(MeiMicrotonalTests, explicitGesturalValueOverridesSoundButNotWrittenIdentity)
{
    libmei::Note meiNote;
    meiNote.SetPname(libmei::PITCHNAME_c);
    meiNote.SetOct(4);
    libmei::Accid meiAccid;
    meiAccid.SetAccid(libmei::ACCIDENTAL_WRITTEN_s);
    meiAccid.SetAccidGes(libmei::ACCIDENTAL_GESTURAL_sd);

    bool warning = true;
    const Convert::PitchStruct pitch = Convert::pitchFromMEI(meiNote, meiAccid, engraving::Interval(), warning);
    EXPECT_FALSE(warning);
    EXPECT_EQ(pitch.accidType, AccidentalType::SHARP);
    EXPECT_EQ(pitch.pitch, 60);
    EXPECT_DOUBLE_EQ(pitch.centOffset, 50.0);
    EXPECT_EQ(pitch.tpc2, engraving::step2tpc(0, engraving::AccidentalVal::SHARP));
}

TEST(MeiMicrotonalTests, gesturalMappingRoundTripsAllRepresentableSoundingValues)
{
    constexpr std::array<libmei::data_ACCIDENTAL_GESTURAL, 24> values { {
        libmei::ACCIDENTAL_GESTURAL_n,
        libmei::ACCIDENTAL_GESTURAL_s,
        libmei::ACCIDENTAL_GESTURAL_f,
        libmei::ACCIDENTAL_GESTURAL_ss,
        libmei::ACCIDENTAL_GESTURAL_ff,
        libmei::ACCIDENTAL_GESTURAL_ts,
        libmei::ACCIDENTAL_GESTURAL_tf,
        libmei::ACCIDENTAL_GESTURAL_su,
        libmei::ACCIDENTAL_GESTURAL_sd,
        libmei::ACCIDENTAL_GESTURAL_fu,
        libmei::ACCIDENTAL_GESTURAL_fd,
        libmei::ACCIDENTAL_GESTURAL_xu,
        libmei::ACCIDENTAL_GESTURAL_ffd,
        libmei::ACCIDENTAL_GESTURAL_bms,
        libmei::ACCIDENTAL_GESTURAL_kms,
        libmei::ACCIDENTAL_GESTURAL_bs,
        libmei::ACCIDENTAL_GESTURAL_ks,
        libmei::ACCIDENTAL_GESTURAL_kf,
        libmei::ACCIDENTAL_GESTURAL_bf,
        libmei::ACCIDENTAL_GESTURAL_kmf,
        libmei::ACCIDENTAL_GESTURAL_bmf,
        libmei::ACCIDENTAL_GESTURAL_koron,
        libmei::ACCIDENTAL_GESTURAL_sori,
        libmei::ACCIDENTAL_GESTURAL_NONE,
    } };

    for (libmei::data_ACCIDENTAL_GESTURAL value : values) {
        bool importWarning = true;
        const Convert::AccidentalSemantics semantics = Convert::accidGesFromMEI(value, importWarning);
        EXPECT_FALSE(importWarning) << static_cast<int>(value);

        bool exportWarning = true;
        const libmei::data_ACCIDENTAL_GESTURAL exported = Convert::accidGesToMEI(semantics, exportWarning);
        EXPECT_FALSE(exportWarning) << static_cast<int>(value);
        const libmei::data_ACCIDENTAL_GESTURAL expected
            = value == libmei::ACCIDENTAL_GESTURAL_NONE ? libmei::ACCIDENTAL_GESTURAL_n : value;
        EXPECT_EQ(exported, expected) << static_cast<int>(value);
    }
}

TEST(MeiMicrotonalTests, fullImportAndPitchExportPreserveMicrotonalTokens)
{
    auto importFunc = [](engraving::MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        MeiReader meiReader(nullptr);
        return meiReader.import(score, path);
    };
    std::unique_ptr<engraving::MasterScore> score(
        engraving::ScoreRW::readScore(u"data/microtonal-roundtrip.mei", false, importFunc));
    ASSERT_TRUE(score);

    std::vector<const engraving::Note*> notes;
    score->scanElements([&notes](engraving::EngravingItem* item) {
        if (item->isNote()) {
            notes.push_back(engraving::toNote(item));
        }
    });
    ASSERT_EQ(notes.size(), PITCH_CASES.size());
    constexpr std::array<int, 5> expectedBasePitches { 61, 62, 64, 65, 67 };
    for (size_t i = 0; i < PITCH_CASES.size(); ++i) {
        ASSERT_TRUE(notes[i]->accidental());
        EXPECT_EQ(notes[i]->accidental()->accidentalType(), PITCH_CASES[i].museScore);
        EXPECT_EQ(notes[i]->pitch(), expectedBasePitches[i]);
        EXPECT_DOUBLE_EQ(notes[i]->centOffset(), PITCH_CASES[i].cents);
        const auto converted = Convert::pitchToMEI(
            notes[i], notes[i]->accidental(), engraving::Interval());
        EXPECT_EQ(converted.second.GetAccid(), PITCH_CASES[i].written);
        EXPECT_FALSE(converted.second.HasAccidGes());
    }
}
} // namespace
} // namespace mu::iex::mei
