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
#include <unordered_map>
#include <vector>

#include "types/string.h"
#include "types/types.h"

namespace mu::engraving {
class Score;
class Measure;
class Volta;
class Jump;

using MeasureRepeatsStaffMapping = std::unordered_map<const Measure*, const Measure*>;
using MeasureRepeatsMapping = std::unordered_map<staff_idx_t, MeasureRepeatsStaffMapping>;

class RepeatListElement;
using RepeatListElementList = std::vector<RepeatListElement*>;

//---------------------------------------------------------
//   RepeatSegment
//---------------------------------------------------------

class RepeatSegment
{
public:
    int tick;           // start tick
    int utick;
    double utime;
    double timeOffset;
    double pause;
    int playbackCount;

    RepeatSegment(int playbackCount);

    bool isEmpty() const;
    int len() const;

    Measure const* firstMeasure() const { return m_measureList.empty() ? nullptr : m_measureList.front(); }
    Measure const* lastMeasure() const { return m_measureList.empty() ? nullptr : m_measureList.back(); }

    /// The general measure list of this repeat segment
    const std::vector<const Measure*>& measureList() const;

    /// The measure list per staff, taking measure repeats into account
    const std::vector<const Measure*>& measureList(staff_idx_t staffIdx) const;

private:
    friend class RepeatList;

    bool addMeasure(const Measure* measure);
    void addMeasure(const Measure* measure, staff_idx_t staffIdx);
    void popMeasure();

    std::vector<const Measure*> m_measureList;
    std::unordered_map<staff_idx_t, std::vector<const Measure*> > m_measureLists;
};

//---------------------------------------------------------
//   RepeatList
//---------------------------------------------------------

class RepeatList : public std::vector<RepeatSegment*>
{
public:
    RepeatList(Score* s);
    RepeatList(const RepeatList&) = delete;
    RepeatList& operator=(const RepeatList&) = delete;
    ~RepeatList();

    void update(bool expand);
    void setScoreChanged() { m_scoreChanged = true; }
    const Score* score() const { return m_score; }

    int utick2tick(int tick) const;
    int tick2utick(int tick) const;
    int utime2utick(double secs) const;
    double utick2utime(int) const;
    void updateTempo();
    int ticks() const;

    std::vector<RepeatSegment*>::const_iterator findRepeatSegmentFromUTick(int utick) const;

private:
    void unwind();
    void flatten();

    void collectRepeatListElements();
    std::pair<std::vector<RepeatListElementList>::const_iterator, RepeatListElementList::const_iterator> findMarker(
        String label, std::vector<RepeatListElementList>::const_iterator referenceSectionIt,
        RepeatListElementList::const_iterator referenceRepeatListElementIt) const;

    void performJump(std::vector<RepeatListElementList>::const_iterator sectionIt,
                     RepeatListElementList::const_iterator repeatListElementTargetIt, bool withRepeats, int* const playbackCount,
                     Volta const** const activeVolta, RepeatListElement const** const startRepeatReference) const;

    void updateMeasureRepeatsMapping();
    const Measure* playbackMeasure(staff_idx_t staffIdx, const Measure* measure) const;

    void addMeasureToRepeatSegment(RepeatSegment* segment, const Measure* measure);

    /// Adds all measures up to and including measure to the repeat segment
    void addUpToMeasureToRepeatSegment(RepeatSegment* segment, const Measure* measure);

    Score* m_score = nullptr;
    mutable unsigned m_idx1, m_idx2; // cached values

    bool m_expanded = false;
    bool m_scoreChanged = true;

    std::set<std::pair<Jump const* const, int> > m_jumpsTaken; // take the jumps only once, so track them during unwind
    std::vector<RepeatListElementList> m_rlElements; // all elements of the score that influence the RepeatList

    MeasureRepeatsMapping m_measureRepeatsMapping;
};
} // namespace mu::engraving
#endif
