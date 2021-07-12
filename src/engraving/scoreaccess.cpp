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

using namespace mu::engraving;

Ms::MasterScore* ScoreAccess::createMasterScore()
{
    return new Ms::MasterScore();
}

Ms::MasterScore* ScoreAccess::createMasterScore(const Ms::MStyle& style)
{
    return new Ms::MasterScore(style);
}

Ms::Score::FileError ScoreAccess::loadMscz(Ms::MasterScore* masterScore, mu::engraving::MsczReader& msczFile, bool ignoreVersionError)
{
    return masterScore->loadMscz(msczFile, ignoreVersionError);
}
