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

#pragma once

#include <memory>

#include "modularity/imodulesetup.h"

#include "muse_framework_config.h"

namespace muse::mi {
#ifdef MUSE_MULTICONTEXT_WIP
class SingleProcessProvider;
#else
class MultiProcessProvider;
#endif

class MultiWindowsModule : public modularity::IModuleSetup
{
public:
    std::string moduleName() const override;
    void registerExports() override;
    void resolveImports() override;
    void onPreInit(const IApplication::RunMode& mode) override;

    modularity::IContextSetup* newContext(const muse::modularity::ContextPtr& ctx) const override;

private:
#ifdef MUSE_MULTICONTEXT_WIP
    std::shared_ptr<SingleProcessProvider> m_windowsProvider;
#else
    std::shared_ptr<MultiProcessProvider> m_windowsProvider;
#endif
};

class MultiWindowsContext : public modularity::IContextSetup
{
public:
    MultiWindowsContext(const modularity::ContextPtr& ctx)
        : modularity::IContextSetup(ctx)
    {
    }

    void resolveImports() override;
};
}
