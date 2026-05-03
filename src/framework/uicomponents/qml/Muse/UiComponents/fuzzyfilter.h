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

#include <optional>
#include <string>
#include <vector>

#include <QObject>
#include <QPersistentModelIndex>
#include <QString>

#include <QtQmlIntegration/qqmlintegration.h>

#include "global/stringsearch.h"

#include "filter.h"

namespace muse::uicomponents {
// TODO: needs notifications from the model
class FuzzyFilter : public Filter
{
    Q_OBJECT
    Q_PROPERTY(QString fuzzyPattern READ fuzzyPattern WRITE setFuzzyPattern NOTIFY dataChanged)
    Q_PROPERTY(QString roleName READ roleName WRITE setRoleName NOTIFY dataChanged)
    Q_PROPERTY(Qt::CaseSensitivity caseSensitivity READ caseSensitivity WRITE setCaseSensitivity NOTIFY dataChanged)

    QML_ELEMENT

public:
    explicit FuzzyFilter(QObject* parent = nullptr);

    bool acceptsRow(int sourceRow, const QModelIndex& sourceParent, const SortFilterProxyModel&) override;
    void invalidate() override;

    QString fuzzyPattern() const;
    void setFuzzyPattern(const QString&);

    QString roleName() const;
    void setRoleName(const QString&);

    Qt::CaseSensitivity caseSensitivity() const;
    void setCaseSensitivity(Qt::CaseSensitivity);

    std::optional<double> getScore(const QPersistentModelIndex& sourceIndex, const SortFilterProxyModel&);

private:
    void compilePattern();

    std::optional<double> calcScore(const QModelIndex& sourceIndex, const SortFilterProxyModel&);

    QString m_fuzzyPattern;
    QString m_roleName;
    Qt::CaseSensitivity m_caseSensitivity = Qt::CaseSensitive;

    std::vector<std::u32string> m_patternTokens;
    FuzzyMatcher m_matcher;
    QHash<QPersistentModelIndex, double> m_scoreCache;
};
}
