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

#ifndef MU_ENGRAVING_SCORECOMP_H
#define MU_ENGRAVING_SCORECOMP_H

#include "engraving/libmscore/score.h"

namespace mu::engraving {
class ScoreComp
{
public:

    static bool saveCompareScore(Ms::Score*, const QString& saveName, const QString& compareWithLocalPath);
    static bool saveCompareMimeData(QByteArray mimeData, const QString& saveName, const QString& compareWithLocalPath);
    static bool compareFiles(const QString& fullPath1, const QString& fullPath2);
};
}

#endif // MU_ENGRAVING_SCORECOMP_H
