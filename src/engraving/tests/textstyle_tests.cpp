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

#include "engraving/style/textstyle.h"
#include "engraving/types/typesconv.h"

using namespace mu::engraving;

class Engraving_TextStyleTests : public ::testing::Test
{
};

//! Every text style must fill all TEXT_STYLE_SIZE entries. A short initializer list
//! compiles, but leaves the remaining entries zeroed ({ Undefined, Sid::pageWidth, Pid::SUBTYPE }),
TEST_F(Engraving_TextStyleTests, allEntriesDefined)
{
    for (int t = 0; t < int(TextStyleType::TEXT_TYPES); ++t) {
        const TextStyleType type = TextStyleType(t);
        const TextStyle* style = textStyle(type);
        ASSERT_TRUE(style) << TConv::toXml(type).ascii();

        size_t idx = 0;
        for (const TextStyleProperty& prop : *style) {
            EXPECT_NE(prop.type, TextStylePropertyType::Undefined)
                << "Text style " << TConv::toXml(type).ascii() << " has an undefined entry at index " << idx;
            ++idx;
        }
    }
}
