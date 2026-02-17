/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "instrumentsscenemodule.h"

#include "modularity/ioc.h"

#include "internal/selectinstrumentscenario.h"
#include "internal/instrumentsuiactions.h"
#include "internal/instrumentsactionscontroller.h"

#include "interactive/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

using namespace mu::instrumentsscene;
using namespace muse;
using namespace muse::modularity;

static const std::string mname("instrumentsscene");

std::string InstrumentsSceneModule::moduleName() const
{
    return mname;
}

void InstrumentsSceneModule::resolveImports()
{
    auto ir = globalIoc()->resolve<interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("musescore://instruments/select"), "MuseScore.InstrumentsScene", "InstrumentsDialog");
    }
}

IContextSetup* InstrumentsSceneModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new InstrumentsSceneContext(ctx);
}

// Context

void InstrumentsSceneContext::registerExports()
{
    m_actionsController = std::make_shared<InstrumentsActionsController>(iocContext());

    ioc()->registerExport<notation::ISelectInstrumentsScenario>(mname, new SelectInstrumentsScenario(iocContext()));
}

void InstrumentsSceneContext::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(std::make_shared<InstrumentsUiActions>(iocContext()));
    }
}

void InstrumentsSceneContext::onInit(const IApplication::RunMode&)
{
    m_actionsController->init();
}
