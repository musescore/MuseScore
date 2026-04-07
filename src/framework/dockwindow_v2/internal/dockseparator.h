/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#ifndef MUSE_DOCK_DOCKSEPARATOR_H
#define MUSE_DOCK_DOCKSEPARATOR_H

#include "kddockwidgets/src/qtquick/views/Separator.h"

namespace KDDockWidgets::Core {
class Separator;
}

namespace muse::dock {
class DockSeparator : public KDDockWidgets::QtQuick::Separator
{
    Q_OBJECT

    Q_PROPERTY(bool isSeparatorVisible READ isSeparatorVisible NOTIFY isSeparatorVisibleChanged)
    Q_PROPERTY(bool showResizeCursor READ showResizeCursor NOTIFY showResizeCursorChanged)

public:
    explicit DockSeparator(KDDockWidgets::Core::Separator* controller, QQuickItem* parent = nullptr);

    bool isSeparatorVisible() const;
    bool showResizeCursor() const;

signals:
    void isSeparatorVisibleChanged();
    void showResizeCursorChanged();

private:
    void initAvailability();

    bool m_isSeparatorVisible = true;
    bool m_inited = false;
};
}

#endif // MUSE_DOCK_DOCKSEPARATOR_H
