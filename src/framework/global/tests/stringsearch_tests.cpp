/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "global/stringsearch.h"

using ::testing::ElementsAre;

namespace muse {
TEST(Global_StringSearchTests, testFuzzySearchEdgeCases)
{
    FuzzyMatcher matcher;
    EXPECT_THAT(matcher(U"", U""), ElementsAre(FuzzyMatch { 0, 0, 0 }));
    EXPECT_THAT(matcher(U"surgery", U""), ElementsAre(FuzzyMatch { 0, 0, 0 }, FuzzyMatch { 1, 1, 0 },
                                                      FuzzyMatch { 2, 2, 0 }, FuzzyMatch { 3, 3, 0 },
                                                      FuzzyMatch { 4, 4, 0 }, FuzzyMatch { 5, 5, 0 },
                                                      FuzzyMatch { 6, 6, 0 }, FuzzyMatch { 7, 7, 0 }));
    EXPECT_THAT(matcher(U"", U"survey"), ElementsAre(FuzzyMatch { 0, 0, 6 }));
}

TEST(Global_StringSearchTests, testFuzzySearch)
{
    FuzzyMatcher matcher(2);
    EXPECT_THAT(matcher(U"surgery", U"survey"), ElementsAre(FuzzyMatch { 0, 5, 2 }, FuzzyMatch { 0, 6, 2 },
                                                            FuzzyMatch { 0, 7, 2 }));

    // exact
    matcher.reset(0);
    EXPECT_THAT(matcher(U"hayhayneedlehayhayhay", U"needle"), ElementsAre(FuzzyMatch { 6, 12, 0 }));

    matcher.reset(1);
    // 1 insertion
    EXPECT_THAT(matcher(U"hayhayneedlehayhayhay", U"nedle"), ElementsAre(FuzzyMatch { 6, 12, 1 }));
    // 1 change
    EXPECT_THAT(matcher(U"hayhayneedlehayhayhay", U"nnedle"), ElementsAre(FuzzyMatch { 6, 12, 1 }));
    // 1 deletion
    EXPECT_THAT(matcher(U"hayhayneedlehayhayhay", U"neoedle"), ElementsAre(FuzzyMatch { 6, 12, 1 }));
    // 1 transposition
    EXPECT_THAT(matcher(U"hayhayneedlehayhayhay", U"neelde"), ElementsAre(FuzzyMatch { 6, 12, 1 }));

    // multiple matches with different edit distance
    EXPECT_THAT(matcher(U"haynedlehayneedlehayhayhay", U"needle"),
                ElementsAre(FuzzyMatch { 3, 8, 1 }, FuzzyMatch { 11, 16, 1 }, FuzzyMatch { 11, 17, 0 },
                            FuzzyMatch { 11, 18, 1 }));
    matcher.reset(0);
    // return best match
    EXPECT_THAT(matcher(U"haynedlehayneedlehayhayhay", U"needle"), ElementsAre(FuzzyMatch { 11, 17, 0 }));
    // correct start pos
    EXPECT_THAT(matcher(U"aaab", U"ab"), ElementsAre(FuzzyMatch { 2, 4, 0 }));
}
}
