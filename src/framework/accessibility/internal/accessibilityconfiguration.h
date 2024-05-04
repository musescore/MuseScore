/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MUSE_ACCESSIBILITY_ACCESSIBILITYCONFIGURATION_H
#define MUSE_ACCESSIBILITY_ACCESSIBILITYCONFIGURATION_H

#include "../iaccessibilityconfiguration.h"

#include "modularity/ioc.h"
#include "ui/inavigationcontroller.h"

namespace muse::accessibility {
class AccessibilityConfiguration : public IAccessibilityConfiguration, public Injectable
{
    Inject<ui::INavigationController> navigationController { this };

public:
    AccessibilityConfiguration(const modularity::ContextPtr& ctx)
        : Injectable(ctx) {}

    ~AccessibilityConfiguration() override;

    void init();

    bool enabled() const override;
    bool active() const override;

private:
    bool m_inited = false;
};
}

#endif // MUSE_ACCESSIBILITY_ACCESSIBILITYCONFIGURATION_H
