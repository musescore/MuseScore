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
#include "webbridgemodule.h"

#ifdef Q_OS_WASM
#include "internal/memfilesystem.h"
#endif

#include "log.h"

using namespace mu::webbridge;

//! NOTE It can work in two cases:
//! 1. app-web configuration and wasm build for browser (main case)
//! 2. app-web configuration and desktop build (develop case)

std::string WebBridgeModule::moduleName() const
{
    return "webbridge";
}

void WebBridgeModule::registerExports()
{
#ifdef Q_OS_WASM
    ioc()->unregister<muse::io::IFileSystem>(moduleName());
    ioc()->registerExport<muse::io::IFileSystem>(moduleName(), new MemFileSystem());
#endif


}

void WebBridgeModule::onStartApp()
{
    int k = 17;
}
