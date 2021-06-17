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
#include "diagnosticsmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"

using namespace mu::diagnostics;
using namespace mu::framework;

std::string DiagnosticsModule::moduleName() const
{
    return "diagnostics";
}

void DiagnosticsModule::registerExports()
{
}

void DiagnosticsModule::resolveImports()
{
    auto ir = framework::ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://autobot/main"), "MuseScore/Diagnostics/DiagnosticsDialog.qml");
    }
}

void DiagnosticsModule::registerUiTypes()
{
    // qmlRegisterType<DiagnosticsModel>("MuseScore.Diagnostics", 1, 0, "DiagnosticsModel");
}

void DiagnosticsModule::onInit(const framework::IApplication::RunMode&)
{
}
