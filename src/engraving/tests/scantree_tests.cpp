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

#include "dom/masterscore.h"

#include "utils/scorerw.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

static const String ALL_ELEMENTS_DATA_DIR("all_elements_data/");

//---------------------------------------------------------
//   TestTreeModel
///   Ensures that the tree model is consistent. Starting
///   from Score each element in the tree should have the
///   correct parent element, the one whose children list
///   it appears in.
//---------------------------------------------------------

class Engraving_ScanTreeTests : public ::testing::Test
{
public:
    void tstTree(String file);
    void traverseTree(EngravingObject* element);
};

static String elementToText(EngravingObject* element)
{
    if (element == nullptr) {
        return u"nullptr";
    }
    if (element->isEngravingItem()) {
        return toEngravingItem(element)->accessibleInfo();
    }
    return element->translatedTypeUserName();
}

void Engraving_ScanTreeTests::tstTree(String file)
{
    MasterScore* score = ScoreRW::readScore(ALL_ELEMENTS_DATA_DIR + file);
    traverseTree(score);
}

//---------------------------------------------------------
//   Checks whether parent element of current element is
//   correct, then recursively checks all children.
//---------------------------------------------------------

void Engraving_ScanTreeTests::traverseTree(EngravingObject* element)
{
    EngravingObjectList children = element->scanChildren();
    for (EngravingObject* child : children) {
        // child should never be nullptr
        if (!child) {
            LOGD() << "EngravingItem returned nullptr in treeChild()!";
            LOGD() << "EngravingItem: " << elementToText(element);
            LOGD() << "Number of children: " << children.size();
            LOGD() << "Children: ";
            for (EngravingObject* child2 : children) {
                LOGD() << child2;
            }
        }
        EXPECT_TRUE(child);
        // if parent is not correct print some logging info and exit
        if (child->scanParent() != element) {
            LOGD() << "EngravingItem does not have correct parent!";
            LOGD() << "EngravingItem name: " << elementToText(child);
            LOGD() << "Parent in tree model: " << elementToText(child->scanParent());
            LOGD() << "Expected parent: " << elementToText(element);
        }
        EXPECT_EQ(child->scanParent(), element);

        // recursively apply to the rest of the tree
        traverseTree(child);
    }
}

TEST_F(Engraving_ScanTreeTests, tstTreeElements)
{
    tstTree(u"layout_elements.mscx");
}

TEST_F(Engraving_ScanTreeTests, tstTreeTablature)
{
    tstTree(u"layout_elements_tab.mscx");
}

TEST_F(Engraving_ScanTreeTests, tstTreeMoonlight)
{
    tstTree(u"moonlight.mscx");
}

TEST_F(Engraving_ScanTreeTests, DISABLED_tstTreeGoldberg) // too long
{
    tstTree(u"goldberg.mscx");
}
