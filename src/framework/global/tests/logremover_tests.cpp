/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "logremover.h"

namespace mu {
class LogRemoverTests : public ::testing::Test
{
public:
};

TEST_F(LogRemoverTests, ParseDate)
{
    EXPECT_EQ(LogRemover::parseDate("path/to/logs/MuseScore_210629_154033.log"), QDate(2021, 6, 29));
    EXPECT_EQ(LogRemover::parseDate("path/to_to/logs/MuseScore_210709_154033.log"), QDate(2021, 7, 9));
    EXPECT_EQ(LogRemover::parseDate("path/to/logs/MuseScore_210709__154033.log"), QDate());
    EXPECT_EQ(LogRemover::parseDate("path/to/logs/MuseScore_210709_154033_.log"), QDate());
    EXPECT_EQ(LogRemover::parseDate("path/to/logs/MuseScore_210709_154033-.log"), QDate(2021, 7, 9));
}
}
