/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "appshellstate.h"

using namespace mu::appshell;

static const QString NOTATION_NAVIGATOR_VISIBLE_KEY("showNavigator");

bool AppShellState::isNotationNavigatorVisible() const
{
    return uistate()->isVisible(NOTATION_NAVIGATOR_VISIBLE_KEY, false);
}

void AppShellState::setIsNotationNavigatorVisible(bool visible)
{
    uistate()->setIsVisible(NOTATION_NAVIGATOR_VISIBLE_KEY, visible);
}

muse::async::Notification AppShellState::isNotationNavigatorVisibleChanged() const
{
    return uistate()->isVisibleChanged(NOTATION_NAVIGATOR_VISIBLE_KEY);
}
