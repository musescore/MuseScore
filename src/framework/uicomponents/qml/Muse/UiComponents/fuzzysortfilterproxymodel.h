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

#include <string>
#include <vector>

#include <QSortFilterProxyModel>
#include <QString>

#include <qqmlintegration.h>

namespace muse::uicomponents {
class FuzzySortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterFuzzyPattern READ filterFuzzyPattern WRITE setFilterFuzzyPattern NOTIFY filterFuzzyPatternChanged REQUIRED)
    Q_PROPERTY(QString filterRoleName READ filterRoleName WRITE setFilterRoleName NOTIFY filterRoleNameChanged)

    QML_ELEMENT

public:
    explicit FuzzySortFilterProxyModel(QObject* parent = nullptr);

    QString filterFuzzyPattern() const;
    void setFilterFuzzyPattern(QString);

    QString filterRoleName() const;
    void setFilterRoleName(QString);

signals:
    void filterFuzzyPatternChanged(QString);
    void filterRoleNameChanged(QString);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const override;

private:
    QString m_pattern;
    std::vector<std::string> m_compiledPattern;
    QString m_filterRoleName;
};
}
