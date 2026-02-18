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
#pragma once

#include <optional>
#include <vector>

#include "importtef.h"

namespace mu::iex::tabledit {
struct Ending {
    int duration { 1 };
    int number { 1 };
};

struct MeasureStatus
{
    bool barlineDouble { false };
    bool barlineEnd { false };
    std::optional<Ending> ending;
    int repeatCount { 0 };
    bool repeatEnd { false };
    bool repeatStart { false };
    bool isRegular();
};

struct SimplifiedItem {
    int first { 0 };
    int last { 0 };
    constexpr bool isConsecutiveWith(const SimplifiedItem& rhs) const { return last + 1 == rhs.first; }
    constexpr bool operator==(const SimplifiedItem& rhs) const { return first == rhs.first && last == rhs.last; }
    constexpr bool overlapsWith(const SimplifiedItem& rhs) const { return !(rhs.last < first || rhs.first > last); }
    constexpr SimplifiedItem& operator+=(const SimplifiedItem& rhs)
    {
        if (isConsecutiveWith(rhs)) {
            last = rhs.last;
        }
        return *this;
    }
};

class ReadingList
{
public:
    void calculate(const size_t nMeasures, const std::vector<TefReadingListItem>& tefReadingList);
    const std::vector<MeasureStatus>& status() const { return m_status; }
private:
    void initializeList(const size_t nMeasures, const std::vector<TefReadingListItem>& tefReadingList);
    void analyze();
    std::vector<SimplifiedItem> m_list;
    std::vector<MeasureStatus> m_status;
};
} // namespace mu::iex::tabledit
