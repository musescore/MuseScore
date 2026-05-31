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

// Staff scale (MAG) import: score size header field, Encore 4.x LINE per-staff hint, and the v0xA6 global size.

#include <gtest/gtest.h>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"

#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_StaffSize : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

// Encore scoreSize=2 -> Staff Properties Scale = 75 % (Pid::MAG = 0.75).
TEST_F(Tst_StaffSize, score_size2_sets_staff_scale_75pct)
{
    MasterScore* score = readEncoreScore("importer_score_size2.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_score_size2.enc";
    ASSERT_FALSE(score->staves().empty());
    const double mag = score->staff(0)->staffType(mu::engraving::Fraction(0, 1))->userMag();
    EXPECT_NEAR(mag, 0.75, 1e-6) << "Expected staff scale 75 % for scoreSize=2, got " << mag * 100 << " %";
    delete score;
}

// Encore scoreSize=3 -> Staff Properties Scale = 100 % (Pid::MAG = 1.00).
TEST_F(Tst_StaffSize, score_size3_sets_staff_scale_100pct)
{
    MasterScore* score = readEncoreScore("importer_score_size3.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_score_size3.enc";
    ASSERT_FALSE(score->staves().empty());
    const double mag = score->staff(0)->staffType(mu::engraving::Fraction(0, 1))->userMag();
    EXPECT_NEAR(mag, 1.00, 1e-6) << "Expected staff scale 100 % for scoreSize=3, got " << mag * 100 << " %";
    delete score;
}

// Encore 4.x staff size comes from the LINE staff entry byte, not the unrelated header field.
// See ENCORE_FORMAT.md §System block (LINE). Regression guard: byte[13]=1 -> Size=2 -> 75%.
TEST_F(Tst_StaffSize, enc4x_line_staff_size_hint_size2_sets_75pct)
{
    MasterScore* score = readEncoreScore("importer_enc4x_line_size2_70pct.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_enc4x_line_size2_70pct.enc";
    ASSERT_FALSE(score->staves().empty());
    const double mag = score->staff(0)->staffType(mu::engraving::Fraction(0, 1))->userMag();
    EXPECT_NEAR(mag, 0.75, 1e-6)
        << "Encore 4.x: byte[13]=1 in LINE staff entry must yield 75% scale, got " << mag * 100 << "%";
    delete score;
}

// Encore 4.x: byte[13]=2 -> Size=3 -> 100%. The header field must not override the LINE-derived size.
TEST_F(Tst_StaffSize, enc4x_line_staff_size_hint_size3_sets_100pct)
{
    MasterScore* score = readEncoreScore("importer_enc4x_line_size3_75pct.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_enc4x_line_size3_75pct.enc";
    ASSERT_FALSE(score->staves().empty());
    const double mag = score->staff(0)->staffType(mu::engraving::Fraction(0, 1))->userMag();
    EXPECT_NEAR(mag, 1.00, 1e-6)
        << "Encore 4.x: byte[13]=2 in LINE staff entry must yield 100% scale, got " << mag * 100 << "%";
    delete score;
}
