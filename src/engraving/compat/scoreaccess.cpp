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

using namespace mu::engraving::compat;

Ms::MasterScore* ScoreAccess::createMasterScore()
{
    return new Ms::MasterScore(nullptr);
}

Ms::MasterScore* ScoreAccess::createMasterScoreWithBaseStyle()
{
    return new Ms::MasterScore(DefaultStyle::baseStyle(), nullptr);
}

Ms::MasterScore* ScoreAccess::createMasterScoreWithDefaultStyle()
{
    return new Ms::MasterScore(DefaultStyle::defaultStyle(), nullptr);
}

Ms::MasterScore* ScoreAccess::createMasterScore(const Ms::MStyle& style)
{
    return new Ms::MasterScore(style, nullptr);
}

Ms::Score::FileError ScoreAccess::loadMscz(Ms::MasterScore* masterScore, mu::engraving::MscReader& msczFile, bool ignoreVersionError)
{
    return masterScore->loadMscz(msczFile, ignoreVersionError);
}
