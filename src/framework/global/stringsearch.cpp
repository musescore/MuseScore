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

#include "stringsearch.h"

#include <algorithm>

bool muse::operator==(const FuzzyMatch& left, const FuzzyMatch& right)
{
    return left.beginPos == right.beginPos
           && left.endPos == right.endPos
           && left.editDistance == right.editDistance;
}

bool muse::operator!=(const FuzzyMatch& left, const FuzzyMatch& right)
{
    return !(left == right);
}

namespace muse {
FuzzyMatcher::FuzzyMatcher(const std::size_t maxDistance)
    : m_maxDistance{maxDistance}
{
}

FuzzyMatcher& FuzzyMatcher::operator()(const std::u32string_view text, const std::u32string_view pattern)
{
    return match(text, pattern);
}

FuzzyMatcher& FuzzyMatcher::match(const std::u32string_view text, const std::u32string_view pattern)
{
    m_numPatternPrefixes = pattern.size() + 1;
    m_numTextPrefixes = text.size() + 1;
    m_prefixTable.clear();
    m_prefixTable.resize(m_numPatternPrefixes * m_numTextPrefixes);

    for (std::size_t textSize = 0; textSize < m_numTextPrefixes; ++textSize) {
        // 0 insertions to transform empty pattern to any text prefix.
        // This allows the match to start at any position in text.
        m_prefixTable[index(0, textSize)] = PrefixMatch{ textSize, 0 };
    }

    for (std::size_t patternSize = 1; patternSize < m_numPatternPrefixes; ++patternSize) {
        // patternSize deletions to transform any pattern prefix to empty text
        m_prefixTable[index(patternSize, 0)] = PrefixMatch{ 0, patternSize };

        for (std::size_t textSize = 1; textSize < m_numTextPrefixes; ++textSize) {
            if (pattern[patternSize - 1] == text[textSize - 1]) {
                m_prefixTable[index(patternSize, textSize)] = m_prefixTable[index(patternSize - 1, textSize - 1)];
                continue;
            }

            const PrefixMatch insertMatch = m_prefixTable[index(patternSize, textSize - 1)];
            const PrefixMatch replaceMatch = m_prefixTable[index(patternSize - 1, textSize - 1)];
            const PrefixMatch deleteMatch = m_prefixTable[index(patternSize - 1, textSize)];
            PrefixMatch minMatch = std::min(insertMatch, std::min(replaceMatch, deleteMatch));

            if (patternSize > 1 && textSize > 1
                && pattern[patternSize - 1] == text[textSize - 2]
                && pattern[patternSize - 2] == text[textSize - 1]) {
                const PrefixMatch transpositionMatch = m_prefixTable[index(patternSize - 2, textSize - 2)];
                minMatch = std::min(transpositionMatch, minMatch);
            }

            m_prefixTable[index(patternSize, textSize)] = PrefixMatch{ minMatch.beginPos, minMatch.distance + 1 };
        }
    }

    return *this;
}

std::optional<FuzzyMatch> FuzzyMatcher::findMatch(const std::size_t textEnd) const
{
    for (std::size_t textSize = textEnd; textSize < m_numTextPrefixes; ++textSize) {
        const PrefixMatch match = m_prefixTable[index(m_numPatternPrefixes - 1, textSize)];
        if (match.distance <= m_maxDistance) {
            return FuzzyMatch{ match.beginPos, textSize, match.distance };
        }
    }

    return std::nullopt;
}

bool FuzzyMatcher::empty() const
{
    return !findMatch(0).has_value();
}

void FuzzyMatcher::reset(const std::size_t maxDistance)
{
    clear();
    m_maxDistance = maxDistance;
}

void FuzzyMatcher::clear()
{
    m_numPatternPrefixes = 0;
    m_numTextPrefixes = 0;
    m_prefixTable.clear();
}

FuzzyMatcher::const_iterator FuzzyMatcher::begin() const
{
    return Iterator(*this);
}

FuzzyMatcher::const_iterator FuzzyMatcher::end() const
{
    return Iterator();
}

std::size_t FuzzyMatcher::maxDistance() const
{
    return m_maxDistance;
}

std::size_t FuzzyMatcher::index(const std::size_t patternSize, const std::size_t textSize) const
{
    return m_numTextPrefixes * patternSize + textSize;
}

FuzzyMatcher::Iterator::Iterator(const FuzzyMatcher& matcher)
    : m_matcher{&matcher}
{
    next(0);
}

FuzzyMatcher::Iterator::reference FuzzyMatcher::Iterator::operator*() const
{
    return *m_match;
}

FuzzyMatcher::Iterator::pointer FuzzyMatcher::Iterator::operator->() const
{
    return m_match.operator->();
}

FuzzyMatcher::Iterator& FuzzyMatcher::Iterator::operator++()
{
    next(m_match->endPos + 1);
    return *this;
}

FuzzyMatcher::Iterator FuzzyMatcher::Iterator::operator++(int)
{
    Iterator prev = *this;
    operator++();

    return prev;
}

bool FuzzyMatcher::Iterator::operator==(const Iterator& right) const
{
    return m_matcher == right.m_matcher && m_match == right.m_match;
}

bool FuzzyMatcher::Iterator::operator!=(const Iterator& right) const
{
    return !(*this == right);
}

void FuzzyMatcher::Iterator::next(const std::size_t textEnd)
{
    m_match = m_matcher->findMatch(textEnd);
    if (!m_match) {
        m_matcher = nullptr;
    }
}
}
