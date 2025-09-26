/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include <emscripten/val.h>

#include "webapi.h"

using namespace muse;
using namespace mu::appjs;

std::string AppJsModule::moduleName() const
{
    return "appjs";
}

void AppJsModule::onInit(const muse::IApplication::RunMode&)
{
    WebApi::instance()->init();
}

void AppJsModule::onDeinit()
{
    WebApi::instance()->deinit();
}

void AppJsModule::onStartApp()
{
    emscripten::val::module_property("onStartApp")();
}
