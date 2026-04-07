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

#include "docktitlebar.h"

using namespace muse::dock;

DockTitleBar::DockTitleBar(KDDockWidgets::Core::TitleBar* controller, QQuickItem* parent)
    : KDDockWidgets::QtQuick::TitleBar(controller, parent)
{
}

void DockTitleBar::init()
{
    KDDockWidgets::QtQuick::TitleBar::init();

    // Suppress the default title bar because we add our own. Otherwise
    // the default title bar steals the mouse events from our title bar.
    setHeight(0);
}
