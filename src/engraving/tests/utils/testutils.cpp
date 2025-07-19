/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#include "testutils.h"

#include "gtest/gtest.h"

#include "dom/excerpt.h"
#include "dom/masterscore.h"
#include "dom/part.h"
#include "dom/score.h"

using namespace mu;
using namespace mu::engraving;

mu::engraving::Score* mu::engraving::TestUtils::createPart(MasterScore* masterScore, size_t partNumber)
{
    std::vector<Part*> parts;
    parts.push_back(masterScore->parts().at(partNumber));
    Score* nscore = masterScore->createScore();

    Excerpt* ex = new Excerpt(masterScore);
    ex->setExcerptScore(nscore);
    ex->setParts(parts);
    ex->setName(parts.front()->partName());
    Excerpt::createExcerpt(ex);

    masterScore->excerpts().push_back(ex);
    masterScore->setExcerptsChanged(true);

    EXPECT_TRUE(nscore);
    return nscore;
}

void TestUtils::createParts(MasterScore* masterScore, size_t numberOfParts)
{
    EXPECT_TRUE(numberOfParts > 0);

    for (size_t part = 0; part < numberOfParts; part++) {
        createPart(masterScore, part);
    }
}
