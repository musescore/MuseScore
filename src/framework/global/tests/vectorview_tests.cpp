/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "vectorview.h"

TEST(Global_Types_VectorViewTests, NonEmpty)
{
    std::vector<int> v { 1, 2, 3, 4, 5 };

    muse::VectorView<int> view { v };

    EXPECT_FALSE(view.empty());
}

TEST(Global_Types_VectorViewTests, Empty)
{
    std::vector<int> v;

    muse::VectorView<int> view { v };

    EXPECT_TRUE(view.empty());
}

TEST(Global_Types_VectorViewTests, Front)
{
    std::vector<int> v { 1, 2, 3, 4, 5 };

    muse::VectorView<int> view { v };

    EXPECT_EQ(view.front(), 1);
}

TEST(Global_Types_VectorViewTests, PopFront)
{
    const std::vector<int> original{ 1, 2, 3, 4, 5 };
    std::vector<int> v = original;

    muse::VectorView<int> view{ v };

    EXPECT_EQ(view.pop_front(), 1);

    EXPECT_EQ(v, original);

    EXPECT_EQ(view.begin(), v.begin() + 1);
    EXPECT_EQ(view.end(), v.end());
}

TEST(Global_Types_VectorViewTests, Back)
{
    std::vector<int> v { 1, 2, 3, 4, 5 };

    muse::VectorView<int> view { v };

    EXPECT_EQ(view.back(), 5);
}

TEST(Global_Types_VectorViewTests, PopBack)
{
    const std::vector<int> original{ 1, 2, 3, 4, 5 };
    std::vector<int> v = original;

    muse::VectorView<int> view { v };

    EXPECT_EQ(view.pop_back(), 5);

    EXPECT_EQ(v, original);

    EXPECT_EQ(view.begin(), v.begin());
    EXPECT_EQ(view.end(), v.end() - 1);
}

TEST(Global_Types_VectorViewTests, Remove)
{
    std::vector<int> v { 1, 2, 3, 4, 5, 3, 6 };

    muse::VectorView<int> view { v };

    // Remove element that is in the view
    EXPECT_TRUE(view.remove(3));

    EXPECT_EQ(view.begin(), v.begin());
    EXPECT_EQ(view.end(), v.end() - 2);

    // Can't compare the whole vector, because the values beyond the view are unspecified
    EXPECT_EQ((std::vector(view.begin(), view.end())),
              (std::vector<int> { 1, 2, 4, 5, 6 }));

    // Remove an element that is not in the view
    EXPECT_FALSE(view.remove(10));

    EXPECT_EQ(view.begin(), v.begin());
    EXPECT_EQ(view.end(), v.end() - 2);

    EXPECT_EQ((std::vector(view.begin(), view.end())),
              (std::vector<int> { 1, 2, 4, 5, 6 }));
}

TEST(Global_Types_VectorViewTests, RemoveIf)
{
    std::vector<int> v { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    muse::VectorView<int> view { v };

    auto isEven = [](int n) { return n % 2 == 0; };

    // Predicate sometimes returns true
    EXPECT_TRUE(view.remove_if(isEven));

    EXPECT_EQ(view.begin(), v.begin());
    EXPECT_EQ(view.end(), v.end() - 4);

    EXPECT_EQ((std::vector(view.begin(), view.end())),
              (std::vector<int> { 1, 3, 5, 7, 9 }));

    // Predicate always returns false, so nothing is removed
    EXPECT_FALSE(view.remove_if(isEven));

    EXPECT_EQ(view.begin(), v.begin());
    EXPECT_EQ(view.end(), v.end() - 4);

    EXPECT_EQ((std::vector(view.begin(), view.end())),
              (std::vector<int> { 1, 3, 5, 7, 9 }));
}

TEST(Global_Types_VectorViewTests, ReverseRemove)
{
    std::vector<int> v { 1, 2, 3, 4, 5, 3, 6 };

    muse::VectorView<int> view { v };

    // Remove element that is in the view
    EXPECT_TRUE(view.reverse_remove(3));

    EXPECT_EQ(view.begin(), v.begin() + 2);
    EXPECT_EQ(view.end(), v.end());

    EXPECT_EQ((std::vector(view.begin(), view.end())),
              (std::vector<int> { 1, 2, 4, 5, 6 }));

    // Remove an element that is not in the view
    EXPECT_FALSE(view.reverse_remove(10));

    EXPECT_EQ(view.begin(), v.begin() + 2);
    EXPECT_EQ(view.end(), v.end());

    EXPECT_EQ((std::vector(view.begin(), view.end())),
              (std::vector<int> { 1, 2, 4, 5, 6 }));
}

TEST(Global_Types_VectorViewTests, ReverseRemoveIf)
{
    std::vector<int> v { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    muse::VectorView<int> view { v };

    auto isEven = [](int n) { return n % 2 == 0; };

    // Predicate sometimes returns true
    EXPECT_TRUE(view.reverse_remove_if(isEven));

    EXPECT_EQ(view.begin(), v.begin() + 4);
    EXPECT_EQ(view.end(), v.end());

    EXPECT_EQ((std::vector(view.begin(), view.end())),
              (std::vector<int> { 1, 3, 5, 7, 9 }));

    // Predicate always returns false, so nothing is removed
    EXPECT_FALSE(view.reverse_remove_if(isEven));

    EXPECT_EQ(view.begin(), v.begin() + 4);
    EXPECT_EQ(view.end(), v.end());

    EXPECT_EQ((std::vector(view.begin(), view.end())),
              (std::vector<int> { 1, 3, 5, 7, 9 }));
}
