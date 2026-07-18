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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QtCore/qtypes.h>
#include <gtest/gtest.h>

#include "engraving/api/v1/score.h"
#include "engraving/api/v1/elements.h"

#include "engraving/tests/utils/scorerw.h"

using namespace muse;
using namespace mu::engraving;

static const String SPANNERS_DATA_DIR("../../tests/spanners_data/");

/// Test Plugin API score.spanners property
/// Verify that score.spanners exposes all spanners in the score
TEST(Engraving_ApiSpannerTests, scoreSpanners)
{
    // Load a score file
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"glissando01.mscx");
    ASSERT_TRUE(score);

    // Create Plugin API wrapper for the score
    apiv1::Score apiScore(score);

    // Get spanners using Plugin API
    QQmlListProperty<apiv1::Spanner> scoreSpanners = apiScore.spanners();

    // Basic sanity checks: the property should return a valid list
    EXPECT_NE(scoreSpanners.count, nullptr);
    EXPECT_NE(scoreSpanners.at, nullptr);

    qsizetype spannerCount = scoreSpanners.count(&scoreSpanners);
    EXPECT_GE(spannerCount, 0) << "Count should be non-negative";

    // Get spanners directly from the score for comparison
    auto domSpanners = score->spannerList();

    // The Plugin API should expose the same number of spanners
    EXPECT_EQ(spannerCount, (qsizetype)domSpanners.size())
        << "Plugin API should expose all spanners from the score";

    // Verify each spanner can be accessed and has valid properties
    for (int i = 0; i < spannerCount; i++) {
        auto* item = scoreSpanners.at(&scoreSpanners, i);
        apiv1::Spanner* apiItem = qobject_cast<apiv1::Spanner*>(item);
        ASSERT_TRUE(apiItem) << "Spanner " << i << " should be a valid Spanner API object";
        ASSERT_TRUE(apiItem->spanner()) << "Spanner " << i << " should have a valid underlying Spanner";

        track_idx_t track = apiItem->spanner()->track();
        EXPECT_LT(track, score->ntracks()) << "Spanner " << i << "'s underlying Spanner should not be garbage";
    }

    delete score;
}
