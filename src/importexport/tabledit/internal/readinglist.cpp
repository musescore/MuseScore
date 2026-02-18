/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "readinglist.h"

namespace mu::iex::tabledit {
bool MeasureStatus::isRegular()
{
    if (barlineDouble || barlineEnd || repeatStart || repeatEnd) {
        return false;
    }

    return true;
}

// return the number of exact matches at the front of the list
// due to compaction, this is rare

size_t findExactMatches(const std::vector<SimplifiedItem>& list)
{
    if (list.size() == 0) {
        return 0; // shouldn't happen
    }
    size_t res { 1 };
    while (res < list.size() && list.at(0) == list.at(res)) {
        ++res;
    }
    return res;
}

// return the number of exact matches at the second items on the list

size_t findExactMatchesFrom2nd(const std::vector<SimplifiedItem>& list)
{
    if (list.size() == 0) {
        return 0; // shouldn't happen
    }
    size_t res { 1 };
    while (res + 1 < list.size() && list.at(1) == list.at(res + 1)) {
        ++res;
    }
    return res;
}

// copy all valid TefReadingListItems to m_list
// with the measure numbers changed from one-based to zero-based
// and items describing consecutive sets of measures merged

void ReadingList::initializeList(const size_t nMeasures, const std::vector<TefReadingListItem>& tefReadingList)
{
    UNUSED(nMeasures);
    for (const auto& tefItem : tefReadingList) {
        if (/* todo: item is valid */ true) {
            SimplifiedItem i;
            i.first = tefItem.firstMeasure - 1;
            i.last = tefItem.lastMeasure - 1;
            if (m_list.empty()) {
                // just add
                m_list.push_back(i);
            } else if (m_list.back().isConsecutiveWith(i)) {
                m_list.back() += i;
            } else {
                // just add
                m_list.push_back(i);
            }
        }
        LOGN("items");
        for (auto i : m_list) {
            LOGN("first %d last %d", i.first + 1, i.last + 1);
        }
    }
}

void ReadingList::calculate(const size_t nMeasures, const std::vector<TefReadingListItem>& tefReadingList)
{
    LOGN("reading list size %zu number of measures %zu", tefReadingList.size(), nMeasures);
    if (nMeasures == 0) {
        return;
    }
    initializeList(nMeasures, tefReadingList);
    m_status.resize(nMeasures);
    for (const auto& stat : m_status) {
        LOGN("1 repeatEnd %d repeatStart %d", stat.repeatEnd, stat.repeatStart);
    }
    if (!m_list.empty()) {
        analyze();
    }
    for (const auto& stat : m_status) {
        LOGN("2 repeatEnd %d repeatStart %d", stat.repeatEnd, stat.repeatStart);
    }
}

void ReadingList::analyze()
{
    // todo: check handling single remaining segment
    while (m_list.size()) {
        size_t itemsUsed { findExactMatches(m_list) };
        LOGN("findExactMatches() count %zu", itemsUsed);
        bool overlapWithNext { m_list.size() > itemsUsed && m_list.at(0).overlapsWith(m_list.at(itemsUsed)) };
        LOGN("overlapWithNext %s", (overlapWithNext ? "true" : "false"));
        if (itemsUsed > 1) {
            m_status[m_list.at(0).first].repeatStart = true;
            m_status[m_list.at(itemsUsed - 1).last].repeatEnd = true;
            if (itemsUsed > 2) {
                m_status[m_list.at(itemsUsed - 1).last].repeatCount = static_cast<int>(itemsUsed);
            }
            m_list.erase(m_list.begin(), m_list.begin() + itemsUsed);
            continue;
        }

        if (m_list.size() >= 2 && m_list.at(0).overlapsWith(m_list.at(1))) {
            LOGN("overlap %d-%d %d-%d", m_list.at(0).first, m_list.at(0).last, m_list.at(1).first, m_list.at(1).last);
            if (m_list.at(0).first <= m_list.at(1).first) {
                m_status[m_list.at(1).first].repeatStart = true;
            } else {
                // TableEdit:
                // starts playing at list.at(1).first
                // repeats from list.at(0).first
                // but does not show where to start playing
                // MuseScore: tbd
                // test file 010.tef
                LOGN("overlap: second starts before first");
                m_status[m_list.at(1).first].repeatStart = true;
            }
            if (m_list.at(0).last < m_list.at(1).last) {
                m_status[m_list.at(0).last].repeatEnd = true;
                itemsUsed = 1;
            } else if (m_list.at(0).last == m_list.at(1).last) {
                m_status[m_list.at(0).last].repeatEnd = true;
                itemsUsed = 2;
            } else {
                // todo: how to handle at(0) not consecutive with at(2) ?
                // todo: handle multiple repeats of first ending (alternative_ending_2a, 4b and 4d)
                // -> first measure in ending 1 gets count = #repeats - 1
                // todo: handle multiple repeats of second ending (alternative_ending_2b and 4e)
                // -> first measure in ending 2 gets count = #repeats - 1
                LOGN("overlap: second ends before first");
                if (m_list.size() > 2 && m_list.at(0).isConsecutiveWith(m_list.at(2))) {
                    // alternative ending
                    m_status[m_list.at(0).last].repeatEnd = true;
                    Ending ending2;
                    ending2.number = 2;
                    m_status[m_list.at(2).first].ending = ending2;
                    const int measuresInEnding1 { m_list.at(0).last - m_list.at(1).last };
                    const size_t ending1Repeats { findExactMatchesFrom2nd(m_list) };
                    LOGN("ending 1 end %d measures %d repeats %zu", m_list.at(0).last, measuresInEnding1, ending1Repeats);
                    Ending ending1;
                    ending1.duration = measuresInEnding1;
                    ending1.number = 1;
                    m_status[m_list.at(0).last - measuresInEnding1 + 1].ending = ending1;
                }
                itemsUsed = 2;
            }
            m_list.erase(m_list.begin(), m_list.begin() + itemsUsed);
            continue;
        }

        itemsUsed = 1;
        m_list.erase(m_list.begin(), m_list.begin() + itemsUsed);
        continue;
    }
    if (m_status.back().isRegular()) {
        m_status.back().barlineEnd = true;
    }
}
} // namespace mu::iex::tabledit
