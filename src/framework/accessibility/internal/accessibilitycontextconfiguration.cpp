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

#include "accessibilitycontextconfiguration.h"

using namespace muse::accessibility;

bool AccessibilityContextConfiguration::isAccessibleActive() const
{
    return configuration()->isAccessibleActive();
}

bool AccessibilityContextConfiguration::isAccessibleEnabled() const
{
    if (!configuration()->isAccessibleActive()) {
        return false;
    }

    if (!navigationController()) {
        return false;
    }

    //! NOTE Accessibility available if navigation is used
    return navigationController()->activeSection() != nullptr;
}
