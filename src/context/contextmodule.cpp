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
#include "contextmodule.h"

#include "modularity/ioc.h"
#include "internal/globalcontext.h"
#include "internal/uicontextresolver.h"

using namespace mu::context;

static std::shared_ptr<GlobalContext> s_globalContext = std::make_shared<GlobalContext>();
static std::shared_ptr<UiContextResolver> s_uicontextResolver = std::make_shared<UiContextResolver>();

std::string ContextModule::moduleName() const
{
    return "context";
}

void ContextModule::registerExports()
{
    modularity::ioc()->registerExport<IGlobalContext>(moduleName(), s_globalContext);
    modularity::ioc()->registerExport<IUiContextResolver>(moduleName(), s_uicontextResolver);
}

void ContextModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (mode != framework::IApplication::RunMode::Editor) {
        return;
    }

    s_uicontextResolver->init();
}

void ContextModule::onDeinit()
{
    s_globalContext->setCurrentNotationProject(nullptr);
}
