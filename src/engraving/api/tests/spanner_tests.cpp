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

#include "engraving/dom/boxline.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mcursor.h"

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

/// Test Plugin API BoxLine properties
TEST(Engraving_ApiSpannerTests, boxLineProperties)
{
    // A one-staff score with a box over the first two measures
    MCursor c;
    c.setTimeSig(Fraction(4, 4));
    c.createScore(nullptr, u"test");
    c.addPart(u"voice");
    c.move(0, Fraction(0, 1));
    c.addTimeSig(Fraction(4, 4));
    for (int i = 0; i < 2; ++i) {
        c.addChord(60, TDuration(DurationType::V_WHOLE));
    }
    MasterScore* score = c.score();

    BoxLine* box = Factory::createBoxLine(score->dummy());
    box->setTrack(0);
    box->setTrack2(0);
    box->setTick(Fraction(0, 1));
    box->setTicks(Fraction(2, 1));
    score->addSpanner(box);
    score->doLayout();

    apiv1::EngravingItem* apiBox = apiv1::wrap(box, apiv1::Ownership::SCORE);
    EXPECT_DOUBLE_EQ(apiBox->property("boxLineHeight").toDouble(), 2.0);
    EXPECT_FALSE(apiBox->property("boxLineCenterOnStaff").toBool());

    score->startCmd(TranslatableString::untranslatable("API spanner tests"));
    EXPECT_TRUE(apiBox->setProperty("boxLineHeight", 4.5));
    EXPECT_TRUE(apiBox->setProperty("boxLineCenterOnStaff", true));
    score->endCmd();
    EXPECT_EQ(box->boxHeight(), Spatium(4.5));
    EXPECT_TRUE(box->centerOnStaff());

    delete score;
}
