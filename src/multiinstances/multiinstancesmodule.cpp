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
#include <QTimer>
#include "internal/ipcchannel.h"
#include "internal/multiinstancesuiactions.h"
#include "internal/multiinstancesprovider.h"

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "dev/multiinstancesdevmodel.h"

using namespace mu::mi;
using namespace mu::framework;

//static QTimer s_testTimer;
//static IpcChannel s_testChannel;
//static int s_testCounter = 0;

static std::shared_ptr<MultiInstancesProvider> s_multiInstancesProvider = std::make_shared<MultiInstancesProvider>();

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
    ioc()->registerExport<IMultiInstancesProvider>(moduleName(), s_multiInstancesProvider);
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

void MultiInstancesModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::Editor) {
        return;
    }

    s_multiInstancesProvider->init();

//    s_testTimer.setInterval(2000);
//    s_testTimer.setSingleShot(true);
//    QObject::connect(&s_testTimer, &QTimer::timeout, []() {
//        ++s_testCounter;
//        IpcChannel::Msg msg;
//        msg.method = "test_ping";
//        msg.args << QString::number(s_testCounter);

//        s_testChannel.send(msg);

//        s_testTimer.start();
//    });

//    s_testTimer.start();
}
