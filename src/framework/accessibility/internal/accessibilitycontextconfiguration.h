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

#include "../iaccessibilitycontextconfiguration.h"

#include "modularity/ioc.h"
#include "ui/inavigationcontroller.h"
#include "../iaccessibilityconfiguration.h"

namespace muse::accessibility {
class AccessibilityContextConfiguration : public IAccessibilityContextConfiguration, public muse::Contextable
{
    GlobalInject<IAccessibilityConfiguration> configuration;
    ContextInject<ui::INavigationController> navigationController = { this };

public:
    AccessibilityContextConfiguration(const muse::modularity::ContextPtr& iocCtx)
        : Contextable(iocCtx) {}

    bool isAccessibleActive() const override;
    bool isAccessibleEnabled() const override;
};
}
