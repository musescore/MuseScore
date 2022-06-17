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

#ifndef MU_ENGRAVING_SCORERW_H
#define MU_ENGRAVING_SCORERW_H

#include "types/string.h"

#include "engraving/libmscore/masterscore.h"

namespace mu::engraving {
class ScoreRW
{
public:
    ScoreRW() = default;

    static String rootPath();

    static MasterScore* readScore(const String& path, bool isAbsolutePath = false);
    static bool saveScore(Score* score, const String& name);
    static EngravingItem* writeReadElement(EngravingItem* element);
    static bool saveMimeData(ByteArray mimeData, const String& saveName);
};
}

#endif // MU_ENGRAVING_SCORERW_H
