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

class Filter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY dataChanged)
    /// Determines whether the SortFilterProxyModel should react asynchronously to the `dataChanged` signal.
    Q_PROPERTY(bool async READ async WRITE setAsync NOTIFY dataChanged)

    QML_ELEMENT;
    QML_UNCREATABLE("")

public:
    explicit Filter(QObject* parent = nullptr);

    virtual bool acceptsRow(int sourceRow, const QModelIndex& sourceParent, const SortFilterProxyModel&) = 0;

    bool enabled() const;
    void setEnabled(bool enabled);

    bool async() const;
    void setAsync(bool async);

signals:
    void dataChanged();

private:
    bool m_enabled = true;
    bool m_async = false;
};
}
