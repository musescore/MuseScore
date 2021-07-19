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
#include "autobotmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"

#include "internal/autobot.h"
#include "internal/autobotconfiguration.h"
#include "view/autobotmodel.h"

#include "engraving/draw/painter.h"
#include "internal/draw/abpaintprovider.h"

using namespace mu::autobot;

static const std::shared_ptr<Autobot> s_autobot = std::make_shared<Autobot>();

std::string AutobotModule::moduleName() const
{
    return "autobot";
}

void AutobotModule::registerExports()
{
    modularity::ioc()->registerExport<IAutobot>(moduleName(), s_autobot);
    modularity::ioc()->registerExport<IAutobotConfiguration>(moduleName(), new AutobotConfiguration());

    draw::Painter::extended = AbPaintProvider::instance();
}

void AutobotModule::resolveImports()
{
    auto ir = modularity::ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://autobot/main"), "MuseScore/Autobot/AutobotDialog.qml");
    }
}

void AutobotModule::registerUiTypes()
{
    qmlRegisterType<AutobotModel>("MuseScore.Autobot", 1, 0, "AutobotModel");
}

void AutobotModule::onInit(const framework::IApplication::RunMode&)
{
    s_autobot->init();
}
