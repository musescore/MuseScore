/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

namespace mu::engraving {
class EngravingItem;
class Staff;
class Part;
class Score;
class ChordRest;
class Measure;
class Slur;
}

namespace mu::iex::musedata {
//---------------------------------------------------------
//   MuseData
//    used importing Musedata files
//---------------------------------------------------------

class MuseData
{
    int _division;
    engraving::Fraction curTick;
    QList<QStringList> parts;
    engraving::Score* score;
    engraving::ChordRest* chordRest;
    int ntuplet;
    engraving::Measure* measure;
    int voice;
    engraving::Slur* slur[4];

    void musicalAttribute(QStringView s, engraving::Part*);
    void readPart(const QStringList& sl, engraving::Part*);
    void readNote(engraving::Part*, QStringView s);
    void readChord(engraving::Part*, QStringView s);
    void readRest(engraving::Part*, QStringView s);
    void readBackup(QStringView s);
    engraving::Measure* createMeasure();
    int countStaves(const QStringList& sl);
    void openSlur(int idx, const engraving::Fraction& tick, engraving::Staff* staff, int voice, mu::engraving::EngravingItem* startChord);
    void closeSlur(int idx, const engraving::Fraction& tick, engraving::Staff* staff, int voice, engraving::EngravingItem* endChord);
    QString diacritical(QStringView);

public:
    MuseData(engraving::Score* s) { score = s; }
    bool read(const QString&);
    void convert();
};
} // namespace Ms
#endif
