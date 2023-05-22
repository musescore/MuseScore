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
#include "multiinstancesmodule.h"

#include <QQmlEngine>

#include "internal/multiinstancesuiactions.h"
#include "internal/multiinstancesprovider.h"

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "dev/multiinstancesdevmodel.h"

using namespace mu::mi;
using namespace mu::modularity;

static void multiinstances_init_qrc()
{
    Q_INIT_RESOURCE(multiinstances);
}

std::string MultiInstancesModule::moduleName() const
{
    return "multiinstances";
}

void MultiInstancesModule::registerExports()
{
    m_multiInstancesProvider = std::make_shared<MultiInstancesProvider>();

    ioc()->registerExport<IMultiInstancesProvider>(moduleName(), m_multiInstancesProvider);
}

void MultiInstancesModule::resolveImports()
{
    auto ir = ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://devtools/multiinstances/info"), "MuseScore/MultiInstances/MultiInstancesDevDialog.qml");
    }

    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<MultiInstancesUiActions>());
    }
}

void MultiInstancesModule::registerUiTypes()
{
    qmlRegisterType<MultiInstancesDevModel>("MuseScore.MultiInstances", 1, 0, "MultiInstancesDevModel");
}

void MultiInstancesModule::registerResources()
{
    multiinstances_init_qrc();
}

void MultiInstancesModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (mode != framework::IApplication::RunMode::GuiApp) {
        return;
    }

    m_multiInstancesProvider->init();
}
