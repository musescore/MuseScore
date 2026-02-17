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
#include "muse_framework_config.h"
#include "global/api/iapiregister.h"

#include "internal/interactive.h"
#include "internal/interactiveuriregister.h"
#include "types/uri.h"

#ifdef Q_OS_WASM
#include "internal/platform/web/webinteractive.h"
#endif

#include "api/interactiveapi.h"

#include "dev/testdialog.h"

using namespace muse::interactive;
using namespace muse::modularity;

static const std::string mname("interactive");

std::string InteractiveModule::moduleName() const
{
    return mname;
}

void InteractiveModule::registerExports()
{
    auto interactive = std::make_shared<Interactive>(globalCtx());
#ifdef Q_OS_WASM
    globalIoc()->registerExport<IInteractive>(mname, new WebInteractive(interactive));
#else
    globalIoc()->registerExport<IInteractive>(mname, interactive);
#endif

    globalIoc()->registerExport<IInteractiveProvider>(mname, interactive);
    globalIoc()->registerExport<IInteractiveUriRegister>(mname, new InteractiveUriRegister());
}

void InteractiveModule::registerApi()
{
    using namespace muse::api;

    auto api = globalIoc()->resolve<IApiRegister>(mname);
    if (api) {
        api->regApiCreator(mname, "MuseApi.Interactive", new ApiCreator<InteractiveApi>());

        api->regGlobalEnum(mname, QMetaEnum::fromType<InteractiveApi::ButtonCode>());
    }
}

void InteractiveModule::resolveImports()
{
    auto ir = globalIoc()->resolve<IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("muse://interactive/standard"), "Muse.Interactive", "StandardDialog");
        ir->registerQmlUri(Uri("muse://interactive/error"), "Muse.Interactive", "ErrorDetailsView");
        ir->registerQmlUri(Uri("muse://interactive/progress"), "Muse.Interactive", "ProgressDialog");
        ir->registerQmlUri(Uri("muse://interactive/selectfile"), "Muse.Interactive", "FileDialog");
        ir->registerQmlUri(Uri("muse://interactive/selectdir"), "Muse.Interactive", "FolderDialog");

        ir->registerQmlUri(Uri("muse://devtools/interactive/sample"), "Muse.Interactive", "SampleDialog");
        ir->registerWidgetUri<TestDialog>(Uri("muse://devtools/interactive/testdialog"));
    }
}

IContextSetup* InteractiveModule::newContext(const ContextPtr& ctx) const
{
    return new InteractiveContext(ctx);
}

void InteractiveContext::registerExports()
{
#ifdef MUSE_MULTICONTEXT_WIP
    // forward to context
    auto globalUriRegister = globalIoc()->resolve<IInteractiveUriRegister>(mname);
    ioc()->registerExport<IInteractiveUriRegister>(mname, globalUriRegister);

    auto interactive = std::make_shared<Interactive>(iocContext());
    ioc()->registerExport<IInteractive>(mname, interactive);
    ioc()->registerExport<IInteractiveProvider>(mname, interactive);
#endif
}
