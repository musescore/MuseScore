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

#ifndef __REPEATLIST_H__
#define __REPEATLIST_H__

#include <set>
#include <vector>

#include "global/allocator.h"
#include "types/string.h"

namespace mu::engraving {
class Score;
class Measure;
class Volta;
class Jump;
class RepeatListElement;
using RepeatListElementList = std::vector<RepeatListElement*>;

//---------------------------------------------------------
//   RepeatSegment
//---------------------------------------------------------

class RepeatSegment
{
    OBJECT_ALLOCATOR(engraving, RepeatSegment)
public:
    int tick;           // start tick
    int utick;
    double utime;
    double timeOffset;
    double pause;
    int playbackCount;

    RepeatSegment(int playbackCount);

    void addMeasure(Measure const* const);
    void addMeasures(Measure const* const);
    bool containsMeasure(Measure const* const) const;
    bool isEmpty() const;
    int len() const;
    void popMeasure();

    Measure const* firstMeasure() const { return m_measureList.empty() ? nullptr : m_measureList.front(); }
    Measure const* lastMeasure() const { return m_measureList.empty() ? nullptr : m_measureList.back(); }

    const std::vector<const Measure*>& measureList() const;

    friend class RepeatList;
private:
    std::vector<const Measure*> m_measureList;
};

//---------------------------------------------------------
//   RepeatList
//---------------------------------------------------------

class RepeatList : public std::vector<RepeatSegment*>
{
    OBJECT_ALLOCATOR(engraving, RepeatList)

    Score* _score = nullptr;
    mutable unsigned idx1, idx2;     // cached values

    bool _expanded = false;
    bool _scoreChanged = true;

    std::set<std::pair<Jump const* const, int> > _jumpsTaken;     // take the jumps only once, so track them during unwind
    std::vector<RepeatListElementList> _rlElements;   // all elements of the score that influence the RepeatList

    void collectRepeatListElements();
    std::pair<std::vector<RepeatListElementList>::const_iterator, RepeatListElementList::const_iterator> findMarker(
        String label, std::vector<RepeatListElementList>::const_iterator referenceSectionIt,
        RepeatListElementList::const_iterator referenceRepeatListElementIt) const;

    void performJump(std::vector<RepeatListElementList>::const_iterator sectionIt,
                     RepeatListElementList::const_iterator repeatListElementTargetIt, bool withRepeats, int* const playbackCount,
                     Volta const** const activeVolta, RepeatListElement const** const startRepeatReference) const;
    void unwind();
    void flatten();

public:
    RepeatList(Score* s);
    RepeatList(const RepeatList&) = delete;
    RepeatList& operator=(const RepeatList&) = delete;
    ~RepeatList();

    void update(bool expand);
    void setScoreChanged() { _scoreChanged = true; }
    const Score* score() const { return _score; }

    int utick2tick(int tick) const;
    int tick2utick(int tick) const;
    int utime2utick(double secs) const;
    double utick2utime(int) const;
    void updateTempo();
    int ticks() const;

    std::vector<RepeatSegment*>::const_iterator findRepeatSegmentFromUTick(int utick) const;
};
} // namespace mu::engraving
#endif
