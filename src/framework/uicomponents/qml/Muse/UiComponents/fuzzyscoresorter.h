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

#include <QMetaObject>
#include <QObject>
#include <QPointer>

#include <QtQmlIntegration/qqmlintegration.h>

#include "fuzzyfilter.h"
#include "sorter.h"

namespace muse::uicomponents {
class FuzzyScoreSorter : public Sorter
{
    Q_OBJECT
    Q_PROPERTY(muse::uicomponents::FuzzyFilter * fuzzyFilter READ fuzzyFilter WRITE setFuzzyFilter NOTIFY dataChanged REQUIRED)

    QML_ELEMENT

public:
    explicit FuzzyScoreSorter(QObject* parent = nullptr);

    bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight, const SortFilterProxyModel&) override;

    FuzzyFilter* fuzzyFilter() const;
    void setFuzzyFilter(FuzzyFilter*);

private:
    QPointer<FuzzyFilter> m_fuzzyFilter;
    QMetaObject::Connection m_filterChangedConnection;
};
}
