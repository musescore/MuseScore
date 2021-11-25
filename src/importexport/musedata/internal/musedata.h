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

#ifndef __MUSEDATA_H__
#define __MUSEDATA_H__

#include "engraving/types/fraction.h"

namespace Ms {
class Staff;
class Part;
class Score;
class ChordRest;
class Measure;
class Slur;

//---------------------------------------------------------
//   MuseData
//    used importing Musedata files
//---------------------------------------------------------

class MuseData
{
    int _division;
    Fraction curTick;
    QList<QStringList> parts;
    Score* score;
    ChordRest* chordRest;
    int ntuplet;
    Measure* measure;
    int voice;
    Slur* slur[4];

    void musicalAttribute(QString s, Part*);
    void readPart(QStringList sl, Part*);
    void readNote(Part*, const QString& s);
    void readChord(Part*, const QString& s);
    void readRest(Part*, const QString& s);
    void readBackup(const QString& s);
    Measure* createMeasure();
    int countStaves(const QStringList& sl);
    void openSlur(int idx, const Fraction& tick, Staff* staff, int voice);
    void closeSlur(int idx, const Fraction& tick, Staff* staff, int voice);
    QString diacritical(QString);

public:
    MuseData(Score* s) { score = s; }
    bool read(const QString&);
    void convert();
};
} // namespace Ms
#endif
