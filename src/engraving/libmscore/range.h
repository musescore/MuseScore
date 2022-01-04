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

#ifndef __DURATIONLIST_H__
#define __DURATIONLIST_H__

#include "types/fraction.h"

namespace Ms {
class EngravingItem;
class Measure;
class Tuplet;
class Segment;
class Spanner;
class ScoreRange;
class ChordRest;
class Score;

//---------------------------------------------------------
//   TrackList
//---------------------------------------------------------

class TrackList : public QList<EngravingItem*>
{
    Fraction _duration;
    ScoreRange* _range;
    int _track { 0 };

    Tuplet* writeTuplet(Tuplet* parent, Tuplet* tuplet, Measure*& measure, Fraction& rest) const;
    void append(EngravingItem*);
    void appendTuplet(Tuplet* srcTuplet, Tuplet* dstTuplet);
    void combineTuplet(Tuplet* dst, Tuplet* src);

public:
    TrackList(ScoreRange* r) { _range = r; }
    ~TrackList();

    Fraction ticks() const { return _duration; }
    ScoreRange* range() const { return _range; }

    int track() const { return _track; }
    void setTrack(int val) { _track = val; }

    void read(const Segment* fs, const Segment* ls);
    bool write(Score*, const Fraction&) const;

    void appendGap(const Fraction&, Ms::Score* score);
    bool truncate(const Fraction&);
    void dump() const;
};

//---------------------------------------------------------
//   Annotation
//---------------------------------------------------------

struct Annotation {
    Fraction tick;
    EngravingItem* e;
};

//---------------------------------------------------------
//   ScoreRange
//---------------------------------------------------------

class ScoreRange
{
    QList<TrackList*> tracks;
    Segment* _first;
    Segment* _last;

protected:
    QList<Spanner*> spanner;
    QList<Annotation> annotations;

public:
    ScoreRange() {}
    ~ScoreRange();
    void read(Segment* first, Segment* last, bool readSpanner = true);
    bool write(Score*, const Fraction&) const;
    Fraction ticks() const;
    Segment* first() const { return _first; }
    Segment* last() const { return _last; }
    void fill(const Fraction&);
    bool truncate(const Fraction&);

    friend class TrackList;
};
}     // namespace Ms
#endif
