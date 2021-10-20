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

#ifndef __TESTUTILS_H__
#define __TESTUTILS_H__

#include "libmscore/engravingitem.h"

namespace Ms {
class MScore;
class MasterScore;
class Score;

//---------------------------------------------------------
//   MTest
//---------------------------------------------------------

class MTest
{
protected:
    Ms::MScore* mscore;
    QString root;       // root path of test source

    MTest();
    Ms::MasterScore* readScore(const QString& name);
    bool saveScore(Ms::Score*, const QString& name) const;

    bool compareFiles(const QString& saveName, const QString& compareWith) const;
    bool saveCompareScore(Ms::Score*, const QString& saveName, const QString& compareWith) const;

    void initMTest(const QString& root);

public:
    static bool compareFilesFromPaths(const QString& f1, const QString& f2);
};
}

void initMuseScoreResources();

#endif
