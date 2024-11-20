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

#include <gtest/gtest.h>

#include "dom/lyrics.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/page.h"
#include "dom/rest.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "dom/tuplet.h"
#include "dom/note.h"

#include "utils/scorerw.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

static const String ALL_ELEMENTS_DATA_DIR("all_elements_data/");

class Engraving_LayoutElementsTests : public ::testing::Test
{
public:
    void tstLayoutAll(String file);
};

//---------------------------------------------------------
//   isLayoutDone
//    For use with Score::scanElements in tstLayoutAll
//    sets data (treated as bool*) to false if element is
//    not laid out, otherwise doesn't change the value of
//    data.
//---------------------------------------------------------

static void isLayoutDone(void* data, EngravingItem* e)
{
    bool* result = static_cast<bool*>(data);
    if (e->isTuplet()) {
        Tuplet* t = toTuplet(e);
        if (!t->hasBracket() || !t->number()) {
            // in this case tuplet will not have valid bbox.
            // TODO: how to check this case?
            return;
        }
    }
    if (e->isTimeSig()) {
        const Staff* st = e->staff();
        if (!st->staffType(e->tick())->genTimesig()) {
            // Some staff types require not to have a time
            // signature displayed. This is a valid exception.
            return;
        }
    }
    if (e->isRest() && toRest(e)->shouldNotBeDrawn()) {
        // another valid exception
        return;
    }
    if (e->isTimeTickAnchor()) {
        // not expected to be laid out
        return;
    }
    if (e->isLyricsLineSegment() && toLyricsLineSegment(e)->lyricsLine()->isEndMelisma()) {
        // Melisma line may be omitted if too short
        return;
    }
    if (e->isLayoutBreak() || e->isSystemLockIndicator()) {
        return;
    }

    // If layout of element is done it (usually?) has a valid
    // bounding box (bbox).
    if (e->visible() && !e->ldata()->bbox().isValid()) {
        (*result) = false;
        // Print some info about the element to make test more useful...
        if (Measure* m = toMeasure(e->findMeasure())) {
            LOGD("Layout of %s is not done (page %zu, measure %d)", e->typeName(), m->system()->page()->no() + 1,
                 m->no() + 1);
        } else {
            LOGD("Layout of %s is not done", e->typeName());
        }
    }
}

//---------------------------------------------------------
//   tstLayoutAll
//    Test that all elements in the score are laid out
//---------------------------------------------------------

void Engraving_LayoutElementsTests::tstLayoutAll(String file)
{
    MasterScore* score = ScoreRW::readScore(ALL_ELEMENTS_DATA_DIR + file);
    // readScore should also do layout of the score

    for (LayoutMode mode : { LayoutMode::PAGE }) {
        score->setLayoutMode(mode);
        bool layoutDone = true;
        for (Score* s : score->scoreList()) {
            s->scanElements(&layoutDone, isLayoutDone, /* all */ true);
            EXPECT_TRUE(layoutDone);
        }
    }
}

TEST_F(Engraving_LayoutElementsTests, tstLayoutElements)
{
    tstLayoutAll(u"layout_elements.mscx");
}

TEST_F(Engraving_LayoutElementsTests, tstLayoutTablature)
{
    tstLayoutAll(u"layout_elements_tab.mscx");
}

TEST_F(Engraving_LayoutElementsTests, tstLayoutMoonlight)
{
    tstLayoutAll(u"moonlight.mscx");
}

// FIXME goldberg.mscx does not pass the test because of some
// TimeSig and Clef elements. Need to check it later!
TEST_F(Engraving_LayoutElementsTests, DISABLED_tstLayoutGoldberg)
{
    tstLayoutAll(u"goldberg.mscx");
}

TEST_F(Engraving_LayoutElementsTests, tstLayoutCrossStaffArp)
{
    MasterScore* score = ScoreRW::readScore(ALL_ELEMENTS_DATA_DIR + "cross_staff_arp.mscx");
    EXPECT_TRUE(score);

    // the y-position where the bottom staff is
    double staff1yPre = score->systems().front()->staves().at(1)->y();

    // re-layout the score
    score->update();
    score->doLayout();

    // the bottom staff should not have moved
    double staff1yPost = score->systems().front()->staves().at(1)->y();
    EXPECT_FLOAT_EQ(staff1yPre, staff1yPost);

    delete score;
}
