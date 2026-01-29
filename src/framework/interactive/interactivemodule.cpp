/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include "interactivemodule.h"

#include "global/modularity/ioc.h"
#include "global/api/iapiregister.h"

#include "internal/interactive.h"
#include "internal/interactiveprovider.h"
#include "internal/interactiveuriregister.h"

#ifdef Q_OS_WASM
#include "internal/platform/web/webinteractive.h"
#endif

#include "api/interactiveapi.h"

using namespace muse::interactive;
using namespace muse::modularity;

std::string InteractiveModule::moduleName() const
{
    return "interactive";
}

void InteractiveModule::registerExports()
{
#ifdef Q_OS_WASM
    std::shared_ptr<IInteractive> originInteractive = std::make_shared<Interactive>(iocContext());
    ioc()->registerExport<IInteractive>(moduleName(), new WebInteractive(originInteractive));
#else
    ioc()->registerExport<IInteractive>(moduleName(), new Interactive(iocContext()));
#endif
    ioc()->registerExport<IInteractiveProvider>(moduleName(), new InteractiveProvider(iocContext()));
    ioc()->registerExport<IInteractiveUriRegister>(moduleName(), new InteractiveUriRegister());
}

void InteractiveModule::registerApi()
{
    using namespace muse::api;

    auto api = ioc()->resolve<IApiRegister>(moduleName());
    if (api) {
        api->regApiCreator(moduleName(), "MuseApi.Interactive", new ApiCreator<InteractiveApi>());

        api->regGlobalEnum(moduleName(), QMetaEnum::fromType<InteractiveApi::ButtonCode>());
    }
}

void InteractiveModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://interactive/standard"), "Muse.Interactive", "StandardDialog");
        ir->registerQmlUri(Uri("muse://interactive/error"), "Muse.Interactive", "ErrorDetailsView");
        ir->registerQmlUri(Uri("muse://interactive/progress"), "Muse.Interactive", "ProgressDialog");
        ir->registerQmlUri(Uri("muse://interactive/selectfile"), "Muse.Interactive", "FileDialog");
        ir->registerQmlUri(Uri("muse://interactive/selectdir"), "Muse.Interactive", "FolderDialog");
    }
}
