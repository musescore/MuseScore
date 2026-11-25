/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

#include <QString>

#include "global/types/number.h"
#include "engraving/automation/automationtypes.h"

namespace mu::notation {
class NotationAutomationDisplayMapping
{
public:
    static qreal defaultDisplayValue(mu::engraving::AutomationType type);
    static double toDisplay(mu::engraving::AutomationType type, bool isMuseSampler, muse::real_t value);
    static muse::real_t fromDisplay(mu::engraving::AutomationType type, bool isMuseSampler, double displayValue,
                                    std::optional<muse::real_t> existingValue = std::nullopt);

    static QString valueText(mu::engraving::AutomationType type, muse::real_t value);
};
}
