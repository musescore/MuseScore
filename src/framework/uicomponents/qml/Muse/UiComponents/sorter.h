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

#include <QObject>

#include <QtQmlIntegration/qqmlintegration.h>

class QModelIndex;

namespace muse::uicomponents {
class SortFilterProxyModel;

class Sorter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY dataChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY dataChanged)

    QML_ELEMENT;
    QML_UNCREATABLE("")

public:
    explicit Sorter(QObject* parent = nullptr);

    virtual bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight, const SortFilterProxyModel&) = 0;

    Qt::SortOrder sortOrder() const;
    void setSortOrder(Qt::SortOrder sortOrder);

    bool enabled() const;
    void setEnabled(bool enabled);

signals:
    void dataChanged();

private:
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
    bool m_enabled = false;
};
}
