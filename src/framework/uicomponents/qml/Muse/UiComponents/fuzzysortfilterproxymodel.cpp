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

#include "fuzzysortfilterproxymodel.h"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include <QtVersionChecks>

#include "global/stringutils.h"

namespace muse::uicomponents {
namespace {
constexpr double MAX_ERROR_RATE = 0.5;

struct FuzzyMatch {
    size_t startPos = 0;
    size_t endPos = 0;
    size_t editDistance = 0;
};

FuzzyMatch findBestMatch(const std::string_view text, const std::string_view pattern)
{
    size_t bestEndColumn = 0;
    size_t bestDistance = SIZE_MAX;

    const size_t numRows = pattern.size() + 1;
    const size_t numColumns = text.size() + 1;
    std::vector<size_t> distances(numRows * numColumns);

    for (size_t rowIdx = 0; rowIdx < numRows; ++rowIdx) {
        distances[numColumns * rowIdx] = rowIdx;
    }

    for (size_t i = 0; i < pattern.size(); ++i) {
        const size_t rowIdx = i + 1;
        for (size_t j = 0; j < text.size(); ++j) {
            const size_t columnIdx = j + 1;
            const size_t replaceDistance = distances[numColumns * (rowIdx - 1) + columnIdx - 1];
            const size_t cellIdx = numColumns * rowIdx + columnIdx;
            if (pattern[i] == text[j]) {
                distances[cellIdx] = replaceDistance;
            } else {
                const size_t insertDistance = distances[numColumns * rowIdx + columnIdx - 1];
                const size_t deleteDistance = distances[numColumns * (rowIdx - 1) + columnIdx];
                distances[cellIdx] = 1 + std::min(replaceDistance, std::min(insertDistance, deleteDistance));
            }
        }
    }

    const size_t lastRowOffset = numColumns * (numRows - 1);
    for (size_t columnIdx = 0; columnIdx < numColumns; ++columnIdx) {
        const size_t distance = distances[lastRowOffset + columnIdx];
        if (distance < bestDistance) {
            bestDistance = distance;
            bestEndColumn = columnIdx;
        }
    }

    size_t bestStartColumn = bestEndColumn;
    size_t rowIdx = numRows - 1;
    while (rowIdx > 0 && bestStartColumn > 0) {
        const size_t replaceDistance = distances[numColumns * (rowIdx - 1) + bestStartColumn - 1];
        const size_t insertDistance = distances[numColumns * rowIdx + bestStartColumn - 1];
        const size_t deleteDistance = distances[numColumns * (rowIdx - 1) + bestStartColumn];
        const size_t minDistance = std::min(replaceDistance, std::min(insertDistance, deleteDistance));
        if (minDistance == deleteDistance) {
            --rowIdx;
        } else if (minDistance == replaceDistance) {
            --rowIdx;
            --bestStartColumn;
        } else {
            --bestStartColumn;
        }
    }

    return FuzzyMatch{ bestStartColumn - 1, bestEndColumn, bestDistance };
}
}

FuzzySortFilterProxyModel::FuzzySortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &QAbstractProxyModel::sourceModelChanged, this, [this] {
        const int role = roleNames().key(m_filterRoleName.toUtf8(), -1);
        setFilterRole(role);
    });
}

QString FuzzySortFilterProxyModel::filterFuzzyPattern() const
{
    return m_pattern;
}

void FuzzySortFilterProxyModel::setFilterFuzzyPattern(const QString pattern)
{
    m_pattern = pattern.simplified();
    const QString caseAdjustedPattern = filterCaseSensitivity() == Qt::CaseInsensitive
                                        ? m_pattern.toLower()
                                        : m_pattern;
    m_compiledPattern.clear();
    strings::split(caseAdjustedPattern.toStdString(), m_compiledPattern, " ");

    if (m_pattern.isEmpty()) {
        sort(-1, Qt::AscendingOrder);
    } else {
        sort(filterKeyColumn(), Qt::DescendingOrder);
    }

    invalidate();

    emit filterFuzzyPatternChanged(m_pattern);
}

QString FuzzySortFilterProxyModel::filterRoleName() const
{
    return QString::fromUtf8(roleNames().value(filterRole()));
}

void FuzzySortFilterProxyModel::setFilterRoleName(const QString roleName)
{
    m_filterRoleName = roleName;

    const int role = roleNames().key(roleName.toUtf8(), -1);
    setFilterRole(role);

    emit filterRoleNameChanged(roleName);
}

bool FuzzySortFilterProxyModel::filterAcceptsRow(const int sourceRow, const QModelIndex& sourceParent) const
{
    if (m_pattern.isEmpty() || m_compiledPattern.empty()) {
        return true;
    }

    const QAbstractItemModel* sourceModel = this->sourceModel();
    const QModelIndex sourceIndex = sourceModel->index(sourceRow, filterKeyColumn(), sourceParent);
    const QString filterKey = sourceModel->data(sourceIndex, filterRole())
                              .toString();

    const std::string caseAdjustedFilterKey = filterCaseSensitivity() == Qt::CaseInsensitive
                                              ? filterKey.toLower().toStdString()
                                              : filterKey.toStdString();

    // TODO: deal with overlap?
    // TODO: prioritize full word matches
    // TODO: prioritize start of word
    double score = 0.0;
    for (const auto& patternToken : m_compiledPattern) {
        const FuzzyMatch bestMatch = findBestMatch(caseAdjustedFilterKey, patternToken);
        const double errorRate = static_cast<double>(bestMatch.editDistance) / patternToken.size();

        if (errorRate < MAX_ERROR_RATE) {
            score += 1.0 - errorRate;
        }
    }

    return score > 0.5;
}

bool FuzzySortFilterProxyModel::lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const
{
    if (m_pattern.isEmpty() || m_compiledPattern.empty()) {
        return sourceLeft < sourceRight;
    }

    const QAbstractItemModel* sourceModel = this->sourceModel();
    const QString leftKey = sourceModel->data(sourceLeft, filterRole())
                            .toString();
    const std::string caseAdjustedLeftKey = filterCaseSensitivity() == Qt::CaseInsensitive
                                            ? leftKey.toLower().toStdString()
                                            : leftKey.toStdString();

    double leftScore = 0.0;
    for (const auto& patternToken : m_compiledPattern) {
        const FuzzyMatch bestMatch = findBestMatch(caseAdjustedLeftKey, patternToken);
        const double errorRate = static_cast<double>(bestMatch.editDistance) / patternToken.size();

        if (errorRate < MAX_ERROR_RATE) {
            leftScore += 1.0 - errorRate;
        }
    }

    const QString rightKey = sourceModel->data(sourceRight, filterRole())
                             .toString();
    const std::string caseAdjustedRightKey = filterCaseSensitivity() == Qt::CaseInsensitive
                                             ? rightKey.toLower().toStdString()
                                             : rightKey.toStdString();

    double rightScore = 0.0;
    for (const auto& patternToken : m_compiledPattern) {
        const FuzzyMatch bestMatch = findBestMatch(caseAdjustedRightKey, patternToken);
        const double errorRate = static_cast<double>(bestMatch.editDistance) / patternToken.size();

        if (errorRate < MAX_ERROR_RATE) {
            rightScore += 1.0 - errorRate;
        }
    }

    return leftScore < rightScore;
}
}
