/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "appjsmodule.h"

#include "webapi.h"

#ifdef Q_OS_WASM
#include <emscripten/val.h>
#endif

using namespace muse;
using namespace mu::appjs;

std::string AppJsModule::moduleName() const
{
    return "appjs";
}

void AppJsModule::onStartApp()
{
#ifdef Q_OS_WASM
    emscripten::val::module_property("onStartApp")();
#endif
}

muse::modularity::IContextSetup* AppJsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new AppJsContext(ctx);
}

// Context

void AppJsContext::onInit(const muse::IApplication::RunMode&)
{
    WebApi::instance()->init(iocContext());
}

void AppJsContext::onDeinit()
{
    WebApi::instance()->deinit();
}
