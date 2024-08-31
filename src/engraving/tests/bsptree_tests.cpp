/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include "dom/bsp.h"
#include "dom/page.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String BSPTREE_DATA_DIR(u"bsptree_data/");

class Engraving_BspTreeTests : public ::testing::Test
{
};

/**
 * @brief PlaybackModelTests_NearestNeighbor
 * @details Check that BspTree::nearestNeighbor returns the expected note when passing it a certain position
 */
TEST_F(Engraving_BspTreeTests, NearestNeighbor)
{
    Score* score = ScoreRW::readScore(BSPTREE_DATA_DIR + u"nearest_neighbor.mscx");
    EXPECT_TRUE(score);

    Page* page = score->pages().at(0);
    EXPECT_TRUE(page);
    EXPECT_FALSE(page->elements().empty());

    // [GIVEN] A set of notes in scattered positions, and a BspTree containing those notes
    BspTree bsp;
    std::set<EngravingItem*> notes;
    bsp.initialize(page->pageBoundingRect(), static_cast<int>(page->elements().size()));
    for (EngravingItem* elem : page->elements()) {
        if (elem->isNote()) {
            notes.emplace(elem);
            bsp.insert(elem);
        }
    }

    EXPECT_FALSE(notes.empty());

    // [WHEN] Iterating through the set of notes, and passing each note's position to nearestNeighbor
    for (EngravingItem* note : notes) {
        EngravingItem* nn = bsp.nearestNeighbor(note->pagePos());
        // [THEN] The returned engraving item should match the item whose position was passed in
        EXPECT_EQ(nn, note);
    }

    bsp.clear();

    // [GIVEN] A set of notes in scattered positions, and a BspTree containing a single note
    EngravingItem* singleNote = *notes.begin();
    EXPECT_TRUE(singleNote);

    bsp.initialize(page->pageBoundingRect(), static_cast<int>(page->elements().size()));
    bsp.insert(singleNote);

    // [WHEN] Iterating through the set of notes, and passing each note's position to nearestNeighbor
    for (EngravingItem* note : notes) {
        EngravingItem* nn = bsp.nearestNeighbor(note->pagePos());
        // [THEN] The nearest neighbor should always return singleNote
        EXPECT_EQ(nn, singleNote);
    }
}
