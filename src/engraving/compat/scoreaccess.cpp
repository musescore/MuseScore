/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "scoreaccess.h"
#include "style/defaultstyle.h"
#include "libmscore/masterscore.h"

using namespace mu::engraving::compat;

mu::engraving::MasterScore* ScoreAccess::createMasterScore()
{
    return new mu::engraving::MasterScore();
}

mu::engraving::MasterScore* ScoreAccess::createMasterScoreWithBaseStyle()
{
    return new mu::engraving::MasterScore(DefaultStyle::baseStyle());
}

mu::engraving::MasterScore* ScoreAccess::createMasterScoreWithDefaultStyle()
{
    return new mu::engraving::MasterScore(DefaultStyle::defaultStyle());
}

mu::engraving::MasterScore* ScoreAccess::createMasterScore(const mu::engraving::MStyle& style)
{
    return new mu::engraving::MasterScore(style);
}

bool ScoreAccess::exportPart(mu::engraving::MscWriter& mscWriter, mu::engraving::Score* partScore)
{
    return partScore->masterScore()->exportPart(mscWriter, partScore);
}
