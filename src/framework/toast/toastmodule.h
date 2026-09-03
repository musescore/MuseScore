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
#pragma once

#include <memory>
#include <string>

#include "modularity/imodulesetup.h"

namespace muse::toast {
class ToastService;
class ToastProvider;

class ToastModule : public modularity::IModuleSetup
{
public:
    std::string moduleName() const override;
    void registerExports() override;

    modularity::IContextSetup* newContext(const modularity::ContextPtr& ctx) const override;

private:
    std::shared_ptr<ToastProvider> m_toastProvider;
    std::shared_ptr<ToastService> m_toastService;
};

class ToastContext : public modularity::IContextSetup
{
public:
    ToastContext(const modularity::ContextPtr& ctx)
        : modularity::IContextSetup(ctx) {}

    void onDeinit() override;
};
}
