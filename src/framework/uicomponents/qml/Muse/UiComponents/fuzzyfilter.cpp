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

#include "fuzzyfilter.h"

#include "global/stringutils.h"

#include "sortfilterproxymodel.h"

namespace muse::uicomponents {
FuzzyFilter::FuzzyFilter(QObject* parent)
    : Filter(parent)
{
}

bool FuzzyFilter::acceptsRow(int sourceRow, const QModelIndex& sourceParent, const SortFilterProxyModel& proxyModel)
{
    const QModelIndex sourceIndex = proxyModel.sourceModel()->index(sourceRow, 0, sourceParent);
    const std::optional<double> score = getScore(sourceIndex, proxyModel);

    return score.has_value();
}

void FuzzyFilter::invalidate()
{
    m_scoreCache.clear();
}

QString FuzzyFilter::fuzzyPattern() const
{
    return m_fuzzyPattern;
}

void FuzzyFilter::setFuzzyPattern(const QString& fuzzyPattern)
{
    const QString simplifiedPattern = fuzzyPattern.simplified();
    if (m_fuzzyPattern == simplifiedPattern) {
        return;
    }

    m_fuzzyPattern = simplifiedPattern;
    compilePattern();

    emit dataChanged();
}

QString FuzzyFilter::roleName() const
{
    return m_roleName;
}

void FuzzyFilter::setRoleName(const QString& roleName)
{
    if (m_roleName == roleName) {
        return;
    }

    invalidate();

    m_roleName = roleName;
    emit dataChanged();
}

Qt::CaseSensitivity FuzzyFilter::caseSensitivity() const
{
    return m_caseSensitivity;
}

void FuzzyFilter::setCaseSensitivity(const Qt::CaseSensitivity caseSensitivity)
{
    if (m_caseSensitivity == caseSensitivity) {
        return;
    }

    compilePattern();

    m_caseSensitivity = caseSensitivity;
    emit dataChanged();
}

std::optional<double> FuzzyFilter::getScore(const QPersistentModelIndex& sourceIndex, const SortFilterProxyModel& proxyModel)
{
    auto scoreIt = m_scoreCache.find(sourceIndex);
    if (scoreIt != m_scoreCache.end()) {
        return scoreIt.value();
    }

    std::optional<double> score = calcScore(sourceIndex, proxyModel);
    // don't cache score of filtered out items because the cache
    // is always reset before filtering and therefore only used for sorting
    // already filtered items
    if (!score) {
        return score;
    }

    m_scoreCache.try_emplace(sourceIndex, *score);

    return score;
}

void FuzzyFilter::compilePattern()
{
    invalidate();

    m_patternTokens.clear();
    const std::u32string pattern = caseSensitivity() == Qt::CaseInsensitive
                                   ? m_fuzzyPattern.toLower().toStdU32String()
                                   : m_fuzzyPattern.toStdU32String();
    strings::split(pattern, std::back_inserter(m_patternTokens), U" ");
}

std::optional<double> FuzzyFilter::calcScore(const QModelIndex& sourceIndex, const SortFilterProxyModel& proxyModel)
{
    const int role = proxyModel.roleFromRoleName(m_roleName);
    if (role == -1) {
        return std::nullopt;
    }

    const QString rawText = proxyModel.sourceModel()->data(sourceIndex, role)
                            .toString();
    const std::u32string text = caseSensitivity() == Qt::CaseInsensitive
                                ? rawText.toLower().toStdU32String()
                                : rawText.toStdU32String();

    double score = 0.0;
    for (const auto& patternToken : m_patternTokens) {
        const size_t tokenSize = patternToken.size();
        if (tokenSize == 0) {
            continue;
        }

        constexpr size_t MIN_TOKEN_SIZE_FOR_FUZZY_MATCH = 4;
        constexpr size_t CHARS_PER_ERROR = 8;
        const size_t maxDistance = tokenSize >= MIN_TOKEN_SIZE_FOR_FUZZY_MATCH
                                   ? 1 + tokenSize / CHARS_PER_ERROR
                                   : 0;
        m_matcher.reset(maxDistance);

        const double inverseTokenSize = 1.0 / tokenSize;
        std::optional<double> bestTokenScore;

        for (const auto& match : m_matcher(text, patternToken)) {
            const double matchSimilarity = 1.0 - (match.editDistance * inverseTokenSize);
            double matchScore = 5.0 * matchSimilarity;

            const bool isMatchStartAtStartOfWord = match.beginPos == 0
                                                   || text[match.beginPos - 1] == U' ';
            if (isMatchStartAtStartOfWord) {
                const bool isMatchEndAtEndOfWord = match.endPos == text.size()
                                                   || text[match.endPos] == U' ';
                if (isMatchEndAtEndOfWord) {
                    matchScore += 2.0 * inverseTokenSize;
                } else {
                    matchScore += inverseTokenSize;
                }
            }

            if (bestTokenScore < matchScore) {
                bestTokenScore = matchScore;
            }
        }

        // no match for token found -> no score for entire pattern
        if (!bestTokenScore) {
            return bestTokenScore;
        }

        score += *bestTokenScore;
    }

    return score;
}
}
