/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <cstddef>
#include <iterator>
#include <optional>
#include <string_view>
#include <vector>

namespace muse {
struct FuzzyMatch {
    std::size_t beginPos = 0;
    std::size_t endPos = 0;
    std::size_t editDistance = 0;
};

bool operator==(const FuzzyMatch&, const FuzzyMatch&);
bool operator!=(const FuzzyMatch&, const FuzzyMatch&);

class FuzzyMatcher
{
public:
    using value_type = FuzzyMatch;

    static constexpr auto UNLIMITED_DISTANCE = static_cast<std::size_t>(-1);

    explicit FuzzyMatcher(std::size_t maxDistance = UNLIMITED_DISTANCE);

    FuzzyMatcher& operator()(std::u32string_view text, std::u32string_view pattern);
    FuzzyMatcher& match(std::u32string_view text, std::u32string_view pattern);

    bool empty() const;
    std::optional<FuzzyMatch> findMatch(std::size_t textEnd) const;

    void reset(std::size_t maxDistance = UNLIMITED_DISTANCE);
    void clear();

    std::size_t maxDistance() const;

    class Iterator
    {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = FuzzyMatcher::value_type;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::input_iterator_tag;

        Iterator() = default;
        explicit Iterator(const FuzzyMatcher&);

        reference operator*() const;
        pointer operator->() const;

        Iterator& operator++();
        Iterator operator++(int);

        bool operator==(const Iterator&) const;
        bool operator!=(const Iterator&) const;

    private:
        void next(std::size_t textEnd);

        const FuzzyMatcher* m_matcher = nullptr;
        std::optional<FuzzyMatch> m_match;
    };

    using const_iterator = Iterator;

    const_iterator begin() const;
    const_iterator end() const;

private:
    struct PrefixMatch {
        std::size_t beginPos = 0;
        std::size_t distance = 0;

        friend bool operator<(const PrefixMatch& left, const PrefixMatch& right)
        {
            return left.distance < right.distance;
        }
    };

    std::size_t index(std::size_t patternSize, std::size_t textSize) const;

    std::size_t m_numPatternPrefixes = 0;
    std::size_t m_numTextPrefixes = 0;
    std::size_t m_maxDistance = 0;
    std::vector<PrefixMatch> m_prefixTable;
};
}
