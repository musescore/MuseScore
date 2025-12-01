/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <qqmlintegration.h>

#include <QObject>

#include "tableviewcell.h"

namespace muse::uicomponents {
class TableViewListCell : public TableViewCell
{
    Q_OBJECT
    QML_ELEMENT;

    Q_PROPERTY(QString current READ current WRITE setCurrent NOTIFY currentChanged FINAL)

public:
    explicit TableViewListCell(QObject* parent = nullptr);

    QString current() const;
    void setCurrent(const QString& current);

signals:
    void currentChanged();

private:
    QString m_current;
};
}
