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
#ifndef MU_ENGRAVING_SCOREACCESS_H
#define MU_ENGRAVING_SCOREACCESS_H

#include "libmscore/masterscore.h"

//! NOTE This is a temporary class for controlling (master)score access
//! See Project class description for detail

namespace mu::engraving::compat {
class ScoreAccess
{
public:

    static mu::engraving::MasterScore* createMasterScore();
    static mu::engraving::MasterScore* createMasterScoreWithBaseStyle();
    static mu::engraving::MasterScore* createMasterScoreWithDefaultStyle();
    static mu::engraving::MasterScore* createMasterScore(const mu::engraving::MStyle& style);

    static bool exportPart(mu::engraving::MscWriter& mscWriter, mu::engraving::Score* partScore);
};
}

#endif // MU_ENGRAVING_SCOREACCESS_H
