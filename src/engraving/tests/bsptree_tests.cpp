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

static const String BSPTREE_DATA_DIR("bsptree_data/");

class Engraving_BspTreeTests : public ::testing::Test
{
};

/**
 * @brief PlaybackModelTests_NearestNeighbor
 * @details Put a single note in a bsp tree, then iterate through the positions of all elements on the page
 *          The nearest neighbour should always be the note
 */
TEST_F(Engraving_BspTreeTests, NearestNeighbor)
{
    // [GIVEN] A BspTree containing a single note
    Score* score = ScoreRW::readScore(BSPTREE_DATA_DIR + "nearest_neighbor/nearest_neighbor.mscx");
    Page* page = score->pages()[0];

    BspTree bsp;
    EngravingItem* note = nullptr;
    bsp.initialize(page->pageBoundingRect(), page->elements().size());
    for (EngravingItem* elem : page->elements()) {
        if (elem->isNote()) {
            bsp.insert(elem);
            note = elem;
        }
    }

    // [WHEN] Iterating through the positions of every element on the page
    for (EngravingItem* elem : page->elements()) {
        // [THEN] The nearest neighbor should always be the note
        ASSERT_EQ(bsp.nearestNeighbor(elem->pagePos()), note);
    }
}
