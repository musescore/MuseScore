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

#include <QString>

#include "engraving/libmscore/masterscore.h"

namespace mu::engraving {
class ScoreRW
{
public:
    ScoreRW() = default;

    static QString rootPath();

    static Ms::MasterScore* readScore(const QString& path, bool isAbsolutePath = false);
    static bool saveScore(Ms::Score* score, const QString& name);
    static Ms::EngravingItem* writeReadElement(Ms::EngravingItem* element);
    static bool saveMimeData(QByteArray mimeData, const QString& saveName);
};
}

#endif // MU_ENGRAVING_SCORERW_H
