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
#include "webworkerapi.h"

#include "global/globalmodule.h"

#include "log.h"

using namespace muse;
using namespace web::worker;

WebWorkerApi* WebWorkerApi::instance()
{
    static WebWorkerApi w;
    return &w;
}

void WebWorkerApi::init()
{
    static GlobalModule globalModule;

    globalModule.registerExports();
    globalModule.registerApi();
    globalModule.onPreInit(IApplication::RunMode::ConsoleApp);
    globalModule.onInit(IApplication::RunMode::ConsoleApp);

    LOGI() << "Inited";
}
