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

#include "engraving/compat/engravingcompat.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/gradualtempochange.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/sticking.h"
#include "engraving/dom/tempotext.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String COMPAT400_DATA_DIR("compat400_data/");

class Engraving_Compat400Tests : public ::testing::Test
{
public:
    MasterScore* compat(const char* filename);
};

MasterScore* Engraving_Compat400Tests::compat(const char* filename)
{
    String _filename = String::fromUtf8(filename);
    MasterScore* score = ScoreRW::readScore(COMPAT400_DATA_DIR + _filename + u".mscx");
    if (!score) {
        return nullptr;
    }

    compat::EngravingCompat::doPostLayoutCompatIfNeeded(score);

    return score;
}

TEST_F(Engraving_Compat400Tests, offsetAlignHarmony)
{
    MasterScore* score = compat("offset-align-harmony");
    ASSERT_TRUE(score);

    int harmonyCount = 0;
    for (Segment* s = score->firstMeasure()->first(); s; s = s->next1()) {
        EngravingItem* item = s->findAnnotation(ElementType::HARMONY, 0, 0);
        if (!item) {
            continue;
        }
        Harmony* harmony = toHarmony(item);
        EXPECT_EQ(harmony->offset().y() / harmony->spatium(), -4.0);
        ++harmonyCount;
    }
    EXPECT_EQ(harmonyCount, 4);

    delete score;
}

TEST_F(Engraving_Compat400Tests, offsetAlignSticking)
{
    MasterScore* score = compat("offset-align-sticking");
    ASSERT_TRUE(score);

    int stickingCount = 0;
    for (Segment* s = score->firstMeasure()->first(); s; s = s->next1()) {
        EngravingItem* item = s->findAnnotation(ElementType::STICKING, 0, 0);
        if (!item) {
            continue;
        }
        Sticking* sticking = toSticking(item);
        EXPECT_EQ(sticking->offset().y() / sticking->spatium(), 4.5);
        ++stickingCount;
    }
    EXPECT_EQ(stickingCount, 3);

    delete score;
}

TEST_F(Engraving_Compat400Tests, offsetAlignDynamics)
{
    MasterScore* score = compat("offset-align-dynamics");
    ASSERT_TRUE(score);

    int dynamicCount = 0;
    for (Segment* s = score->firstMeasure()->first(); s; s = s->next1()) {
        EngravingItem* item = s->findAnnotation(ElementType::DYNAMIC, 0, 0);
        if (!item) {
            continue;
        }
        Dynamic* dynamic = toDynamic(item);
        EXPECT_EQ(dynamic->offset().y() / dynamic->spatium(), 4.5);
        ++dynamicCount;
    }
    EXPECT_EQ(dynamicCount, 2);

    int hairpinSegmentCount = 0;
    for (auto it = score->spannerMap().cbegin(); it != score->spannerMap().cend(); ++it) {
        Spanner* spanner = it->second;
        if (!spanner->isHairpin()) {
            continue;
        }
        for (SpannerSegment* seg : spanner->spannerSegments()) {
            EXPECT_EQ(seg->offset().y() / seg->spatium(), 4.5);
            ++hairpinSegmentCount;
        }
    }
    EXPECT_EQ(hairpinSegmentCount, 1);

    delete score;
}

TEST_F(Engraving_Compat400Tests, offsetAlignTempo)
{
    MasterScore* score = compat("offset-align-tempo");
    ASSERT_TRUE(score);

    int tempoTextCount = 0;
    for (Segment* s = score->firstMeasure()->first(); s; s = s->next1()) {
        EngravingItem* item = s->findAnnotation(ElementType::TEMPO_TEXT, 0, 0);
        if (!item) {
            continue;
        }
        TempoText* tempoText = toTempoText(item);
        EXPECT_EQ(tempoText->offset().y() / tempoText->spatium(), -4.5);
        ++tempoTextCount;
    }
    EXPECT_EQ(tempoTextCount, 1);

    int tempoChangeSegmentCount = 0;
    for (auto it = score->spannerMap().cbegin(); it != score->spannerMap().cend(); ++it) {
        Spanner* spanner = it->second;
        if (!spanner->isGradualTempoChange()) {
            continue;
        }
        for (SpannerSegment* seg : spanner->spannerSegments()) {
            EXPECT_EQ(seg->offset().y() / seg->spatium(), -4.5);
            ++tempoChangeSegmentCount;
        }
    }
    EXPECT_EQ(tempoChangeSegmentCount, 1);

    delete score;
}
